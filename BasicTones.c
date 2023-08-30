// This is an example program to show music programming on the LaunchPad.
// You can run this program without modification.
// The program will play 8 basic notes in selected C scale: Happy Birthday, 
// doe ray mi fa so la ti 
// C   D   E  F  G  A  B
// in forward and backward order and repeat forever.
// Hardware connection: 
// Positive logic Speaker/Headset is connected to PA2.
// Authors: Min He
// Date: August 28, 2018

// 1. Pre-processor Directives Section
// Constant declarations to access port registers using 
// symbolic names instead of addresses

// add port F 
// add both edge interrupts
// add global scope index to tell what note you're on


#include "tm4c123gh6pm.h"
#include "SysTick.h"

#define TEMPO 2 // each tone uses the same duration 

// 2. Declarations Section
// Constants

// initial values for piano major tones: 
// Assume system clock is 16MHz.
const unsigned long Tone_Tab[] = 
// Note name: C, D, E, F, G, A, B, C'
// Offset:0, 1, 2, 3, 4, 5, 6, 7
{30534,27211,24242,22923,20408,18182,16194,15289}; // C4 Major notes
//{15289,13621,12135,11454,10204,9091,8099,7645}; // C5 Major notes 
//{7645,6810,6067,5727,5102,4545,4050,3822}; // C6 Major notes 

//   Function Prototypes
void Speaker_Init(void);
void Delay(void);
extern void EnableInterrupts(void);
void GPIOPortF_Handler(void);
void Switch_Init(void);
extern void WaitForInterrupt(void);
// 3. Subroutines Section
// MAIN: Mandatory for a C Program to be executable
int main(void){
  unsigned char i,j;
	Switch_Init(); //port F init
  Speaker_Init();
  SysTick_Init();
  EnableInterrupts();  // SysTick uses interrupts
  while(1){
		// start with first note and goes forwards
		for (i=0;i<8;i++) {
	    j=0;
		  // load the inital value for current note
		  SysTick_Set_Current_Note(Tone_Tab[i]);
		  SysTick_start();
	    while (j++<TEMPO) // play the note for specified duration: tempo control
	      Delay();
			SysTick_stop();
	  }
	  // i=8 or i--;
		// start with last note and goes backwards
	  for (;i>0;i--) {  // i=8 to 1
	    j=0;
		  // load the inital value for current note
		  SysTick_Set_Current_Note(Tone_Tab[i-1]); //grabs notes from tone table
		  SysTick_start();
	    while (j++<TEMPO) // play the note for specified duration 
	      Delay();
			SysTick_stop(); //exits that note
	  }
		//press SW1 or SW2
		WaitForInterrupt();
  }
}

// Subroutine to wait 0.1 sec
// Inputs: None
// Outputs: None
// Notes: ...
void Delay(void){
	unsigned long volatile time;
  time = 727240*50/91;  // 0.1sec
  while(time){
		time--;
  }
}
// Make PA2 an output, enable digital I/O, ensure alt. functions off
void Speaker_Init(void){ volatile unsigned long delay;
  SYSCTL_RCGC2_R |= 0x01;           // 1) activate clock for Port A
  delay = SYSCTL_RCGC2_R;           // allow time for clock to start
                                    // 2) no need to unlock PA2
  GPIO_PORTA_PCTL_R &= ~0x00000F00; // 3) regular GPIO
  GPIO_PORTA_AMSEL_R &= ~0x04;      // 4) disable analog function on PA2
  GPIO_PORTA_DIR_R |= 0x04;         // 5) set direction to output
  GPIO_PORTA_AFSEL_R &= ~0x04;      // 6) regular port function
  GPIO_PORTA_DEN_R |= 0x04;         // 7) enable digital port
  GPIO_PORTA_DR8R_R |= 0x04;        // 8) optional: enable 8 mA drive on PA2 to increase the voice volumn
}
void GPIOPortF_Handler(void){
	unsigned char i,j;
	unsigned long sw1,sw2;
	i = 0;
	while(1){
		sw1 = GPIO_PORTF_DATA_R & 0x10;
		sw2 = GPIO_PORTF_DATA_R & 0x01;
		if(sw1 == 0x00){ // sw1 is pressed
			i++;
			if (i > 7){
				i = 0;
			}
		  // load the inital value for current note
			SysTick_Set_Current_Note(Tone_Tab[i]);
			SysTick_start();
			Delay();
			SysTick_stop();
			}
		else if(sw2 == 0x00){ //sw2 is pressed
			if (i == 0){
				i = 7;
			}
			else
				i--;
		  // load the inital value for current note
			SysTick_Set_Current_Note(Tone_Tab[i]); //grabs notes from tone table
			SysTick_start();
			Delay();
			SysTick_stop(); //exits that not
			}
		}
	}	
// Initialize the	onboard two switches. PORT F
void Switch_Init(void){
	SYSCTL_RCGCGPIO_R |= 0x20;
	SYSCTL_RCGC2_R |= SYSCTL_RCGC2_GPIOF;	// Activate F clocks
	while ((SYSCTL_RCGC2_R&SYSCTL_RCGC2_GPIOF)==0){};
	GPIO_PORTF_LOCK_R = 0x4C4F434B;   // unlock GPIO Port F
	GPIO_PORTF_CR_R = 0x1F;           // allow changes to PF4-0
  // only PF0 needs to be unlocked, other bits can't be locked
  GPIO_PORTF_AMSEL_R &= ~0x1F;      // 3) disable analog function
  GPIO_PORTF_PCTL_R &= ~0x0000FFFF; // 4) GPIO clear bit PCTL 
	GPIO_PORTF_DIR_R &= ~0x1F;        //	6) PF0-PF4 input/switches		
  GPIO_PORTF_DIR_R |= 0x0E;         // 6) PF1-PF3 output/ LEDs
  GPIO_PORTF_AFSEL_R &= ~0x1F;      // 7) no alternate function     
  GPIO_PORTF_DEN_R |= 0x1F;         // 8) enable digital pins PF4-PF0
  GPIO_PORTF_PUR_R |= 0x11;     		//     enable weak pull-up on PF4 and PF0
  GPIO_PORTF_IS_R &= ~0x11;     		// PF4 and PF0 are edge-sensitive
  GPIO_PORTF_IBE_R |= 0x11;    		//     PF4 and PF0 are both edges
  GPIO_PORTF_IEV_R |= 0x11;   			//     PF4 and PF0 falling edge event
  GPIO_PORTF_ICR_R |= 0x11;      		// clear flag4
  GPIO_PORTF_IM_R |= 0x11;      		//  arm interrupt on PF4 and PF0
  NVIC_PRI7_R = (NVIC_PRI7_R&0xFF1FFFFF)|0x00A00000; // (g) priority 5
  NVIC_EN0_R |= 0x40000000;      // (h) enable interrupt 30 in NVIC 	
}