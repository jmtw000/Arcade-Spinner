/*   Arcade Spinner v0.5
*    Copyright 2018 Joe W (jmtw000 a/t gmail.com)
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

#define pinA 2    //The pins that the rotary encoder's A and B terminals are connected to.
#define pinB 3

//Create a Joystick object.
Joystick_ Joystick(JOYSTICK_DEFAULT_REPORT_ID,JOYSTICK_TYPE_GAMEPAD,
  6, 0,                  // Button Count, Hat Switch Count
  true, false, false,     // X, but no Y or Z Axis. We need at least one axis even though it's not really used.
  false, false, false,   // No Rx, Ry, or Rz
  false, false,          // No rudder or throttle
  false, false, false);  // No accelerator, brake, or steering;


//The previous state of the AB pins
volatile int previousReading = 0;

//Keeps track of how much the encoder has been moved
volatile int rotPosition = 0;

// Last state of 4 the buttons
int lastButtonState[6] = {1,1,1,1,1,1};



void setup() {
  //No need to set the pin modes with DDRx = DDRx | 0b00000000 as we're using all input and that's the initial state of the pins
  
  //Use internal input resistors for all the pins we're using  
  PORTD = 0b10010011; //Digital pins 2, 3, 4, and 6.
  PORTB = 0b00110000; //Digital pins 8 and 9
  PORTC = 0b01000000; //Digital pin 5
  PORTE = 0b01000000; //Digital pin 7

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

  //Set the currentReading variable to the current state of encoder terminals A and B which are conveniently located in bits 0 and 1 (digital pins 2 and 3) of PINB
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
     rotPosition++;                   //update the position of the encoder
  }

  //Going to the left
  if(combinedReading == 0b0001 ||
     combinedReading == 0b0111 ||
     combinedReading == 0b1110 ||
     combinedReading == 0b1000) {
     rotPosition--;                   //update the position of the encoder
     
  }

  //Save the previous state of the A and B terminals for next time
  previousReading = currentReading;
}


void loop(){ 
  int currentButtonState;
  
  //If the encoder has moved 2 or more transitions move the mouse in the appropriate direction 
  //and update the position variable to reflect that we moved the mouse.
  if(rotPosition >= 2) {
    Mouse.move(1,0,0);
    rotPosition -= 2;
  } else if(rotPosition <= -2) {
    Mouse.move(-1,0,0);
    rotPosition += 2;
  } 

  //Iterate through the 6 buttons (0-5) assigning the current state of the pin for each button, HIGH(0b00000001) or LOW(0b00000000), to the currentState variable
  for (int button = 0; button < 6; button++){
    
    //Button 0 on digital pin 4
    if(button==0)
      currentButtonState = (PIND & 0b00010000) >> 4;

    //Button 1 on digital pin 5
    if(button==1)
      currentButtonState = (PINC & 0b01000000) >> 6;

    //Button 2 on digital pin 6
    if(button==2)
      currentButtonState = (PIND & 0b10000000) >> 7;

    //Button 3 on digital pin 7
     if(button==3)
      currentButtonState = (PINE & 0b01000000) >> 4;

    //Button 4 on digital pin 8
    if(button==4)
      currentButtonState = (PINB & 0b00010000) >> 4;

    //Button 5 on digital pin 9
    if(button==5)
      currentButtonState = (PINB & 0b00100000) >> 5;


    //If the current state of the pin for each button is different than last time, update the joystick button state
    if(currentButtonState != lastButtonState[button])
      Joystick.setButton(button, !currentButtonState);
      
  //Save the last button state for each button for next time
  lastButtonState[button] = currentButtonState;
  }

}
