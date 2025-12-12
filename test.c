#include <xc.h>

#pragma config OSC = INTIO67
#pragma config WDT = OFF
#define _XTAL_FREQ 1000000

// ?? 7???(0~9)
const unsigned char SEG_TABLE[10] = {
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
    0b01101111  //9
};

void displayDigit(unsigned char n){
    LATD = SEG_TABLE[n];
}

void main(void) {
    unsigned char num = 0;

    OSCCON = 0b01110010;
    ADCON1 = 0x0F;
    
    TRISD = 0x00;   //RE0~RE6 output
    LATD = 0x00;
    TRISC = 0x00;
    LATC = 0x00;

    while(1) {
        displayDigit(num);
        __delay_ms(10000);

        num++;
        if(num >= 10)
            num = 0;
        //LATD = 0b01111111;
        //LATC = 0b01111111;
    }
}
