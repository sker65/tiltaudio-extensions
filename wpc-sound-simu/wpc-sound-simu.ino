void setup() {
  // set gpio mode
  DDRC = 255; // port c komplett ausgang
  DDRA = 255; // port a komplett auf ausgang
  PORTA = 255; // high
  PORTC = 255; // high
  pinMode(LED_BUILTIN, OUTPUT);
}

// port A = pin 22 - 29 get direkt auf den WPC daten bus 1 -> D7, 15 -> D0,

int data[] = { 
  0x03, 0xe5, 0x03, 0xE6, 0x03, 0xE7, 
  0x00, 0x01,

  0x03, 0xe5, 0x03, 0xE6, 0x03, 0xE7, 
  0x01, 0x01,

  0x03, 0xe5, 0x03, 0xE6, 0x03, 0xE7, 
  0x00, 0xcd,

  0x03, 0xe5, 0x03, 0xE6, 0x03, 0xE7, 
  0x04, 0x12,

  0,0,
  };

int lenData = 8*4 + 2;

/*
 * sound address is 0x1C = port C bit 0-4 ---> pin 30 - 37  PC0:37 - PC7:30 
 * port c bit 5 = RW -> low
 * port c bit 6 = WDEN -> pluse to low
 */

// PA0:22 -> CD0 wpc15
// PA1:23 -> CD1 wpc13
// PA2:24 -> CD2 wpc11
// PA3:25 -> CD3 wpc9
// PA4:26 -> CD4 wpc7
// PA5:27 -> CD5 wpc5
// PA6:28 -> CD6 wpc3
// PA7:29 -> CD7 wpc1


// PC0:37 -> CA0 wpc25
// PC1:36 -> CA1 wpc23
// PC2:35 -> CA2 wpc21
// PC3:34 -> CA3 wpc19
// PC4:33 -> CA4 wpc17

// PC5:32 -> RW:wpc31
// PC6:31 -> WDEN:wpc29
 
int ledState = 0;

void play(int start, int end ) {
  for( int i = start; i <= end; i++ ) {
    delayMicroseconds(900);
    PORTA = data[i];
    PORTC = 0b01011100;      // sound address + rw = low, with wden high
    delayMicroseconds(20);
    PORTC = 0b00011100;      // sound address + rw = low, with wden low
    delayMicroseconds(30);
    PORTC = 0b01011100;      // sound address + rw = low, with wden high
  }
}

void toggleLed() {
  digitalWrite(LED_BUILTIN, ledState);
  ledState = !ledState;
}

int c = 0;

void loop() {
  if( (c % 20 )  == 0 ) {
    play(0,7);
    delay(200);
    toggleLed();
  }

  play(8,15);
  delay(200);
  toggleLed();
  
  play(16,23);
  delay(200);
  toggleLed();

  play(24,31);
  delay(400);
  toggleLed();

  data[32] = random(4);
  data[33] = random(256);
  play(32,33);
  delay(400);

  c++;
  
}