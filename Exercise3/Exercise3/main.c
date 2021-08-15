#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>

#define del 1
#define ped 4

void moveForward();
void turnLeft();
void turnRight();
void turnAround();

// Logic flags
int timerInterr = 0;
int switchInterr1 = 0;
int switchInterr2 = 0;
int adcInterr = 0;
int corners = 0;

int main()
{
	PORTD.DIR |=  PIN1_bm ; // PIN1 is output
	PORTD.DIR |=  PIN2_bm ; // PIN2 is output
	PORTD.DIR |=  PIN3_bm ; // PIN3 is output
	
	PORTD.OUT |= PIN1_bm; // LED1 is off
	PORTD.OUT |= PIN2_bm; // LED2 is off
	PORTD.OUT |= PIN3_bm; // LED3 is off

	// Begin accepting interrupt signals
	sei();
	
	// Pull up enable and Interrupt enabled with sense on both edges
	PORTB.PIN5CTRL |= PORT_PULLUPEN_bm | PORT_ISC_BOTHEDGES_gc;
	
	// Pull up enable and Interrupt enabled with sense on both edges
	PORTF.PIN5CTRL |= PORT_PULLUPEN_bm | PORT_ISC_BOTHEDGES_gc;
	
	// Initialize the ADC for Free-Running mode
	ADC0.CTRLA |= ADC_RESSEL_10BIT_gc; // 10-bit resolution
	ADC0.CTRLA |= ADC_FREERUN_bm; // Free-Running mode enabled
	ADC0.CTRLA |= ADC_ENABLE_bm; // Enable ADC
	ADC0.MUXPOS |= ADC_MUXPOS_AIN7_gc; // The bit
	// Enable Debug Mode
	ADC0.DBGCTRL |= ADC_DBGRUN_bm;
	// Window Comparator Mode
	ADC0.WINLT |= 10; // Set threshold
	ADC0.INTCTRL |= ADC_WCMP_bm; // Enable Interrupts for WCM
	ADC0.CTRLE |= ADC_WINCM0_bm; // Interrupt when RESULT < WINLT

	ADC0.COMMAND |= ADC_STCONV_bm; // Start Conversion
	
	moveForward();
	
	// Main while loop (terminates when machine reaches the initial spot)
	while(corners < 8 && corners > -1)
	{
		// If turn around switch is pressed
		if (switchInterr2)
		{
			if (adcInterr)
			{
				turnRight();
				corners--;
				moveForward();
			}
			else if (switchInterr1)
			{
				turnLeft();
				corners--;
				moveForward();
			}
		}
		else
		{
			if (adcInterr)
			{
				turnLeft();
				corners++;
				moveForward();
			}
			else if (switchInterr1)
			{
				turnRight();
				corners++;
				moveForward();
			}
		}
	}
}void moveForward(){	PORTD.OUTCLR = PIN1_bm; // LED1 is on}void turnLeft(){	PORTD.OUTCLR = PIN2_bm; // LED2 is on	_delay_ms(del); // Small delay
	
	// 16-bit counter high and low
	TCA0.SINGLE.CNT = 0; // Clear counter
	TCA0.SINGLE.CTRLB = 0; // Normal Mode (TCA_SINGLE_WGMODE_NORMAL_gc)
	TCA0.SINGLE.CMP0 = ped; // When reaches this value -> interrupt CLOCK FREQUENCY/1024
	TCA0.SINGLE.CTRLA = TCA_SINGLE_CLKSEL_DIV1024_gc; // (= 0x7<<1)
	TCA0.SINGLE.CTRLA |= 1; // Enable
	TCA0.SINGLE.INTCTRL = TCA_SINGLE_CMP0_bm; // Interrupt Enable (=0x10)
	
	while (timerInterr == 0)
	{
		
	}		PORTD.OUT |= PIN2_bm; // LED2 is off		// Reset flag
	timerInterr = 0;	adcInterr = 0;}void turnRight(){		PORTD.OUTCLR = PIN3_bm; // LED3 is on	_delay_ms(del); // Small delay
	
	// 16-bit counter high and low
	TCA0.SINGLE.CNT = 0; // Clear counter
	TCA0.SINGLE.CTRLB = 0; // Normal Mode (TCA_SINGLE_WGMODE_NORMAL_gc)
	TCA0.SINGLE.CMP0 = ped; // When reaches this value -> interrupt CLOCK FREQUENCY/1024
	TCA0.SINGLE.CTRLA = TCA_SINGLE_CLKSEL_DIV1024_gc; // (= 0x7<<1)
	TCA0.SINGLE.CTRLA |= 1; // Enable
	TCA0.SINGLE.INTCTRL = TCA_SINGLE_CMP0_bm; // Interrupt Enable (=0x10)
	
	while (timerInterr == 0)
	{
		
	}		PORTD.OUT |= PIN3_bm; // LED3 is off		// Reset flag
	timerInterr = 0;	switchInterr1 = 0;}void turnAround(){	PORTD.OUTCLR = PIN1_bm; // LED1 is on	PORTD.OUTCLR = PIN2_bm; // LED2 is on	PORTD.OUTCLR = PIN3_bm; // LED2 is on		_delay_ms(del); // Small delay
	
	// 16-bit counter high and low
	TCA0.SINGLE.CNT = 0; // Clear counter
	TCA0.SINGLE.CTRLB = 0; // Normal Mode (TCA_SINGLE_WGMODE_NORMAL_gc)
	TCA0.SINGLE.CMP0 = ped; // When reaches this value -> interrupt CLOCK FREQUENCY/1024
	TCA0.SINGLE.CTRLA = TCA_SINGLE_CLKSEL_DIV1024_gc; // (= 0x7<<1)
	TCA0.SINGLE.CTRLA |= 1; // Enable
	TCA0.SINGLE.INTCTRL = TCA_SINGLE_CMP0_bm; // Interrupt Enable (=0x10)
	
	while (timerInterr == 0)
	{
		
	}		PORTD.OUT |= PIN1_bm; // LED1 is off	PORTD.OUT |= PIN2_bm; // LED2 is off	PORTD.OUT |= PIN3_bm; // LED3 is off		// Reset flag
	timerInterr = 0;	}// Timer interruptISR(TCA0_CMP0_vect){
	// Disable
	TCA0.SINGLE.CTRLA = 0;
	// Clear flag
	int intflags = TCA0.SINGLE.INTFLAGS;
	TCA0.SINGLE.INTFLAGS = intflags;
	
	// Set flag
	timerInterr = 1;
}// Switch interrupt 1ISR(PORTF_PORT_vect)
{
	// Clear the interrupt flag
	int intflags = PORTF.INTFLAGS;
	PORTF.INTFLAGS=intflags;
	
	// Set flag
	switchInterr1 = 1;
}

// Switch interrupt 2ISR(PORTB_PORT_vect)
{
	// Clear the interrupt flag
	int intflags = PORTB.INTFLAGS;
	PORTB.INTFLAGS=intflags;
	
	// Set flag
	switchInterr2 = 1;
	
	// Decrease Counter
	corners--;
}

// ADC interruptISR(ADC0_WCOMP_vect)
{
	int intflags = ADC0.INTFLAGS;
	ADC0.INTFLAGS = intflags;
	
	// Set flag
	adcInterr = 1;
}
