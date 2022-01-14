/*
  TILT!Audio extension script to control a IO22D08 board based on werner rothschopf's work
  (original copyright below).
  Modifications by S. Rinke:
  - added Wire.h for I2C control
  - removed the IO22D08Timer extension class that was only used for timed relay switching (not used for now)
  article about this script see
  https://werner.rothschopf.net/microcontroller/202104_arduino_pro_mini_relayboard_IO22D08_en.htm
  --------
  IO22D08
  DC 12V 8 Channel Pro mini PLC Board Relay Shield Module
  for Arduino Multifunction Delay Timer Switch Board

  Hardware:
  4 bit Common Cathode Digital Tube Module (two shift registers)
  8 relays (one shift register)
  8 optocoupler
  4 discrete input keys (to GND)

  ---Segment Display Screen----
  --A--
  F---B
  --G--
  E---C
  --D--
   __  __   __  __
  |__||__|.|__||__|
  |__||__|'|__||__|
  ----------------------

  available on Aliexpress: https://s.click.aliexpress.com/e/_A0tJEK

  some code parts based on the work of
  cantone-electonics
  http://www.canton-electronics.com

  this version

  by noiasca
  2021-04-01 OOP (2340/122)
  2021-03-31 initial version (2368/126)
  1999-99-99 OEM Version (2820/101)
*/

#include <Wire.h>

#define I2C_MSG_IN_SIZE 3
#define I2C_MSG_OUT_SIZE 2
#define I2C_ADDRESS 0x5E
volatile uint8_t sendBuffer[I2C_MSG_OUT_SIZE];

// Pin connected to latch of Digital Tube Module
// ST Store
// de: Der Wechsel von Low auf High kopiert den Inhalt des Shift Registers in das Ausgaberegister bzw. Speicherregister
const uint8_t latchPin = A2;
// Pin connected to clock of Digital Tube Module
// SH clock Shift Clock Pin
// de: Übernahme des Data Signals in das eigentliche Schieberegister
const uint8_t clockPin = A3;
// Pin connected to data of Digital Tube Module
const uint8_t dataPin = 13;
// Pin connected to 595_OE of Digital Tube Module
// Output Enable to activate outputs Q0 – Q7  - first device: Relay IC
const uint8_t OE_595 = A1;
// A4 - unused - not connected - I2C SDA - can be used for an additional LCD display
// A5 - unused - not connected - I2C SCL - can be used for an additional LCD display
// A6 - unused - not connected
// A7 - unused - not connected
const uint8_t optoInPin[]{ 2, 3, 4, 5, 6, A0, 12, 11 }; // the input GPIO's with optocoupler - LOW active
const uint8_t keyInPin[]{ 7, 8, 9, 10 };                // the input GPIO's with momentary buttons - LOW active
const uint8_t noOfOptoIn = sizeof( optoInPin );         // calculate the number of opto inputs
const uint8_t noOfKeyIn = sizeof( keyInPin );           // calculate the number of discrete input keys
const uint8_t noOfRelay = 8;                            // relays on board driven connected to shift registers
byte key_value;                                         // the last pressed key

// the base class implements the basic functionality
// which should be the same for all sketches with this hardware:
// begin()     init the hardware
// setNumber() send an integer to the internal display buffer
// pinWrite()  switch on/off a relay
// update()    refresh the display
// tick()      keep internals running
class IO22D08 {
protected:
	uint8_t dat_buf[4];          // the display buffer - reduced to 4 as we only have 4 digits on this board
	uint8_t relay_port;          // we need to keep track of the 8 relays in this variable
	uint8_t com_num;             // Digital Tube Common - actual digit to be shown
	uint32_t previousMillis = 0; // time keeping for periodic calls
	bool TUBE_ON[4] = { 1, 1, 1, 1 };

	// low level HW access to shift registers
	// including mapping of pins
	void update() {
		static const uint8_t TUBE_NUM[4] = { 0xfe, 0xfd, 0xfb, 0xf7 }; // Tuble bit number - the mapping to commons
		// set to FF to switch digit off
		// currently only the first 10 characters (=numbers) are used, but I keep the definitions
		//        NO.:0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22 23 24 25 26 27 28*/
		// Character :0,1,2,3,4,5,6,7,8,9,A, b, C, c, d, E, F, H, h, L, n, N, o, P, r, t, U, -,  ,*/
		const uint8_t TUBE_SEG[29] = {
		    0xc0, 0xf9, 0xa4, 0xb0, 0x99, 0x92, 0x82, 0xf8, 0x80, 0x90, // 0 .. 9
		    0x88, 0x83, 0xc6, 0xa7, 0xa1, 0x86, 0x8e, 0x89, 0x8b, 0xc7, // A, b, C, c, d, E, F, H, h, L,
		    0xab, 0xc8, 0xa3, 0x8c, 0xaf, 0x87, 0xc1, 0xbf, 0xff };     // n, N, o, P, r, t, U, -,  ,
		uint8_t tube_dat; // Common Cathode Digital Tube, bit negated - sum of all segments to be activated
		uint8_t bit_num;  // digital tube common gemappt auf tuble bit number
		uint8_t display_l, display_h, relay_dat; // three databytes of payload to be shifted to the shift registers
		if( com_num < 3 )
			com_num++;
		else
			com_num = 0;                  // next digit
		uint8_t dat = dat_buf[com_num]; // Data to be displayed
		tube_dat = TUBE_SEG[dat];       // Common Cathode Digital Tube, bit negated - sum of all segments
		bit_num = TUBE_ON[com_num] ? ~TUBE_NUM[com_num] : 0; // digital tube common gemappt auf tuble bit number
		display_l = ( ( tube_dat & 0x10 ) >> 3 );            // Q4   <-D1 -3    SEG_E
		display_l |= ( ( bit_num & 0x01 ) << 2 );            // DIGI0<-D2 +2
		display_l |= ( ( tube_dat & 0x08 ) >> 0 );           // Q3   <-D3 0     SEG_D
		display_l |= ( ( tube_dat & 0x01 ) << 4 );           // Q0   <-D4 -4    SEG_A
		display_l |= ( ( tube_dat & 0x80 ) >> 2 );           // Q7   <-D5 -2    SEG_DP - Colon - only on digit 1 ?
		display_l |= ( ( tube_dat & 0x20 ) << 1 );           // Q5   <-D6 1     SEG_F
		display_l |= ( ( tube_dat & 0x04 ) << 5 );           // Q2   <-D7 5     SEG_C
		// output U3-D0 is not connected,
		// on the schematic the outputs of the shiftregisters are internally marked with Q, here we use U3-D to refeer to
		// the latched output)
		display_h = ( ( bit_num & 0x02 ) >> 0 );   // DIGI1<-D1 0
		display_h |= ( ( bit_num & 0x04 ) >> 0 );  // DIGI2<-D2 0
		display_h |= ( ( tube_dat & 0x40 ) >> 3 ); // Q6   <-D3 -3    SEG_G
		display_h |= ( ( tube_dat & 0x02 ) << 3 ); // Q1   <-D4 3     SEG_B
		display_h |= ( ( bit_num & 0x08 ) << 2 );  // DIGI3<-D5 2
		// Outputs U4-D0, U4-D6 and U4-D7 are not connected

		relay_dat = ( ( relay_port & 0x7f ) << 1 ); // map Pinout 74HC595 to ULN2803: 81234567
		relay_dat = relay_dat | ( ( relay_port & 0x80 ) >> 7 );

		// ground latchPin and hold low for as long as you are transmitting
		digitalWrite( latchPin, LOW );
		// as the shift registers are daisy chained we need to shift out to all three 74HC595
		// hence, one single class for the display AND the relays ...
		// de: das ist natürlich ein Käse dass wir hier einen gemischten Zugriff auf das Display und die Relais machen
		// müssen
		shiftOut( dataPin, clockPin, MSBFIRST, display_h ); // data for U3 - display
		shiftOut( dataPin, clockPin, MSBFIRST, display_l ); // data for U4 - display
		shiftOut( dataPin, clockPin, MSBFIRST, relay_dat ); // data for U5 - Relay
		// return the latch pin high to signal chip that it no longer needs to listen for information
		digitalWrite( latchPin, HIGH );
	}

public:
	IO22D08() {}

	void begin() {
		digitalWrite( OE_595, LOW ); // Enable Pin of first 74HC595
		pinMode( latchPin, OUTPUT );
		pinMode( clockPin, OUTPUT );
		pinMode( dataPin, OUTPUT );
		pinMode( OE_595, OUTPUT );
	}

	void digitOnOff( int digit, bool onOff ) {
		if( digit >= 0 && digit <= 3 )
			TUBE_ON[digit] = onOff;
	}

	// fills the internal buffer for the digital outputs (relays)
	void pinWrite( uint8_t pin, uint8_t mode ) {
		// pin am ersten shiftregister ein oder ausschalten
		if( mode == LOW )
			bitClear( relay_port, pin );
		else
			bitSet( relay_port, pin );
		update(); // optional: call the shiftout process (but will be done some milliseconds later anyway)
	}

	// this is a first simple "print number" method
	// right alligned, unused digits with zeros
	// should be reworked for a nicer print/write
	void setNumber( int display_dat ) {
		dat_buf[0] = display_dat / 1000;
		display_dat = display_dat % 1000;
		dat_buf[1] = display_dat / 100;
		display_dat = display_dat % 100;
		dat_buf[2] = display_dat / 10;
		dat_buf[3] = display_dat % 10;
	}

	// this method should be called in loop as often as possible
	// it will refresh the multiplex display
	void tick() {
		uint32_t currentMillis = millis();
		if( currentMillis - previousMillis > 3 ) // each 2 / 4 milliseconds gives a stable display on pro Mini 8MHz
		{
			update();
			previousMillis = currentMillis;
		}
	}
};

IO22D08 board; // create an instance of the relay board with timer extension

void setup() {
	// Serial.begin(9600); // slow for 8Mhz Pro Mini
	// Serial.println("\nIO22D08 board");
	// init i2c bus
	Wire.begin( I2C_ADDRESS );
	Wire.onReceive( receiveEvent );
	Wire.onRequest( requestEvent );

	for( auto& i : optoInPin )
		pinMode( i, INPUT_PULLUP ); // init the optocoupler
	for( auto& i : keyInPin )
		pinMode( i, INPUT_PULLUP ); // init the discrete input keys

	board.begin(); // prepare the board hardware
}

uint8_t optoIn = 0;
uint8_t keyIn = 0;

void readInput() {
	for( size_t i = 0; i < noOfOptoIn; i++ ) {
		if( digitalRead( optoInPin[i] ) == LOW )
			bitSet( optoIn, i );
		else
			bitClear( optoIn, i );
	}
	for( size_t i = 0; i < noOfKeyIn; i++ ) {
		if( digitalRead( keyInPin[i] ) == LOW )
			bitSet( keyIn, i );
		else
			bitClear( keyIn, i );
	}
}

uint32_t nextTick = 0;
int n = 0;
uint32_t relay = 0;
uint32_t digit = 0;

void loop() {
	readInput();  // handle input pins
	board.tick(); // timekeeping for display
	uint32_t now = millis();
	// only testing
	if( now > nextTick ) {
		nextTick = now + 500;
		// increment number
		// board.setNumber( n++ );
		// cycle relays
		//		board.pinWrite( relay, LOW );
		relay++;
		if( relay == 8 )
			relay = 0;
		//		board.pinWrite( relay, HIGH );
		// cycle digits
		//		board.digitOnOff( digit, false );
		if( digit++ == 4 )
			digit = 0;
		//		board.digitOnOff( digit, true );

		// set relays according keys
		// for( size_t i = 0; i < noOfKeyIn; i++ ) {
		//	board.pinWrite( i, bitRead( keyIn, i ) != 0 );
		// }
	}
}

#define SET_RELAY 1
#define SET_NUMBER 2
#define SET_DIGIT 3

byte digits[4] = { 0 };

int rangeLimit( int v, int min, int max ) { return v < min ? min : ( v > max ? max : v ); }

void receiveEvent( int count ) {
	if( count == I2C_MSG_IN_SIZE ) {
		byte cmd = Wire.read();
		byte v1 = Wire.read();
		int v2 = Wire.read();
		switch( cmd ) {
		case SET_RELAY:
			board.pinWrite( v1, v2 );
			break;
		case SET_NUMBER:
			if( v1 >= 0 && v1 <= 3 )
				digits[v1] = rangeLimit( v2, 0, 9 );
			board.setNumber( digits[0] * 1000 + digits[1] * 100 + digits[2] * 10 + digits[3] );
			break;
		case SET_DIGIT:
			board.digitOnOff( v1, v2 != 0 );
			break;

		default:
			break;
		}
	}
}

void clearSendBuffer() {
	for( byte x = 0; x < sizeof( sendBuffer ); x++ )
		sendBuffer[x] = 0;
}

void requestEvent() {
	sendBuffer[0] = optoIn;
	sendBuffer[1] = keyIn;
	Wire.write( (const uint8_t*)sendBuffer, sizeof( sendBuffer ) );
}