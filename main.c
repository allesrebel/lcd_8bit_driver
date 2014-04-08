#include <msp430.h>

//Bool
#define bool 	char
#define true 	1
#define false 	0

/*
 * TODO:	Should make the library take in a clock speed and base timings
 * 			on that. (calculations could be done in lcd_init)
 */

/*	------Pin Map------
 * LCD Pins		- MSP430 Pins
 * Enable Pin 	- P2.0
 * R/W Pin		- P2.1
 * RS Pin		- P2.2
 * D0 Pin		- P1.0
 * D1 Pin		- P1.1
 * D2 Pin		- P1.2
 * D3 Pin		- P1.3
 * D4 Pin		- P1.4
 * D5 Pin		- P1.5
 * D6 Pin		- P1.6
 * D7 Pin		- P1.7
 */

/*
 * Aliases of the important control sigs
 */
#define	RS 	BIT2
#define	RW	BIT1
#define EN	BIT0

/*
 * Functions that LCD Library Will Have
 */

/*
 * Busy Check -
 * 	Params - int* ACaddr, passes the location of Address counter if given (optional)
 * 	Returns true if busy!
 * 	Return false if ready for next instruction!
 */
int lcd_isbusy(int* ac_addr) {
	// Set direction of Pins (both RS, R/W as outputs)
	P2DIR |= RS | RW;
	return true;
}

/*
 * sendEnable - Toggles enable to process instruction/data
 * 	-note LCD relies on this to actually clock the data in
 * 	-note Have Data ready prior to calling this
 */
void sendEnable() {
	//Set Enable to output
	P2DIR |= EN;
	//Set Enable to output logical HIGH
	P2OUT |= EN;

	//Hold for atleast 230 ns (~4cycles at 16Mhz)
	__delay_cycles(4);

	//Set Enable to output Logical LOW
	P2OUT &= (~EN);
}

/*
 * Do the init cycle for the display
 *	Gets the display into 2 line mode and sets up
 *	Everything else
 */
void init_display() {

}

/*
 * main.c
 */
int main(void) {
	WDTCTL = WDTPW | WDTHOLD;	// Stop watchdog timer

	//Check If 16MHz cali data exists
	if (CALBC1_16MHZ == 0xFF || CALDCO_16MHZ == 0xFF) {
		return 0;
	}
	// Set DCO to 16MHz
	BCSCTL1 = CALBC1_16MHZ;
	DCOCTL = CALDCO_16MHZ;

	return 0;
}
