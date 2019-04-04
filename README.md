# NRKbeta AutoCam.
This code is made to perform ad lib video production, and has been tested on Arduino Uno
Ethernet, Mega and Mega ADK.

At NRK we are typically using this in our radio studios. Each participant has a dedicated microphone and camera, and in addition we have a camera covering the whole scene. 

**Based on the input levels from an auxiliary from each of the  microphones, the code decides which camera goes live.**

The logic is like a game with repeated rounds, where the result of each round decides the outcome. In each round, we count how often each of the microphone inputs exceeds a defined threshold (variable: level). 

The outcome will be the camera on the participant who is talking, unless more than one is speaking at ones, or if no one is speaking. In those cases the total camera will be chosen.
     
If one of the participants talk for a long time, we do a short cut back to the previous camera, for a listening shot.

Connect your first microphone to A0, the second to A1 and so forth.