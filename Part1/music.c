// This is startup program for CECS447 Project 1 part 1.
// Hardware connection: 
// Positive logic Speaker/Headset is connected to PA3.
// onboard two switches are used for music box control.
// Authors: Min He
// Date: August 28, 2018

#include "tm4c123gh6pm.h"
#include "music.h"
#include "SysTick.h"

void Delay(void);

static volatile uint8_t currentSong = 0;

// initail values for piano major notes: assume SysTick clock is 16MHz.
const unsigned long Tone_Tab[] =
// initial values for three major notes for 16MHz system clock
// Note name: C, D, E, F, G, A, B
// Offset:0, 1, 2, 3, 4, 5, 6
{30534,27211,24242,22923,20408,18182,16194, // C4 major notes
 15289,13621,12135,11454,10204,9091,8099, // C5 major notes
 7645,6810,6067,5727,5102,4545,4050};// C6 major notes

// Index for notes used in music scores
#define C5 0+7
#define D5 1+7
#define E5 2+7
#define F5 3+7
#define G5 4+7
#define A5 5+7
#define B5 6+7
#define C6 0+2*7
#define D6 1+2*7
#define E6 2+2*7
#define F6 3+2*7
#define G6 4+2*7
#define C4 0
#define D4 1
#define E4 2
#define F4 3
#define G4 4
#define A4 5
#define B4 6
#define C5 0+7
#define D5 1+7
#define E5 2+7
#define F5 3+7
#define G5 4+7
#define A5 5+7
#define B5 6+7
#define C6 0+2*7
#define D6 1+2*7
#define E6 2+2*7
#define F6 3+2*7
#define G6 4+2*7
#define A6 5+2*7
#define B6 6+2*7


#define PAUSE 255				// assume there are less than 255 tones used in any song
#define MAX_NOTES 50  // assume maximum number of notes in any song is 100. You can change this value if you add a long song.
#define MAX_SONGS 3
volatile uint8_t musicOn = 0;

// doe ray mi fa so la ti 
// C   D   E  F  G  A  B
static NTyp Score_Tab[MAX_SONGS][MAX_NOTES] = {  
// score table for Happy Birthday
{
    G4,2,G4,2,A4,4,G4,4,C5,4,B4,4,
// pause so   so   la   so   ray' doe'
   PAUSE,4,  G4,2,G4,2,A4,4,G4,4,D5,4,C5,4,
// pause so   so   so'  mi'  doe' ti   la
   PAUSE,4, G4,2,G4,2,G5,4,E5,4,C5,4,B4,4,A4,8, 
// pause fa'  fa'   mi'  doe' ray' doe'  stop
	 PAUSE,4,  F5,2,F5,2, E5,4,C5,4,D5,4,C5,8, 0,0
},
 
// score table for Mary Had A Little Lamb
{E4, 4, D4, 4, C4, 4, D4, 4, E4, 4, E4, 4, E4, 8, 
 D4, 4, D4, 4, D4, 8, E4, 4, G4, 4, G4, 8,
 E4, 4, D4, 4, C4, 4, D4, 4, E4, 4, E4, 4, E4, 8, 
 D4, 4, D4, 4, E4, 4, D4, 4, C4, 8, 0, 0 },

// score table for Twinkle Twinkle Little Stars
{C4,4,C4,4,G4,4,G4,4,A4,4,A4,4,G4,8,F4,4,F4,4,E4,4,E4,4,D4,4,D4,4,C4,8, 
 G4,4,G4,4,F4,4,F4,4,E4,4,E4,4,D4,8,G4,4,G4,4,F4,4,F4,4,E4,4,E4,4,D4,8, 
 C4,4,C4,4,G4,4,G4,4,A4,4,A4,4,G4,8,F4,4,F4,4,E4,4,E4,4,D4,4,D4,4,C4,8,0,0},

};
static inline uint8_t getDelay(uint8_t note)
{
  return Score_Tab[currentSong][note].delay;
}
static inline uint8_t getToneIndex(uint8_t note)
{
  return Score_Tab[currentSong][note].tone_index;
}

void play_a_song(void)
{
  uint8_t note = 0;
  uint8_t currentToneIndex = 0;
  uint8_t currentDelay = getToneIndex(note);

	while (currentDelay && musicOn)
  {
    currentToneIndex = getToneIndex(note);

    // Silence by disabling SysTick
    if ( currentToneIndex == PAUSE)
    {
			SysTick_stop();
    }
    // Set current note based on Tone Table
		else 
    {
      SysTick_Set_Current_Note(Tone_Tab[currentToneIndex]);
			SysTick_start();
		}
		
		// Play current note for specified duration
		for (uint8_t j = 0; j < currentDelay; j++) 
    {
			Delay();
    }
		
    // Increment note
		SysTick_stop();
    currentDelay = getDelay(note);
    note++;
  }
}

// Round robin song selection
void next_song(void)
{
  currentSong++;
  currentSong = (currentSong + 1) % 3;
}

// Getter for current music state
unsigned char is_music_on(void)
{
  return musicOn;
}

// Turn music on
void turn_off_music(void)
{
  musicOn = 0;
  SysTick_stop();
}

// Turn music off
void turn_on_music(void)
{
  musicOn = 1;
  SysTick_start();
}

// Make PA3 an output to the speaker, enable digital I/O, ensure alt. functions off
void Music_Init(void)
{ 
  volatile unsigned long delay;
  SYSCTL_RCGC2_R |= 0x01;           // 1) activate clock for Port A
  delay = SYSCTL_RCGC2_R;           // allow time for clock to start
                                    // 2) no need to unlock PA3
  GPIO_PORTA_PCTL_R &= ~0x0000F000; // 3) regular GPIO
  GPIO_PORTA_AMSEL_R &= ~0x08;      // 4) disable analog function on PA3
  GPIO_PORTA_DIR_R |= 0x08;         // 5) set direction to output
  GPIO_PORTA_AFSEL_R &= ~0x08;      // 6) regular port function
  GPIO_PORTA_DEN_R |= 0x08;         // 7) enable digital port
  GPIO_PORTA_DR8R_R |= 0x08;        // 8) optional: enable 8 mA drive on PA3 to increase the voice volumn
}

// Subroutine to wait 0.1 sec
void Delay(void){
	unsigned long volatile time;
  time = 727240*20/91;  // 0.1sec for 16MHz
  while(time)
  {
		time--;
  }
}
