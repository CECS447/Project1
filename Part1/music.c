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

// Used for score tab 2D array control
typedef enum
{
  MAX_NOTES = 50,
  MAX_SONGS = 3,
} SCORE_TAB_MAX;


/**************************** Static Data Structures *****************************/

static NTyp Score_Tab[MAX_SONGS][MAX_NOTES] = 
{  
  // Mary Had a little Lamb
  {
    E4, 4, D4, 4, C4, 4, D4, 4, E4, 4, E4, 4, E4, 8, 
    D4, 4, D4, 4, D4, 8, E4, 4, G4, 4, G4, 8,
    E4, 4, D4, 4, C4, 4, D4, 4, E4, 4, E4, 4, E4, 8, 
    D4, 4, D4, 4, E4, 4, D4, 4, C4, 8, 0, 0
  },

  // score table for Twinkle Twinkle Little Stars
  {
    C4, 4, C4, 4, G4, 4, G4, 4, A4, 4, A4, 4, G4, 8, F4, 4, F4, 4, E4, 4, E4, 4, D4, 4, D4, 4, C4, 8, 
    G4, 4, G4, 4, F4, 4, F4, 4, E4, 4, E4, 4, D4, 8, G4, 4, G4, 4, F4, 4, F4, 4, E4, 4, E4, 4, D4, 8, 
    C4, 4, C4, 4, G4, 4, G4, 4, A4, 4, A4, 4, G4, 8, F4, 4, F4, 4, E4, 4, E4, 4, D4, 4, D4, 4, C4, 8,0,0
  },
  
  // Happy Birthday
  {    C4, 2, C4, 2, D4, 4, C4, 4, F4, 4, E4, 4,
    PAUSE, 4, C4, 2, C4, 2, D4, 4, C4, 4, G4, 4, F4, 4,
    PAUSE, 4, C4, 2, C4, 2, C4, 4, A4, 4, F4, 4, E4, 4, D4, 8, 
    PAUSE, 4, B4, 2, B4, 2, A4, 4, F4, 4, G4, 4, F4, 8,  0, 0
  },
};

// Piano Notes assuming 16MHz SysTick
//   Note: C, D, E, F, G, A, B
// Offset: 0, 1, 2, 3, 4, 5, 6
static const unsigned long Tone_Tab[] =
{ 
 30534,27211,24242,22923,20408,18182,16194, // C4 Major notes
 15289,13621,12135,11454,10204,9091,8099,   // C5 Major notes
 7645,6810,6067,5727,5102,4545,4050         // C6 Major notes
};


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
    if ( currentToneIndex == PAUSE)
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
    DelayMS(5);

    currentDelay = getDelay();
    currentNote++;
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
  currentSong = (currentSong + 1) % 3;
  currentNote = 0;
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
