/*
 * File:   audio_tone.c
 * Author: Kadir
 *
 * Created on September 15, 2014, 6:55 PM
 */


#include "config.h"
#include "audio_tone.h"
#include "utility.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "adf7012.h"
#include <xc.h>

const uint8_t sine_table2[182]= {0x10,0x10,0x11,0x11,0x12,0x12,0x13,0x13,0x14,0x14,0x15,0x15,0x16,0x16,0x17,0x17,0x18,0x18,0x19,0x19,0x19,0x1a,0x1a,0x1b,0x1b,0x1b,0x1c,0x1c,0x1c,0x1d,0x1d,0x1d,0x1d,0x1e,0x1e,0x1e,0x1e,0x1e,0x1e,0x1f,0x1f,0x1f,0x1f,0x1f,0x1f,0x1f,0x1f,0x1f,0x1f,0x1f,0x1f,0x1f,0x1f,0x1e,0x1e,0x1e,0x1e,0x1e,0x1e,0x1d,0x1d,0x1d,0x1d,0x1c,0x1c,0x1c,0x1b,0x1b,0x1b,0x1a,0x1a,0x19,0x19,0x19,0x18,0x18,0x17,0x17,0x16,0x16,0x15,0x15,0x14,0x14,0x13,0x13,0x12,0x12,0x11,0x11,0x10,
0x10,0xf,0xe,0xe,0xd,0xd,0xc,0xc,0xb,0xb,0xa,0xa,0x9,0x9,0x8,0x8,0x7,0x7,0x6,0x6,0x6,0x5,0x5,0x4,0x4,0x4,0x3,0x3,0x3,0x2,0x2,0x2,0x2,0x1,0x1,0x1,0x1,0x1,0x1,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x1,0x1,0x1,0x1,0x1,0x1,0x2,0x2,0x2,0x2,0x3,0x3,0x3,0x4,0x4,0x4,0x5,0x5,0x6,0x6,0x6,0x7,0x7,0x8,0x8,0x9,0x9,0xa,0xa,0xb,0xb,0xc,0xc,0xd,0xd,0xe,0xe,0xf};


//Globals
bool PTT_OFF = false;
uint32_t modem_packet_size = 0;
uint8_t modem_packet[MODEM_MAX_PACKET];

extern bool Change_to_New_Baud;

// Source-specific
static const int TABLE_SIZE                = 182;
static const uint32_t PLAYBACK_RATE        = 31250;    // Timer0 with 32us
static const int BAUD_RATE                 = 1200;
static  uint8_t SAMPLES_PER_BAUD ;

static uint32_t PHASE_DELTA_1200;
static uint32_t PHASE_DELTA_2200;

static uint8_t current_byte;
static uint8_t current_sample_in_baud;    // 1 bit = SAMPLES_PER_BAUD samples

bool MODEM_TRANSMITTING = false;

static uint8_t  phase_delta;               // 1200/2200 for standard AX.25
static uint8_t  phase;                     // Phase pointer for sine table
static uint32_t packet_pos;                // Next bit to be sent out

void Configure_Audio(void){
	SAMPLES_PER_BAUD = 26;             //26 samples will be taken for every baud
        PHASE_DELTA_1200 = 7;              //jump 7 samples  on table for 1200Hz sine
        PHASE_DELTA_2200 = 13;             //jump 13 samples on table for 2200Hz sine
}

void Modem_Setup(void)
{
   Configure_Audio();                      //Configure Audio variables for AFSK1200
   Radio_Setup();                          //Reset configurations for ADF7012
}

void Modem_Flush_Frame(void)
{
  phase_delta = PHASE_DELTA_1200;
  phase = 0;
  packet_pos = 0;
  current_sample_in_baud = 0;
  MODEM_TRANSMITTING = true;

  ADF7012_CLEAR_DATA_PIN;
  Delay_ms(1);
  
/*
    try_to_push_button:
    if(!Ptt_On()){     //means power is bad
        Modem_Setup(); //try to reconfigure adf7012
        Delay_ms(200);
        goto try_to_push_button;
    }
*/
  Ptt_On(); /* TODO remove this line when working on real hardware */

  Delay_ms(100);
  Timer0_Start();
}

void Sinus_Generator(void) {

    uint8_t Audio_Signal;
    static uint8_t tone_index = 0;
if (MODEM_TRANSMITTING == true) {

   if (packet_pos == modem_packet_size) {
      MODEM_TRANSMITTING = false;             //Flag to check all packet content is transmitted
      Timer0_Stop();
      
      Send_Vcxo_Signal(0x10);                 //DAC output to Vdd/2
     
      PTT_OFF = true;                         //Flag to pull Ptt_Off() function outside of ISR
      
      goto end_generator;                     //return from the function
    }

    // If we have changed to new baud already
    if (current_sample_in_baud == 0) {   
      if ((packet_pos & 7) == 0)              // Load up next byte
        current_byte = modem_packet[packet_pos >> 3];
      else
        current_byte = current_byte >> 1 ;    // Load up next bit
      if ((current_byte & 0x01) == 0) {
        // Toggle tone (1200 or 2200)
         if(tone_index){
            phase_delta = PHASE_DELTA_1200;
            tone_index = 0;
          }
          else{
            phase_delta = PHASE_DELTA_2200;
            tone_index = 1;
          }
      }
    }

    phase += phase_delta;
    if(phase >= TABLE_SIZE)                    //No modulus instruction for CPU, takes more cycles to operate when compiling modulus operation
        phase = phase - TABLE_SIZE;

   
    Audio_Signal = *(sine_table2 + phase);     //Take the appropriate Audio sample from the table
    Send_Vcxo_Signal(Audio_Signal);            //Output the Audio sample to DAC output

    current_sample_in_baud++;
    
    if(Change_to_New_Baud == true) {           //Change to new baud if the the required time for a single baud is spent
      current_sample_in_baud = 0;
      packet_pos++;
      Change_to_New_Baud = false;
    }
     
  }

end_generator:

  return;

}
