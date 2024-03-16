#define SYSCTL_RCGCGPIO_R (*((volatile unsigned long *) 0x400FE608))
#define GPIO_PORTF_DEN_R (*((volatile unsigned long *) 0x4002551C))
#define GPIO_PORTF_DIR_R (*((volatile unsigned long *) 0x40025400))
#define GPIO_PORTF_DATA_R (*((volatile unsigned long *) 0x40025010))

int main(void)

{
    volatile unsigned long i;
    SYSCTL_RCGCGPIO_R = 0x00000020;
    GPIO_PORTF_DEN_R = 0x00000004;
    GPIO_PORTF_DIR_R = 0x00000004;

    while (1)
    {
        GPIO_PORTF_DATA_R = 0x00000004;
        for(i = 0; i < 200000; i++);
        GPIO_PORTF_DATA_R = 0x00000000;
        for(i = 0; i < 200000; i++);

    }
}
