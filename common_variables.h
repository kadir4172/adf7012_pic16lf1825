/* 
 * File:   common_variables.h
 * Author: Kadir
 *
 * Created on September 21, 2014, 3:22 PM
 */

#ifndef COMMON_VARIABLES_H
#define	COMMON_VARIABLES_H

#ifdef	__cplusplus
extern "C" {
#endif

/*
 * common_variables.h
 *
 *  Created on: Jun 14, 2014
 *      Author: kadir
 */

#include <stdbool.h>
#include <stdint.h>

extern uint32_t Set_Delay (uint32_t);
extern uint8_t Check_Delay(uint32_t);
extern bool Delay_ms(uint32_t);
extern uint32_t timeout_check;
extern uint8_t timeout_flag;

extern bool Write_Adf7012_Reg(uint8_t*, uint8_t);
extern bool Gpio_Config(void);
extern bool Read_Adf7012_Muxout(uint32_t*);
extern bool Reverse_Array(uint8_t* ,uint8_t);
extern void Radio_Setup(void);
extern void Ptt_On(void);
extern void Ptt_Off(void);
extern int Get_Powerlevel(void);
extern void Set_Freq(uint32_t);


//#define ADC_CHANNEL 7

#endif /* COMMON_VARIABLES_H_ */


#ifdef	__cplusplus
}
#endif

#endif	/* COMMON_VARIABLES_H */

