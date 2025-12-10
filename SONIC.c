#include <xc.h>

#pragma config OSC = INTIO67
#pragma config WDT = OFF
#define _XTAL_FREQ 1000000


void ultrasonic_trigger(void) {
    LATBbits.LATB0 = 1;
    __delay_us(10);
    LATBbits.LATB0 = 0;
}


unsigned int ultrasonic_get_us(void) {
    unsigned int t = 0;
    unsigned int timeout = 0;

    // ? ECHO=1
    while (!PORTBbits.RB1 && timeout < 30000) timeout++;
    if (timeout >= 30000) return 60000;   

    T1CONbits.TMR1ON = 1;

    while (PORTBbits.RB1 && t < 60000) {
        t = (TMR1H << 8) | TMR1L;
    }

    T1CONbits.TMR1ON = 0;
    return t;
}


float ultrasonic_distance_cm(void) {
    unsigned int us;

    TMR1H = 0;
    TMR1L = 0;

    ultrasonic_trigger();
    us = ultrasonic_get_us();

    if (us >= 60000) return 999;  

    return (float)us * 0.01715 / 2; 
}


void main(void) {
    float d;

    OSCCON = 0b01110010;  
    ADCON1 = 0x0F;        

    TRISBbits.TRISB0 = 0;
    TRISBbits.TRISB1 = 1;

    TRISAbits.TRISA0 = 0;
    LATAbits.LATA0 = 0;

    T1CON = 0b00000001;

    while (1) {
        d = ultrasonic_distance_cm();

        if (d < 10.0) {
            LATAbits.LATA0 = 1;   // ? ????
        } else {
            LATAbits.LATA0 = 0;   // ? ????
        }

        __delay_ms(150);
    }
}
