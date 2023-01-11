/* Libraries */

#include <msp430.h> 
#include <msp430fr2355.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <msp430.h>

/* Variables */


float add=0.0;
float aumento=.8; //Variable que dicta el aumento en el registro de comparación para aumentar el PWM. .5
float Tcarga = 0.0;
float Naumentos = 0.0;
float fijo;

int bandera=0;
float fPWM = 0.0;
int fAdd=0;
int DC=0;


/* Main */

void main(void)
{
    WDTCTL = WDTPW | WDTHOLD;                 // Stop WDT

    P2DIR |= BIT0;                     // P2.0 as OUTPUT for PWM on TIMER B1
    P2SEL1 &= ~BIT0;
    P2SEL0 |= BIT0;

    P2REN=0; //Este registro indica habilitacion de resistencia pullup y pulldown (no indica cual tipo, solo habilita la config)
    P2REN|=BIT3;
    P2OUT|=BIT3; //P2.3 (BOTON) tiene pull up

    P1DIR |= BIT0;                                // P1.0 output Just to corroborate that the interruption is working
    P1OUT |= BIT0;                                // P1.0 high

    P1DIR |= BIT6;                     // P1.6 and P1.7 output
    P1SEL1 |= BIT6;                   // P1.6 and P1.7 options select


    // Disable the GPIO power-on default high-impedance mode to activate
    // previously configured port settings
    PM5CTL0 &= ~LOCKLPM5;

    //Timer B1 for PWM
    fPWM = 50000; //en Hz
    fPWM = 1/fPWM;
    fPWM *= 1000000; //Queda en uS

    DC = fPWM*(0.88); //

    TB0CCR0 = fPWM-1;                           // PWM Period 50kHz
    TB0CCTL1 = OUTMOD_7;                      // CCR1 reset/set
    TB0CCTL0 |= CCIE; //interrupt ENABLEn 
    TB0CCR1 = DC;
    TB0CTL = TBSSEL__SMCLK | MC__UP | TBCLR;  // SMCLK, up mode, clear TBR

    Tcarga = 1800; // Parametro en mS
    Tcarga = Tcarga/10;
    Naumentos = Tcarga/100; //Obtenemos el numero de aumentos necesario para que tarde Tcarga
    Naumentos *= 1000;
    aumento = DC/Naumentos; //Obtenemos el valor de aumento para cada ciclo
    fijo=aumento;


    __bis_SR_register(LPM0_bits | GIE);          // Enter LPM0 w/ interrupt
    __no_operation();                         // For debugger

}

// Timer0_B0 interrupt service routine

#if defined(__TI_COMPILER_VERSION__) || defined(__IAR_SYSTEMS_ICC__)
#pragma vector = TIMER0_B0_VECTOR
__interrupt void Timer0_B0_ISR (void)
#elif defined(__GNUC__)
void __attribute__ ((interrupt(TIMER0_B0_VECTOR))) Timer0_B0_ISR (void)
#else
#error Compiler not supported!
#endif
{
    //P1OUT ^= BIT0;
    //TB0CCTL0 &= ~CCIFG; //Limpiar bandera de interrupcion

  if((P2IN&BIT3)==0){
       bandera=1;
       P1OUT ^= BIT0;
       _delay_cycles(1000000);

   }


  if (bandera==1){
      add += aumento;
      TB0CCR1 = fijo + add;

      //Bandera del boton, si se presiona el boton que la bandera se encienda y que se apague hasta que se llene
      if (add >= (DC-fijo)){
          TB0CCR1=0;
          add = 0;
          bandera=0;

      }

    }
 }
