/*Channel 4 on the TM4C123 is PD3 and channel 5 is PD2.*/
#include <stdint.h>
#include<stdio.h>
#include "inc/tm4c123gh6pm.h"
void enable_irq(void);
volatile int result;
volatile int samplecount;
volatile float resultf;
volatile int volt_acc;

volatile int samplecount2;
volatile float resultf2;
volatile int current_acc;
volatile int current_less, current_more, voltage_more, off_flag;

int main(void)
{
    current_less=0; current_more=0; voltage_more=0; off_flag=0;
    samplecount=0; volt_acc=0;
    //count=0;

    /* Enable Peripheral Clocks */
        SYSCTL_RCGCPWM_R |= 1;       /* enable clock to PWM0 */
        SYSCTL_RCGCGPIO_R |= 2;   /* enable clock to PORTB */
        SYSCTL_RCGCGPIO_R |= 0x08; /* enable clock to PE (AIN0 is on PE3) */
        SYSCTL_RCGCADC_R |= 3;     /* enable clock to ADC0 */
       // SYSCTL_RCC_R &= ~0x00100000; /* no pre-divide for PWM clock */
        //PLL_Init();

        /* Enable port PF3 for PWM0 M1PWM7 */
        GPIO_PORTB_AFSEL_R = 0x40;      /* PB6 uses alternate function */
        GPIO_PORTB_PCTL_R &= ~0x0F000000;
        GPIO_PORTB_PCTL_R |= 0x04000000;
        GPIO_PORTB_DEN_R |= 0x40;       /* pin digital */
        /* initialize PE3 for AIN0 input  */
        GPIO_PORTD_AFSEL_R |=0x0C;   /* enable alternate function */
        GPIO_PORTD_DEN_R &= ~0x0C;    /* disable digital function */
        GPIO_PORTD_AMSEL_R |=0x0C;   /* enable analog function */

        PWM0_0_CTL_R = 0;            /* stop counter */
        PWM0_0_GENA_R = 0x0000008C;  /* M1PWM7 output set when reload, */
                                     /* clear when match PWMCMPA */
        PWM0_0_LOAD_R = 800;       /* set load value for 20kHz (80MHz/4000) */
        PWM0_0_CMPA_R = 400;       /* set duty cycle to min */
        PWM0_0_CTL_R = 1;            /* start timer */
        PWM0_ENABLE_R = 1;        /* start PWM0 ch7 */








    /* enable clocks */
    SYSCTL_RCGCGPIO_R |= 0x08; /* enable clock to PE (AIN0 is on PE3) */
    SYSCTL_RCGCADC_R |= 3;     /* enable clock to ADC0 */
    /* initialize PE3 for AIN0 input  */
    GPIO_PORTD_AFSEL_R |=0x0C;   /* enable alternate function */
    GPIO_PORTD_DEN_R &= ~0x0C;    /* disable digital function */
    GPIO_PORTD_AMSEL_R |=0x0C;   /* enable analog function */
    /* initialize ADC0 */
    ADC0_ACTSS_R &= ~8;
    ADC1_ACTSS_R &= ~8; /* disable SS3 during configuration */
    ADC0_EMUX_R &= ~0xF000;
    ADC1_EMUX_R &= ~0xF000;/* software trigger conversion */
    ADC0_SSMUX3_R = 4;
    ADC1_SSMUX3_R = 5;/* get input from channel 5 */
    ADC0_SSCTL3_R |= 6;
    ADC1_SSCTL3_R |= 6;/* take one sample at a time, set flag at 1st sample */
    ADC0_ACTSS_R |= 8;
    ADC1_ACTSS_R |= 8;
    NVIC_ST_RELOAD_R = 16000-1;  /* count value for systick interrupt*/
    NVIC_ST_CTRL_R = 7;             /* enable SysTick interrupt, use system clock */
    enable_irq();/* enable ADC0 sequencer 3 */
    while(1)
        {
        ;}
}

void inline enable_irq(void)
{
    __asm  ("    CPSIE  I\n");
}

void SysTick_Handler(void)
{
    ADC0_PSSI_R |= 8;      /* start a conversion sequence 3 */
    while((ADC0_RIS_R & 8) == 0);/* wait for conversion complete */
    result = ADC0_SSFIFO3_R;
    resultf = result*3.3/4096;
    resultf = resultf*100;
    result = (int)(resultf);

    if(samplecount<100){
        volt_acc=volt_acc+result;
        samplecount++;
    }

    if(samplecount==100){
        samplecount=0;
        volt_acc = volt_acc/100;

        if(volt_acc < 177)
            {
                voltage_more = 0;
            }

        if(volt_acc >= 177)
        {
            voltage_more++;
            if(voltage_more==2)
            {
                //PWM0_0_CMPA_R = 4000;       /* set duty cycle to 0 */
                PWM0_ENABLE_R = 0;
                off_flag = 1;
            }
        }
        //printf("Channel-4\t%d\t",volt_acc);
        volt_acc=0;
    }

    ADC0_ISC_R = 8;

    ADC1_PSSI_R |= 8;      /* start a conversion sequence 3 */
    while((ADC1_RIS_R & 8) == 0);/* wait for conversion complete */
    result = ADC1_SSFIFO3_R;

    resultf2 = result*3.3/4096;
        resultf2 = resultf2*100;
        result = (int)(resultf2);

        if(samplecount2<100){
            current_acc=current_acc+result;
            samplecount2++;
        }

        if(samplecount2==100){
            samplecount2=0;
            current_acc = current_acc/100;
            if(current_acc<118 && off_flag==0)
                        {
                            current_less++;
                            if(current_less==2)
                            {
                                PWM0_0_CMPA_R = PWM0_0_CMPA_R - 40;       /*increase duty cycle  by 1% */
                                if(PWM0_0_CMPA_R == 880)
                                {
                                    off_flag = 1;
                                    //PWM0_0_CMPA_R = 4000;
                                    PWM0_ENABLE_R = 0;
                                }
                                current_less = 0;
                            }
                        }
            if(current_acc>122 && off_flag==0)
                {
                current_more++;
                if(current_more==2)
                {
                    PWM0_0_CMPA_R = PWM0_0_CMPA_R + 40;       /*decrease duty cycle  by 1% */
                    current_more = 0;
                }
                }
            if(current_acc>=118 && current_acc<=122)
            {
                current_less = 0;
                current_more = 0;
            }
            //printf("Channel-5\t%d\n\n",current_acc);
            current_acc=0;
        }


    ADC1_ISC_R = 8;  /* clear completion flag */
}

