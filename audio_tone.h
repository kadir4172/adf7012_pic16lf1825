/* 
 * File:   audio_tone.h
 * Author: Kadir
 *
 * Created on September 21, 2014, 3:35 PM
 */

#ifndef AUDIO_TONE_H
#define	AUDIO_TONE_H

#ifdef	__cplusplus
extern "C" {
#endif


#include <stdint.h>

#define MODEM_MAX_PACKET 512

extern uint8_t modem_packet[MODEM_MAX_PACKET];   
extern uint32_t modem_packet_size;                

void Modem_Setup();
void Modem_Start();
void Modem_Flush_Frame();
void Modem_Set_Tx_Freq(uint32_t freq);
int Modem_Get_Powerlevel();
bool Modem_Busy();





#ifdef	__cplusplus
}
#endif

#endif	/* AUDIO_TONE_H */

