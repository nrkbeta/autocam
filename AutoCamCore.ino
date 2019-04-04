/*
NRKbeta AutoCam.
This code is made to perform ad lib video production, and has been tested on Uno
Ethernet, Mega and Mega ADK.

At NRK we are typically using this in out radio studios. Here each participant 
has a dedicated microphone and camera, and in addition we have a camera covering
the whole scene. Based on the input levels from an auxiliary from each of the 
microphones, the code decides which camera goes live.

The logic is like a game with repeated rounds, where the result of each round
decides the outcome. In each round, we count how often each of the microphone
inputs exceeds a defined  threshold (variable: level). The outcome will be the 
camera on the participant who is talking, unless more than one is speaking at ones,
or no one is speaking. In those cases the total camera will be chosen.
     
If one of the participants talk for a long time, we do a short cut back to the
previous camera, for a listening shot.

Connect your first microphone to A0, the second to A1 and so forth.
*/

//////////////////SETUP START//////////////////////////

int level = 200;    // this is the trigger level to exceed from the microphones when talking in them, and the most crusial value to get right.
                    // analog levels are in the range of  0-1024, normaly from 0-5v, but this can be altered by altering the analogReference
                    // in the setup section of this sketch. (https://www.arduino.cc/reference/en/language/functions/analog-io/analogreference/)

int inputs =  4;    // number of michrophones to monitor. (Do NOT monitor inputs that ar not connected to an audio source, as they will report random values)
int camstart = 1;   // input number on the video mixer of the camera corresponding to the first microphone to monitor (the rest must follow in order)
int total = 5;      // input channel for total camera on video mixer

int rhythm = 1500;  // round duration i milliseconds (should be in the range of 1000-2000)
int swing = 50;     // round duration variation in percentage (should be in the range of 10-70)
int simultan = 30;  // simultaneous talk sensitivity before cut to total camera, in percent. 

#define board 53    // number of analog pin 0 on the Arduino use 13 for Uno & Ethernet and 53 for Mega & ADK
#define debug 1     // 1 to output debug info to the serial monitor, 0 to not

   //////////////////SETUP END//////////////////////////


int active=total;   // sets the active camera to the total camera at start
int last=total;     // sets the last camera to the total camera at start

int sample;         // variable that holds the current value read from the analog input
int win;            // variable that holds the video source of the winner of the most recent round
int count;          // variable that holds the number of samples in the most recent round  
int rounds;         // variable that holds the number of rounds since the most recent cut
int noone;          // variable that holds the number of rounds of consecutive silence
int multi;          // variable that holds the number of simultaneous active inputs in the most recent round  
int simulL;         // variable that holds the number of rounds of consecutive simultaneous active inputs
String  reason;     // an explanation for the cut printed during debug

int back = active;   // variable that holds the previous video source, for cut backs
long timeout;        // variable that holds end time of the current round
boolean cutBack=0;   // switch that is set to 1 if we do a cut back

void setup(){
 analogReference(INTERNAL);  // try INTERNAL for Uno and INTERNAL1V1 for Mega at line audio levels
 Serial.begin(115200);       // start the serial port to print debug messages
 if(debug){
   Serial.println("NRKBeta AutoCam");
   Serial.print("trigger level: ");
   Serial.print(level);
   Serial.print("\t rhythm: ");
   Serial.print(rhythm);
   Serial.print("ms\t swing: ");
   Serial.print(swing);
   Serial.print("%\t Simultaneous: ");
   Serial.print(simultan);
   Serial.println("%");
  }
 }
 
void loop(){
 AutoCam();      
}

void AutoCam(){
 
  //////////////////ROUND START//////////////////////
  // reset variables for each round
  int input[]={0,0,0,0,0,0};    // reset trigger count array
  count = 0;                    // reset count of samples in round 

  //determine round duration in ms
  int dur = rhythm + ((random(100)) * swing * 0.0002 * rhythm) - (swing * 0.01 * rhythm); // adding the swing to round duration   
  if (active == total){
   dur = 1.4 * dur;  // hold totals longer
   }  
  timeout = dur + millis();
  
  //read analog inputs for duration of round 
  while (timeout > millis()){             // start round
    for (int i = 1; i <=  inputs; i++) {  // loop through audio inputs
       sample = analogRead(board+i);  
       if(sample > level){              
             input[i]=input[i]+1;         // add point if input is over the trigger level
           }    
       }
     count++;                             // count samples in round
  }

  if(debug==1){                           // log results to console
    Serial.print("Round Duration: ");
    Serial.print(dur);
    Serial.print("\t| Samples: ");
    Serial.print(count);
    for (int i = 1; i <=  inputs; i++) {  
      Serial.print("\t| input: ");
      Serial.print(i);
      Serial.print(": ");
      Serial.print(input[i]);
    }
    Serial.println();
  }
  //////////////////ROUND END////////////////////
   
  /////////ACT BASED ON RESULTS /////////////////
    active = total;      // default camera is total
    multi= 0;            // reset simultaneous counter
    win=0;               // reset winner
    reason = "Speaking"; // reset reason to cut

    // loop through results for round  
    // find the input with the highest score in round
    for (int i = 1; i <=  inputs; i++) {    
       if (input[i] > win){                 
          win=input[i];
          active = i + (camstart-1);
          }   
        }   
   
    // log multiple inputs that are active during round
    for (int i = 1; i <=  inputs; i++) {               
       if (input[i] > simultan*(float(win)/100)){  
          multi++;  
          }
        } 

     rounds ++;  // count round with equal result

     // if the same source has been active for 4-8 rounds, do cut back to previous source
     if(rounds > random(4,8)){
       active = back;
       cutBack = 1;
       rounds = 0;
       reason = "Cut Back";
       }

    // count rounds of simultaneous active inputs, cut to total after 2
    if (multi > 1){
        simulL++;
        if (simulL > 2){
          active = total;
          reason = "Simultaneous";
        }
      }

    // check for none 
    if(multi == 1){simulL = 0;}
    if(multi == 0){
      noone++;
      if (noone == 3+(random(4))){
          active = total;
          reason = "Silence";
        }  
      }
     
    // if active has changed, change source on video mixer  
    if (active != last) {
        videomix(active);
        back=last;
        last = active;
        rounds=0;
        }
     }

void videomix(int cam){
 // send commands to your video mixer
 if(debug){
   Serial.print(" .   -    cut to camera ");
   Serial.print(cam);
   Serial.print(" - reason: ");
   Serial.println(reason);
 }
}

