#include <xc.h>

#pragma config OSC = INTIO67
#pragma config WDT = OFF
#define _XTAL_FREQ 125000
int state = 0;//0->idle 1->warning 2->passing 3->rising
int is_idle = 1;
int duty_now = 45;//0
int dir = -1;  //1=pos, -1 = neg                   


void pwm_set_val(int value) {
    if (value <= 16) value = 16;   // -90
    if (value >= 75) value = 75;   // +90
    CCPR1L = (unsigned char)(value >> 2);
    CCP1CONbits.DC1B = (unsigned char)(value & 0x03);
}

//135??44units?
void move_steps_with_bounce(void) {
    unsigned char steps_left = 29;   
    while (steps_left--) {
        
        if (duty_now >= 45) dir = -1;   
        if (duty_now <= 16) dir = 1;   

        duty_now += (dir > 0 ? 1 : -1);   
        pwm_set_val(duty_now);
        __delay_ms(200);                   
    }
}

const unsigned char SEG_TABLE[11] = {
    // gfedcba
    0b00111111, //0
    0b00000110, //1
    0b01011011, //2
    0b01001111, //3
    0b01100110, //4
    0b01101101, //5
    0b01111101, //6
    0b00000111, //7
    0b01111111, //8
    0b01101111,  //9
    0b00101010  //10(idle)         
};

void displayDigit(unsigned char n){
    LATD = SEG_TABLE[n];
}

void ultrasonic1_trigger(void) {
    LATBbits.LATB0 = 1;
    __delay_us(10);
    LATBbits.LATB0 = 0;
}


unsigned int ultrasonic1_get_us(char *valid) {
    unsigned int t = 0;
    unsigned int timeout = 0;

    *valid = 0;   // ????

    // ? echo HIGH
    while (!PORTBbits.RB1 && timeout < 120000) timeout++;
    if (timeout >= 120000) return 0;

    TMR1H = 0;
    TMR1L = 0;
    T1CONbits.TMR1ON = 1;

    while (PORTBbits.RB1 && t < 60000) {
        t = (TMR1H << 8) | TMR1L;
    }

    T1CONbits.TMR1ON = 0;

    *valid = 1;   // ? ????
    return t;
}



float ultrasonic1_distance_cm(char *valid) {
    unsigned int us;

    ultrasonic1_trigger();
    us = ultrasonic1_get_us(valid);

    if (!(*valid)) return -1;  // ? ??????

    return (float)us * 0.549; // 125kHz ??
}

void ultrasonic2_trigger(void) {
    LATAbits.LATA4 = 1;
    __delay_us(10);
    LATAbits.LATA4 = 0;
}


unsigned int ultrasonic2_get_us(char *valid2) {
    unsigned int t = 0;
    unsigned int timeout = 0;

    *valid2 = 0;   // ????

    // ? echo HIGH
    while (!PORTAbits.RA5 && timeout < 120000) timeout++;
    if (timeout >= 120000) return 0;

    TMR1H = 0;
    TMR1L = 0;
    T1CONbits.TMR1ON = 1;

    while (PORTAbits.RA5 && t < 60000) {
        t = (TMR1H << 8) | TMR1L;
    }

    T1CONbits.TMR1ON = 0;

    *valid2 = 1;   // ? ????
    return t;
}



float ultrasonic2_distance_cm(char *valid2) {
    unsigned int us;

    ultrasonic2_trigger();
    us = ultrasonic2_get_us(valid2);

    if (!(*valid2)) return -1;  // ? ??????

    return (float)us * 0.549; // 125kHz ??
}

void state_3(void) {
    LATAbits.LATA2 = 0;//green led
    LATAbits.LATA1 = 0;//yellow led
    LATAbits.LATA0 = 1;//red led
    LATAbits.LATA3 = 1;//BUZZER ON
    dir = 1;
    move_steps_with_bounce();
    return;
    
}

void state_2(void) {
    state = 2;
    LATAbits.LATA2 = 0;//green led
    LATAbits.LATA1 = 0;//yellow led
    LATAbits.LATA0 = 1;//red led
    LATAbits.LATA3 = 1;//BUZZER ON
    displayDigit(10);
    move_steps_with_bounce();
    int d = 11;
    int valid2 = 0;
    d = ultrasonic2_distance_cm(&valid2);
    if(valid2 && d < 10) {
        state = 3;
        is_idle = 1;
        state_3();
    }
    return;
}

void state_1(void) {
    state = 1;
    LATAbits.LATA2 = 0;//green led
    LATAbits.LATA1 = 1;//yellow led
    LATAbits.LATA0 = 0;//red led
    LATAbits.LATA3 = 1;//BUZZER ON
    //7-seg countdown 9 to 0
    for(int i = 9; i >= 0; i--) {
        displayDigit(i);
        __delay_ms(1000);
    }
    state_2();
    return;
}

void state_0(void) {
    LATAbits.LATA2 = 1;//green led
    LATAbits.LATA1 = 0;//yellow led
    LATAbits.LATA0 = 0;//red led
    CCPR1L = 0x0B; 
    CCP1CONbits.DC1B = 0b01; //motor:0
    LATAbits.LATA3 = 0;//buzzer close
    displayDigit(10);//7-seg
    return;
}



void main(void) {
    float d;
    char valid;
    OSCCON = 0b00010010;  
    ADCON1 = 0x0F;        

    //ultrasonic 1 & 2
    TRISBbits.TRISB0 = 0;
    TRISBbits.TRISB1 = 1;
    TRISAbits.TRISA4 = 0;  // TRIG: output
    TRISAbits.TRISA5 = 1;  // ECHO: input

    //LEDs
    TRISAbits.TRISA0 = 0; //RED
    LATAbits.LATA0 = 0;
    TRISAbits.TRISA1 = 0;//YELLOW
    LATAbits.LATA1 = 0;
    TRISAbits.TRISA2 = 0;//GREEN
    LATAbits.LATA2 = 0;
    
    // Buzzer pin
    TRISAbits.TRISA3 = 0;
    LATAbits.LATA3 = 0;
    
    //7-SEG 
    TRISD = 0x00;   //RD0~RDS6 output
    LATD = 0x00;
    
    //MOTOR // CCP1/RC2
    TRISC = 0;
    LATC = 0;
    INTCON2bits.RBPU = 0; 
    // Timer2 -> On, prescaler -> 4
    T2CONbits.TMR2ON = 0b1;
    T2CONbits.T2CKPS = 0b01;
    CCP1CONbits.CCP1M = 0b1100;
    PR2 = 0x9B;
    CCPR1L = 0x0B; 
    CCP1CONbits.DC1B = 0b01;
    
    T1CON = 0b00000001;

    while (1) {
        state_0();
        d = ultrasonic1_distance_cm(&valid);

        if (valid && d < 10.0) {
            state = 1;
            is_idle = 0;
            state_1();
        } else {
            state = 0;
            state_0();
        }

        __delay_ms(10);
    }
}
