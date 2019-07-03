/*   Arcade Spinner v0.6
*    Copyright 2018 Joe W (jmtw000 a/t gmail.com)
*                   Craig B - Updated code for mouse movement modes(DROP, ACCM) and case statement for Button port bit validation
*    
*    This program is free software: you can redistribute it and/or modify
*    it under the terms of the GNU General Public License as published by
*    the Free Software Foundation, either version 3 of the License, or
*    (at your option) any later version.
*
*    This program is distributed in the hope that it will be useful,
*    but WITHOUT ANY WARRANTY; without even the implied warranty of
*    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*    GNU General Public License for more details.
*
*    You should have received a copy of the GNU General Public License
*    along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

#include "Mouse.h"
#include <Joystick.h>

//Mouse movement modes - pick one to compile specific code
//#define NORM   //Mouse movement Normal; every movement processed; may lag in fast movement
//#define DROP   //Mouse movement Drop; drop extra movements; smooth movement, no lag
#define ACCM   //Mouse movement Accelerated; the faster you turn the encoder the larger the individual mouse movements, no lag
//#define STATE    //Mouse movement STATE; A state variable (1=right, 2=left) is set at each interrupt depending on the direction the encoder is turning and the mouse is moved in loop() based on that. No lag.
                 

#define pinA 2    //The pins that the rotary encoder's A and B terminals are connected to.
#define pinB 3
#define maxBut 6


//Create a Joystick object.
Joystick_ Joystick(JOYSTICK_DEFAULT_REPORT_ID,JOYSTICK_TYPE_GAMEPAD,
  maxBut, 0,             // Button Count, Hat Switch Count
  true, false, false,    // X, but no Y or Z Axis. We need at least one axis even though it's not really used.
  false, false, false,   // No Rx, Ry, or Rz
  false, false,          // No rudder or throttle
  false, false, false);  // No accelerator, brake, or steering;


//The previous state of the AB pins
volatile int previousReading = 0;

#ifndef STATE //not needed in STATE mode
//Keeps track of how much the encoder has been moved
volatile int rotPosition = 0;
#endif

#if !defined(STATE) || !defined(NORM) //not needed in STATE or NORM modes
volatile int rotMulti = 0;
#endif

volatile int state = 0; //0=no movement, 1=right, 2=left

//Set the initial last state of the buttons depending on the max number of buttons defined in maxBut

#if maxBut==1
int lastButtonState[maxBut] = {1};
#endif

#if maxBut==2
int lastButtonState[maxBut] = {1,1};
#endif

#if maxBut==3
int lastButtonState[maxBut] = {1,1,1};
#endif

#if maxBut==4
int lastButtonState[maxBut] = {1,1,1,1};
#endif

#if maxBut==5
int lastButtonState[maxBut] = {1,1,1,1,1};
#endif

#if maxBut==6
int lastButtonState[maxBut] = {1,1,1,1,1,1};
#endif

#if maxBut==7
int lastButtonState[maxBut] = {1,1,1,1,1,1,1};
#endif

#if maxBut==8
int lastButtonState[maxBut] = {1,1,1,1,1,1,1,1};
#endif

#if maxBut==9
int lastButtonState[maxBut] = {1,1,1,1,1,1,1,1,1};
#endif

#if maxBut==10
int lastButtonState[maxBut] = {1,1,1,1,1,1,1,1,1,1};
#endif

void setup() {
  //No need to set the pin modes with DDRx = DDRx | 0b00000000 as we're using all input and that's the initial state of the pins
  //Use internal input resistors for all the pins we're using  
  PORTD = 0b11010011; //Digital pins D2, D3, D4, D6, and D12.
  PORTB = 0b11110000; //Digital pins D8, D9, D10, D11
  //PORTB = 0b01110010; //Digital pins D8, D9, D10, and D15 Pro Micro Craig B
  PORTC = 0b11000000; //Digital pin D5 and D13
  PORTE = 0b01000000; //Digital pin D7
  //PORTF = 0b11000000; //Digital pin A0 & A1
  
  //Start the joystick
  Joystick.begin();
  
  //Center the X axis on the joystick
  Joystick.setXAxis(511);

  //Set up the interrupt handler for the encoder's A and B terminals on digital pins 2 and 3 respectively. Both interrupts use the same handler.
  attachInterrupt(digitalPinToInterrupt(pinA), pinChange, CHANGE); 
  attachInterrupt(digitalPinToInterrupt(pinB), pinChange, CHANGE);

  //Start the mouse
  Mouse.begin();
  }

//Interrupt handler
void pinChange() {

  //Set the currentReading variable to the current state of encoder terminals A and B which are conveniently located in bits 0 and 1 (digital pins 2 and 3) of PIND
  //This will give us a nice binary number, eg. 0b00000011, representing the current state of the two terminals.
  //You could do int currentReading = (digitalRead(pinA) << 1) | digitalRead(pinB); to get the same thing, but it would be much slower.
  int currentReading = PIND & 0b00000011;

  //Take the nice binary number we got last time there was an interrupt and shift it to the left by 2 then OR it with the current reading.
  //This will give us a nice binary number, eg. 0b00001100, representing the former and current state of the two encoder terminals.

  int combinedReading  = (previousReading << 2) | currentReading; 

  //Now that we know the previous and current state of the two terminals we can determine which direction the rotary encoder is turning.

  //Going to the right
  if(combinedReading == 0b0010 || 
     combinedReading == 0b1011 ||
     combinedReading == 0b1101 || 
     combinedReading == 0b0100) {
     
     #ifndef STATE
     rotPosition++;                   //update the position of the encoder
     #endif

     #ifdef STATE
     state=1;
     #endif
  }

  //Going to the left
  if(combinedReading == 0b0001 ||
     combinedReading == 0b0111 ||
     combinedReading == 0b1110 ||
     combinedReading == 0b1000) {
     #ifndef STATE
     rotPosition--;                   //update the position of the encoder
     #endif

     #ifdef STATE
     state=2;
     #endif
  }


  //Save the previous state of the A and B terminals for next time
  previousReading = currentReading;
}


void loop(){ 
  int currentButtonState;
//int rotMulti;

#ifdef ACCM
  //If the encoder has moved 2 or more transitions move the mouse in the appropriate direction 
  //and update the position variable to reflect that we moved the mouse. Accelerated move.
  if(rotPosition >= 2 || rotPosition <= -2) {
    rotMulti = rotPosition;// >> 1;
    Mouse.move(rotMulti,0,0);
    rotPosition -= (rotMulti);// << 1);
  }
#endif 

#ifdef DROP
  //If the encoder has moved 2 or more transitions move the mouse in the appropriate direction 
  //and update the position variable to reflect that we moved the mouse. Smooth drop extra moves.
  if(rotPosition >= 2) {
    Mouse.move(1,0,0);
    rotPosition = 0;
  } else if(rotPosition <= -2) {
    Mouse.move(-1,0,0);
    rotPosition = 0;
  } 
#endif 

#ifdef NORM
  //If the encoder has moved 2 or more transitions move the mouse in the appropriate direction 
  //and update the position variable to reflect that we moved the mouse. Smooth movement but lags.
  if(rotPosition >= 2) {
    Mouse.move(1,0,0);
    rotPosition -= 2;
  } else if(rotPosition <= -2) {
    Mouse.move(-1,0,0);
    rotPosition += 2;
  } 
#endif 

#ifdef STATE
switch (state){
  case 0:
    break;
  case 1:
    Mouse.move(1,0,0);
    break;
  case 2:
    Mouse.move(-1,0,0);
    break;
}
state=0;
#endif

  //Iterate through the 10 buttons (0-9) assigning the current state of the pin for each button, HIGH(0b00000001) or LOW(0b00000000), to the currentState variable
  int button = 0;
  do {
    switch ( button ) {
      case 0:  //on digital pin 4, PD4 - Arcade Button 0
        currentButtonState = (PIND & 0b00010000) >> 4; //logical AND the 8-bit pin reading with a mask to isolate the specific bit we're interested in and then shift it to the end of the byte
        break;
      case 1:  //on digital pin 5, PC6 - Arcade Button 1
        currentButtonState = (PINC & 0b01000000) >> 6;
        break;
      case 2:  //on digital pin 6, PD7 - Arcade Button 2
        currentButtonState = (PIND & 0b10000000) >> 7;
        break;
      case 3:  //on digital pin 7, PE6 - Arcade Button 3
        currentButtonState = (PINE & 0b01000000) >> 6;
        break;
      case 4:  //on digital pin 8, PB4 - Arcade Button 4
        currentButtonState = (PINB & 0b00010000) >> 4;
        break;
      case 5:  //on digital pin 9, PB5 - Arcade Button 5
        currentButtonState = (PINB & 0b00100000) >> 5;
        break;
      case 6: //on digital pin 10, PB6 - Arcade Button 6
        currentButtonState = (PINB & 0b01000000) >> 6;
        break;
      case 7: //on digital pin 11, PB7 - Arcade Button 7
        currentButtonState = (PINB & 0b10000000) >> 7;
        break;
      case 8: //on digital pin 12, PD6 - Arcade Button 8
        currentButtonState = (PIND & 0b01000000) >> 6;
        break;
      case 9: //on digital pin 13, PC7 - Arcade Button 9
        currentButtonState = (PINC & 0b10000000) >> 6;
        break;
      default: //should never happen
        currentButtonState = 0b00000000;
        break;

    /*Pro Micro 10 buttons (2 unused) Craig B
     * switch ( button ) {
      case 0:  //on digital pin 4, PD4 - Arcade Button 1
        currentButtonState = (PIND & 0b00010000) >> 4;
        break;
      case 1:  //on digital pin 5, PC6 - Arcade Button 2
        currentButtonState = (PINC & 0b01000000) >> 6;
        break;
      case 2:  //on digital pin 6, PD7 - Arcade Button 3
        currentButtonState = (PIND & 0b10000000) >> 7;
        break;
      case 3:  //on digital pin 7, PE6 - Arcade Button 4
        currentButtonState = (PINE & 0b01000000) >> 6;
        break;
      case 4:  //on digital pin 8, PB4 - Arcade Button 5
        currentButtonState = (PINB & 0b00010000) >> 4;
        break;
      case 5:  //on digital pin 9, PB5 - Arcade Button 6
        currentButtonState = (PINB & 0b00100000) >> 5;
        break;
      case 8:  //on digital pin 10, PB6 - COIN/Select Button 9
        currentButtonState = (PINB & 0b01000000) >> 6;
        break;
      case 9:  //on digital pin 15, PB1 - PLAYER/Start Button 10
        currentButtonState = (PINB & 0b00000010) >> 1;
        break; 
      default: //Extra digital pins 16, PB2 and 14, PB3
        currentButtonState = 0b00000000;
        break;
       */
    }
    //If the current state of the pin for each button is different than last time, update the joystick button state
    if(currentButtonState != lastButtonState[button])
      Joystick.setButton(button, !currentButtonState);
      
    //Save the last button state for each button for next time
    lastButtonState[button] = currentButtonState;

    ++button;
  } while (button < maxBut);

}
