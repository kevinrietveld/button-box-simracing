// Arduino Pro Micro Button box with 3 encoders, 15 buttons and 4 function led
// this sketch was created and designed
// by Kevin Rietveld

//
// - Version 0.01
//


#include <Keyboard.h>
#include <Keypad.h> 

// Variables
#define ENABLE_PULLUPS
#define NUMROTARIES 3
#define NUMBUTTONS 15
#define NUMROWS 4
#define NUMCOLS 4

#define ledpinA0 A0
#define ledpinA1 A1
#define ledpinA2 A2
#define ledpinA3 A3

#define DIR_CCW 0x10
#define DIR_CW 0x20

#define R_START 0x0
#define R_CW_FINAL 0x1
#define R_CW_BEGIN 0x2
#define R_CW_NEXT 0x3
#define R_CCW_BEGIN 0x4
#define R_CCW_FINAL 0x5
#define R_CCW_NEXT 0x6

boolean ledA0_state;
boolean ledA1_state;
boolean ledA2_state;
boolean ledA3_state;

// Define the symbols on the buttons of the keypads               
char buttons[NUMROWS][NUMCOLS] = {
  // 15  14  16, 10
  { 'a','b','c','d' },     // 6
  { 'e','f','g','h' },     // 7
  { 'i','j','k','l' },     // 8   
  { 'm','n','o','p' }      // 9
};

// Define the rotary encoders
struct rotariesdef {
  byte pin1;
  byte pin2;
  int ccwchar;
  int cwchar;
  volatile unsigned char state;
};

rotariesdef rotaries[NUMROTARIES] {
  { 0, 1, 'q', 'r' }, // encoder connected to pin 0 and 1 (TX0/RX1)
  { 2, 3, 's', 't' }, // encoder connected to pin 2 and 3
  { 4, 5, 'u', 'v' }, // encoder connected to pin 4 and 5
};

// Define the mapping between button and led 
int ledMap[][2] = {

  // Buttons
  { 'a', ledpinA0 },
  { 'b', ledpinA0 },
  { 'c', ledpinA0 },
  { 'd', ledpinA0 },
  { 'e', ledpinA0 },
  { 'f', ledpinA0 },
  { 'g', ledpinA0 },
  { 'h', ledpinA0 },
  { 'i', ledpinA0 },
  { 'j', ledpinA0 },
  { 'k', ledpinA0 },
  { 'l', ledpinA0 },
  { 'm', ledpinA0 },
  { 'n', ledpinA0 },
  { 'o', ledpinA0 },

  // Rotary
  { 'q', ledpinA0 },
  { 'r', ledpinA0 },
  { 's', ledpinA0 },
  { 't', ledpinA0 },
  { 'u', ledpinA0 },
  { 'v', ledpinA0 }
};


const unsigned char ttable[7][4] = {
  // R_START
  { R_START,    R_CW_BEGIN,  R_CCW_BEGIN, R_START },
  // R_CW_FINAL
  { R_CW_NEXT,  R_START,     R_CW_FINAL,  R_START | DIR_CW },
  // R_CW_BEGIN
  { R_CW_NEXT,  R_CW_BEGIN,  R_START,     R_START },
  // R_CW_NEXT
  { R_CW_NEXT,  R_CW_BEGIN,  R_CW_FINAL,  R_START },
  // R_CCW_BEGIN
  { R_CCW_NEXT, R_START,     R_CCW_BEGIN, R_START },
  // R_CCW_FINAL
  { R_CCW_NEXT, R_CCW_FINAL, R_START,     R_START | DIR_CCW },
  // R_CCW_NEXT
  { R_CCW_NEXT, R_CCW_FINAL, R_CCW_BEGIN, R_START },
};

byte rowPins[NUMROWS] = { 6,  7,  8,  9 };  // Connect to the row pinouts of the keypad
byte colPins[NUMCOLS] = { 15, 14, 16, 10 }; // Connect to the column pinouts of the keypad

// initialize an instance of class NewKeypad
Keypad buttbx = Keypad(makeKeymap(buttons), rowPins, colPins, NUMROWS, NUMCOLS);

void initLeds() {

  // initialize all analog pins as output for leds
  pinMode(ledpinA0,OUTPUT);  
  pinMode(ledpinA1,OUTPUT);
  pinMode(ledpinA2,OUTPUT);
  pinMode(ledpinA3,OUTPUT);

  analogWrite(ledpinA0,128);
  analogWrite(ledpinA1,128);
  analogWrite(ledpinA2,128);
  analogWrite(ledpinA3,128);

  // Led 0 always on 
  digitalWrite(ledpinA0,HIGH);
}

void blinkLedRow(int times = 3, bool stayOnAfterBlink) {
  for (int i = 0; i < times; i++)
  {
    digitalWrite(ledpinA1,HIGH); delay(250);
    digitalWrite(ledpinA2,HIGH); delay(250);
    digitalWrite(ledpinA3,HIGH); delay(250);
    digitalWrite(ledpinA1,LOW);
    digitalWrite(ledpinA2,LOW);
    digitalWrite(ledpinA3,LOW);
  }

  if(stayOnAfterBlink)
  {
    digitalWrite(ledpinA1,HIGH);
    digitalWrite(ledpinA2,HIGH);
    digitalWrite(ledpinA3,HIGH);
  }
}

void setup() {
  // Initialize leds
  initLeds();
  blinkLedRow(3, true);
  
  Keyboard.begin();
  rotary_init();
}

void loop() {      
  CheckAllEncoders();
  CheckAllButtons();
}

// Key as input, gets the corresponding led, wirte the key and blinks the led
void buttonPress(char keyPress) {
  for (int idx = 0; idx < NUMBUTTONS; idx++)
  {
    char key = ledMap[idx][0];
    if(key == keyPress)
    {
      uint8_t led = ledMap[idx][1];
      Keyboard.write(key);
      digitalWrite(led, LOW);
      delay(100);
      Keyboard.release(key);
      digitalWrite(led, HIGH);
    }
  }
}

void CheckAllButtons(void) {
  char key = buttbx.getKey();
  buttonPress(key);
}

/* Call this once in setup() */
void rotary_init() {
  for (int i = 0; i < NUMROTARIES; i++) {
    pinMode(rotaries[i].pin1, INPUT);
    pinMode(rotaries[i].pin2, INPUT);
    #ifdef ENABLE_PULLUPS
      digitalWrite(rotaries[i].pin1, HIGH);
      digitalWrite(rotaries[i].pin2, HIGH);
    #endif
  }
}

/* Read input pins and process for events. Call this either from a
 * loop or an interrupt (eg pin change or timer).
 *
 * Returns 0 on no event, otherwise 0x80 or 0x40 depending on the direction.
*/
unsigned char rotary_process(int _i) {
  unsigned char pinstate = (digitalRead(rotaries[_i].pin2) << 1) | digitalRead(rotaries[_i].pin1);
  rotaries[_i].state = ttable[rotaries[_i].state & 0xf][pinstate];

  return (rotaries[_i].state & 0x30);
}


void CheckAllEncoders(void) {
  for (int i = 0; i < NUMROTARIES; i++) {
    unsigned char result = rotary_process(i);
    if (result) {
      buttonPress(result == DIR_CCW ? rotaries[i].ccwchar : rotaries[i].cwchar);
      // Maybe use delay of 50 and without key release
    }
  }
}