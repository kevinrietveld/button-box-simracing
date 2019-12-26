// Arduino Pro Micro Button box with 3 encoders, 15 buttons and 4 function led
// this sketch was created and designed
// by Kevin Rietveld

//
// - Version 0.02
//


#include <Keypad.h>
#include <Joystick.h>

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
  {  0,  1,  2,  3  },     // 6
  {  4,  5,  6,  7  },     // 7
  {  8,  9,  10, 11 },     // 8   
  {  12, 13, 14, 15 }      // 9
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
  { 0, 1, 16, 17 }, // encoder connected to pin 0 and 1 (TX0/RX1)
  { 2, 3, 18, 19 }, // encoder connected to pin 2 and 3
  { 4, 5, 20, 21 }, // encoder connected to pin 4 and 5
};

// Define the mapping between button and led 
int ledMapButtons[][2] = {
  // Buttons
  { 0 , ledpinA0 },
  { 1 , ledpinA0 },
  { 2 , ledpinA0 },
  { 3 , ledpinA0 },
  { 4 , ledpinA0 },
  { 5 , ledpinA0 },
  { 6 , ledpinA0 },
  { 7 , ledpinA0 },
  { 8 , ledpinA0 },
  { 9 , ledpinA0 },
  { 10, ledpinA0 },
  { 11, ledpinA0 },
  { 12, ledpinA0 },
  { 13, ledpinA0 },
  { 14, ledpinA0 },
  { 15, ledpinA0 }
};

// Define the mapping between button and led 
int ledMapRotaries[][2] = {
  // Rotary
  { 16, ledpinA0 },
  { 17, ledpinA0 },
  { 18, ledpinA0 },
  { 19, ledpinA0 },
  { 20, ledpinA0 },
  { 21, ledpinA0 }
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

Joystick_ Joystick(JOYSTICK_DEFAULT_REPORT_ID, 
  JOYSTICK_TYPE_JOYSTICK, 32, 0,
  false, false, false, false, false, false,
  false, false, false, false, false);

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

void blinkLedRow(int times = 3, bool stayOnAfterBlink = true) {
  for (int i = 0; i <= times; i++)
  {
    digitalWrite(ledpinA1,HIGH); delay(250);
    digitalWrite(ledpinA2,HIGH); delay(250);
    digitalWrite(ledpinA3,HIGH); delay(250);
    digitalWrite(ledpinA1,LOW);
    digitalWrite(ledpinA2,LOW);
    digitalWrite(ledpinA3,LOW);
    delay(250);
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
  
  Joystick.begin();
  rotary_init();
}

void loop() {      
  CheckAllEncoders();
  CheckAllButtons();
}

void buttonPress() {
  for (int idx = 0; idx < NUMBUTTONS; idx++)
  {
    if(buttbx.key[idx].stateChanged)
    {
      uint8_t led = ledMapButtons[idx][1];
      
      switch(buttbx.key[idx].kstate)
      {
        case PRESSED:
        case HOLD:
          // Pressed
          Joystick.setButton(buttbx.key[idx].kchar, 1);
          digitalWrite(led, LOW);
          break;    
        case RELEASED:
        case IDLE:
          // Released
          Joystick.setButton(buttbx.key[idx].kchar, 0);
          digitalWrite(led, HIGH);
          break;
      }
      
    }
  }
}

void CheckAllButtons(void) {
  if(buttbx.getKeys())
  {
    buttonPress();
  }
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
  for (int idx = 0; idx < NUMROTARIES; idx++) {
    unsigned char result = rotary_process(idx);
    
    if (result == DIR_CCW) {
      uint8_t led = ledMapRotaries[idx][1];
      Joystick.setButton(rotaries[idx].ccwchar, 1);
      digitalWrite(led, LOW);
      delay(50);
      Joystick.setButton(rotaries[idx].ccwchar, 0);
      digitalWrite(led, HIGH);
    }
    else if (result == DIR_CW) {
      uint8_t led = ledMapRotaries[idx][1];
      Joystick.setButton(rotaries[idx].cwchar, 1);
      digitalWrite(led, LOW);
      delay(50);
      Joystick.setButton(rotaries[idx].cwchar, 0);
      digitalWrite(led, HIGH);
    }
  }
}