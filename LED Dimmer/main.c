#include "bits.h"
#include "tm4c123gh6pm.h"

int main(void)
{
    int pulseWidth = 100;
    int maxValue = 4095;
    // Clock Port E and B, Clock PWM0, Clock ADC
    SYSCTL_RCGCGPIO_R = BIT_1 | BIT_4;
    SYSCTL_RCGCPWM_R = BIT_0;
    SYSCTL_RCGCADC_R = BIT_0;

    // Setting up Port E
    GPIO_PORTE_AFSEL_R |= BIT_3;
    GPIO_PORTE_DEN_R &= ~BIT_3;
    GPIO_PORTE_DIR_R &= ~BIT_3;

    // Enable analog on PE3
    GPIO_PORTE_AMSEL_R |= BIT_3;

    // Setting up Port B
    GPIO_PORTB_AFSEL_R |= BIT_6;
    // setting PB6 alternate function for PWM output
    GPIO_PORTB_PCTL_R |= 0x4000000;
    GPIO_PORTB_DIR_R |= BIT_6;
    GPIO_PORTB_DEN_R |= BIT_6;

    // disable SS3
    ADC0_ACTSS_R &= ~BIT_3;

    // Continuously sample
    ADC0_EMUX_R |= 0xF << 12;

    // select every sample source within MUX3
    ADC0_SSMUX3_R |= 0; // see if 0xf works better

    // enable end bit
    ADC0_SSCTL3_R |= BIT_1;

    // enable
    ADC0_ACTSS_R |= BIT_3;

    // clear flag
    ADC0_ISC_R = BIT_3;

    // Count down mode and disabled
    PWM0_0_CTL_R |= 0x00;

    // Setting load value for time (one tick from PWM clock is 0.0625 us)
    // Duty cycle will be a percentage of this square wave (onTime + offTime)
    PWM0_0_LOAD_R = pulseWidth/0.0625;

    // set to value to trigger event (IN THIS EXAMPLE ITS HALF THE PWM0_LOAD)
    PWM0_0_CMPA_R = 0;

    // Set pwmA to be high when counter reaches
    PWM0_0_GENA_R |= BIT_2 | BIT_3 | BIT_7;

    // enable PWM
    PWM0_0_CTL_R |= BIT_0;

    // enable PWM0 output
    PWM0_ENABLE_R |= BIT_0;

    while (1)
    {
        /*int i;
        for(i = 0; i < 1000; i++);
        PWM0_0_CMPA_R++;
        j++;
        if(j == 1599) {
            j = 0;
            PWM0_0_CMPA_R = 0;
        }*/
        // Enable triggers for SS3
        ADC0_PSSI_R = BIT_3;
        while((ADC0_RIS_R & BIT_3) == 0x00);
        PWM0_0_CMPA_R = (ADC0_SSFIFO3_R/maxValue) * PWM0_0_LOAD_R;
        ADC0_ISC_R = BIT_3;
    }
}
