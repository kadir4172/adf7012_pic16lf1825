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

//#define Debug_Modem_Packet

extern  bool PTT_OFF;
extern void Ptt_Off(void);
extern uint8_t modem_packet[MODEM_MAX_PACKET];
void Sinus_Generator(void);
extern bool MODEM_TRANSMITTING;

uint8_t test  = 0;
uint8_t test2 = 0;
uint16_t dac_flag = 0;




uint8_t adc_sonuc_high = 0;
uint8_t adc_sonuc_low  = 0;

uint16_t Systick_Counter = 0; //Her 833us de bir tick sayar

#ifdef Debug_Modem_Packet
extern uint8_t modem_packet[MODEM_MAX_PACKET];
uint8_t k = 0;
#endif

/*
 * 
 */
void interrupt global_interrupt(){
    
    INTCON &= ~0x80; //Global interrupt disable in ISR

    //ADC1 interrupt
    if(ADIF){
        adc_sonuc_high = ADRESH;
        adc_sonuc_low  = ADRESL;

        ADIF = 0;
        return;
    }
    //ADC1 interrupt

    //Timer1 interrupt
    if(PIR1 & 0x04){
       //Timer1 ISR
        Systick_Counter += 1;
       //Timer1 ISR

       //reset Timer1 registers
       TMR1H = 0x00;
       TMR1L = 0x00;
       
       PIR1 &= ~0x04; //Clear Timer1 interrupt flag

       //ADCON0 |= 0b00000010; //ADC Go
       return;
    }
    //Timer1 interrupt

    //Timer0 interrupt
    if(INTCON & 0x04){
       //Timer0 ISR
       Sinus_Generator();
       //Timer0 ISR
  
       INTCON &= ~0x04; //Clear Timer0 interrupt flag
       return;
    }

 


    INTCON |= 0x80; //Global interrupt enabled again
}

void System_Start(void){
    
    //Internal RC osc with 4xPLL operating at 32MHz
    OSCCON  = 0x00;
    OSCCON |= 0b11110000;
    OSCTUNE = 0x00;

    
    //Timer0 icin gerekli konfigurasyonlar
    TMR0CS = 0; //Internal clock source (Fosc/4)
    PSA = 1;    // Prescaler kullanmiyoruz
    //Timer0 icin gerekli konfigurasyonlar

    //Timer1 icin gerekli konfigurasyonlar
    //Timer1 always count
    TMR1ON = 1;
    TMR1GE = 0;

    //Fosc/4
    TMR1CS1 = 0;
    TMR1CS0 = 0;

    // 1/8 prescaler
    T1CKPS1 = 1;
    T1CKPS0 = 1;

    //software interrupt on compare event
    CCP1M3 = 1;
    CCP1M2 = 0;
    CCP1M1 = 1;
    CCP1M1 = 0;
    //Timer1 icin gerekli konfigurasyonlar


    //Dac0 icin gerekli konfigurasyonlar
    DACOE = 1;
    DACPSS1 = 0;
    DACPSS0 = 0;
    DACNSS = 0;
    //Dac0 icin gerekli konfigurasyonlar

   
    //ADC1 icin gerekli konfigurasyonlar
    ANSA1 = 1; //RA1 analog input
    ADCON0 &= 0b10000011;
    ADCON0 |= 0b00000100; //AN1 channel select
    ADNREF  = 0; //Vref- = GND
    ADPREF1 = 0;
    ADPREF0 = 0; //Vref+ = Vdd
    ADCS2  = 1;
    ADCS1  = 1;
    ADCS0  = 0;  //Fosc/64 for conversion clock
    ADFM = 1; // output on right hand side
    //ADC1 icin gerekli konfigurasyonlar


    //reset interrupt flags
    TMR0IF = 0;
    TMR1IF = 0;
    CCP1IF = 0;
    ADIF = 0; //clear interrupt flag
    //reset interrupt flags

    //Global Interrupt ve Peripheral Interrupt Enable
    INTCON |= 0xC0;

  
    
}


int main(void) {
    unsigned int adc_sonuc;
    System_Start();
    while (!(OSCSTAT & (0x01))){} //HFIOFS Osc. stable bit bekle
    /* TODO timeout a bakip software reset verilmeli */

    Gpio_Config();
 
    Timer1_Start(); //833us lik Timer1 baslatilsin

    Dac0_Start_Hold();
    
    Adc1_Start();

    ADF7021_CHIP_POWER_DOWN;        //CE pini asagi cek
    Delay_ms(10);
    ADF7021_LOAD_REGISTER_DISABLE;  //LE pinini yukari cek, load register disable olsun
    Delay_ms(10);
    ADF7021_CHIP_POWER_UP;          //CE pinini yukari cek, ADF7012 enable olsun
    Delay_ms(10);
   
    Delay_ms(200);


  s_address beacon_address[2] = {{"CUBEYY", 5},{"CUBEXX", 6}};

  Ax25_Send_Header(beacon_address,2);
  Ax25_Send_String("HELLO");
  Ax25_Send_Footer();



    Modem_Setup();
    Delay_ms(100);
    Adf_Lock(); //try to achieve a good PLL lock, otherwise use default vco configuration
    //ADF7012_CLEAR_DATA_PIN ;
    Delay_ms(100);
/*
    try_to_push_button:
    if(!Ptt_On()){     //means power is bad
        Modem_Setup(); //try to reconfigure adf7012
        Delay_ms(200);
        goto try_to_push_button;
    }
*/
     Ptt_On();
     
     
     while(1){
       
           if(PTT_OFF){
		  Ptt_Off();
		  PTT_OFF  = false;
	  }


	  Modem_Flush_Frame();
          while(MODEM_TRANSMITTING);
	  Delay_ms(2000);

    

#ifdef Debug_Modem_Packet
        
          for (k=0; k< MODEM_MAX_PACKET; k++){
          Spi_Byte_Send(modem_packet[k]);
          }
          Delay_ms(3000);
#endif
         }
    return (EXIT_SUCCESS);
}

// -----------------------------------------------------------------------


