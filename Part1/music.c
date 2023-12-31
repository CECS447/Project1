// This is startup program for CECS447 Project 1 part 1.
// Hardware connection: 
// Positive logic Speaker/Headset is connected to PA3.
// onboard two switches are used for music box control.
// Authors: Min He
// Date: August 28, 2018

#include <stdint.h>
#include <stdbool.h>

#include "tm4c123gh6pm.h"
#include "music.h"
#include "SysTick.h"

/************ Enums and Macros ************/

// Available songs
typedef enum
{
  LITTLE_LAMB = 0,
  TWINKLE_LITTLE_STAR,
  HAPPY_BIRTHDAY,
} SONG_INDEX;

uint8_t octave = 0;

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

  C4 = 0 + 7,
  D4 = 1 + 7,
  E4 = 2 + 7,
  F4 = 3 + 7,
  G4 = 4 + 7,
  A4 = 5 + 7,
  B4 = 6 + 7,

  C5 = 0+2*7,
  D5 = 1+2*7,
  E5 = 2+2*7,
  F5 = 3+2*7,
  G5 = 4+2*7,
  A5 = 5+2*7,
  B5 = 6+2*7,
  
  C6 = 0+3*7,
  D6 = 1+3*7,
  E6 = 2+3*7,
  F6 = 3+3*7,
  G6 = 4+3*7,
  A6 = 5+3*7,
  B6 = 6+3*7,

  SILENCE = 255,
} NOTE_INDEX;

// Used for score tab 2D array control
typedef enum
{
  MAX_NOTES = 50,
  MAX_SONGS = 3,
} SCORE_TAB_MAX;


/**************************** Static Data Structures *****************************/

static NTyp Score_Tab[MAX_SONGS][MAX_NOTES] = 
{  
  // Mary Had A Little Lamb
  {E3, 4, D3, 4, C3, 4, D3, 4, E3, 4, E3, 4, E3, 8, 
  D3, 4, D3, 4, D3, 8, E3, 4, G3, 4, G3, 8,
  E3, 4, D3, 4, C3, 4, D3, 4, E3, 4, E3, 4, E3, 8, 
  D3, 4, D3, 4, E3, 4, D3, 4, C3, 8, 0, 0 },

  // Twinkle Twinkle Little Stars
  {C3,4,C3,4,G3,4,G3,4,A3,4,A3,4,G3,8,F3,4,F3,4,E3,4,E3,4,D3,4,D3,4,C3,8, 
  G3,4,G3,4,F3,4,F3,4,E3,4,E3,4,D3,8,G3,4,G3,4,F3,4,F3,4,E3,4,E3,4,D3,8, 
  C3,4,C3,4,G3,4,G3,4,A3,4,A3,4,G3,8,F3,4,F3,4,E3,4,E3,4,D3,4,D3,4,C3,8,0,0},

{
  // Happy Birthday
// so   so   la   so   doe' ti
   G3,2,G3,2,A3,4,G3,4,C4,4,B3,4,
// pause so   so   la   so   ray' doe'
   SILENCE,4,  G3,2,G3,2,A3,4,G3,4,D4,4,C4,4,
// pause so   so   so'  mi'  doe' ti   la
   SILENCE, 4, G3,2,G3,2,G4,4,E4,4,C4,4,B3,4,A3,8, 
// pause fa'  fa'   mi'  doe' ray' doe'  stop
     SILENCE,4,  F4,2,F4,2, E4,4,C4,4,D4,4,C4,8, SILENCE,0
  },
};
// Piano Notes assuming 16MHz SysTic
//   Note: C, D, E, F, G, A, B
// Offset: 0, 1, 2, 3, 4, 5, 6
static const unsigned long Tone_Tab[] =
// calculate reload value for the whole period:Reload value = Fclk/Ft = 16MHz/Ft
// lower C octave:130.813, 146.832,164.814,174.614,195.998, 220,246.942
// calculate reload value for the whole period:Reload value = Fclk/Ft = 16MHz/Ft
{122137/2,108844/2,96970/2,91429/2,81633/2,72727/2,64777/2,                 // C3 Major notes
 30534,27211,24242,22923,20408,18182,16194,     // C4 Major notes
 15289,13621,12135,11454,10204,9091,8099,       // C5 Major notes
 7645,6810,6067,5727,5102,4545,4050};           // C6 Major notes


/******** Global Variables *********/
volatile bool musicOn = 0;


/******** Static Local Variables *********/
volatile uint8_t currentNote    = 0;
static volatile SONG_INDEX currentSong = 0;


/******** Static Local Function Declarations ********/
static void DelayMS(uint16_t milliseconds);
static inline uint8_t getDelay(void);
static inline uint8_t getToneIndex(void);


/******** Static Local Function Definitions *********/

/******************************************************************
  Name: DelayMS

  Description:
    Uses a hardcoded value to busy wait the indicated milliseconds.
    Assumes 16MHz SysTick.

  Args:
    milliseconds = the amout of milliseconds to loiter 
 ******************************************************************/
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
  return Score_Tab[currentSong][currentNote].delay;
}

/***************************************************************
  Name: getToneIndex

  Description:
    Returns the tone index of the current note in the score tab.
 ***************************************************************/

static inline uint8_t getToneIndex(void)
{
  return Score_Tab[currentSong][currentNote].tone_index;
}


/************************************** Public Functon Definitions ******************************************/

/**********************************************************************************************
  Name: play_a_song

  Description:
    Plays the current song selected by the program, by stepping through the 2D array score tab.
 **********************************************************************************************/
void play_a_song(void)
{
  uint8_t currentToneIndex = 0;
  uint8_t currentDelay = getDelay();
	while (currentDelay && musicOn)
  {
    currentToneIndex = getToneIndex();

    // Silence by disabling SysTick
    if ( currentToneIndex == SILENCE)
    {
			SysTick_stop();
    }
    // Set current note based on Tone Table
		else 
    {
      SysTick_Set_Current_Note(Tone_Tab[currentToneIndex + (octave * 7)]);
			SysTick_start();
		}
		
		// Play current note for specified duration; delay is in 100ms intervals.
		DelayMS(currentDelay * 100);
		
    // Increment note
		SysTick_stop();

    // Delay for break in notes
    DelayMS(25);

    currentNote++;
    currentDelay = getDelay();

  }
  currentNote = 0;
  DelayMS(100);
}

/******************************************************************************************
  Name: Music_Init

  Description:
    Initialize the GPIO that drives the buzzer.    
 ******************************************************************************************/
void Music_Init(void)
{ 
  volatile unsigned long delay;
  SYSCTL_RCGC2_R |= 0x01;            // 1) activate clock for Port A
  delay = SYSCTL_RCGC2_R;            // allow time for clock to start
                                     // 2) no need to unlock PA3
  GPIO_PORTA_PCTL_R  &= ~0x0000F000; // 3) regular GPIO
  GPIO_PORTA_AMSEL_R &= ~0x08;       // 4) disable analog function on PA3
  GPIO_PORTA_DIR_R   |=  0x08;       // 5) set direction to output
  GPIO_PORTA_AFSEL_R &= ~0x08;       // 6) regular port function
  GPIO_PORTA_DEN_R   |=  0x08;       // 7) enable digital port
  GPIO_PORTA_DR8R_R  |=  0x08;       // 8) optional: enable 8 mA drive on PA3 to increase the voice volumn
}

/******************************************************************************************
  Name: next_song

  Description:
    Selects the next song in a round robin fashion. Resets the current note so that the new 
    queued song starts from the beginning.
 ******************************************************************************************/
void next_song(void)
{
  currentSong = (currentSong + 1) % 4;
}

/******************************************************************************************
  Name: is_music_on

  Description:
    Getter function for the musicOn variable, representing music on / off state.
 ******************************************************************************************/
bool is_music_on(void)
{
  return musicOn;
}

/******************************************************************************************
  Name: turn_off_music

  Description:
    Sets the musicOn variable to false, and ceases music operation by disabling SysTick.
    Sets current note to 0 to ensure next playback resets the song.
 ******************************************************************************************/
void turn_off_music(void)
{
  musicOn = false;
  SysTick_stop();
  currentNote = 0;
}

/******************************************************************************************
  Name: turn_on_music

  Description:
    Sets the musicOn variable to true, and starts music operation by enabling SysTick.
 ******************************************************************************************/
void turn_on_music(void)
{
  musicOn = true;
  SysTick_start();
}
