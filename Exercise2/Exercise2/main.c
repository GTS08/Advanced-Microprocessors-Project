#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>

#define del 1
#define ped 20

// Logic flags
int pedInterr = 0;
int timerInterr = 0;

// PIN0 = BIG ROAD TRAFFIC LIGHT
// PIN1 = PEDESTRIAN TRAFFIC LIGHT
// PIN2 = SMALL ROAD TRAFFIC LIGHT

// Function declarations
void setBigRoadRedAndSmallRoadGreen();
void setBigRoadGreenAndSmallRoadRed();
void setPedestrianTimer();

int main()
{
	PORTD.DIR |=  PIN0_bm ; // PIN0 is output
	PORTD.DIR |=  PIN1_bm ; // PIN1 is output
	PORTD.DIR |=  PIN2_bm ; // PIN2 is output
	
	PORTD.OUTCLR = PIN0_bm; // LED0 is on (Green Light)
	PORTD.OUT |= PIN1_bm; // LED1 is off (Red Light)
	PORTD.OUT |= PIN2_bm; // LED2 is off (Red Light)
	
	// Pull up enable and Interrupt enabled with sense on both edges
	PORTF.PIN5CTRL |= PORT_PULLUPEN_bm | PORT_ISC_BOTHEDGES_gc;
	
	// Begin accepting interrupt signals
	sei();
	
	// Initialize random number generator
	srand(time(NULL));
	
	// Initialize variables
	int randVal;
	int thereIsACar;
	
	// Main while loop
	while (1)
	{
		// Check if there is a car on the small road (simulate sensor)
		randVal = rand();
		if ((randVal % 10 == 0) || (randVal % 10 == 5) || (randVal % 10 == 8))
		{
			thereIsACar = 1;
		}
		else
		{
			thereIsACar = 0;
		}
		
		// Check if a pedestrian pressed the button 
		if (pedInterr)
		{
			setBigRoadRedAndSmallRoadGreen();
			setPedestrianTimer();
		}
		else
		{
			if (thereIsACar)
			{
				setBigRoadRedAndSmallRoadGreen();
			}
			else
			{
				setBigRoadGreenAndSmallRoadRed();
			}
		}
	}
	
	// Disable interrupts
	cli();
}

void setBigRoadRedAndSmallRoadGreen()
{
	PORTD.OUTCLR = PIN2_bm; // LED2 is on (Green Light)
	PORTD.OUT |= PIN0_bm; // LED0 is off (Red Light)
}

void setBigRoadGreenAndSmallRoadRed()
{
	PORTD.OUTCLR = PIN0_bm; // LED0 is on (Green Light)
	PORTD.OUT |= PIN2_bm; // LED1 is off (Red Light)
}

void setPedestrianTimer()
{
	PORTD.OUTCLR = PIN1_bm; // LED1 is on (Green Light)
	_delay_ms(del); // Small delay
	
	// 16-bit counter high and low
	TCA0.SINGLE.CNT = 0; // Clear counter
	TCA0.SINGLE.CTRLB = 0; // Normal Mode (TCA_SINGLE_WGMODE_NORMAL_gc)
	TCA0.SINGLE.CMP0 = ped; // When reaches this value -> interrupt CLOCK FREQUENCY/1024
	TCA0.SINGLE.CTRLA = TCA_SINGLE_CLKSEL_DIV1024_gc; // (= 0x7<<1)
	TCA0.SINGLE.CTRLA |= 1; // Enable
	TCA0.SINGLE.INTCTRL = TCA_SINGLE_CMP0_bm; // Interrupt Enable (=0x10)
	
	while (timerInterr == 0)
	{
	}
	PORTD.OUT |= PIN1_bm; // LED1 is off (Red Light)
	
	// Reset flags
	timerInterr = 0;
	pedInterr = 0;
}


ISR(PORTF_PORT_vect)
{
	// Clear the interrupt flag
	int intflags = PORTF.INTFLAGS;
	PORTF.INTFLAGS=intflags;
	
	// Set flag
	pedInterr = 1;
}
ISR(TCA0_CMP0_vect){
	// Disable
	TCA0.SINGLE.CTRLA = 0;
	// Clear flag
	int intflags = TCA0.SINGLE.INTFLAGS;
	TCA0.SINGLE.INTFLAGS = intflags;
	
	// Set flag
	timerInterr = 1;
}