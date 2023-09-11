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
// doe ray mi fa so la ti 
// C   D   E  F  G  A  B
NTyp Score_Tab[MAX_SONGS][MAX_NOTES] = {  
// score table for Happy Birthday
{C4,2,C4,2,D4,4,C4,4,F4,4,E4,8,C4,2,C4,2,D4,4,C4,4,G4,4,F4,8,C4,2,C4,2,
 C5,4,A4,4,F4,4,E4,4,D4,8,B4,2,B4,2,A4,4,F4,4,G4,4,F4,12},
 
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

void play_a_song(void)
{
uint8_t i=0, j;
	while (Score_Tab[i]->delay) {
		if (Score_Tab[i]->tone_index==PAUSE) // index = 255 indicate a pause: stop systick
			SysTick_stop(); // silence tone, turn off SysTick timer
		else {
			SysTick_Set_Current_Note(Tone_Tab[Score_Tab[i]->tone_index]);
			SysTick_start();
		}
		
		// tempo control: 
		// play current note for duration 
		// specified in the music score table
		for (j=0;j<Score_Tab[i]->delay;j++) 
			Delay();
		
		SysTick_stop();
		i++;  // move to the next note
  }
}

void next_song(void)
{
}

unsigned char is_music_on(void)
{
  return 0;
}

void turn_off_music(void)
{
}

void turn_on_music(void)
{
}

// Make PA3 an output to the speaker, enable digital I/O, ensure alt. functions off
void Music_Init(void){ 
}

// Subroutine to wait 0.1 sec
// Inputs: None
// Outputs: None
// Notes: ...
void Delay(void){
	unsigned long volatile time;
  time = 727240*20/91;  // 0.1sec for 16MHz
//  time = 727240*100/91;  // 0.1sec for 80MHz
  while(time){
		time--;
  }
}
