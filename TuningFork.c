// TuningFork.c Lab 12
// Runs on LM4F120/TM4C123
// Use SysTick interrupts to create a squarewave at 440Hz.  
// There is a positive logic switch connected to PA3, PB3, or PE3.
// There is an output on PA2, PB2, or PE2. The output is 
//   connected to headphones through a 1k resistor.
// The volume-limiting resistor can be any value from 680 to 2000 ohms
// The tone is initially off, when the switch goes from
// not touched to touched, the tone toggles on/off.
//                   |---------|               |---------|     
// Switch   ---------|         |---------------|         |------
//
//                    |-| |-| |-| |-| |-| |-| |-|
// Tone     ----------| |-| |-| |-| |-| |-| |-| |---------------
//
// Daniel Valvano, Jonathan Valvano
// March 8, 2014

/* This example accompanies the book
   "Embedded Systems: Introduction to ARM Cortex M Microcontrollers",
   ISBN: 978-1469998749, Jonathan Valvano, copyright (c) 2013
   "Embedded Systems: Real Time Interfacing to ARM Cortex M Microcontrollers",
   ISBN: 978-1463590154, Jonathan Valvano, copyright (c) 2013

 Copyright 2014 by Jonathan W. Valvano, valvano@mail.utexas.edu
    You may use, edit, run or distribute this file
    as long as the above copyright notice remains
 THIS SOFTWARE IS PROVIDED "AS IS".  NO WARRANTIES, WHETHER EXPRESS, IMPLIED
 OR STATUTORY, INCLUDING, BUT NOT LIMITED TO, IMPLIED WARRANTIES OF
 MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE APPLY TO THIS SOFTWARE.
 VALVANO SHALL NOT, IN ANY CIRCUMSTANCES, BE LIABLE FOR SPECIAL, INCIDENTAL,
 OR CONSEQUENTIAL DAMAGES, FOR ANY REASON WHATSOEVER.
 For more information about my classes, my research, and my books, see
 http://users.ece.utexas.edu/~valvano/
 */


#include "TExaS.h"
#include "..//tm4c123gh6pm.h"

unsigned long Switch = 0;
unsigned char switchPressedLastTime = 0;
unsigned char toggleOrQuiet = 0;

// basic functions defined at end of startup.s
void DisableInterrupts(void); // Disable interrupts
void EnableInterrupts(void);  // Enable interrupts
void WaitForInterrupt(void);  // low power mode

// **************Sound_Init*********************
// Initialize SysTick periodic interrupts
// Input: none
// Output: none
// input from PA3, output from PA2, SysTick interrupts
void Sound_Init(void){ 
	unsigned long volatile delay;

	SYSCTL_RCGC2_R |= 0x01;
	delay = SYSCTL_RCGC2_R;
	GPIO_PORTA_DEN_R &= ~0x08;
	GPIO_PORTA_DIR_R &= ~0x08; // MAKE PA2 A OUTPUT
	GPIO_PORTA_DIR_R |= 0x04; // MAKE PA3 A INPUT
	GPIO_PORTA_AFSEL_R &= ~0x08; //DIASBLE ALT FUNCTION ON PA2 AND PA3
	GPIO_PORTA_AFSEL_R &= ~0x04;
	GPIO_PORTA_DEN_R |= 0x0C; // ENABLE DIGITAL I/O ON PA2 AND PA3
	GPIO_PORTA_AMSEL_R &= ~0x08; /// DISABLE ANALOG FUNCTIONALITY OF PA
	GPIO_PORTA_AMSEL_R &= ~0x04;
	GPIO_PORTA_PCTL_R &= ~0x0000FF00;
	NVIC_ST_CTRL_R = 0; // disable SysTick during setup
	NVIC_ST_RELOAD_R = 90908; // reload value
	NVIC_ST_CURRENT_R = 0; // any write to current clears it
	NVIC_SYS_PRI3_R = (NVIC_SYS_PRI3_R&0x00FFFFFF)|0x40000000;
	NVIC_ST_CTRL_R = 0x07;
	EnableInterrupts();
} 
// Interrupt service routine
// Executed every 880Hz
void SysTick_Handler(void){

  if (toggleOrQuiet)
		GPIO_PORTA_DATA_R ^= 0x04;     // toggle PA2 440 Hz tone output
	else
		//turn off tone
		GPIO_PORTA_DATA_R &= ~0x04; //make PA2 output low - tone off
		
}


int main(void){// activate grader and set system clock to 80 MHz
  unsigned char flag = 0;
	
  TExaS_Init(SW_PIN_PA3, HEADPHONE_PIN_PA2,ScopeOn); 
	//PLL_Init();                 // 80 MHz
  Sound_Init();        //90909 count?
  Switch = (GPIO_PORTA_DATA_R &0x08);     // this reads switch state
  while(1){
		while (Switch == 0x08 && flag ==0) {       // i defined switch as PA3
			Switch = (GPIO_PORTA_DATA_R &0x08);     // this reads switch state
			// this turns toggle off as sometimes depending on switch it could turn off high so this stops it doing that
			GPIO_PORTA_DATA_R = 0x00;  
			toggleOrQuiet =0 ;  // toggleOrQuiet = 0 means no toggle (quiet)
		}  // closed that while ...  will loop here while switch pressed and flag 0

		while (Switch == 0x00 && flag == 0) {
			Switch = (GPIO_PORTA_DATA_R &0x08);     // this reads switch state
			toggleOrQuiet =0 ;                      // toggleOrQuiet = 0 means no toggle (quiet)
		} //closed that while ...  will loop here while switch not pressed and flag 0

		flag = 1; // change flag now getting to toggle sound
		while (Switch == 0x08 && flag == 1) {
			Switch = (GPIO_PORTA_DATA_R &0x08);   // this reads switch state
			// toggleOrQuiet = 1 this will now trigger Systick_Handler and loop here till switch pressed off
			toggleOrQuiet = 1;  
		} // close bracket for this while

		while (Switch == 0x00 && flag == 1){
			Switch = (GPIO_PORTA_DATA_R &0x08);            // this reads switch state
			// toggleOrQuiet = 1 will trigger Systick_Handler and you have sound even though button not pressed 
			toggleOrQuiet = 1;     
		} // closing bracket 

		flag=0; // change flag going to loop around to while(1) loop

	} // closing bracket of first While (1) this keeps program looping

}  // int main closing bracket

