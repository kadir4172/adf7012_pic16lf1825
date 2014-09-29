/* 
 * File:   main.c
 * Author: Kadir
 *
 * Created on September 15, 2014, 6:55 PM
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "configuration.h"
#include <stdbool.h>
#include "utility.h"
#include <string.h>
#include <math.h>
#include "ax25.h"
#include "audio_tone.h"

//#define Debug_Modem_Packet                 //uncomment this line to debug the data encoded with AX25

#define BATTERY_MEAS_EVERY_MILISECOND 100000 //batarya olcme periyodu [ms]

extern  bool PTT_OFF;                        // PTT_OFF flag, not to pull Ptt_Off function within an interrupt

extern void Sinus_Generator(void);           //function which generates audio signal
extern bool MODEM_TRANSMITTING;              //flag to check whether modem_packet is fully transmitted

uint8_t adc__high = 0;                       //Adc return values
uint8_t adc__low  = 0;

uint32_t Systick_Counter = 0;                //Counter with period of 833 us
bool Change_to_New_Baud = false;             //Flag to check for changing to new baud

#ifdef Debug_Modem_Packet
extern uint8_t modem_packet[MODEM_MAX_PACKET];
uint8_t k = 0;
#endif

/*
 * 
 */
void interrupt global_interrupt(){          //single interrupt vector to handle all of ISR's
    
    INTCON &= ~0x80;                        //Global interrupt disable in ISR

    //ADC interrupt
    if(ADIF){
        //ADC1 ISR
        adc__high = ADRESH;
        adc__low  = ADRESL;

        ADIF = 0;
        //ADC1 ISR
        return;
    }
    //ADC interrupt
    

    //Timer1 interrupt
    if(PIR1 & 0x04){
       //Timer1 ISR
        Change_to_New_Baud = true;         //Change to new baud in Sinus_Generator()
       

       //reset Timer1 registers
       TMR1H = 0x00;
       TMR1L = 0x00;
       
       PIR1 &= ~0x04; //Clear Timer1 interrupt flag

       Systick_Counter += 1;
       if(Systick_Counter > BATTERY_MEAS_EVERY_MILISECOND){
            Systick_Counter = 0;
            ADCON0 |= 0b00000010;         //If period overruns for battery reading start the ADC conversion
        }
       //Timer1 ISR
       return;
    }
    //Timer1 interrupt

    //Timer0 interrupt
    if(INTCON & 0x04){
       //Timer0 ISR
       Sinus_Generator();                //Call Sinus_Generator() with Playback_Rate
        
       INTCON &= ~0x04;                  //Clear Timer0 interrupt flag
        //Timer0 ISR
       return;
    }
    //Timer0 interrupt
 


    INTCON |= 0x80;                       //Global interrupt enabled again
}

void System_Start(void){

    //Watchdog timer configuration for 128 seconds
      WDTPS4 = 1;
      WDTPS3 = 0;
      WDTPS2 = 0;
      WDTPS1 = 0;
      WDTPS0 = 1;
    //Watchdog timer configuration for 128 seconds

    //Internal RC osc with 4xPLL operating at 32MHz
      OSCCON  = 0x00;
      OSCCON |= 0b11110000;
      OSCTUNE = 0x00;
    //Internal RC osc with 4xPLL operating at 32MHz
    
    //Configurations for Timer0
      TMR0CS = 0;                 //Internal clock source (Fosc/4)
      PSA    = 1;                    //Do not use Prescaler
    //Configurations for Timer0

    //Configurations for Timer1
      TMR1ON = 1;                //Timer1 always count
      TMR1GE = 0;

      TMR1CS1 = 0;               //Fosc/4
      TMR1CS0 = 0;

      T1CKPS1 = 1;               //1/8 prescaler
      T1CKPS0 = 1;

      CCP1M3 = 1;               //Software interrupt on compare event
      CCP1M2 = 0;
      CCP1M1 = 1;
      CCP1M1 = 0;
    //Configurations for Timer1
      
    //Configurations for Dac0
      DACOE   = 1;
      DACPSS1 = 0;
      DACPSS0 = 0;
      DACNSS  = 0;
    //Configurations for Dac0

   
    //Configurations for Adc1
      ANSA1   = 1;                //RA1 analog input
      ADCON0 &= 0b10000011;
      ADCON0 |= 0b00000100;     //AN1 channel select
      ADNREF  = 0;              //Vref- = GND
      ADPREF1 = 0;
      ADPREF0 = 0;              //Vref+ = Vdd
      ADCS2   = 1;
      ADCS1   = 1;
      ADCS0   = 0;               //Fosc/64 for conversion clock
      ADFM    = 1;                 //Output on right hand side
    //Configurations for Adc1


    //Reset Interrupt Flags
      TMR0IF = 0;
      TMR1IF = 0;
      CCP1IF = 0;
      ADIF   = 0;
    //Reset Interrupt Flags

    //Global Interrupt ve Peripheral Interrupt Enable
      INTCON |= 0xC0;
}


int main(void) {
    System_Start();
    while (!(OSCSTAT & (0x01))){}    //Wait for HFIOFS Osc. stable bit
    /* TODO check the timeout somehow, CPU clock is not stable, implement a dummy counter or WDT will handle this */

    Gpio_Config();                   //Gpio configuration
 
    Timer1_Start();                  //Timer1 with 833 us period

    Dac0_Start_Hold();               //Start Dac output and make the output Vdd/2
    
    Adc1_Start();                    //Just configure Adc1 peripheral, conversion will be started within Timer1 ISR

    ADF7021_CHIP_POWER_DOWN;         //CE pin low , to reset the configuration on ADF7012
    Delay_ms(10);
    ADF7021_LOAD_REGISTER_DISABLE;   //LE pin high, to disable loading registers
    Delay_ms(10);
    ADF7021_CHIP_POWER_UP;           //CE pin high, to enable ADF7012
    Delay_ms(10);
   
    Delay_ms(200);


    s_address beacon_address[2] = {{"CUBEYY", 5},{"CUBEXX", 6}};  //Source and destination adresses with callsigns

    Ax25_Send_Header(beacon_address,2);                           //Header with 2 adresses
    Ax25_Send_String("HELLO");                                    //Send string
    Ax25_Send_Footer();                                           //Send Footer



    Modem_Setup();                                               //Set modem reset configurations to ADF7012
    Delay_ms(100);
    Adf_Lock();                                                  //Try to achieve a good PLL lock, otherwise use default vco configuration
    Delay_ms(100);
/*
    try_to_push_button:
    if(!Ptt_On()){     //means power is bad
        Modem_Setup(); //try to reconfigure adf7012
        Delay_ms(200);
        goto try_to_push_button;
    }
*/
     Ptt_On(); /* TODO remove this line when working on real hardware */
     
     
     while(1){
          if(PTT_OFF){
		  Ptt_Off();                                    //Turn of PTT
		  PTT_OFF  = false;
	  }


	  Modem_Flush_Frame();                                 //Transmit modem_packet[]
          while(MODEM_TRANSMITTING);
	  Delay_ms(2000);

    

#ifdef Debug_Modem_Packet
        
          for (k=0; k< MODEM_MAX_PACKET; k++){
          Spi_Byte_Send(modem_packet[k]);
          }
          Delay_ms(3000);
#endif

          CLRWDT();
         }
    return (EXIT_SUCCESS);
}

// -----------------------------------------------------------------------


