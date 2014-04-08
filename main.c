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
void lcd_print(char* string);
int lcd_isbusy();
void lcd_setCursor(int x, int y);

/*
 * main.c
 */
int main(void) {
	WDTCTL = WDTPW | WDTHOLD;	// Stop watchdog timer

	//Check If 16MHz cali data exists
	if (CALBC1_1MHZ == 0xFF || CALDCO_1MHZ == 0xFF) {
		return 0;
	}

	// Set DCO to 1MHz
	BCSCTL1 = CALBC1_1MHZ;
	DCOCTL = CALDCO_1MHZ;

	lcd_init();

	lcd_print("Hello");

	lcd_setCursor(0,1);

	lcd_print("World!!!");


	while (1) {
		;
	}
}

/*
 * lcd_setcursor - set the location for the cursor
 * 	params int x - x location of desired location
 * 	params int y - y location of desired location
 *
 * 	here's what screen looks like:
 * 		x->
 * 		y (0,0) (0,1) (0,2) ... (0, 16)
 * 		| (1,0) (1,1) (1,2) ... (1, 16)
 * 		v
 */
void lcd_setCursor(int x, int y){
	//TODO: make this less hacky
	if(y == 0){
		//top row

		//force cursor to top left
		lcd_instWrite(0x80);

		//shift over required spaces
		while (x < 0){
			lcd_instWrite(0x14);
			--x;
		}
	}
	else{
		//bottom row

		//force cursor to top left
		lcd_instWrite(0xC0);

		//shift over required spaces
		while (x < 0){
			lcd_instWrite(0x14);
			--x;
		}
	}
}

/*
 * lcd_dataWrite - Core Function
 * 	Writes the data given to ram of lcd
 */
void lcd_dataWrite(char data) {
	// Set direction of Pins (both RS, R/W as outputs)
	P2DIR |= RS | RW;
	// Set Direction of DataBus pins (D0 - D7) to output
	P1DIR |= DB;

	//Set (RS and R/W) to (1,0)
	P2OUT |= RS;
	P2OUT &= (~RW);
	//Set the output register to data
	P1OUT = data;

	//resulting command is
	//	RS	RW	DB
	//	1	0	data(7 -> 0)

	//Send the command to the LCD controller
	lcd_sendEnable();

	//wait 50us for command to process
	__delay_cycles(50);
}

/*
 * lcd_readBFandAC - Core Function
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

	//Let LCD Process Controller
	lcd_sendEnable();

	//wait 50us for command to process
	__delay_cycles(50);

	return P1IN;
}

/*
 * lcd_instWrite - Core Function
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

	//wait 50us for command to process
	__delay_cycles(50);
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

	//Hold for atleast 230 ns (~1cycle at 1Mhz)
	__delay_cycles(4); //4 just to be safe

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
 * lcd_print(char* str) - print string to lcd
 * 	param - c string of characters
 * 	-note relies on \0 to know when string is done
 */
void lcd_print(char* str) {
	int i = 0;
	while (str[i] != '\0') {
		//send over the data
		lcd_dataWrite(str[i]);
		//shift cursor to the right
		lcd_instWrite(0x06);
		//increment the pointer's address
		i++;
	}
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

	//send Display ON/OFF command with display on, cursor on, blink on (00 0000 1111)
	lcd_instWrite(0x0F);

	//send DisplayClear cmd (00 0000 0001)
	lcd_instWrite(0x01);

	__delay_cycles(2000);

	//send goHome cmd (00 0000 0010)
	lcd_instWrite(0x02);

	while(lcd_isbusy() == 1){
		;
	}
}
