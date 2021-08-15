#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>

#define N 10

// Functions
int randomValue();
void storeValue(int ar[], int randVal);
int readLastValue(int ar[]);
void outputLED(int n);

// Variables
int arr[N] = {-1, -1, -1, -1, -1, -1, -1, -1, -1, -1}; // Array to store values
int randVal = 0;
int lastVal = 0;

// Logic flags
int switchInterr5 = 0;
int switchInterr6 = 0;
int cmp0Interr = 0;
int cmp1_ovf_Interr = 0;

int main(){
	PORTD.DIR |=  PIN0_bm ; // PIN0 is output
	PORTD.DIR |=  PIN1_bm ; // PIN1 is output
	PORTD.DIR |=  PIN2_bm ; // PIN2 is output
	PORTD.DIR |=  PIN3_bm ; // PIN3 is output
	
	PORTD.OUT |= PIN0_bm; // LED0 is off
	PORTD.OUT |= PIN1_bm; // LED1 is off
	PORTD.OUT |= PIN2_bm; // LED2 is off
	PORTD.OUT |= PIN3_bm; // LED3 is off
	
	// Begin accepting interrupt signals
	sei();
	
	// Pull up enable and Interrupt enabled with sense on both edges
	PORTB.PIN5CTRL |= PORT_PULLUPEN_bm | PORT_ISC_BOTHEDGES_gc;
	
	// Pull up enable and Interrupt enabled with sense on both edges
	PORTF.PIN5CTRL |= PORT_PULLUPEN_bm | PORT_ISC_BOTHEDGES_gc;
	
	// Initialize random number generator
	srand(time(NULL));

	PORTD.DIR |= PIN1_bm; // PIN is output
	// Prescaler = 1024
	TCA0.SINGLE.CTRLA=TCA_SINGLE_CLKSEL_DIV1024_gc;
	TCA0.SINGLE.PER = 54; // Select the resolution
	TCA0.SINGLE.CMP0 = 27; // Select the duty cycle
	TCA0.SINGLE.CMP1 = 10; // Select the duty cycle
	// Select Single_Slope_PWM
	TCA0.SINGLE.CTRLB |= TCA_SINGLE_WGMODE_SINGLESLOPE_gc;
	// Enable interrupt Overflow
	TCA0.SINGLE.INTCTRL = TCA_SINGLE_OVF_bm;
	// Enable interrupt COMP0
	TCA0.SINGLE.INTCTRL |= TCA_SINGLE_CMP0_bm;
	// Enable interrupt COMP1
	TCA0.SINGLE.INTCTRL |= TCA_SINGLE_CMP1_bm;
	TCA0.SINGLE.CTRLA |= TCA_SINGLE_ENABLE_bm; //Enable
	
	// Main while loop
	while (1)
	{
		if (switchInterr5 && cmp1_ovf_Interr)
		{
			randVal = randomValue();
			storeValue(arr, randVal);
		}
		else if (switchInterr6 && cmp0Interr)
		{
			lastVal = readLastValue(arr);
			outputLED(lastVal);
		}
	}
}

int randomValue()
{
	int val = rand() % 16;
	
	
	
	return val;
}

void storeValue(int ar[], int randVal)
{
	for (int i = 0; i < N; i++)
	{
		if (ar[i] == -1)
		{
			ar[i] = randVal;
			break;
		}
	}
	
	// Reset flags
	cmp1_ovf_Interr = 0;
	switchInterr5 = 0;
}

int readLastValue(int ar[])
{
	int val = 0;
	for (int i = 0; i < N; i++)
	{
		if ((ar[i] == -1) && (i != 0))
		{
			val = ar[i-1];
			break;
		}
	}
	

	
	
	return val;
}

void outputLED(int n)
{
	int binArr[] = {0, 0, 0, 0};
	int bin = 0;
	int rem, i = 1, j=0;
	while (n != 0)
	{
		rem = n % 2;
		n /= 2;
		bin += rem * i;
		i *= 10;
		
		binArr[j] = rem;
		j++;
	}
	
	// Reset LEDs
	PORTD.OUT |= PIN0_bm; // LED0 is off
	PORTD.OUT |= PIN1_bm; // LED1 is off
	PORTD.OUT |= PIN2_bm; // LED2 is off
	PORTD.OUT |= PIN3_bm; // LED3 is off
	
	// Switch LEDs
	if (binArr[0] == 1)
	{
		PORTD.OUTCLR = PIN0_bm; // LED0 is on
	}
	if (binArr[1] == 1)
	{
		PORTD.OUTCLR = PIN1_bm; // LED1 is on
	}
	if (binArr[2] == 1)
	{
		PORTD.OUTCLR = PIN2_bm; // LED2 is on
	}
	if (binArr[3] == 1)
	{
		PORTD.OUTCLR = PIN3_bm; // LED3 is on
	}
	
	// Reset flags
	cmp0Interr = 0;
	switchInterr6 = 0;
	
}

// Overflow interrupt (rising edge)ISR(TCA0_OVF_vect){
	// Clear the interrupt flag
	int intflags = TCA0.SINGLE.INTFLAGS;
	TCA0.SINGLE.INTFLAGS = intflags;
	
	// Set flag
	cmp1_ovf_Interr = 1;
}// CMP0 interrupt (falling edge)ISR(TCA0_CMP0_vect){
	// Clear the interrupt flag
	int intflags = TCA0.SINGLE.INTFLAGS;
	TCA0.SINGLE.INTFLAGS = intflags;
	
	// Set flag
	cmp0Interr = 1;
}

// CMP1 interrupt (falling edge)ISR(TCA0_CMP1_vect){
	// Clear the interrupt flag
	int intflags = TCA0.SINGLE.INTFLAGS;
	TCA0.SINGLE.INTFLAGS = intflags;
	
	// Set flag
	cmp1_ovf_Interr = 1;
}

// Switch interrupt 5ISR(PORTF_PORT_vect)
{
	// Clear the interrupt flag
	int intflags = PORTF.INTFLAGS;
	PORTF.INTFLAGS=intflags;
	
	// Set flag
	switchInterr5 = 1;
}

// Switch interrupt 6ISR(PORTB_PORT_vect)
{
	// Clear the interrupt flag
	int intflags = PORTB.INTFLAGS;
	PORTB.INTFLAGS=intflags;
	
	// Set flag
	switchInterr6 = 1;
}
