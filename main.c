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


extern  bool PTT_OFF;
extern void Ptt_Off(void);
extern uint8_t modem_packet[MODEM_MAX_PACKET];

uint8_t test  = 0;
uint8_t test2 = 0;
uint16_t dac_flag = 0;

uint8_t i = 0;
//unsigned char sine_table[26] = {0x10,0x14,0x17,0x1b,0x1d,0x1f,0x20,0x20,0x1f,0x1d,0x1b,0x17,0x14,0x10,0xc,0x9,0x5,0x3,0x1,0x0,0x0,0x1,0x3,0x5,0x9,0xc};
uint8_t sine_table[26] = {0x11,0x14,0x18,0x1b,0x1e,0x20,0x21,0x21,0x20,0x1e,0x1b,0x18,0x14,0x11,0xd,0x9,0x6,0x3,0x1,0x0,0x0,0x1,0x3,0x6,0x9,0xd};

uint8_t sine_table2[14] = {0x10,0x16,0x1c,0x1f,0x1f,0x1c,0x16,0x10,0x9,0x3,0x0,0x0,0x3,0x9};

uint8_t adc_sonuc_high = 0;
uint8_t adc_sonuc_low  = 0;


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

       ADCON0 |= 0b00000010; //ADC Go
      
    }
    //Timer1 interrupt

    //Timer0 interrupt
    if(INTCON & 0x04){
       //Timer0 ISR
        DACCON1 = sine_table[i++];
        if (i==25)
            i=0;
    
       //Timer0 ISR
  
       INTCON &= ~0x04; //Clear Timer0 interrupt flag
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
    

    //Global Interrupt ve Peripheral Interrupt Enable
    INTCON |= 0xC0;

  
    
}


int main(void) {
    unsigned int adc_sonuc;
    System_Start();
    while (!(OSCSTAT & (0x01))){} //HFIOFS Osc. stable bit bekle
    /* TODO timeout a bakip software reset verilmeli */

    Gpio_Config();
    
    Timer0_Start(); //32us  lik Timer0 baslatilsin
    Timer1_Start(); //833us lik Timer1 baslatilsin

    Dac0_Start();
    
    Adc1_Start();

    ADF7021_CHIP_POWER_DOWN;        //CE pini asagi cek
    Delay_ms(10);
    ADF7021_LOAD_REGISTER_DISABLE;  //LE pinini yukari cek, load register disable olsun
    Delay_ms(10);
    ADF7021_CHIP_POWER_UP;          //CE pinini yukari cek, ADF7012 enable olsun
    Delay_ms(10);
   
    Delay_ms(200);


  s_address beacon_address[2] = {{"CUBEYY", 5},{"CUBEXX", 7}};

  Ax25_Send_Header(beacon_address,2);
  Ax25_Send_String("HELLO");
  Ax25_Send_Footer();



  Modem_Setup();
  Delay_ms(100);
  ADF7012_CLEAR_DATA_PIN ;
  Delay_ms(100);
  Ptt_On();

     while(1){
           if(PTT_OFF){
		  Ptt_Off();
		  PTT_OFF  = false;
	  }


	  Modem_Flush_Frame();
	  Delay_ms(200);
          Delay_ms(10);
          //Spi_Byte_Send(0x17);
     }
    return (EXIT_SUCCESS);
}


/*TEST RUTIN

 * if(PORTCbits.RC5)
         {
             PORTAbits.RA1 = 1;
         }
         else
             PORTAbits.RA1 = 0;
 * 
*/



// -----------------------------------------------------------------------


