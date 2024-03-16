#include "tm4c123gh6pm.h"
#include "bits.h"
#include <stdbool.h>
#include <math.h>
#include<stdio.h>
#include<stdlib.h>

char matrix[4][4] = { { '1', '2', '3', 'A' },
                      { '4', '5', '6', 'B' },
                      { '7', '8', '9', 'C' },
                      { '*', '0', '#', 'D' } };

char bit_array[4] = {BIT_1, BIT_2, BIT_3, BIT_4};

void set_RS(unsigned char rs)
{
    if (rs >= 1)
    {
        GPIO_PORTE_DATA_R |= BIT_0;
    }
    else
    {
        GPIO_PORTE_DATA_R &= ~BIT_0;
    }
}

void set_RW(unsigned char rw)
{
    if (rw >= 1)
    {
        GPIO_PORTE_DATA_R |= BIT_1;
    }
    else
    {
        GPIO_PORTE_DATA_R &= ~BIT_1;
    }
}

void set_EN(unsigned char en)
{
    if (en >= 1)
    {
        GPIO_PORTE_DATA_R |= BIT_2;
    }
    else
    {
        GPIO_PORTE_DATA_R &= ~BIT_2;
    }
}

void set_data_bits(unsigned char data)
{
    GPIO_PORTB_DATA_R = data;
}

void delay()
{
    int i;
    for (i = 0; i < 3000; ++i)
        ;
}

/*void wait_on_busy()
{
    GPIO_PORTB_DIR_R &= ~BIT_7;
    while (GPIO_PORTB_DATA_R & BIT_7)
        ;
    GPIO_PORTB_DIR_R |= BIT_7;
}*/

void write_LCD(unsigned char data, unsigned char rs)
{
    set_RS(rs);
    set_RW(0);
    delay();
    set_EN(1);
    delay();
    set_data_bits(data);
    delay();
    set_EN(0);
    delay();
}

void turn_on_LED()
{
    GPIO_PORTF_DATA_R |= BIT_2;
}

void write_instruction(unsigned char data)
{
    write_LCD(data, 0);
}

void clear_display()
{
    write_instruction(0x01);
}

void return_home()
{
    write_instruction(0x02);
}

void write_char(unsigned char data)
{
    write_LCD(data, 1);
}

void write_str(char *str)
{
    /*int i;
    for(i = 0; i < length; i++) {
        write_char(str[i]);
    }*/

    while(*str != '\0') {
        write_char(*str);
        str++;
    }
}

void init_LCD()
{
    //increment cursor and display shift off
    write_instruction(0x06);

    //cursor and blink on
    write_instruction(0b1100);

    //8-bit mode, 2 lines, 5x8 dots per character
    write_instruction(0x38);

    clear_display();
}

void init_ports(void) {
    // Using port B for DB, port C and port F for keypad, and E for LCD pins
    SYSCTL_RCGCGPIO_R |= BIT_1 | BIT_2 | BIT_4 | BIT_5;

    delay();
    GPIO_PORTC_CR_R |= 0XF0;
    GPIO_PORTF_CR_R |= 0xFF;

    // Set digital enable for all ports
    GPIO_PORTB_DEN_R |= 0xff;
    //GPIO_PORTD_DEN_R |= 0xf;
    GPIO_PORTE_DEN_R |= 0x07;
    GPIO_PORTF_DEN_R |= 0x1E;

    // Set direction for all ports
    GPIO_PORTB_DIR_R |= 0xff;
    //GPIO_PORTD_DIR_R |= 0x00;
    GPIO_PORTE_DIR_R |= 0x07;
    GPIO_PORTF_DIR_R |= 0x1E;

    // Enable pull down resistor on port D
    //GPIO_PORTD_PDR_R |= 0xf0;

    // testing port C
    GPIO_PORTC_DEN_R |= 0xf0;
    GPIO_PORTC_DIR_R |= 0x00;
    GPIO_PORTC_PDR_R |= 0xf0;

}

char getKey() {
    int i;
    int j;
    for(i = 0; i < 4; i++) {
        GPIO_PORTF_DATA_R = bit_array[i];
        delay();
        for(j = 0; j < 4; j++) {
            if((GPIO_PORTC_DATA_R & 0xf0) & (BIT_0 << j+4)) {
                return matrix[i][j];
            }
        }
    }
    return '\0';
}

void resetCal(void) {
    clear_display();
    write_instruction(0x80);
}

bool onlyNum(char button) {
    return button != 'A' && button != 'B' &&
           button != 'C' && button != 'D' &&
           button != '*' && button != '#';
}

void resetStateArrays(char *A, char *B) {
    int i;
    for(i = 0; i < 8; i++) {
        A[i] = '\0';
        B[i] = '\0';
    }
}

int getLength(char* str) {
    int i = 0;
    while(*str != '\0') {
        i++;
    }
    return i;
}

int charToInt(char input) {
    switch(input) {
    case '1':
        return 1;
    case '2':
        return 2;
    case '3':
        return 3;
    case '4':
        return 4;
    case '5':
        return 5;
    case '6':
        return 6;
    case '7':
        return 7;
    case '8':
        return 8;
    case '9':
        return 9;
    case '0':
        return 0;
    default:
        return '\0';
    }
}

char intToChar(int input) {
    switch(input) {
    case 1:
        return '1';
    case 2:
        return '2';
    case 3:
        return '3';
    case 4:
        return '4';
    case 5:
        return '5';
    case 6:
        return '6';
    case 7:
        return '7';
    case 8:
        return '8';
    case 9:
        return '9';
    case 0:
        return '0';
    default:
        return '\0';
    }
}

void printProduct(int product) {
    if (product == 0) {
        write_char('0');
        return;
    }

    unsigned int reverseProduct = 0;
    int digitCount = 0;

    while (product > 0) {
        reverseProduct = reverseProduct * 10 + product % 10;
        product /= 10;
        digitCount++;
    }

    while (digitCount > 0) {
        write_char('0' + reverseProduct % 10);
        reverseProduct /= 10;
        digitCount--;

        if (reverseProduct == 0) {
            while (digitCount > 0) {
                write_char('0');
                digitCount--;
            }
        }
    }
}


void calculate(char* A, char* B) {
    int aInt = atoi(A);
    int bInt = atoi(B);
    printProduct(aInt * bInt);
}

void displayProduct(char* A, char* B) {
    clear_display();
    write_instruction(0xC0);
    calculate(A, B);
}

void run_calculator() {
    char A[9] = {'\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0'};
    char B[9] = {'\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0'};
    char state = 'A';

    int i = 0;
    while (1) {
        char button = getKey();
        while(button == '\0') {
            button = getKey();
        }

        while(getKey() != '\0');

        if (i >= 8) {
            if(state == 'A') {
                state = 'B';
                resetCal();
                i = 0;
            } else {
                state = 'D';
                displayProduct(A, B);
                i = 0;
            }
        }

        if (button == '*' && state == 'A') {
            state = 'B';
            resetCal();
            i = 0;
        }

        if (button == '#' && state != 'D') {
            state = 'D';
            displayProduct(A, B);
            i = 0;
        }

        if (button == 'C') {
            state = 'A';
            resetStateArrays(A, B);
            resetCal();
            i = 0;
        }

        if (state == 'A' && onlyNum(button)) {
            A[i] = button;
            write_char(button);
            i++;
        } else if (state == 'B' && onlyNum(button)) {
            B[i] = button;
            write_char(button);
            i++;
        }
    }
}

int main(void)
{
    // initalize all necessary ports
    init_ports();

    // initialize the LCD display
    init_LCD();

	while(1) {
	    run_calculator();
	}
}
