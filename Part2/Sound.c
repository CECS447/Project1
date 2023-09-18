t // Sound.c
// This is the starter file for CECS 447 Project 1 Part 2
// By Dr. Min He
// September 10, 2022
// Port B bits 5-0 outputs to the 6-bit DAC
// Port D bits 3-0 inputs from piano keys: CDEF:doe ray mi fa,negative logic connections.
// Port F is onboard LaunchPad switches and LED
// SysTick ISR: PF3 is used to implement heartbeat

#include "tm4c123gh6pm.h"
#include "Sound.h"
#include <stdint.h>

// define bit addresses for Port B bits 0,1,2,3,4,5 => DAC inputs 
#define DAC (*((volatile unsigned long *)0x4000501C))
unsigned char Index;  	
// 6-bit: value range 0 to 2^6-1=63, 64 samples
const uint8_t SineWave[64] = {32,35,38,41,44,47,49,52,54,56,58,59,61,62,62,63,63,63,62,62,
															61,59,58,56,54,52,49,47,44,41,38,35,32,29,26,23,20,17,15,12,
															10, 8, 6, 5, 3, 2, 2, 1, 1, 1, 2, 2, 3, 5, 6, 8,10,12,15,17,
															20,23,26,29};

// initial values for piano major tones.
// Assume SysTick clock frequency is 16MHz.
const uint32_t tonetab[] =
// C, D, E, F, G, A, B
// 1, 2, 3, 4, 5, 6, 7
// lower C octave:130.813, 146.832,164.814,174.614,195.998, 220,246.942
// calculate reload value for the whole period:Reload value = Fclk/Ft = 16MHz/Ft
{122137,108844,96970,91429,81633,72727,64777,
 30534*2,27211*2,24242*2,22923*2,20408*2,18182*2,16194*2, // C4 Major notes
 15289*2,13621*2,12135*2,11454*2,10204*2,9091*2,8099*2,   // C5 Major notes
 7645*2,6810*2,6067*2,5727*2,5102*2,4545*2,4050*2};        // C6 Major notes

// Constants
// Index for notes used in the music scores
typedef enum
{
  C4 = 0,
  D4 = 1,
  E4 = 2,
  F4 = 3,
  G4 = 4,
  A4 = 5,
  B4 = 6,

  C5 = 0+7,
  D5 = 1+7,
  E5 = 2+7,
  F5 = 3+7,
  G5 = 4+7,
  A5 = 5+7,
  B5 = 6+7,

  C6 = 0+2*7,
  D6 = 1+2*7,
  E6 = 2+2*7,
  F6 = 3+2*7,
  G6 = 4+2*7,
  A6 = 5+2*7,
  B6 = 6+2*7,

  PAUSE = 255,
} NOTE_INDEX;

// Keys masks
typedef enum
{
  KEY_C_MASK = 0x01, 
  KEY_D_MASK = 0x02, 
  KEY_E_MASK = 0x03, 
  KEY_F_MASK = 0x04,

} KEY_MASKS;


#define MAX_NOTES 255 // maximum number of notes for a song to be played in the program
#define MAX_SONGS 3   // number of songs in the play list.
#define SILENCE MAX_NOTES // use the last valid index to indicate a silence note. The song can only have up to 254 notes. 
#define NUM_OCT  3   // number of octave defined in tontab[]
#define NUM_NOTES_PER_OCT 7  // number of notes defined for each octave in tonetab
#define NVIC_EN0_PORTF 0x40000000  // enable PORTF edge interrupt
#define NVIC_EN0_PORTD 0x00000008  // enable PORTD edge interrupt
#define NUM_SAMPLES 64  // number of sample in one sin wave period

static NTyp playlist[MAX_SONGS][MAX_NOTES] = 
{  
  { // Happy Birthday
    G4,    2, G4, 2, A4, 4, G4, 4, C5, 4, B4, 4,
    SILENCE, 4, G4, 2, G4, 2, A4, 4, G4, 4, D5, 4, C5, 4,
    SILENCE, 4, G4, 2, G4, 2, G5, 4, E5, 4, C5, 4, B4, 4, A4, 8, 
    SILENCE, 4, F5, 2, F5, 2, E5, 4, C5, 4, D5, 4, C5, 8,  0, 0
  },

  { // Mary Had a Little Lamb
    E4, 4, D4, 4, C4, 4, D4, 4, E4, 4, E4, 4, E4, 8, 
    D4, 4, D4, 4, D4, 8, E4, 4, G4, 4, G4, 8,
    E4, 4, D4, 4, C4, 4, D4, 4, E4, 4, E4, 4, E4, 8, 
    D4, 4, D4, 4, E4, 4, D4, 4, C4, 8,  0, 0 
  },


  { // Twinkle Twinkle Little Star
    C4, 4, C4, 4, G4, 4, G4, 4, A4, 4, A4, 4, G4, 8, F4, 4, F4, 4, E4, 4, E4, 4, D4, 4, D4, 4, C4, 8, 
    G4, 4, G4, 4, F4, 4, F4, 4, E4, 4, E4, 4, D4, 8, G4, 4, G4, 4, F4, 4, F4, 4, E4, 4, E4, 4, D4, 8, 
    C4, 4, C4, 4, G4, 4, G4, 4, A4, 4, A4, 4, G4, 8, F4, 4, F4, 4, E4, 4, E4, 4, D4, 4, D4, 4, C4, 8, 0, 0
  },
};


// File scope golbal
volatile uint8_t curr_song=0;      // 0: Happy Birthday, 1: Mary Had A Little Lamb. 2: Twinkle Twinkle Little Stars
volatile uint8_t stop_play=1;      // 0: continue playing a song, 1: stop playing a song
volatile uint8_t octave=0;         // 0: lower C, 1: middle C, 2: upper C

																		// **************DAC_Init*********************
// Initialize 6-bit DAC  on Port B
// Input: none
// Output: none
void DAC_Init(void){
  SYSCTL_RCGC2_R |= SYSCTL_RCGC2_GPIOB; // activate port B
  GPIO_PORTB_AMSEL_R &= ~0x3F;      // no analog 
  GPIO_PORTB_PCTL_R &= ~0x00FFFFFF; // regular function
  GPIO_PORTB_DIR_R |= 0x3F;      // make PB0-5 out
  GPIO_PORTB_AFSEL_R &= ~0x3F;   // disable alt funct on PB0-5
  GPIO_PORTB_DEN_R |= 0x3F;      // enable digital I/O on PB0-5
  GPIO_PORTB_DR8R_R |= 0x3F;        // enable 8 mA drive on PB0-5	
}

// **************Sound_Start*********************
// Set reload value and enable systick timer
// Input: time duration to be generated in number of machine cycles
// Output: none
void Sound_Stop(void)
{
	NVIC_ST_CTRL_R &= ~NVIC_ST_CTRL_ENABLE;
}

void Sound_Start(unsigned long period)
{
  NVIC_ST_RELOAD_R = period-1;// reload value
  NVIC_ST_CURRENT_R = 0;      // any write to current clears it
	NVIC_ST_CTRL_R |= NVIC_ST_CTRL_ENABLE;
}

// Interrupt service routine
// Executed based on number of sampels in each period
void SysTick_Handler(void){
	Index = (Index+1)%64;
	DAC = SineWave[Index]; // output to DAC: 6-bit data
}

void GPIOPortF_Handler(void){
	// simple debouncing code: generate 20ms to 30ms delay
	for (uint32_t time=0;time<72724;time++) {}
}

// Dependency: Requires PianoKeys_Init to be called first, assume at any time only one key is pressed
// Inputs: None
// Outputs: None
// Description: Rising/Falling edge interrupt on PD6-PD0. Whenever any 
// button is pressed, or released the interrupGt will trigger.
void GPIOPortD_Handler(void){  
  // simple debouncing code: generate 20ms to 30ms delay
	for (uint32_t time=0;time<72724;time++) {}

  if (GPIO_PORTD_RIS_R & KEY_C_MASK)
  {
    GPIO_PORTD_RIS_R |= KEY_C_MASK; // Ack interrupt 
  }
  else if (GPIO_PORTD_RIS_R & KEY_D_MASK)
  {
    GPIO_PORTD_RIS_R |= KEY_D_MASK; // Ack interrupt 
  }
  else if (GPIO_PORTD_RIS_R & KEY_E_MASK)
  {
    GPIO_PORTD_RIS_R |= KEY_E_MASK; // Ack interrupt 
  }
  else if (GPIO_PORTD_RIS_R & KEY_F_MASK)
  {
    GPIO_PORTD_RIS_R |= KEY_F_MASK; // Ack interrupt 
  }
}


// Subroutine to wait 0.1 sec
// Inputs: None
// Outputs: None
// Notes: ...
void Delay(void){
	uint32_t volatile time;
  time = 727240*20/91;  // 0.1sec
  while(time){
		time--;
  }
}

void play_a_song()
{
}


