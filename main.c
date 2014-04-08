#include <msp430.h>

//Bool
#include <stdbool.h>

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
#define D7	BIT7
#define DB	BIT0 | BIT1 | BIT2 | BIT3 | BIT4 | BIT5 | BIT6 | BIT7

/*
 * Functions that LCD Library Will Have
 * Function descriptions at implementation!
 */
//CORE Functions (TODO: Write some tests for these!)
void lcd_sendEnable();
void lcd_instWrite(char data);
char lcd_readBFandAC();
void lcd_dataWrite(char data);
char lcd_dataRead();

//Derived Functions
void lcd_init();
int lcd_isbusy();

/*
 * main.c
 */
int main(void) {
	WDTCTL = WDTPW | WDTHOLD;	// Stop watchdog timer

	//Check If 16MHz cali data exists
	if (CALBC1_1MHZ == 0xFF || CALDCO_1MHZ == 0xFF) {
		return 0;
	}

	// Set DCO to 16MHz
	BCSCTL1 = CALBC1_1MHZ;
	DCOCTL = CALDCO_1MHZ;

	lcd_init();

	while(1){
		;
	}
}

/*
 * lcd_readBFandAC - Core Function that
 * 	Reads the Busy flag and along with where the
 * 	address counter is currently at.
 * 	returns - char with BF | AC 6 downto 0
 */
char lcd_readBFandAC() {
	// Set direction of Pins (both RS, R/W as outputs)
	P2DIR |= RS | RW;
	// Set Direction of DataBus pins (D0 - D7) to input
	P1DIR &= (~DB);

	//Set CMD pins (RS,RW) to (0,1) for read mode
	P2OUT &= (~RS);
	P2OUT |= RW;

	//Let LCD Process Command
	lcd_sendEnable();

	return P1IN;
}

/*
 * lcd_instWrite - Core Function that
 * 	Writes to the instruction register of the
 * 	lcd controller.
 *
 * 	param - char of the data bus
 * 		-note bits for char should be like:
 * 			D7 D6 ... D0
 */
void lcd_instWrite(char data) {
	// Set direction of Pins (both RS, R/W as outputs)
	P2DIR |= RS | RW;
	// Set Direction of DataBus pins (D0 - D7) to output
	P1DIR |= DB;

	//Set both (RS and R/W) to (0,0)
	P2OUT &= (~RS) & (~RW);
	//Set the output register to data
	P1OUT = data;

	//resulting command is
	//	RS	RW	DB
	//	0	0	data(7 -> 0)

	//Send the command to the LCD display
	lcd_sendEnable();
}

/*
 * lcd_sendEnable - Toggles enable to process instruction/data
 * 	-note LCD relies on this to actually clock the data in
 * 	-note Have Data ready prior to calling this
 */
void lcd_sendEnable() {
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
 * lcd_isbusy - Busy Check
 * 	Params - int* ACaddr, passes the location of Address counter if given (optional)
 * 	Returns int true if busy!
 * 				false if ready for next instruction!
 */
int lcd_isbusy() {
	char input = lcd_readBFandAC();

	//Busy Flag is on D7, read that in
	//if it's high => busy
	if ((P2IN & D7)== D7)
		return true;
	else
		return false;
}

/*
 * lcd_init - Do the init cycle for the display
 *	Gets the display into 2 line mode and sets up
 *	Everything else
 *	TODO: allow passing in options to set them (bitwise maybe)
 */
void lcd_init() {
	//Wait for LCD to start up (~30ms)
	__delay_cycles(30000);

	//send FunctionSet cmd with 2 line mode and display on (00 0011 11XX)
	lcd_instWrite(0x3C);

	//wait until device is ready for next command
	while (lcd_isbusy() == true) {
		;
	}

	//send Display ON/OFF command with display on, cursor on, blink on (00 0000 1111)
	lcd_instWrite(0x0F);

	//wait until device is ready for next command
	while (lcd_isbusy() == true) {
		;
	}

	//send DisplayClear cmd (00 0000 0001)
	lcd_instWrite(0x01);

	//wait until device is ready for next command
	while (lcd_isbusy() == true) {
		;
	}

	//send EntryModeSet cmd with increment on, entire shift on (00 0000 0111)
	lcd_instWrite(0x07);

	//wait until device is ready for next command
	while (lcd_isbusy() == true) {
		;
	}
}
