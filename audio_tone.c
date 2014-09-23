
#include "config.h"
#include "audio_tone.h"
#include "utility.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
//#include "LPC17xx.h"                        /* LPC17xx definitions */
//#include "dac.h"
//#include "timer.h"
#include "adf7012.h"
#include <xc.h>

const uint8_t sine_table2[182]= {0x10,0x10,0x11,0x11,0x12,0x12,0x13,0x13,0x14,0x14,0x15,0x15,0x16,0x16,0x17,0x17,0x18,0x18,0x19,0x19,0x19,0x1a,0x1a,0x1b,0x1b,0x1b,0x1c,0x1c,0x1c,0x1d,0x1d,0x1d,0x1d,0x1e,0x1e,0x1e,0x1e,0x1e,0x1e,0x1f,0x1f,0x1f,0x1f,0x1f,0x1f,0x1f,0x1f,0x1f,0x1f,0x1f,0x1f,0x1f,0x1f,0x1e,0x1e,0x1e,0x1e,0x1e,0x1e,0x1d,0x1d,0x1d,0x1d,0x1c,0x1c,0x1c,0x1b,0x1b,0x1b,0x1a,0x1a,0x19,0x19,0x19,0x18,0x18,0x17,0x17,0x16,0x16,0x15,0x15,0x14,0x14,0x13,0x13,0x12,0x12,0x11,0x11,0x10,
0x10,0xf,0xe,0xe,0xd,0xd,0xc,0xc,0xb,0xb,0xa,0xa,0x9,0x9,0x8,0x8,0x7,0x7,0x6,0x6,0x6,0x5,0x5,0x4,0x4,0x4,0x3,0x3,0x3,0x2,0x2,0x2,0x2,0x1,0x1,0x1,0x1,0x1,0x1,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x1,0x1,0x1,0x1,0x1,0x1,0x2,0x2,0x2,0x2,0x3,0x3,0x3,0x4,0x4,0x4,0x5,0x5,0x6,0x6,0x6,0x7,0x7,0x8,0x8,0x9,0x9,0xa,0xa,0xb,0xb,0xc,0xc,0xd,0xd,0xe,0xe,0xf};


//Globals
bool PTT_OFF = false;
uint32_t modem_packet_size = 0;
uint8_t modem_packet[MODEM_MAX_PACKET];


// Source-specific
static const int TABLE_SIZE                = 182;
static const uint32_t PLAYBACK_RATE        = 31250;    // 32us lik Timer0
static const int BAUD_RATE                 = 1200;
static  uint8_t SAMPLES_PER_BAUD ;

static uint32_t PHASE_DELTA_1200;
static uint32_t PHASE_DELTA_2200;

static uint8_t current_byte;
static uint8_t current_sample_in_baud;    // 1 bit = SAMPLES_PER_BAUD samples

bool MODEM_TRANSMITTING = false;

static uint8_t phase_delta;                // 1200/2200 for standard AX.25
static uint8_t phase;                      // Fixed point 9.7 (2PI = TABLE_SIZE)
static uint32_t packet_pos;                 // Next bit to be sent out

void Modem_Init(void){
	SAMPLES_PER_BAUD = 26;
        PHASE_DELTA_1200 = 7;
        PHASE_DELTA_2200 = 13;
}

void Modem_Setup(void)
{
   Modem_Init();
   Radio_Setup();
}

bool Modem_Busy(void)
{
  return MODEM_TRANSMITTING;
}


void Modem_Set_Tx_Freq(uint32_t freq)
{
  Set_Freq(freq);
}

void Modem_Flush_Frame(void)
{
  phase_delta = PHASE_DELTA_1200;
  phase = 0;
  packet_pos = 0;
  current_sample_in_baud = 0;
  MODEM_TRANSMITTING = true;

  // Key the radio
  Ptt_On();

  Delay_ms(100);
  Timer0_Start();
}

// This is called at PLAYBACK_RATE Hz to load the next sample.
void Sinus_Generator(void) {

    uint8_t Audio_Signal;
    static uint8_t tone_index = 0;
if (MODEM_TRANSMITTING == true) {

    // If done sending packet
    if (packet_pos == modem_packet_size) { //mplab
    //if (packet_pos == 50) {
      MODEM_TRANSMITTING = false;             // End of transmission
      Timer0_Stop();
      
      Send_Vcxo_Signal(0x10); //DAC cikisi = Vdd/2
     
      PTT_OFF = true;
      
      goto end_generator;           // Done, gather ISR stats
    }

    // If sent SAMPLES_PER_BAUD already, go to the next bit
    if (current_sample_in_baud == 0) {    // Load up next bit
      if ((packet_pos & 7) == 0)          // Load up next byte
        current_byte = modem_packet[packet_pos >> 3];
      else
        current_byte = current_byte >> 1 ; 
      if ((current_byte & 0x01) == 0) {
        // Toggle tone (1200 <> 2200)
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
    if(phase > 181)    //mod alma instruction yok, software versiyonu yavas calisiyor
        phase = phase - 182;

   
    Audio_Signal = *(sine_table2 + phase);
    Send_Vcxo_Signal(Audio_Signal); //DAC cikisina ornegi yazalim
    
    if(++current_sample_in_baud == SAMPLES_PER_BAUD) {
      current_sample_in_baud = 0;
      packet_pos++;
     
    }
  }

end_generator:

  return;

}
