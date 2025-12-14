#include <xc.h>

#pragma config OSC = INTIO67
#pragma config WDT = OFF

#define _XTAL_FREQ 125000

/* =========================
   State Definitions
========================= */
#define STATE_TRAFFIC   0   // Normal traffic light cycle
#define STATE_BLOCKED   1   // Train detected, gate closing
#define STATE_RELEASE   2   // Waiting for train exit, gate opening

int state = STATE_TRAFFIC;
int last_trigger = 0;
/* =========================
   Motor PWM Control
========================= */
int duty_now = 45;
int dir = -1;   // -1: closing gate, 1: opening gate

void pwm_set_val(int value) {
    if (value <= 16) value = 16;
    if (value >= 75) value = 75;
    CCPR1L = (unsigned char)(value >> 2);
    CCP1CONbits.DC1B = (unsigned char)(value & 0x03);
    CCPR2L = (unsigned char)(value >> 2);//motor2
    CCP2CONbits.DC2B = (unsigned char)(value & 0x03);//motor2
}

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

/* =========================
   7-Segment Display (Single Digit)
========================= */
const unsigned char SEG_TABLE[11] = {
    0b00111111, // 0
    0b00000110, // 1
    0b01011011, // 2
    0b01001111, // 3
    0b01100110, // 4
    0b01101101, // 5
    0b01111101, // 6
    0b00000111, // 7
    0b01111111, // 8
    0b01101111, // 9
    0b00101010  // idle symbol
};

void displayDigit(unsigned char n) {
    LATD = SEG_TABLE[n];
}

/* =========================
   Ultrasonic Sensor 1
========================= */
void ultrasonic1_trigger(void) {
    LATBbits.LATB0 = 1;
    __delay_us(10);
    LATBbits.LATB0 = 0;
}

unsigned int ultrasonic1_get_us(char *valid) {
    unsigned int t = 0, timeout = 0;
    *valid = 0;

    // Wait for ECHO to go HIGH
    while (!PORTBbits.RB1 && timeout < 120000) timeout++;
    if (timeout >= 120000) return 0;

    // Measure HIGH pulse width using Timer1
    TMR1H = 0;
    TMR1L = 0;
    T1CONbits.TMR1ON = 1;

    while (PORTBbits.RB1 && t < 60000) {
        t = (TMR1H << 8) | TMR1L;
    }

    T1CONbits.TMR1ON = 0;
    *valid = 1;
    return t;
}

float ultrasonic1_distance_cm(char *valid) {
    ultrasonic1_trigger();
    unsigned int us = ultrasonic1_get_us(valid);
    if (!(*valid)) return -1;
    return (float)us * 0.549;
}

/* =========================
   Ultrasonic Sensor 2
========================= */
void ultrasonic2_trigger(void) {
    LATAbits.LATA4 = 1;
    __delay_us(10);
    LATAbits.LATA4 = 0;
}

unsigned int ultrasonic2_get_us(char *valid) {
    unsigned int t = 0, timeout = 0;
    *valid = 0;

    // Wait for ECHO to go HIGH
    while (!PORTAbits.RA5 && timeout < 120000) timeout++;
    if (timeout >= 120000) return 0;

    TMR1H = 0;
    TMR1L = 0;
    T1CONbits.TMR1ON = 1;

    while (PORTAbits.RA5 && t < 60000) {
        t = (TMR1H << 8) | TMR1L;
    }

    T1CONbits.TMR1ON = 0;
    *valid = 1;
    return t;
}

float ultrasonic2_distance_cm(char *valid) {
    ultrasonic2_trigger();
    unsigned int us = ultrasonic2_get_us(valid);
    if (!(*valid)) return -1;
    return (float)us * 0.549;
}

/* =========================
   Traffic Light Cycle + Countdown
========================= */
void traffic_cycle(void) {
    char v1, v2;
    float d1, d2;
    last_trigger = 0;
    // GREEN light for 10 seconds
    LATAbits.LATA2 = 1;
    LATAbits.LATA1 = 0;
    LATAbits.LATA0 = 0;

    for (int t = 9; t >= 0; t--) {
        displayDigit(t);
        __delay_ms(1000);
        d1 = ultrasonic1_distance_cm(&v1);
        d2 = ultrasonic2_distance_cm(&v2);
        if (v1 && d1 < 10) {
            last_trigger = 1;
            state = STATE_BLOCKED;
            return;
        }

        if (v2 && d2 < 10) {
            last_trigger = 2;
            state = STATE_BLOCKED;
            return;
        }

    }

    // YELLOW light for 3 seconds
    LATAbits.LATA2 = 0;
    LATAbits.LATA1 = 1;
    LATAbits.LATA0 = 0;

    for (int t = 3; t >= 0; t--) {
        displayDigit(t);
        __delay_ms(1000);
        d1 = ultrasonic1_distance_cm(&v1);
        d2 = ultrasonic2_distance_cm(&v2);
        if (v1 && d1 < 10) {
            last_trigger = 1;
            state = STATE_BLOCKED;
            return;
        }

        if (v2 && d2 < 10) {
            last_trigger = 2;
            state = STATE_BLOCKED;
            return;
        }

    }

    // RED light for 10 seconds
    LATAbits.LATA2 = 0;
    LATAbits.LATA1 = 0;
    LATAbits.LATA0 = 1;

    for (int t = 9; t >= 0; t--) {
        displayDigit(t);
        __delay_ms(1000);
        d1 = ultrasonic1_distance_cm(&v1);
        d2 = ultrasonic2_distance_cm(&v2);
        if (v1 && d1 < 10) {
            last_trigger = 1;
            state = STATE_BLOCKED;
            return;
        }

        if (v2 && d2 < 10) {
            last_trigger = 2;
            state = STATE_BLOCKED;
            return;
        }

    }
}

/* =========================
   Gate Closing State
========================= */
void blocked_state(void) {
    LATAbits.LATA2 = 0;
    LATAbits.LATA1 = 0;
    LATAbits.LATA0 = 1;
    LATAbits.LATA3 = 1; // Buzzer ON

    displayDigit(10);

    dir = -1;
    move_steps_with_bounce();

    state = STATE_RELEASE;
}

/* =========================
   Gate Opening State
========================= */
void release_state(void) {
    char v1, v2;
    float d1, d2;

    while (1) {
        d1 = ultrasonic1_distance_cm(&v1);
        d2 = ultrasonic2_distance_cm(&v2);

        if (last_trigger == 1) {
            // Train entered from ultrasonic1 side
            if (v2 && d2 < 10) break;
        }

        if (last_trigger == 2) {
            // Train entered from ultrasonic2 side
            if (v1 && d1 < 10) break;
        }

        __delay_ms(100);
    }

    LATAbits.LATA3 = 0;   // Buzzer OFF
    dir = 1;              // Open gate
    move_steps_with_bounce();

    state = STATE_TRAFFIC;
}


/* =========================
   Main Function
========================= */
void main(void) {
    OSCCON = 0b00010010;
    ADCON1 = 0x0F;

    // Ultrasonic sensors
    TRISBbits.TRISB0 = 0;
    TRISBbits.TRISB1 = 1;
    TRISAbits.TRISA4 = 0;
    TRISAbits.TRISA5 = 1;

    // LEDs and buzzer
    TRISA &= 0b11000000;

    // 7-segment display
    TRISD = 0x00;

    // PWM and motor setup
    TRISC = 0;
    INTCON2bits.RBPU = 0;
    T2CONbits.TMR2ON = 1;
    T2CONbits.T2CKPS = 0b01;
    CCP1CONbits.CCP1M = 0b1100;
    CCP1CONbits.CCP2M = 0b1100; //motor2
    PR2 = 0x9B;
    pwm_set_val(duty_now);

    // Timer1
    T1CON = 0b00000001;

    while (1) {
        if (state == STATE_TRAFFIC) traffic_cycle();
        else if (state == STATE_BLOCKED) blocked_state();
        else if (state == STATE_RELEASE) release_state();
    }
}

