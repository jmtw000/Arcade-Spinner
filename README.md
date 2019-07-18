# Arcade-Spinner v0.6
This is an Arduino Micro (or clone) arcade spinner with 6 buttons (up to 10) for use with MAME or any other emulator which can use the X axis of the mouse as a paddle/spinner controller. This code should also work on any board which uses the ATmega32U4 as long as the port pins are mapped to the same "digital pins" as the Micro. I created this spinner because I wanted a cheaper alternative to the commercially available ones. I find it works well for ball and paddle games, Tempest, and also makes a decent controller for driving games.

 To construct this you will need a 2-phase rotary encoder which can operate at 5v and some momentary switch buttons. The rotary encoder I used is: https://www.amazon.com/Signswise-Incremental-Encoder-Dc5-24v-Voltage/dp/B00UTIFCVA 
However, any 2-phase 5v one should work. You can even use the little 20-30 position encoders that come with a lot of Arduino kits, although you'll have to alter the code and I don't recommend using those. You will also need the Arduino joystick library available at: https://github.com/MHeironimus/ArduinoJoystickLibrary

This device will be detected as both a mouse and a joystick/gamepad. The joystick only has X and Y axes which we don't use. At least one axis is needed by the library to declare the joystick and it was found that 2 were required for the controller to be detected on some systems. We just center the X and Y axes in setup() and forget about them. The spinner controls the mouse X axis which is by default mapped to the analog dial in MAME (don't forget to enable the mouse in MAME under advanced options!). The buttons will work as regular gamepad/joystick buttons. The 2400 different positions (transitions) that can be detected on the 600ppr(pulse per revolution) encoder I'm using are way too many for our purposes so they are halved in the code to 1200. The code uses the Atmega32u4 ports directly because that's faster than using digitalRead/Write. I'm not doing any debouncing of the buttons or encoder as it seems to work great for me as is, but you might want to add debouncing depending on your hardware.

The enclosure that I used can be bought at: https://www.galco.com/buy/Hammond-Manufacturing/RL6015BK
The knob for the encoder that I used can be bought at: https://www.amazon.com/gp/product/B01D2IIC3S


You should now be able to use the info found at http://wiki.arcadecontrols.com/index.php/Spinner_Turn_Count to adjust the analog sensitivity in MAME to more accurately simulate the original hardware with this spinner.




7-18-2019

No more mouse movement modes, you shouldn't need them. Also added a Y axis to the controller as it was reported it was not being detected on RetroPie without it. 



7-3-2019

Craig B contributed some code for optimizing button polling and for different mouse movement modes (ACCM/DROP). There is now also a maxBut #define to choose how many buttons you want, up to 10, and #if logic to set the initial state of those buttons. Adding more buttons if you need them or adapting the code to your specific board should be trivial.
