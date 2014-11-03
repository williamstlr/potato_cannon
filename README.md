potato_cannon
=============
Potato Cannon 2
 
 4-6-2014  It's set for debugging right now, it mostly sends PWM through led pins right now to tell me if it's detecting left right up or down movement from the controller
 I can't get the ignition to fire though, that's about where I am
 
 4-7-2014 Rewrote the entire program to make it cleaner. Horizontal test motor with L293 working
 perfectly, Actually, I think I'm going to reevaluate my map function. I seem to be getting values
 over 255 and that screws the motors up.
 Adjusted map from 740 to 758 and from -128 to -127
 
 5-8-2014 Commented all of the code that had to do with the motor pins since I'm sending all meaningful data through the enable pins. I'll rewrite the 
 code later so that it's more condensed and makes more sense.
 Moved enable left and right pin down one pin.
 
 6-11-2014 Moved to revision 3, started cleaning up the code and making things more readable. Added the panLocation potentiometer.
 
 6-13-2014 Added the over-rotation bounce, or at leasted started it. I have yet to test it and see how it works.
 Implemented debugging stuff
 Over-rotation bounce works ok now
 Debugging works kind of like it should. Ideally it would be using booleans, but 1's and 0's are what I got to work.
 
 6-22-2014 Implemented the wire library and setup the xbox controller using I2C. The controller is the master and it continually sends
 it's readings to this device (slave-4). Variables that used to be read directly from analog inputs are now read with Wire.read() except
 the panSensor
 
 7-1-2014 Starting work on the fan and fuel timers
 Fan doesn't turn on -- don't know what happened, but it works
 Valve opens but never closes -- the else statement was keeping the fan set to zero instead of the fuel valve
 panLocation is not being read in so the cannon cannot move side to side
 
 
 overRotationBounce not working, I'm moving it to loop() to see if that fixes it. Didn't fix it
 Moved everything except sleep out of loop() -- Cable was broken
 fuelPin bounces a few times when it's first hit -- It now works
 fan doesn't seem to stay on for as long as the interval is set to -- now works
 
 8-11-2014 Warning buzzer now comes on when there is fuel in the chamber
 changed i2c to work byte by byte, rather than in 7-byte chunks. Should work with RPI now
overTurnBounce starts turning left as soon as I2C becomes available
Cannon only turns left, nothing is coming out on the rightTurnPin

9-13-2014 Added the LCD screen, it's setup up on a "hello world" right now, I still need to add useful info

10-9-2014 I think I've got the sendData function working correctly.

11-3-2014 - Working on I2C communication from node.js
q<F4>
q

