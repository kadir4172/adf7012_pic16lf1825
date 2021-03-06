/* 
 * File:   main.c
 * Author: Kadir
 *
 * Created on September 15, 2014, 6:55 PM
 */

//#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "configuration.h"
#include <stdbool.h>
<<<<<<< HEAD
//#include "utility.h"
=======
>>>>>>> parent of 067f070... [source] utility.c added

uint8_t test  = 0;
uint8_t test2 = 0;
uint16_t systick_counter = 0;
uint16_t dac_flag = 0;

uint8_t i = 0;
//unsigned char sine_table[26] = {0x10,0x14,0x17,0x1b,0x1d,0x1f,0x20,0x20,0x1f,0x1d,0x1b,0x17,0x14,0x10,0xc,0x9,0x5,0x3,0x1,0x0,0x0,0x1,0x3,0x5,0x9,0xc};
//uint8_t sine_table[26] = {0x11,0x14,0x18,0x1b,0x1e,0x20,0x21,0x21,0x20,0x1e,0x1b,0x18,0x14,0x11,0xd,0x9,0x6,0x3,0x1,0x0,0x0,0x1,0x3,0x6,0x9,0xd};

uint8_t sine_table2[14] = {0x10,0x16,0x1c,0x1f,0x1f,0x1c,0x16,0x10,0x9,0x3,0x0,0x0,0x3,0x9};

uint8_t adc_sonuc_high = 0;
uint8_t adc_sonuc_low  = 0;

bool Spi_Byte_Send(uint8_t);
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
<<<<<<< HEAD
       // Systick_Counter += 1; //mplab
=======
        systick_counter += 1;
>>>>>>> parent of 067f070... [source] utility.c added
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
        DACCON1 = sine_table2[i++];
        if (i==13)
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

    //PortA ve PortC konfigurasyonlari
    TRISA |= 0b00000010;
    TRISA &= 0b11001011;
    TRISC |= 0b00100000;
    TRISC &= 0b11100000;
    
}



int CheckDelay(uint8_t t)
{
  return((t - systick_counter));
}

void Delay_ms(uint16_t time_to_delay)
{
  systick_counter = 0;
  while (CheckDelay(time_to_delay));
}



int main(void) {
    unsigned int adc_sonuc;
    System_Start();
    while (!(OSCSTAT & (0x01))){} //HFIOFS Osc. stable bit bekle
    /* TODO timeout a bakip software reset verilmeli */

  //  Timer0_Start(); //32us  lik Timer0 baslatilsin
  //  Timer1_Start(); //833us lik Timer1 baslatilsin

  //  Dac0_Start();
    
  //  Adc1_Start();
   
  //  Delay_ms(200);

     while(1){
<<<<<<< HEAD
   //      Delay_ms(10);
         //Spi_Byte_Send(0x17);
=======
         Delay_ms(10);
         Spi_Byte_Send(0x17);
>>>>>>> parent of 067f070... [source] utility.c added
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

bool Spi_Byte_Send(uint8_t data){
    
    PORTAbits.RA2 = 0;
    uint8_t i;
    uint8_t data_to_send;
    data_to_send = data;
    for(i = 0; i <8; i++){
      
        if(data_to_send & 0x80)
            PORTCbits.RC2 = 1;
        else
            PORTCbits.RC2 = 0;

          data_to_send <<= 1;

        PORTAbits.RA2 = 1;
        Delay_ms(1);
        PORTAbits.RA2 = 0;
        Delay_ms(1);
        
    }
    return true;
}

// -----------------------------------------------------------------------
