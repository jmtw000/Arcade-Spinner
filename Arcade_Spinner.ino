/*   Arcade Spinner v0.7
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

// The code preceded by # is evaluated only at compile time.
// so variables used in that code must be created with #define
#define numButtons  6   // The number of buttons you are using. From 0 to 10.

// Set the initial last state of the buttons depending on the number of buttons defined in numButtons.
#if numButtons == 1
int lastButtonState[numButtons] = {1};
#endif

#if numButtons == 2
int lastButtonState[numButtons] = {1,1};
#endif

#if numButtons == 3
int lastButtonState[numButtons] = {1,1,1};
#endif

#if numButtons == 4
int lastButtonState[numButtons] = {1,1,1,1};
#endif

#if numButtons == 5
int lastButtonState[numButtons] = {1,1,1,1,1};
#endif

#if numButtons == 6
int lastButtonState[numButtons] = {1,1,1,1,1,1};
#endif

#if numButtons == 7
int lastButtonState[numButtons] = {1,1,1,1,1,1,1};
#endif

#if numButtons == 8
int lastButtonState[numButtons] = {1,1,1,1,1,1,1,1};
#endif

#if numButtons == 9
int lastButtonState[numButtons] = {1,1,1,1,1,1,1,1,1};
#endif

#if numButtons == 10
int lastButtonState[numButtons] = {1,1,1,1,1,1,1,1,1,1};
#endif

const int rotEncPinA = 3;    // Digital pin where the rotary encoder A terminal is connected.
const int rotEncPinB = 2;    // Digital pin where the rotary encoder B terminal is connected.

// Volatile vars because they are modified by interruptions (not in main loop), so need to use RAM, because if they are stored in registers their value can be inaccurate.
volatile int rotEncPrevRead = 0;    // The previous state of the AB pins.
volatile int rotEncMove = 0;        // Keeps track of how much the encoder has been moved.
volatile int rotEncShifted = 0;     // Used to shift the rotEncMove value. Explained in later code.
 
// Create a Joystick object. X and Y must be true to avoid problems detected on Raspberry Pi.
Joystick_ Joystick(JOYSTICK_DEFAULT_REPORT_ID,JOYSTICK_TYPE_GAMEPAD,
  numButtons, 0,         // Button Count, Hat Switch Count
  true, true, false,     // X, Y, but no Z Axis. We need at least two axes even though they're not used.
  false, false, false,   // No Rx, Ry, or Rz
  false, false,          // No rudder or throttle
  false, false, false);  // No accelerator, brake, or steering;

/**
 * The setup
 */
void setup() {  
  // No need to set the pin modes with DDRx = DDRx | 0b00000000 or pinMode function, 
  // as we're using all input and that's the default initial state of the pins.
    
  // Set initial state of digital pins using the port registers (We are setting all digital pins to HIGH)
  // https://www.arduino.cc/en/Reference/PortManipulation
  // Be aware of the differences with used board ATmega 32U4 AKA Arduino Leonardo
  // https://www.arduino.cc/en/Hacking/PinMapping32u4

  PORTB = PORTB | 0b11110000; // Digital pins D8(bit 4), D9(bit 5), D10(bit 6), D11(bit 7).    
  PORTD = PORTD | 0b11010011; // Digital pins D2(bit 1), D3 (bit 0), D4(bit 4), D12(bit 6) and D6(bit 7).
  PORTC = PORTC | 0b11000000; // Digital pin D5(bit 6) and D13(bit 7).
  PORTE = PORTE | 0b01000000; // Digital pin D7(bit 6).
  
  // Start the joystick
  Joystick.begin();
  
  // Start the mouse
  Mouse.begin();
  
  // Set up the interrupt handler for the encoder's A and B terminals on digital pins 2 and 3 respectively. 
  // Both interrupts use the same handler.
  attachInterrupt(digitalPinToInterrupt(rotEncPinA), rotEncChange, CHANGE); // Any change goes to interruption 
  attachInterrupt(digitalPinToInterrupt(rotEncPinB), rotEncChange, CHANGE); 
}

/**
 * Interrupt handler for rotary encoder changes
 */
void rotEncChange() {

  // Get the current state of encoder terminals A and B which are conveniently located in bits 0 and 1 (digital pins 2 and 3) of PIND.  
  // You could do it with [digitalRead] function, but it would be much slower. We use port register to avoid input lags.
  int rotEncReading = PIND & 0b00000011; // Use the & operator to apply a mask for retrieve only the digital inputs related to rotary encoder A and B terminals.

  // Take the binary number stored in rotEncReading and shift it to the left by 2
  // Example: the result of 0b00000011<<2 will be 0b00001100		
  // Then OR it with the current reading.
  // Result is the previous reading is stored in bits 2 and 3 while current reading in bits 0 and 1.
  int rotEncCombinedReading  = (rotEncPrevRead << 2) | rotEncReading; 

  // Now that we know the previous state (b2,b3) and current state (b1,b0) of the rotary encoder, we can determine which direction is turning.
  
  // Clockwise (right)
  //    A B
  // t1 0 0 
  // t2 1 0
  // t3 1 1
  // t4 0 1
  
  // Counter clockwise (left)
  //    A B
  // t1 0 0 
  // t2 0 1
  // t3 1 1
  // t4 1 0

  // Going to the right
  if(rotEncCombinedReading == 0b0010 || 
     rotEncCombinedReading == 0b1011 ||
     rotEncCombinedReading == 0b1101 || 
     rotEncCombinedReading == 0b0100) {
     
     rotEncMove++;                   // Update the position of the encoder
  }
  // Going to the left
  if(rotEncCombinedReading == 0b0001 ||
     rotEncCombinedReading == 0b0111 ||
     rotEncCombinedReading == 0b1110 ||
     rotEncCombinedReading == 0b1000) {
     
     rotEncMove--;                   // Update the position of the encoder     
  }

  // Save the previous state of the A and B terminals for next time
  rotEncPrevRead = rotEncReading;
}

/**
 * The program loop
 */
void loop(){ 

  int currentButtonState;
  
  // If the encoder has moved 1 or more transitions move the mouse in the appropriate direction 
  // and update the rotEncMove variable to reflect that we have moved the mouse. The mouse will move 1/2 
  // the number of pixels of the value currently in the rotPosition variable. We are using 1/2 (rotPosition>>1) because the total number 
  // of transitions(positions) on our encoder is 2400 which is way too high. 1200 positions is more than enough.
  
  if(rotEncMove != 0) {
    rotEncShifted = rotEncMove >> 1;     //copy rotPosition/2 to a temporary variable in case there's an interrupt while we're moving the mouse 
    Mouse.move(rotEncShifted, 0, 0);
    rotEncMove -= (rotEncShifted << 1);  //adjust rotPosition to account for mouse movement
  }

  // Iterate through the 10 buttons (0-9) assigning the current state of the pin for each button, HIGH(0b00000001) or LOW(0b00000000), to the currentState variable
  int button = 0;
  do {
    switch (button) {
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
        currentButtonState = (PINC & 0b10000000) >> 7;
        break;
      default: //should never happen
        currentButtonState = 0b00000000;
        break;
    }
    // If the current state of the pin for each button is different than last time, update the joystick button state
    if(currentButtonState != lastButtonState[button])
      Joystick.setButton(button, !currentButtonState);
      
    // Save the last button state for each button for next time
    lastButtonState[button] = currentButtonState;

    ++button;
  } while (button < numButtons);

}
