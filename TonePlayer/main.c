#include "bits.h"
#include "tm4c123gh6pm.h"
#include <stdlib.h>

#define MULTIPLIER 100

int matrix[4][4] = { {10, 11, 12,  1},
                     {13, 14, 15,  2},
                     {16, 17,  0,  3},
                     {0,   0,  0,  0}};

char bit_array[4] = {BIT_1, BIT_2, BIT_3, BIT_4};

int getKey() {
    int i;
    int j;
    while(1) {
        for(i = 0; i < 4; i++) {
            GPIO_PORTF_DATA_R = bit_array[i];
            for(j = 0; j < 4; j++) {
                if((GPIO_PORTC_DATA_R & 0xf0) & (BIT_0 << j+4)) {
                    return matrix[i][j];
                }/
            }
        }
    }
}

void init_i2c() {
    //address of eeprom is 1010000
    SYSCTL_RCGCI2C_R = BIT_1;
    SYSCTL_RCGCGPIO_R |= BIT_0;

    // Port A Alternate Function
    GPIO_PORTA_AFSEL_R |= BIT_6 | BIT_7;
    GPIO_PORTA_PCTL_R |= GPIO_PCTL_PA7_I2C1SDA | GPIO_PCTL_PA6_I2C1SCL;
    GPIO_PORTA_PUR_R |= BIT_6 | BIT_7;
    GPIO_PORTA_ODR_R |= BIT_7;
    GPIO_PORTA_DEN_R |= BIT_6 | BIT_7;

    // Enable I2C1 Master
    I2C1_MCR_R = I2C_MCR_MFE;
    I2C1_MTPR_R = 0x07;
}

void write_i2c(int data, int lowAddress) {
    int i;
    I2C1_MSA_R = 0xA0;
    I2C1_MDR_R = 0;
    I2C1_MCS_R = 0b00000011;

    while(I2C1_MCS_R & I2C_MCS_BUSY);

    I2C1_MDR_R = lowAddress;
    I2C1_MCS_R = 0b00000001;
    while(I2C1_MCS_R & I2C_MCS_BUSY);

    I2C1_MDR_R = data;
    I2C1_MCS_R = 0b00000101;

    while(I2C1_MCS_R & I2C_MCS_BUSY);
    for(i = 0; i < 10000; i++);
}

int read_i2c(int lowAddress) {
    int data;
    I2C1_MSA_R = 0xA0;
    I2C1_MDR_R = 0x00000000;
    I2C1_MCS_R = 0b00000011;

    while(I2C1_MCS_R & I2C_MCS_BUSY);

    I2C1_MDR_R = lowAddress;
    I2C1_MCS_R = 0b00000001;
    while(I2C1_MCS_R & I2C_MCS_BUSY);

    I2C1_MSA_R = 0xA1;
    I2C1_MCS_R = 0b0001011;
    while(I2C1_MCS_R & I2C_MCS_BUSY);

    data = I2C1_MDR_R;
    I2C1_MCS_R = 0b0000101;
    while(I2C1_MCS_R & I2C_MCS_BUSY);
    return data;
}

void init_pwm() {
    // Clock Port E and B, Clock PWM0, Clock ADC
    // A0, B1, C2, D3, E4, F5
    SYSCTL_RCGCGPIO_R = BIT_1 | BIT_2 | BIT_4 | BIT_5;
    SYSCTL_RCGCPWM_R = BIT_0;
    SYSCTL_RCGCADC_R = BIT_0;

    //Keypad
    GPIO_PORTC_CR_R |= 0XF0;
    GPIO_PORTF_CR_R |= 0xFF;
    GPIO_PORTF_DEN_R |= 0x1E;

    GPIO_PORTF_DIR_R |= 0x1E;

    GPIO_PORTC_DEN_R |= 0xf0;
    GPIO_PORTC_DIR_R |= 0x00;
    GPIO_PORTC_PDR_R |= 0xf0;

    // LED Mode Display
    GPIO_PORTE_DEN_R |= BIT_1 | BIT_2 | BIT_3;
    GPIO_PORTE_DIR_R |= BIT_1 | BIT_2 | BIT_3;

    // Setting up Port B
    GPIO_PORTB_AFSEL_R |= BIT_6;
    // setting PB6 alternate function for PWM output
    GPIO_PORTB_PCTL_R |= 0x4000000;
    GPIO_PORTB_DIR_R |= BIT_6;
    GPIO_PORTB_DEN_R |= BIT_6;

    // Count down mode and disabled
    PWM0_0_CTL_R |= 0x00;

    // Setting load value for time (one tick from PWM clock is 0.0625 us)
    // Duty cycle will be a percentage of this square wave (onTime + offTime)
    PWM0_0_LOAD_R = 1700/0.0625;

    // set to value to trigger event (IN THIS EXAMPLE ITS HALF THE PWM0_LOAD)
    PWM0_0_CMPA_R = PWM0_0_LOAD_R/2;

    // Set pwmA to be high when counter reaches
    PWM0_0_GENA_R |= BIT_2 | BIT_3 | BIT_7;

    // enable PWM
    PWM0_0_CTL_R |= BIT_0;

    // enable PWM0 output
    PWM0_ENABLE_R |= BIT_0;
}

void playSound(int key) {
    int i;
    PWM0_0_LOAD_R = (key*MULTIPLIER)/0.0625;
    for(i = 0; i < 500000; i++);
    PWM0_0_LOAD_R = 0;
}

void recordSound(int key, int counter) {
    write_i2c(key, counter);
}

void playBackSounds(int counter) {
    int i;
    for(i = 0; i < counter; i++) {
        playSound(read_i2c(i));
    }
}

int main(void)
{
    char mode = 'p';
    int counter = 0;
    init_pwm();
    init_i2c();

    GPIO_PORTE_DATA_R = 0x00;
    GPIO_PORTE_DATA_R = BIT_1;

    while (1)
    {
        int key = getKey();
        if(key < 4) {
            if(key == 1) { // play mode
                GPIO_PORTE_DATA_R = 0x00;
                GPIO_PORTE_DATA_R = BIT_1;
                mode = 'p';
            } else if (key == 2) { // record mode
                GPIO_PORTE_DATA_R = 0x00;
                GPIO_PORTE_DATA_R = BIT_2;
                mode = 'r';
            } else if (key == 3) { // playback mode
                GPIO_PORTE_DATA_R = 0x00;
                GPIO_PORTE_DATA_R = BIT_3;
                mode = 'b';
            }
        }
        playSound(key);

        if(mode == 'r' && key != 0 && key != 2) {
            recordSound(key, counter);
            counter++;
        } else if (mode == 'b') {
            playBackSounds(counter);
            counter = 0;
        }
    }
}
