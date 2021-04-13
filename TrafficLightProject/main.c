#include <stdint.h>
#include <stdbool.h>

#include <stdio.h>
#include <stdlib.h>
#include "inc/tm4c123gh6pm.h"
#include "inc/hw_memmap.h"
#include "inc/hw_uart.h"
#include "driverlib/fpu.h"
#include "driverlib/pin_map.h"
#include "inc/hw_types.h"
#include "driverlib/sysctl.h"
#include "driverlib/interrupt.h"
#include "driverlib/gpio.h"
#include "driverlib/timer.h"
#include "driverlib/uart.h"



void UART0Transmit(char c);
void SwitchPressed(void);
void PauseTrafficLight(void);
void ResumeTrafficLight(void);



//UART Sentences Inits
char *CarsNS="Cars NORTH SOUTH";
char *CarsEW="Cars EAST WEST";
char *PedestriansNS="Pedestrians NORTH SOUTH";
char *PedestriansEW="Pedestrians EAST WEST";
size_t i = 0;

uint32_t hold0;
uint32_t hold3;
uint32_t PortAhold;




int main(void)
{
    //timer value to hold for pauses


//40hz clock setting
    SysCtlClockSet( SYSCTL_SYSDIV_5 | SYSCTL_USE_PLL | SYSCTL_XTAL_16MHZ | SYSCTL_OSC_MAIN );
/*Peripherals Enabling*/

    SysCtlPeripheralEnable( SYSCTL_PERIPH_GPIOF );
    SysCtlPeripheralEnable( SYSCTL_PERIPH_GPIOA );
    SysCtlPeripheralEnable( SYSCTL_PERIPH_UART0 );

    SysCtlPeripheralEnable( SYSCTL_PERIPH_GPIOC );
    SysCtlPeripheralEnable( SYSCTL_PERIPH_TIMER0 );
    SysCtlPeripheralEnable( SYSCTL_PERIPH_TIMER1 );
    SysCtlPeripheralEnable( SYSCTL_PERIPH_TIMER2 );
    SysCtlPeripheralEnable( SYSCTL_PERIPH_TIMER3 );
    SysCtlPeripheralEnable( SYSCTL_PERIPH_TIMER4 );
    SysCtlPeripheralEnable( SYSCTL_PERIPH_TIMER5 );
    SysCtlPeripheralEnable( SYSCTL_PERIPH_WTIMER0 );
    SysCtlPeripheralEnable( SYSCTL_PERIPH_WTIMER1 );
    SysCtlPeripheralEnable( SYSCTL_PERIPH_WTIMER2 );





//Configure GPIO Outputs and Inputs
    GPIO_PORTF_LOCK_R = 0x4C4F434B;
    GPIO_PORTF_CR_R = 0x01;
    GPIOPinTypeGPIOInput(GPIO_PORTF_BASE, GPIO_PIN_0 | GPIO_PIN_4);

    GPIOPinTypeGPIOOutput( GPIO_PORTC_BASE, GPIO_PIN_4 | GPIO_PIN_5 | GPIO_PIN_6| GPIO_PIN_7);
    GPIOPinTypeGPIOOutput( GPIO_PORTF_BASE, GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3 );
    GPIOPinTypeGPIOOutput( GPIO_PORTA_BASE,  GPIO_PIN_2 | GPIO_PIN_3 | GPIO_PIN_4| GPIO_PIN_5 | GPIO_PIN_6 | GPIO_PIN_7 );
    GPIOPinConfigure(GPIO_PA0_U0RX);
    GPIOPinConfigure(GPIO_PA1_U0TX);

//Enable Interrupts Globally
    IntMasterEnable();

//Init for Switches Interrupts

    GPIOPadConfigSet(GPIO_PORTF_BASE ,GPIO_PIN_4,GPIO_STRENGTH_2MA,GPIO_PIN_TYPE_STD_WPU);
    GPIOPadConfigSet(GPIO_PORTF_BASE ,GPIO_PIN_0,GPIO_STRENGTH_2MA,GPIO_PIN_TYPE_STD_WPU);

    GPIOIntTypeSet(GPIO_PORTF_BASE,GPIO_PIN_4,GPIO_FALLING_EDGE);
    GPIOIntTypeSet(GPIO_PORTF_BASE,GPIO_PIN_0,GPIO_FALLING_EDGE);

    GPIOIntRegister(GPIO_PORTF_BASE,SwitchPressed);
    IntRegister(INT_GPIOF,SwitchPressed);


    GPIOIntEnable(GPIO_PORTF_BASE, GPIO_INT_PIN_4);
    GPIOIntEnable(GPIO_PORTF_BASE, GPIO_INT_PIN_0);

    GPIOIntEnable(GPIO_PORTF_BASE, GPIO_PIN_0 );
    GPIOIntEnable(GPIO_PORTF_BASE, GPIO_PIN_4 );
    // Enable interrupt for PF4
    IntEnable(INT_GPIOF);


//UART Initialization


     /*Init UART0 */
    UART0_CTL_R = 0; /* disable  */
    UART0_IBRD_R = 260;
    UART0_FBRD_R = 27;
    UART0_CC_R = 0;
    UART0_LCRH_R = 0x60;
    UART0_CTL_R = 0x301; /* enable for UART0, TXE, RXE */


    GPIO_PORTA_DEN_R |= 0x03;
    GPIO_PORTA_AFSEL_R |= 0x03;
    GPIO_PORTA_PCTL_R |= 0x11;



//Start Traffic First then generate interrupt
    GPIOPinWrite(GPIO_PORTA_BASE, GPIO_PIN_2,GPIO_PIN_2);
    GPIOPinWrite(GPIO_PORTA_BASE, GPIO_PIN_5,GPIO_PIN_5);
    GPIOPinWrite(GPIO_PORTC_BASE, GPIO_PIN_6,GPIO_PIN_6);
    GPIOPinWrite(GPIO_PORTC_BASE, GPIO_PIN_5,GPIO_PIN_5);



    size_t length = strlen(CarsNS);
    for (i=0; i < length; i++) {
        UART0Transmit(CarsNS[i]);
    }
    UART0Transmit('\n');


//Timer Configurations
    TimerConfigure( TIMER0_BASE, TIMER_CFG_ONE_SHOT);
    TimerConfigure( TIMER1_BASE, TIMER_CFG_ONE_SHOT);
    TimerConfigure( TIMER2_BASE, TIMER_CFG_ONE_SHOT);
    TimerConfigure( TIMER3_BASE, TIMER_CFG_ONE_SHOT);
    TimerConfigure( TIMER4_BASE, TIMER_CFG_ONE_SHOT);
    TimerConfigure( TIMER5_BASE, TIMER_CFG_ONE_SHOT);
    TimerConfigure( WTIMER0_BASE, TIMER_CFG_ONE_SHOT);
    TimerConfigure( WTIMER1_BASE, TIMER_CFG_ONE_SHOT);
    TimerConfigure( WTIMER2_BASE, TIMER_CFG_ONE_SHOT);




//First timer init//

    TimerLoadSet( TIMER0_BASE, TIMER_A, SysCtlClockGet()*5);
    IntEnable( INT_TIMER0A );
    TimerIntEnable( TIMER0_BASE, TIMER_TIMA_TIMEOUT );
    TimerEnable( TIMER0_BASE, TIMER_A );




}

void Timer0IntHandler(void)
{
    TimerIntClear( TIMER0_BASE, TIMER_TIMA_TIMEOUT );
    GPIOPinWrite(GPIO_PORTA_BASE, GPIO_PIN_2,0);
    GPIOPinWrite(GPIO_PORTA_BASE, GPIO_PIN_3,GPIO_PIN_3);
    TimerLoadSet( TIMER1_BASE, TIMER_A, SysCtlClockGet()*2);
    IntEnable( INT_TIMER1A );
    TimerIntEnable( TIMER1_BASE, TIMER_TIMA_TIMEOUT );
    TimerEnable( TIMER1_BASE, TIMER_A );



}

void Timer1IntHandler(void)
{

    TimerIntClear( TIMER1_BASE, TIMER_TIMA_TIMEOUT );
    GPIOPinWrite(GPIO_PORTA_BASE, GPIO_PIN_3,0);
    GPIOPinWrite(GPIO_PORTA_BASE, GPIO_PIN_4,GPIO_PIN_4);
    TimerLoadSet( TIMER2_BASE, TIMER_A, SysCtlClockGet()*1);
    IntEnable( INT_TIMER2A );
    TimerIntEnable( TIMER2_BASE, TIMER_TIMA_TIMEOUT );
    TimerEnable( TIMER2_BASE, TIMER_A );

}
void Timer2IntHandler(void)
{

    TimerIntClear( TIMER2_BASE, TIMER_TIMA_TIMEOUT );
    GPIOPinWrite(GPIO_PORTA_BASE, GPIO_PIN_5,0);
    GPIOPinWrite(GPIO_PORTA_BASE, GPIO_PIN_7,GPIO_PIN_7);

    size_t length = strlen(CarsEW);
        for (i=0; i < length; i++) {
            UART0Transmit(CarsEW[i]);

        }
    UART0Transmit('\n');


    TimerLoadSet( TIMER3_BASE, TIMER_A, SysCtlClockGet()*5);
    IntEnable( INT_TIMER3A );
    TimerIntEnable( TIMER3_BASE, TIMER_TIMA_TIMEOUT );
    TimerEnable( TIMER3_BASE, TIMER_A );

}
void Timer3IntHandler(void)
{

    TimerIntClear( TIMER3_BASE, TIMER_TIMA_TIMEOUT );
    GPIOPinWrite(GPIO_PORTA_BASE, GPIO_PIN_7,0);
    GPIOPinWrite(GPIO_PORTA_BASE, GPIO_PIN_6,GPIO_PIN_6);

    TimerLoadSet( TIMER4_BASE, TIMER_A, SysCtlClockGet()*2);
    IntEnable( INT_TIMER4A );
    TimerIntEnable( TIMER4_BASE, TIMER_TIMA_TIMEOUT );
    TimerEnable( TIMER4_BASE, TIMER_A );

}
void Timer4IntHandler(void)
{

    TimerIntClear( TIMER4_BASE, TIMER_TIMA_TIMEOUT );
    GPIOPinWrite(GPIO_PORTA_BASE, GPIO_PIN_6,0);
    GPIOPinWrite(GPIO_PORTA_BASE, GPIO_PIN_5,GPIO_PIN_5);
    TimerLoadSet( TIMER5_BASE, TIMER_A, SysCtlClockGet()*1);
    IntEnable( INT_TIMER5A );
    TimerIntEnable( TIMER5_BASE, TIMER_TIMA_TIMEOUT );
    TimerEnable( TIMER5_BASE, TIMER_A );

}
void Timer5IntHandler(void)
{

    TimerIntClear( TIMER5_BASE, TIMER_TIMA_TIMEOUT );
    GPIOPinWrite(GPIO_PORTA_BASE, GPIO_PIN_2,GPIO_PIN_2);
    GPIOPinWrite(GPIO_PORTA_BASE, GPIO_PIN_4,0);
    size_t length = strlen(CarsNS);
        for (i=0; i < length; i++) {
            UART0Transmit(CarsNS[i]);

        }
    UART0Transmit('\n');
    TimerLoadSet( TIMER0_BASE, TIMER_A, SysCtlClockGet()*5);
    IntEnable( INT_TIMER0A );
    TimerIntEnable( TIMER0_BASE, TIMER_TIMA_TIMEOUT );
    TimerEnable( TIMER0_BASE, TIMER_A );

}

void Timer6IntHandler(void)
{
    TimerIntClear( WTIMER0_BASE, TIMER_TIMA_TIMEOUT );
    SysCtlPeripheralEnable( SYSCTL_PERIPH_GPIOF );

}
void Timer7IntHandler(void){
    TimerIntClear( WTIMER1_BASE, TIMER_TIMA_TIMEOUT );
    GPIOPinWrite(GPIO_PORTC_BASE, GPIO_PIN_4, 0);
    GPIOPinWrite(GPIO_PORTC_BASE, GPIO_PIN_5,GPIO_PIN_5);
    GPIO_PORTA_DATA_R=PortAhold;
    ResumeTrafficLight();
    TimerLoadSet( WTIMER0_BASE, TIMER_A, SysCtlClockGet()*1);
    IntEnable( INT_WTIMER0A );
    TimerIntEnable( WTIMER0_BASE, TIMER_TIMA_TIMEOUT );
    TimerEnable( WTIMER0_BASE, TIMER_A );
}
void Timer8IntHandler(void){
    TimerIntClear( WTIMER2_BASE, TIMER_TIMA_TIMEOUT );
    GPIOPinWrite(GPIO_PORTC_BASE, GPIO_PIN_7, 0);
    GPIOPinWrite(GPIO_PORTC_BASE, GPIO_PIN_6,GPIO_PIN_6);
    GPIO_PORTA_DATA_R=PortAhold;

    ResumeTrafficLight();
    TimerLoadSet( WTIMER0_BASE, TIMER_A, SysCtlClockGet()*1);
    IntEnable( INT_WTIMER0A );
    TimerIntEnable( WTIMER0_BASE, TIMER_TIMA_TIMEOUT );
    TimerEnable( WTIMER0_BASE, TIMER_A );
}





void SwitchPressed(void){
        uint32_t status=0;
        status = GPIOIntStatus(GPIO_PORTF_BASE,true);
        if (status & GPIO_PIN_4){
            GPIOIntClear(GPIO_PORTF_BASE,status);
            SysCtlPeripheralDisable( SYSCTL_PERIPH_GPIOF );
//            uint32_t hold0=TimerValueGet(TIMER0_BASE, TIMER_A);
            PauseTrafficLight();
            GPIOPinWrite(GPIO_PORTC_BASE, GPIO_PIN_5,0);
            GPIOPinWrite(GPIO_PORTC_BASE, GPIO_PIN_4, GPIO_PIN_4);
            size_t length = strlen(PedestriansEW);
            for (i=0; i < length; i++) {
                UART0Transmit(PedestriansEW[i]);
            }
            UART0Transmit('\n');
            PortAhold=GPIO_PORTA_DATA_R;
            GPIO_PORTA_DATA_R=0;
            TimerLoadSet( WTIMER1_BASE, TIMER_A, SysCtlClockGet()*2);
            IntEnable( INT_WTIMER1A );
            TimerIntEnable( WTIMER1_BASE, TIMER_TIMA_TIMEOUT );
            TimerEnable( WTIMER1_BASE, TIMER_A );
        }
        else if (status & GPIO_PIN_0){
            GPIOIntClear(GPIO_PORTF_BASE,status);
            SysCtlPeripheralDisable( SYSCTL_PERIPH_GPIOF );
            PauseTrafficLight();
            GPIOPinWrite(GPIO_PORTC_BASE, GPIO_PIN_6,0);
            GPIOPinWrite(GPIO_PORTC_BASE, GPIO_PIN_7, GPIO_PIN_7);


            size_t length = strlen(PedestriansNS);
            for (i=0; i < length; i++) {
                UART0Transmit(PedestriansNS[i]);
            }
            UART0Transmit('\n');
            PortAhold=GPIO_PORTA_DATA_R;
            GPIO_PORTA_DATA_R=0;
            TimerLoadSet( WTIMER2_BASE, TIMER_A, SysCtlClockGet()*2);
            IntEnable( INT_WTIMER2A );
            TimerIntEnable( WTIMER2_BASE, TIMER_TIMA_TIMEOUT );
            TimerEnable( WTIMER2_BASE, TIMER_A );

        }


}





void UART0Transmit(char c)
{
 while((UART0_FR_R & 0x20) != 0); /* wait until Tx buffer not full */
UART0_DR_R = c; /* before giving it another byte */
}


void PauseTrafficLight(void){
    if( GPIOPinRead(GPIO_PORTA_BASE, GPIO_PIN_2) == GPIO_PIN_2 ){
        //hold timer0a value
         hold0=TimerValueGet(TIMER0_BASE, TIMER_A);
    }
    else if( GPIOPinRead(GPIO_PORTA_BASE, GPIO_PIN_7 ) == GPIO_PIN_7){
        //hold timer3a value
         hold3=TimerValueGet(TIMER0_BASE, TIMER_A);
    }
}
void ResumeTrafficLight(){
    if( GPIOPinRead(GPIO_PORTA_BASE, GPIO_PIN_2)==  GPIO_PIN_2 ){

        TimerLoadSet( TIMER0_BASE, TIMER_A, hold0);
        IntEnable( INT_TIMER0A );
        TimerIntEnable( TIMER0_BASE, TIMER_TIMA_TIMEOUT );

        TimerEnable( TIMER0_BASE, TIMER_A );
}

    else if( GPIOPinRead(GPIO_PORTA_BASE, GPIO_PIN_7 )== GPIO_PIN_7){

        TimerLoadSet( TIMER3_BASE, TIMER_A, hold3);
        IntEnable( INT_TIMER3A );
        TimerIntEnable( TIMER3_BASE, TIMER_TIMA_TIMEOUT );

        TimerEnable( TIMER3_BASE, TIMER_A );
}
}



