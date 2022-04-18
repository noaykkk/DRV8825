#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include "driverlib/adc.h"
#include "driverlib/gpio.h"
#include "driverlib/pin_map.h"
#include "driverlib/rom_map.h"
#include "driverlib/sysctl.h"
#include "driverlib/pwm.h"
#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "inc/hw_gpio.h"

// Define const value
#define STEP_L  0x04
#define STEP_R  0x00
#define STEP_ON  0x08
#define STEP_OFF  0x00
#define ELEM_ON  0x10
#define ELEM_OFF  0x00
#define ACTU_EX  0x00
#define ACTU_SH  0x20
#define ACTU_ON  PWM_OUT_5_BIT
#define DCM_ON  PWM_OUT_6_BIT

// Prototypes
void DrawCir(void);
void DrawLine(void);
void DrawSpiral(void);

int main(void)
{
    // Set the clock
    SysCtlClockSet(SYSCTL_SYSDIV_4|SYSCTL_USE_PLL|SYSCTL_XTAL_16MHZ|SYSCTL_OSC_MAIN);

    // Configure PWM Clock
    SysCtlPWMClockSet(SYSCTL_PWMDIV_1);

    // Enable the peripherals
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_PWM1);
    SysCtlDelay(10);

    // Remove the lock
    HWREG(GPIO_PORTF_BASE+GPIO_O_LOCK) = GPIO_LOCK_KEY;
    HWREG(GPIO_PORTF_BASE+GPIO_O_CR) |= 0x01;

    // Configure PF1 as PWM output
    GPIOPinConfigure(GPIO_PF1_M1PWM5);
    GPIOPinConfigure(GPIO_PF2_M1PWM6);
    GPIOPinConfigure(GPIO_PF3_M1PWM7);
    GPIOPinTypePWM(GPIO_PORTF_BASE, GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3);

    // Configure PWM options
    PWMGenConfigure(PWM1_BASE, PWM_GEN_2, PWM_GEN_MODE_DOWN | PWM_GEN_MODE_NO_SYNC);
    PWMGenConfigure(PWM1_BASE, PWM_GEN_3, PWM_GEN_MODE_DOWN | PWM_GEN_MODE_NO_SYNC);

    //Set the Period (expressed in clock ticks)
    PWMGenPeriodSet(PWM1_BASE, PWM_GEN_2, 320);
    PWMGenPeriodSet(PWM1_BASE, PWM_GEN_3, 320);

    //Set PWM duty-50% (Period /2)
    PWMPulseWidthSet(PWM1_BASE, PWM_OUT_5,100);
    PWMPulseWidthSet(PWM1_BASE, PWM_OUT_6,160);
    PWMPulseWidthSet(PWM1_BASE, PWM_OUT_7,200);

    // Enable the PWM generator
    PWMGenEnable(PWM1_BASE, PWM_GEN_2);
    PWMGenEnable(PWM1_BASE, PWM_GEN_3);

    // Set outputs pins
    // PA2 for stepper motor direction, PA3 for stepper motor speed, PA4 for electromagnet on/off,
    // PA5 for actuator direction, PA6 for DC motor direction, PF1 for actuator on/off, PF2 for DC motor on/off
    GPIOPinTypeGPIOOutput(GPIO_PORTA_BASE, GPIO_PIN_2|GPIO_PIN_3|GPIO_PIN_4|GPIO_PIN_5|GPIO_PIN_6);
    GPIOPinTypeGPIOInput(GPIO_PORTF_BASE, GPIO_PIN_0);
//    uint32_t PinData;
//    int count=0;
//
//    for(count=0; count<300;count++)
//    {
//        PinData = GPIOPinRead(GPIO_PORTF_BASE, GPIO_PIN_0);
//
//        // Turn off the Output pins
//        PWMOutputState(PWM1_BASE, ACTU_ON, false);
//
//        GPIOPinWrite(GPIO_PORTA_BASE, GPIO_PIN_2|GPIO_PIN_3|GPIO_PIN_4|GPIO_PIN_5|GPIO_PIN_6, STEP_ON+STEP_R);
//
//        SysCtlDelay(20000);
//
//        GPIOPinWrite(GPIO_PORTA_BASE, GPIO_PIN_2|GPIO_PIN_3|GPIO_PIN_4|GPIO_PIN_5|GPIO_PIN_6, STEP_OFF+STEP_R);
//
//        SysCtlDelay(20000);
//    }
//    PWMOutputState(PWM1_BASE, ACTU_ON, false);
//    DrawCir();
    DrawLine();
//    DrawSpiral();
}

void DrawCir(void)
{
    int count=0;
    // Move to the pick up position
    for(count=0; count<900;count++){
        GPIOPinWrite(GPIO_PORTA_BASE, GPIO_PIN_2|GPIO_PIN_3|GPIO_PIN_4|GPIO_PIN_5|GPIO_PIN_6, STEP_ON+STEP_L);

        SysCtlDelay(20000);

        GPIOPinWrite(GPIO_PORTA_BASE, GPIO_PIN_2|GPIO_PIN_3|GPIO_PIN_4|GPIO_PIN_5|GPIO_PIN_6, STEP_OFF+STEP_L);

        SysCtlDelay(20000);
    }
    SysCtlDelay(20000000);

    // Start pick up
    PWMOutputState(PWM1_BASE, ACTU_ON, true);
    for(count=0; count<50;count++){
        GPIOPinWrite(GPIO_PORTA_BASE, GPIO_PIN_2|GPIO_PIN_3|GPIO_PIN_4|GPIO_PIN_5|GPIO_PIN_6, ELEM_ON+ACTU_EX);

        SysCtlDelay(20000);

        GPIOPinWrite(GPIO_PORTA_BASE, GPIO_PIN_2|GPIO_PIN_3|GPIO_PIN_4|GPIO_PIN_5|GPIO_PIN_6, ELEM_ON+ACTU_EX);

        SysCtlDelay(20000);
    }
    SysCtlDelay(20000*10000);

    // Finish pick then return
    for(count=0; count<4000;count++){
        GPIOPinWrite(GPIO_PORTA_BASE, GPIO_PIN_2|GPIO_PIN_3|GPIO_PIN_4|GPIO_PIN_5|GPIO_PIN_6, ELEM_ON+ACTU_SH);

        SysCtlDelay(20000);

        GPIOPinWrite(GPIO_PORTA_BASE, GPIO_PIN_2|GPIO_PIN_3|GPIO_PIN_4|GPIO_PIN_5|GPIO_PIN_6, ELEM_ON+ACTU_SH);

        SysCtlDelay(20000);
    }
    PWMOutputState(PWM1_BASE, ACTU_ON, false);
    SysCtlDelay(20000000);

    // Move to the spin position
    for(count=0; count<3100;count++){
        GPIOPinWrite(GPIO_PORTA_BASE, GPIO_PIN_2|GPIO_PIN_3|GPIO_PIN_4|GPIO_PIN_5|GPIO_PIN_6, STEP_ON+STEP_L+ELEM_ON);

        SysCtlDelay(20000);

        GPIOPinWrite(GPIO_PORTA_BASE, GPIO_PIN_2|GPIO_PIN_3|GPIO_PIN_4|GPIO_PIN_5|GPIO_PIN_6, STEP_OFF+STEP_L+ELEM_ON);

        SysCtlDelay(20000);
    }
    SysCtlDelay(20000000);

    // Start drop off
    PWMOutputState(PWM1_BASE, ACTU_ON, true);
    for(count=0; count<50;count++){
        GPIOPinWrite(GPIO_PORTA_BASE, GPIO_PIN_2|GPIO_PIN_3|GPIO_PIN_4|GPIO_PIN_5|GPIO_PIN_6, ELEM_ON+ACTU_EX);

        SysCtlDelay(20000);

        GPIOPinWrite(GPIO_PORTA_BASE, GPIO_PIN_2|GPIO_PIN_3|GPIO_PIN_4|GPIO_PIN_5|GPIO_PIN_6, ELEM_ON+ACTU_EX);

        SysCtlDelay(20000);
    }
    SysCtlDelay(20000*10000);

    // spin the pan
    PWMOutputState(PWM1_BASE, DCM_ON, true);
    for(count=0; count<25;count++){
            GPIOPinWrite(GPIO_PORTA_BASE, GPIO_PIN_2|GPIO_PIN_3|GPIO_PIN_4|GPIO_PIN_5|GPIO_PIN_6, ELEM_ON);

            SysCtlDelay(20000);

            GPIOPinWrite(GPIO_PORTA_BASE, GPIO_PIN_2|GPIO_PIN_3|GPIO_PIN_4|GPIO_PIN_5|GPIO_PIN_6, ELEM_ON);

            SysCtlDelay(20000);
        }
    SysCtlDelay(20000*1350);
    PWMOutputState(PWM1_BASE, DCM_ON, false);
    SysCtlDelay(20000*4000);

    // Finish pick then return
    for(count=0; count<4000;count++){
        GPIOPinWrite(GPIO_PORTA_BASE, GPIO_PIN_2|GPIO_PIN_3|GPIO_PIN_4|GPIO_PIN_5|GPIO_PIN_6, ELEM_ON+ACTU_SH);

        SysCtlDelay(20000);

        GPIOPinWrite(GPIO_PORTA_BASE, GPIO_PIN_2|GPIO_PIN_3|GPIO_PIN_4|GPIO_PIN_5|GPIO_PIN_6, ELEM_ON+ACTU_SH);

        SysCtlDelay(20000);
    }
    PWMOutputState(PWM1_BASE, ACTU_ON, false);
    SysCtlDelay(20000000);

    // Back to pick up position
    for(count=0; count<3100;count++){
        GPIOPinWrite(GPIO_PORTA_BASE, GPIO_PIN_2|GPIO_PIN_3|GPIO_PIN_4|GPIO_PIN_5|GPIO_PIN_6, ELEM_ON+STEP_ON+STEP_R);

        SysCtlDelay(20000);

        GPIOPinWrite(GPIO_PORTA_BASE, GPIO_PIN_2|GPIO_PIN_3|GPIO_PIN_4|GPIO_PIN_5|GPIO_PIN_6, ELEM_ON+STEP_OFF+STEP_R);

        SysCtlDelay(20000);
    }
    // Start drop off tools
    PWMOutputState(PWM1_BASE, ACTU_ON, true);
    for(count=0; count<50;count++){
        GPIOPinWrite(GPIO_PORTA_BASE, GPIO_PIN_2|GPIO_PIN_3|GPIO_PIN_4|GPIO_PIN_5|GPIO_PIN_6, ELEM_ON+ACTU_EX);

        SysCtlDelay(20000);

        GPIOPinWrite(GPIO_PORTA_BASE, GPIO_PIN_2|GPIO_PIN_3|GPIO_PIN_4|GPIO_PIN_5|GPIO_PIN_6, ELEM_ON+ACTU_EX);

        SysCtlDelay(20000);
    }
    SysCtlDelay(20000*10000);

    // Cutoff ElecM
    for(count=0; count<100; count++){
        GPIOPinWrite(GPIO_PORTA_BASE, GPIO_PIN_2|GPIO_PIN_3|GPIO_PIN_4|GPIO_PIN_5|GPIO_PIN_6, ELEM_OFF);
        SysCtlDelay(20000);
    }
    SysCtlDelay(20000*2000);
    // Finish drop off tools
    for(count=0; count<4000;count++){
        GPIOPinWrite(GPIO_PORTA_BASE, GPIO_PIN_2|GPIO_PIN_3|GPIO_PIN_4|GPIO_PIN_5|GPIO_PIN_6, ELEM_OFF+ACTU_SH);

        SysCtlDelay(20000);

        GPIOPinWrite(GPIO_PORTA_BASE, GPIO_PIN_2|GPIO_PIN_3|GPIO_PIN_4|GPIO_PIN_5|GPIO_PIN_6, ELEM_OFF+ACTU_SH);

        SysCtlDelay(20000);
    }
    PWMOutputState(PWM1_BASE, ACTU_ON, false);
    SysCtlDelay(20000000);

    // Back to source position
    for(count=0; count<900;count++){
        GPIOPinWrite(GPIO_PORTA_BASE, GPIO_PIN_2|GPIO_PIN_3|GPIO_PIN_4|GPIO_PIN_5|GPIO_PIN_6, ELEM_ON+STEP_ON+STEP_R);

        SysCtlDelay(20000);

        GPIOPinWrite(GPIO_PORTA_BASE, GPIO_PIN_2|GPIO_PIN_3|GPIO_PIN_4|GPIO_PIN_5|GPIO_PIN_6, ELEM_ON+STEP_OFF+STEP_R);

        SysCtlDelay(20000);
    }

    // Stop the actuator
    GPIOPinWrite(GPIO_PORTA_BASE, GPIO_PIN_2|GPIO_PIN_3|GPIO_PIN_4|GPIO_PIN_5|GPIO_PIN_6, 0x00);
}

void DrawLine(void)
{
    int count=0;
    // Move to the pick up position
    for(count=0; count<250;count++){
        GPIOPinWrite(GPIO_PORTA_BASE, GPIO_PIN_2|GPIO_PIN_3|GPIO_PIN_4|GPIO_PIN_5|GPIO_PIN_6, STEP_ON+STEP_L);

        SysCtlDelay(20000);

        GPIOPinWrite(GPIO_PORTA_BASE, GPIO_PIN_2|GPIO_PIN_3|GPIO_PIN_4|GPIO_PIN_5|GPIO_PIN_6, STEP_OFF+STEP_L);

        SysCtlDelay(20000);
    }
    SysCtlDelay(20000000);

    // Start pick up
    PWMOutputState(PWM1_BASE, ACTU_ON, true);
    for(count=0; count<50;count++){
        GPIOPinWrite(GPIO_PORTA_BASE, GPIO_PIN_2|GPIO_PIN_3|GPIO_PIN_4|GPIO_PIN_5|GPIO_PIN_6, ELEM_ON+ACTU_EX);

        SysCtlDelay(20000);

        GPIOPinWrite(GPIO_PORTA_BASE, GPIO_PIN_2|GPIO_PIN_3|GPIO_PIN_4|GPIO_PIN_5|GPIO_PIN_6, ELEM_ON+ACTU_EX);

        SysCtlDelay(20000);
    }
    SysCtlDelay(20000*10000);

    // Finish pick then return
    for(count=0; count<4000;count++){
        GPIOPinWrite(GPIO_PORTA_BASE, GPIO_PIN_2|GPIO_PIN_3|GPIO_PIN_4|GPIO_PIN_5|GPIO_PIN_6, ELEM_ON+ACTU_SH);

        SysCtlDelay(20000);

        GPIOPinWrite(GPIO_PORTA_BASE, GPIO_PIN_2|GPIO_PIN_3|GPIO_PIN_4|GPIO_PIN_5|GPIO_PIN_6, ELEM_ON+ACTU_SH);

        SysCtlDelay(20000);
    }
    PWMOutputState(PWM1_BASE, ACTU_ON, false);
    SysCtlDelay(20000000);

    // Move to the spin position
    for(count=0; count<4650;count++){
        GPIOPinWrite(GPIO_PORTA_BASE, GPIO_PIN_2|GPIO_PIN_3|GPIO_PIN_4|GPIO_PIN_5|GPIO_PIN_6, STEP_ON+STEP_L+ELEM_ON);

        SysCtlDelay(20000);

        GPIOPinWrite(GPIO_PORTA_BASE, GPIO_PIN_2|GPIO_PIN_3|GPIO_PIN_4|GPIO_PIN_5|GPIO_PIN_6, STEP_OFF+STEP_L+ELEM_ON);

        SysCtlDelay(20000);
    }
    SysCtlDelay(20000000);

    // Start drop off
    PWMOutputState(PWM1_BASE, ACTU_ON, true);
    for(count=0; count<50;count++){
        GPIOPinWrite(GPIO_PORTA_BASE, GPIO_PIN_2|GPIO_PIN_3|GPIO_PIN_4|GPIO_PIN_5|GPIO_PIN_6, ELEM_ON+ACTU_EX);

        SysCtlDelay(20000);

        GPIOPinWrite(GPIO_PORTA_BASE, GPIO_PIN_2|GPIO_PIN_3|GPIO_PIN_4|GPIO_PIN_5|GPIO_PIN_6, ELEM_ON+ACTU_EX);

        SysCtlDelay(20000);
    }
    SysCtlDelay(20000*10000);

    // Start draw line
    for(count=0; count<2000;count++){
        GPIOPinWrite(GPIO_PORTA_BASE, GPIO_PIN_2|GPIO_PIN_3|GPIO_PIN_4|GPIO_PIN_5|GPIO_PIN_6, STEP_ON+STEP_L+ELEM_ON);

        SysCtlDelay(20000);

        GPIOPinWrite(GPIO_PORTA_BASE, GPIO_PIN_2|GPIO_PIN_3|GPIO_PIN_4|GPIO_PIN_5|GPIO_PIN_6, STEP_OFF+STEP_L+ELEM_ON);

        SysCtlDelay(20000);
    }
    SysCtlDelay(20000000);


    // Finish draw then return
    for(count=0; count<4000;count++){
        GPIOPinWrite(GPIO_PORTA_BASE, GPIO_PIN_2|GPIO_PIN_3|GPIO_PIN_4|GPIO_PIN_5|GPIO_PIN_6, ELEM_ON+ACTU_SH);

        SysCtlDelay(20000);

        GPIOPinWrite(GPIO_PORTA_BASE, GPIO_PIN_2|GPIO_PIN_3|GPIO_PIN_4|GPIO_PIN_5|GPIO_PIN_6, ELEM_ON+ACTU_SH);

        SysCtlDelay(20000);
    }
    PWMOutputState(PWM1_BASE, ACTU_ON, false);
    SysCtlDelay(20000000);

    // Back to pick up position
    for(count=0; count<6650;count++){
        GPIOPinWrite(GPIO_PORTA_BASE, GPIO_PIN_2|GPIO_PIN_3|GPIO_PIN_4|GPIO_PIN_5|GPIO_PIN_6, ELEM_ON+STEP_ON+STEP_R);

        SysCtlDelay(20000);

        GPIOPinWrite(GPIO_PORTA_BASE, GPIO_PIN_2|GPIO_PIN_3|GPIO_PIN_4|GPIO_PIN_5|GPIO_PIN_6, ELEM_ON+STEP_OFF+STEP_R);

        SysCtlDelay(20000);
    }
    // Start drop off tools
    PWMOutputState(PWM1_BASE, ACTU_ON, true);
    for(count=0; count<50;count++){
        GPIOPinWrite(GPIO_PORTA_BASE, GPIO_PIN_2|GPIO_PIN_3|GPIO_PIN_4|GPIO_PIN_5|GPIO_PIN_6, ELEM_ON+ACTU_EX);

        SysCtlDelay(20000);

        GPIOPinWrite(GPIO_PORTA_BASE, GPIO_PIN_2|GPIO_PIN_3|GPIO_PIN_4|GPIO_PIN_5|GPIO_PIN_6, ELEM_ON+ACTU_EX);

        SysCtlDelay(20000);
    }
    SysCtlDelay(20000*10000);

    // Cutoff ElecM
    for(count=0; count<100; count++){
        GPIOPinWrite(GPIO_PORTA_BASE, GPIO_PIN_2|GPIO_PIN_3|GPIO_PIN_4|GPIO_PIN_5|GPIO_PIN_6, ELEM_OFF);
        SysCtlDelay(20000);
    }
    SysCtlDelay(20000*2000);
    // Finish drop off tools
    for(count=0; count<4000;count++){
        GPIOPinWrite(GPIO_PORTA_BASE, GPIO_PIN_2|GPIO_PIN_3|GPIO_PIN_4|GPIO_PIN_5|GPIO_PIN_6, ELEM_OFF+ACTU_SH);

        SysCtlDelay(20000);

        GPIOPinWrite(GPIO_PORTA_BASE, GPIO_PIN_2|GPIO_PIN_3|GPIO_PIN_4|GPIO_PIN_5|GPIO_PIN_6, ELEM_OFF+ACTU_SH);

        SysCtlDelay(20000);
    }
    PWMOutputState(PWM1_BASE, ACTU_ON, false);
    SysCtlDelay(20000000);

    // Back to source position
    for(count=0; count<250;count++){
        GPIOPinWrite(GPIO_PORTA_BASE, GPIO_PIN_2|GPIO_PIN_3|GPIO_PIN_4|GPIO_PIN_5|GPIO_PIN_6, ELEM_ON+STEP_ON+STEP_R);

        SysCtlDelay(20000);

        GPIOPinWrite(GPIO_PORTA_BASE, GPIO_PIN_2|GPIO_PIN_3|GPIO_PIN_4|GPIO_PIN_5|GPIO_PIN_6, ELEM_ON+STEP_OFF+STEP_R);

        SysCtlDelay(20000);
    }

    // Stop the actuator
    GPIOPinWrite(GPIO_PORTA_BASE, GPIO_PIN_2|GPIO_PIN_3|GPIO_PIN_4|GPIO_PIN_5|GPIO_PIN_6, 0x00);
}

void DrawSpiral(void)
{
    int count=0;
    // Move to the pick up position
    for(count=0; count<1800;count++){
        GPIOPinWrite(GPIO_PORTA_BASE, GPIO_PIN_2|GPIO_PIN_3|GPIO_PIN_4|GPIO_PIN_5|GPIO_PIN_6, STEP_ON+STEP_L);

        SysCtlDelay(20000);

        GPIOPinWrite(GPIO_PORTA_BASE, GPIO_PIN_2|GPIO_PIN_3|GPIO_PIN_4|GPIO_PIN_5|GPIO_PIN_6, STEP_OFF+STEP_L);

        SysCtlDelay(20000);
    }
    SysCtlDelay(20000000);

    // Start pick up
    PWMOutputState(PWM1_BASE, ACTU_ON, true);
    for(count=0; count<50;count++){
        GPIOPinWrite(GPIO_PORTA_BASE, GPIO_PIN_2|GPIO_PIN_3|GPIO_PIN_4|GPIO_PIN_5|GPIO_PIN_6, ELEM_ON+ACTU_EX);

        SysCtlDelay(20000);

        GPIOPinWrite(GPIO_PORTA_BASE, GPIO_PIN_2|GPIO_PIN_3|GPIO_PIN_4|GPIO_PIN_5|GPIO_PIN_6, ELEM_ON+ACTU_EX);

        SysCtlDelay(20000);
    }
    SysCtlDelay(20000*10000);

    // Finish pick then return
    for(count=0; count<4000;count++){
        GPIOPinWrite(GPIO_PORTA_BASE, GPIO_PIN_2|GPIO_PIN_3|GPIO_PIN_4|GPIO_PIN_5|GPIO_PIN_6, ELEM_ON+ACTU_SH);

        SysCtlDelay(20000);

        GPIOPinWrite(GPIO_PORTA_BASE, GPIO_PIN_2|GPIO_PIN_3|GPIO_PIN_4|GPIO_PIN_5|GPIO_PIN_6, ELEM_ON+ACTU_SH);

        SysCtlDelay(20000);
    }
    PWMOutputState(PWM1_BASE, ACTU_ON, false);
    SysCtlDelay(20000000);

    // Move to the spin position
    for(count=0; count<1300;count++){
        GPIOPinWrite(GPIO_PORTA_BASE, GPIO_PIN_2|GPIO_PIN_3|GPIO_PIN_4|GPIO_PIN_5|GPIO_PIN_6, STEP_ON+STEP_L+ELEM_ON);

        SysCtlDelay(20000);

        GPIOPinWrite(GPIO_PORTA_BASE, GPIO_PIN_2|GPIO_PIN_3|GPIO_PIN_4|GPIO_PIN_5|GPIO_PIN_6, STEP_OFF+STEP_L+ELEM_ON);

        SysCtlDelay(20000);
    }
    SysCtlDelay(20000000);

    // Start drop off
    PWMOutputState(PWM1_BASE, ACTU_ON, true);
    for(count=0; count<50;count++){
        GPIOPinWrite(GPIO_PORTA_BASE, GPIO_PIN_2|GPIO_PIN_3|GPIO_PIN_4|GPIO_PIN_5|GPIO_PIN_6, ELEM_ON+ACTU_EX);

        SysCtlDelay(20000);

        GPIOPinWrite(GPIO_PORTA_BASE, GPIO_PIN_2|GPIO_PIN_3|GPIO_PIN_4|GPIO_PIN_5|GPIO_PIN_6, ELEM_ON+ACTU_EX);

        SysCtlDelay(20000);
    }
    SysCtlDelay(20000*10000);

    // Start draw line and spin the pan
    PWMOutputState(PWM1_BASE, DCM_ON, true);
    for(count=0; count<2500;count++){
            GPIOPinWrite(GPIO_PORTA_BASE, GPIO_PIN_2|GPIO_PIN_3|GPIO_PIN_4|GPIO_PIN_5|GPIO_PIN_6, STEP_ON+STEP_L+ELEM_ON);

            SysCtlDelay(20000);

            GPIOPinWrite(GPIO_PORTA_BASE, GPIO_PIN_2|GPIO_PIN_3|GPIO_PIN_4|GPIO_PIN_5|GPIO_PIN_6, STEP_OFF+STEP_L+ELEM_ON);

            SysCtlDelay(20000);
    }
    PWMOutputState(PWM1_BASE, DCM_ON, false);
    SysCtlDelay(20000*4000);


    // Finish draw then return
    for(count=0; count<4000;count++){
        GPIOPinWrite(GPIO_PORTA_BASE, GPIO_PIN_2|GPIO_PIN_3|GPIO_PIN_4|GPIO_PIN_5|GPIO_PIN_6, ELEM_ON+ACTU_SH);

        SysCtlDelay(20000);

        GPIOPinWrite(GPIO_PORTA_BASE, GPIO_PIN_2|GPIO_PIN_3|GPIO_PIN_4|GPIO_PIN_5|GPIO_PIN_6, ELEM_ON+ACTU_SH);

        SysCtlDelay(20000);
    }
    PWMOutputState(PWM1_BASE, ACTU_ON, false);
    SysCtlDelay(20000000);

    // Back to pick up position
    for(count=0; count<3800;count++){
        GPIOPinWrite(GPIO_PORTA_BASE, GPIO_PIN_2|GPIO_PIN_3|GPIO_PIN_4|GPIO_PIN_5|GPIO_PIN_6, ELEM_ON+STEP_ON+STEP_R);

        SysCtlDelay(20000);

        GPIOPinWrite(GPIO_PORTA_BASE, GPIO_PIN_2|GPIO_PIN_3|GPIO_PIN_4|GPIO_PIN_5|GPIO_PIN_6, ELEM_ON+STEP_OFF+STEP_R);

        SysCtlDelay(20000);
    }
    // Start drop off tools
    PWMOutputState(PWM1_BASE, ACTU_ON, true);
    for(count=0; count<50;count++){
        GPIOPinWrite(GPIO_PORTA_BASE, GPIO_PIN_2|GPIO_PIN_3|GPIO_PIN_4|GPIO_PIN_5|GPIO_PIN_6, ELEM_ON+ACTU_EX);

        SysCtlDelay(20000);

        GPIOPinWrite(GPIO_PORTA_BASE, GPIO_PIN_2|GPIO_PIN_3|GPIO_PIN_4|GPIO_PIN_5|GPIO_PIN_6, ELEM_ON+ACTU_EX);

        SysCtlDelay(20000);
    }
    SysCtlDelay(20000*10000);

    // Cutoff ElecM
    for(count=0; count<100; count++){
        GPIOPinWrite(GPIO_PORTA_BASE, GPIO_PIN_2|GPIO_PIN_3|GPIO_PIN_4|GPIO_PIN_5|GPIO_PIN_6, ELEM_OFF);
        SysCtlDelay(20000);
    }
    SysCtlDelay(20000*2000);
    // Finish drop off tools
    for(count=0; count<4000;count++){
        GPIOPinWrite(GPIO_PORTA_BASE, GPIO_PIN_2|GPIO_PIN_3|GPIO_PIN_4|GPIO_PIN_5|GPIO_PIN_6, ELEM_OFF+ACTU_SH);

        SysCtlDelay(20000);

        GPIOPinWrite(GPIO_PORTA_BASE, GPIO_PIN_2|GPIO_PIN_3|GPIO_PIN_4|GPIO_PIN_5|GPIO_PIN_6, ELEM_OFF+ACTU_SH);

        SysCtlDelay(20000);
    }
    PWMOutputState(PWM1_BASE, ACTU_ON, false);
    SysCtlDelay(20000000);

    // Back to source position
    for(count=0; count<1800;count++){
        GPIOPinWrite(GPIO_PORTA_BASE, GPIO_PIN_2|GPIO_PIN_3|GPIO_PIN_4|GPIO_PIN_5|GPIO_PIN_6, ELEM_ON+STEP_ON+STEP_R);

        SysCtlDelay(20000);

        GPIOPinWrite(GPIO_PORTA_BASE, GPIO_PIN_2|GPIO_PIN_3|GPIO_PIN_4|GPIO_PIN_5|GPIO_PIN_6, ELEM_ON+STEP_OFF+STEP_R);

        SysCtlDelay(20000);
    }

    // Stop the actuator
    GPIOPinWrite(GPIO_PORTA_BASE, GPIO_PIN_2|GPIO_PIN_3|GPIO_PIN_4|GPIO_PIN_5|GPIO_PIN_6, 0x00);
}

