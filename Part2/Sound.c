// Sound.c
// This is the starter file for CECS 447 Project 1 Part 2
// By Dr. Min He
// September 10, 2022
// Port B bits 5-0 outputs to the 6-bit DAC
// Port D bits 3-0 inputs from piano keys: CDEF:doe ray mi fa,negative logic connections.
// Port F is onboard LaunchPad switches and LED
// SysTick ISR: PF3 is used to implement heartbeat

#include "tm4c123gh6pm.h"
#include "Sound.h"
#include "ButtonLed.h"
#include <stdint.h>
#include <stdbool.h>

void DelayMS(uint16_t milliseconds);

extern void DisableInterrupts(void); // Disable interrupts
extern void EnableInterrupts(void);  // Enable interrupts

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
{122137,108844,96970,91429,81633,72727,64777,            // C3 Major Notes
 30534*2,27211*2,24242*2,22923*2,20408*2,18182*2,16194*2, // C4 Major notes
 15289*2,13621*2,12135*2,11454*2,10204*2,9091*2,8099*2,   // C5 Major notes
 7645*2,6810*2,6067*2,5727*2,5102*2,4545*2,4050*2};        // C6 Major notes

// Constants
// Index for notes used in the music scores
typedef enum
{
  C3 = 0,
  D3 = 1,
  E3 = 2,
  F3 = 3,
  G3 = 4,
  A3 = 5,
  B3 = 6,

  C4 = 0+7,
  D4 = 1+7,
  E4 = 2+7,
  F4 = 3+7,
  G4 = 4+7,
  A4 = 5+7,
  B4 = 6+7,

  C5 = 0+2*7,
  D5 = 1+2*7,
  E5 = 2+2*7,
  F5 = 3+2*7,
  G5 = 4+2*7,
  A5 = 5+2*7,
  B5 = 6+2*7,

  PAUSE = 255,
} NOTE_INDEX;

// Keys masks
typedef enum
{
  KEY_C_MASK = 0x01, 
  KEY_D_MASK = 0x02, 
  KEY_E_MASK = 0x03, 
  KEY_F_MASK = 0x04,

  SWITCH1_MASK  = 0x10,
  SWITCH2_MASK  = 0x01, 

} MASKS;

extern volatile uint8_t curr_mode;
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
    G3,    2, G3, 2, A3, 4, G3, 4, C4, 4, B3, 4,
    SILENCE, 4, G3, 2, G3, 2, A3, 4, G3, 4, D4, 4, C4, 4,
    SILENCE, 4, G3, 2, G3, 2, G4, 4, E4, 4, C4, 4, B3, 4, A3, 8, 
    SILENCE, 4, F4, 2, F4, 2, E4, 4, C4, 4, D4, 4, C4, 8,  0, 0
  },


  { // Mary Had a Little Lamb
      D4, 4, C4, 4, B3, 4, C4, 4, D4, 4, D4, 4, D4, 8, 
      C4, 4, C4, 4, C4, 8, D4, 4, F4, 4, F4, 8,
      D4, 4, C4, 4, B3, 4, C4, 4, D4, 4, D4, 4, D4, 8, 
      C4, 4, C4, 4, D4, 4, C4, 4, B3, 8,  0, 0 
  },


  { // Twinkle Twinkle Little Star
      C3, 4, C3, 4, G3, 4, G3, 4, A3, 4, A3, 4, G3, 8, F3, 4, F3, 4, E3, 4, E3, 4, D3, 4, D3, 4, C3, 8, 
      G3, 4, G3, 4, F3, 4, F3, 4, E3, 4, E3, 4, D3, 8, G3, 4, G3, 4, F3, 4, F3, 4, E3, 4, E3, 4, D3, 8, 
      C3, 4, C3, 4, G3, 4, G3, 4, A3, 4, A3, 4, G3, 8, F3, 4, F3, 4, E3, 4, E3, 4, D3, 4, D3, 4, C3, 8, 0, 0
  },

};


// File scope golbal
volatile uint8_t curr_song=0;      // 0: Happy Birthday, 1: Mary Had A Little Lamb. 2: Twinkle Twinkle Little Stars
volatile uint8_t stop_play=1;      // 0: continue playing a song, 1: stop playing a song
volatile uint8_t octave=0;         // 0: lower C, 1: middle C, 2: upper C
volatile uint8_t curr_note=0;
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

  NVIC_ST_CTRL_R = 0;         // disable SysTick during setup
  NVIC_SYS_PRI3_R = (NVIC_SYS_PRI3_R&0x00FFFFFF)|0x20000000; // priority 1      
  NVIC_ST_CTRL_R |= NVIC_ST_CTRL_CLK_SRC|NVIC_ST_CTRL_INTEN;  // enable SysTick with core clock and interrupts
}

// **************Sound_Start*********************
// Set reload value and enable systick timer
// Input: time duration to be generated in number of machine cycles
// Output: none
void Sound_Stop(void)
{
	NVIC_ST_CTRL_R &= ~NVIC_ST_CTRL_ENABLE;
}

void Sound_Start(uint32_t period)
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

  if ( GPIO_PORTF_RIS_R & SWITCH1_MASK )
  {
    if ( curr_mode == PIANO )
    {
      curr_mode = AUTO_PLAY;
      GPIO_PORTD_IM_R    &= ~0x0F;        // (f) disarm interrupt on PD0 - PD3
    }
    else if ( curr_mode == AUTO_PLAY)
    {
      curr_mode = PIANO;
      GPIO_PORTD_IM_R |= 0x0F; // renable interrupts
    }
    GPIO_PORTF_ICR_R |= SWITCH1_MASK; // Ack interrupt 
  }
  else if ( GPIO_PORTF_RIS_R & SWITCH2_MASK & (curr_mode == PIANO) )
  {
    octave = (octave + 1) % 3;
    GPIO_PORTF_ICR_R |= SWITCH2_MASK; // Ack interrupt 
  }
  else if ( GPIO_PORTF_RIS_R & SWITCH2_MASK & (curr_mode == AUTO_PLAY) )
  {
    curr_song = (curr_song + 1) % 3;
    curr_note = 0;
    GPIO_PORTF_ICR_R |= SWITCH2_MASK; // Ack interrupt 
  }
}

// Dependency: Requires PianoKeys_Init to be called first, assume at any time only one key is pressed
// Inputs: None
// Outputs: None
// Description: Rising/Falling edge interrupt on PD6-PD0. Whenever any 
// button is pressed, or released the interrupGt will trigger.
void GPIOPortD_Handler(void){  
  // simple debouncing code: generate 20ms to 30ms delay
	for (uint32_t time=0;time<72724;time++) {}
  static bool pressed = true;
  NOTE_INDEX Note = 0;

  // If any of the 4 switches are pressed
  if ( (GPIO_PORTD_DATA_R & 0x0F) != 0 )
  {
    pressed = true;
  }
  else
  {
    GPIO_PORTD_ICR_R |= 0x0F; // Clear interrupts so all checks below fail if not in piano mode
    pressed = false;
  }

  if (GPIO_PORTD_RIS_R & KEY_C_MASK)
  {
    Note = C3;
    GPIO_PORTD_ICR_R |= KEY_C_MASK; // Ack interrupt 
  }
  else if (GPIO_PORTD_RIS_R & KEY_D_MASK )
  {
    Note = D3;
    GPIO_PORTD_ICR_R |= KEY_D_MASK; // Ack interrupt 
  }
  else if (GPIO_PORTD_RIS_R & KEY_E_MASK)
  {
    Note = E3;
    GPIO_PORTD_RIS_R |= KEY_E_MASK; // Ack interrupt 
  }
  else if (GPIO_PORTD_RIS_R & KEY_F_MASK)
  {
    Note = F3;
    GPIO_PORTD_ICR_R |= KEY_F_MASK; // Ack interrupt 
  }

  Note = Note + ( octave * 7);

  if ( pressed )
  {
    Sound_Start(tonetab[Note]/NUM_SAMPLES);
  }
  else
  {
    Sound_Stop();
  }

}


// Subroutine to wait 0.1 sec
// Inputs: None
// Outputs: None
// Notes: ...
void DelayMS(uint16_t milliseconds)
{
	unsigned long volatile ms_decrement;
  ms_decrement = 1600 * milliseconds;
  while(ms_decrement)
  {
    ms_decrement--;
  }
}

/*********************************************************
  Name: getDelay

  Description:
    Returns the delay of the current note in the Score Tab.
 *********************************************************/
static inline uint8_t getDelay(void)
{
  return playlist[curr_song][curr_note].delay;
}

/***************************************************************
  Name: getToneIndex

  Description:
    Returns the tone index of the current note in the score tab.
 ***************************************************************/

static inline uint8_t getToneIndex(void)
{
  return playlist[curr_song][curr_note].tone_index;
}

void play_a_song()
{
 uint8_t currentToneIndex = 0;
  uint8_t currentDelay = getDelay();
	while ( currentDelay && curr_mode == AUTO_PLAY )
  {
    currentToneIndex = getToneIndex();

    // Silence by disabling SysTick
    if ( currentToneIndex == SILENCE)
    {
			Sound_Stop();
    }
    // Set current note based on Tone Table
		else 
    {
      Sound_Start(tonetab[currentToneIndex + (octave * 7) ]/NUM_SAMPLES);
		}
		
		// Play current note for specified duration; delay is in 100ms intervals.
    DelayMS(currentDelay * 100);
		
    // Increment note
		Sound_Stop();
    DelayMS(25);
    curr_note++;
    currentDelay = getDelay();
  }
  curr_note = 0;
  curr_song = (curr_song + 1) % 3;
  DelayMS(100); //Delay between subsequent songs
}


