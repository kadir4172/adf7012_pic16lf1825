/* 
 * File:   utility.h
 * Author: Kadir
 *
 * Created on September 21, 2014, 3:41 PM
 */

#ifndef UTILITY_H
#define	UTILITY_H

#ifdef	__cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <stdint.h>
#include <stdbool.h>

//Hardware Specific Utility Functions

extern void Delay_ms(uint16_t);
extern bool Write_Adf7012_Reg(uint8_t*, uint8_t);
extern bool Gpio_Config(void);
extern bool Read_Adf7012_Muxout(void);
extern bool Reverse_Array(uint8_t* ,uint8_t);
extern bool Send_Vcxo_Signal(uint8_t);
//Hardware Specific Utility Functions



//Hardware Specific Variables/Macros
#define ADF7021_LOAD_REGISTER_ENABLE   PORTAbits.RA4 = 0;
#define ADF7021_LOAD_REGISTER_DISABLE  PORTAbits.RA4 = 1;

#define ADF7021_CHIP_POWER_UP    PORTAbits.RA5 = 1;
#define ADF7021_CHIP_POWER_DOWN  PORTAbits.RA5 = 0;

#define ADF7012_CLEAR_DATA_PIN PORTCbits.RC3 = 0;
#define ADF7012_SET_DATA_PIN   PORTCbits.RC3 = 1;


#define ADF7012_CLEAR_CLK_PIN  PORTCbits.RC4 = 0;
#define ADF7012_SET_CLK_PIN    PORTCbits.RC4 = 1;
//Hardware Specific Variables/Macros

//Common Functions
extern void Radio_Setup(void);
extern bool Ptt_On(void);
extern void Ptt_Off(void);
extern void Set_Freq(uint32_t);
extern bool Spi_Byte_Send(uint8_t);
extern bool Adf_Lock(void);
//Common Functions



//Peripheral functions
extern void Timer0_Start(void);
extern void Timer0_Stop(void);
extern void Timer1_Start(void);
extern void Timer1_Stop(void);
extern void Dac0_Start_Hold(void);
extern void Dac0_Stop(void);
extern void Adc1_Start(void);
extern void Adc1_Stop(void);
//Peripheral functions


#ifdef	__cplusplus
}
#endif

#endif	/* UTILITY_H */

