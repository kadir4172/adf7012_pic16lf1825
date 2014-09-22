/*
 * utility.c
 *
 *  Created on: Jun 28, 2014
 *      Author: kadir
 */

//#include "LPC17xx.h"                        /* LPC17xx definitions */
//#include "ssp.h"
//#include "dac.h"
//#include "adc.h"
#include "utility.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <stdint.h>
#include <stdbool.h>
#include "configuration.h"

uint16_t Systick_Counter = 0; //Her 10 ms de bir tick sayar
uint32_t Delay_Counter   = 0; //Delay_ms fonksiyonunda kullanilacak counter, 10ms de bir tick sayar
uint8_t  timeout_flag = 0;
uint32_t timeout_check;


bool Spi_Byte_Send(uint8_t);
/******************************************************************************
** Function name:		Gpio_Config
**
** Descriptions:		Beacon MCU nun ilgili pinlerini konfigure eder
** Returned value:		returns TRUE if successful
**
******************************************************************************/
bool Gpio_Config(void){

   //PortA ve PortC konfigurasyonlari
    TRISA |= 0b00000010;
    TRISA &= 0b11001011;
    TRISC |= 0b00100000;
    TRISC &= 0b11100000;
    
  return true;

}


int CheckDelay(uint8_t t)
{
  return((t - Systick_Counter));
}

void Delay_ms(uint16_t time_to_delay)
{
  Systick_Counter = 0;
  while (CheckDelay(time_to_delay));
}




/******************************************************************************
** Function name:		Read_Adf7012_Muxout
**
** Descriptions:		adf7012 entegresinin muxout pinini okur
** Returned value:		returns TRUE if successfull
**
******************************************************************************/
bool Read_Adf7012_Muxout(void){

  Delay_ms(1);

  if(PORTCbits.RC5)
      return true;
  else
      return false;
  
}


/******************************************************************************
** Function name:		Write_Adf7012_Reg
**
** Descriptions:		adf7012 entegresine istenen register degerini yazar
** Parameters:			reg_value , yazilacak byte dizisinin ilk karaketerine pointer
** Returned value:		returns TRUE if successfull
**
******************************************************************************/
bool Write_Adf7012_Reg(uint8_t* reg_value, uint8_t size_of_reg){

    uint8_t i = 0;
  Delay_ms(1);
  ADF7021_LOAD_REGISTER_ENABLE;
  Delay_ms(1);

  for (i = 0; i < size_of_reg; i++){
  Spi_Byte_Send(*(reg_value+i));
  }

  Delay_ms(1);
  ADF7021_LOAD_REGISTER_DISABLE;

  Delay_ms(1);
  return true;
}

/******************************************************************************
** Function name:		Send_Vcxo_Signal
**
** Descriptions:		Analog output pinine istenilen degeri yazar
** Parameters:			DAC cikisina yazilacak 32 bit veri
** Returned value:		returns TRUE if successfull
**
******************************************************************************/
bool Send_Vcxo_Signal(uint8_t value){

  DACCON1 = value;   //DAC cikisina value degerini yaz
  return true;
}
/******************************************************************************
** Function name:		Init_Adf7012
**
** Descriptions:		adf7012 entegresini OOK modunda istenen konfigurasyonda baslatir
** Returned value:		returns TRUE if successfull
**
******************************************************************************/
/*
_Bool Init_Adf7012(void){

uint8_t register0[4] = {0x04, 0x11, 0xE0, 0x00};
uint8_t register1[3] = {0x5B, 0x40, 0x01}      ;
uint8_t register2[4] = {0x00, 0x00, 0x81, 0xEE};
uint8_t register3[4] = {0x00, 0x45, 0x20, 0xFF};

Delay_ms(500);

/*send register0

ADF7021_LOAD_REGISTER_ENABLE;

Delay_ms(1);
force_register0:
  SSPSend(PORTNUM, register0, sizeof(register0));
  if(timeout_flag != 0){
    timeout_flag = 0;
    goto force_register0;
  }
  Delay_ms(1);
  ADF7021_LOAD_REGISTER_DISABLE;
  Delay_ms(10);

/*send register1
  ADF7021_LOAD_REGISTER_ENABLE;
  Delay_ms(1);
force_register1:
  SSPSend(PORTNUM, register1, sizeof(register1));
  if(timeout_flag != 0){
    timeout_flag = 0;
    goto force_register1;
  }
  Delay_ms(1);
  ADF7021_LOAD_REGISTER_DISABLE;
  Delay_ms(10);

/*send register2
  ADF7021_LOAD_REGISTER_ENABLE;
  Delay_ms(1);
force_register2:
  SSPSend(PORTNUM, register2, sizeof(register2));
  if(timeout_flag != 0){
     timeout_flag = 0;
     goto force_register2;
   }
  Delay_ms(1);
  ADF7021_LOAD_REGISTER_DISABLE;
  Delay_ms(10);


  /*send register3 
  ADF7021_LOAD_REGISTER_ENABLE;
  Delay_ms(1);
force_register3:
  SSPSend(PORTNUM, register3, sizeof(register3));
  if(timeout_flag != 0){
     timeout_flag = 0;
     goto force_register3;
   }
  Delay_ms(1);
  ADF7021_LOAD_REGISTER_DISABLE;
  Delay_ms(10);

return TRUE;
}

 * */


/******************************************************************************
** Function name:		Reverse_Array
**
** Parameters:			sirasi degistirilecek array ve uzunlugu
** Returned value:		returns TRUE if successfull
**
******************************************************************************/
bool Reverse_Array(uint8_t* input,uint8_t length){
  uint8_t i = 0;
  uint8_t buffer_array[4];
  memcpy(buffer_array, input, length);

  for(i = 0; i<length; i++){
	  *(input+i) = *(buffer_array+(length-1)-i);
  }

  

return true;
}


/******************************************************************************
** Function name:		Spi_Byte_Send
**
** Parameters:			gonderilecek data
** Returned value:		returns TRUE if successfull
**
******************************************************************************/
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




void Timer0_Start(void){
  TMR0 = 0x00;
  TMR0IF = 0;
  TMR0IE = 1;  //Timer0 interrupt enable edelim
}
void Timer0_Stop(void){
  TMR0IE = 0;  //Timer0 interrupt disable edelim
  TMR0 = 0x00;
}

void Timer1_Start(void){
    //reset Timer1 registers
    TMR1H = 0x00;
    TMR1L = 0x00;

    //833 us lik compare registerlari
    CCPR1H = 0x03;
    CCPR1L = 0x41;

    //compare1 modulu interrupt enable
    CCP1IE = 1;
}

void Timer1_Stop(void){
    //reset Timer1 registers
    TMR1H = 0x00;
    TMR1L = 0x00;

    CCP1IE = 0; // compare1 modulu interrupt disable
}

void Dac0_Start(void){
    DACEN = 1;  //DAC output enable
    DACCON1 = 0x10;//Vdd/2 on DACOUT
}

void Dac0_Stop(void){
    DACEN = 0; //DAC output disable
}

void Adc1_Start(void){
    ADIF = 0; //clear interrupt flag
    ADON = 1;
    ADIE = 1; //ADC interrupt enable
    //PIR1 &= 0b10111111;
    //PIE1 |= 0b01000000;
}

void Adc1_Stop(void){
    ADON = 0; //ADC disable
    ADIE = 0; //ADC interrupt disable
}
