
#include <SparkIntervalTimer.h>
#include <neopixel.h>

/*
 Name: Blossom Electron (blossom-3g.ino)
 # Author: Jono + Phil
 # Date: Aug 4 2017
 # Description: Control the Blossom sculpture for LED outputs and motor drivers
 # Gets tweet & instagram counts from particle subscription and responds accordingly
 # 08/08 - Added code for ledChase tail >> this connects and reads gotCount, runs thru line 82 - add printLn after whiles?
*/


int checkCount(String command);
void intro();
void gotCount(const char *name, const char *data);
void ledChaseTail (unsigned char r, unsigned char g, unsigned char b, unsigned int wait, int tail);
void ledBreathLinear(unsigned char r, unsigned char g, unsigned char b, unsigned int wait, int mult);
void ledDetonate(unsigned char r, unsigned char g, unsigned char b, unsigned int wait, unsigned int interval, int mult, int flashes);
int checkWater(int local_ct);
void closeSeq();
void blinkCount(void);
int stringToInt(String input);

#define PIXEL_COUNT 15 // !!!!!!!!!!!!!!!! BE SURE TO UPDATE THIS !!!!!!!!!!!!!!!!
#define PIXEL_PIN A5
#define MOTOR_DIR A4
#define MOTOR_EN A3
#define MOTOR_INC 30 // motor increment is 3 sec (units are 1/10 seconds)
#define PIXEL_TYPE WS2812B
Adafruit_NeoPixel strip(PIXEL_COUNT, PIXEL_PIN, PIXEL_TYPE);
IntervalTimer myTimer;		// 3 for the Core

int counter;
int feed_count;
int r, g, b;
int water;

void setup() {
    pinMode(MOTOR_EN, OUTPUT);
    pinMode(MOTOR_DIR, OUTPUT);
    counter=0;
    feed_count=0;
    water = 0;
    r = 0, g = 160, b = 0;
    // AUTO allocate blinkLED to run every 100ms using hmSec timescale (1000 * 0.1ms period)
    myTimer.begin(blinkCount, 100, hmSec);

    strip.begin();
    strip.show();
    Particle.function("count", checkCount); // Begin serial communication
    Serial.begin(9600);
    Serial.println("Initialization >> setting up stuff!");
    // Listen for the webhook response, and call gotCount()
    Particle.subscribe("hook-response/getFeedBlossomCount", gotCount, MY_DEVICES);
    Particle.publish("getFeedBlossomCount");  // check the count ourselves once (use this again if we want occasional updates)

    // Close petals during initialization to be sure they are all at zero
    digitalWrite(MOTOR_DIR, LOW);  // Set motor DIR to close
    digitalWrite(MOTOR_EN, HIGH); //Set motor EN to on
    for(int i=0;i<16;i++) { // comment out during LED testing ~~~~ ADD THIS BACK IN !!!!!!!!! ~~~~~~~~~
        Serial.println("closing the petals, " + String(16-i) + " sec remaining");
        delay(1000);
        //molding motor pin HIGH
    }
    digitalWrite(MOTOR_EN, LOW); //Set motor EN to off
    Serial.println("Finishing main loop");
}   // ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ end SETUP ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

void loop() {
    int open_ct = 0; // for counting of openings

    while (1){  // main while loop
      intro();  // run through opening LED sequence
      open_ct = checkWater(open_ct); // If there is a water count >> OPEN
      if (open_ct>19){  // Called if fully open
        closeSeq();
        open_ct = 0;
      }

    } // main while loop
} // ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ end MAIN LOOP ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~


// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Introduction - shows intro colors when waiting for incoming watering counts
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void intro(){
  Serial.println("Starting loop - will led breath and then chase");
  for (int i =0; i<3; i++){
  ledBreathLinear(0,70,80,100,6); // run this (100 means a little longer)
  }
  // Run chase tail to indicate "searching for tweets/instas"
  for (int i =0; i<3; i++){
    ledChaseTail(r,g,b,50,6);
  }
  ledBreathLinear(r,g,b,100,6); // run this (100 means a little longer)
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Takes in the water count from gotCount, blinks and opens flowers
// Then returns the appropriate openct
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
int checkWater(int local_ct){
  int change = 0;
  while(water >= 1)
      {
      change = 10;
      if (water+local_ct > 20)    //To make sure it doesnt over-open
          water = 20 - local_ct;
      if (g > 8)
          g -= change;
      else g = 0;
      if (r < 172)
          r += change;
      else r = 180;
      ledChaseTail(r,g,b,100,6);
      Serial.println("Timer is " + String(counter*100) + ", water is " + String(water)); //debug
      Serial.println("Opening: Counter is at " + String(local_ct) + ", Water count is at " +String(water) + " R is " +r+ ", G is " + String(g));
      digitalWrite(MOTOR_DIR, HIGH);  // Set motor DIR to open
      digitalWrite(MOTOR_EN, HIGH); //Set motor EN to on
      delay(80); //
      digitalWrite(MOTOR_EN, LOW);
      local_ct++;
      water = water-1;  // Use up one water count
      }
    return local_ct;

}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Got Count - called with count data comes in (thru a hook), parses it and updates water count
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void gotCount(const char *name, const char *data) {
    int old_count = feed_count;

    String str = String(data);

    if (str != NULL) {
        Serial.println("Data received was: ");
        Serial.println(str);
    }
    else {
        Serial.println("Data read but nothing");
    }

    if (feed_count > 0){  // Make sure this is not the first received count
        feed_count = stringToInt(str); // Convert string count into an integer and compare with the old count
        int temp = feed_count-10;
        Serial.println("Feed count minus 10 is: " + String(temp));
        if (feed_count > old_count){
          water += (feed_count-old_count);
        }
    }

}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Check Count - automagically gets called upon a matching POST request
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
int checkCount(String command)
{
  // look for the matching argument "go" <-- max of 64 characters long
  if(command == "go")
  {
    Serial.println("CheckCount called - requesting count...");
    Particle.publish("getFeedBlossomCount"); //!COMMENTED OUT, resume this if we want automatic GET
    return 1;
  }
  else {
      Serial.println("Function called, wrong argument - try 'go' ");
      return -1;
  }
}


// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// LED Chase Tail - sends a string of LED's at one color along the full length of the strip
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void ledChaseTail (unsigned char r, unsigned char g, unsigned char b, unsigned int wait, int tail){
      //Serial.println("Starting LEDChaseTail!");
     int pixelNum = 0;
     int tail_curr = 0;
     int tail_comp = tail + 1;
     __disable_irq();    // Disable Interrupts
     for (int j = 0; j < strip.numPixels(); j++) // walks thru the whole string
     {
       delay(wait); // milliseconds
        // First, update the color on the next pixel
        strip.setPixelColor(j, r, g, b);
        strip.show();
        // Then update the tail section to remove the Color
        if((j-tail)>=0){
          strip.setPixelColor((j-tail), 0, 0, 0);
          strip.show();
        }

     }
    for (int j = 0; j < (strip.numPixels()-tail); j++) // remove the ending tail colors
     {
         strip.setPixelColor((j+tail), 0, 0, 0);
         strip.show();
         delay(wait); // milliseconds
     }
   }

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// LED Breath Linear - to show one color over all the petals, illuminating in a breathing pattern
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

 void ledBreathLinear(unsigned char r, unsigned char g, unsigned char b, unsigned int wait, int mult){
       char r_int = r/mult;
       char g_int = g/mult;
       char b_int = b/mult;
       __disable_irq();    // Disable Interrupts
       for(int i = 0; i<mult; i++){ // Light up
           char r_curr = i * r_int;
           char g_curr = i * g_int;
           char b_curr = i * b_int;
           for( int p=0; p<strip.numPixels(); p++ ) {
               strip.setPixelColor(p, r_curr , g_curr , b_curr );
           }
           strip.show();
           delay(wait); // milliseconds
       }
       delay(wait*20);
       for(int i = mult; i>0; i--){ // Light down
           char r_curr = i * r_int;
           char g_curr = i * g_int;
           char b_curr = i * b_int;
           for( int p=0; p<strip.numPixels(); p++ ) {
               strip.setPixelColor(p, r_curr , g_curr , b_curr );
           }
           strip.show();
           delay(wait); // milliseconds
       }
       for( int p=0; p<strip.numPixels(); p++ ) { // Turn all lights completely off
               strip.setPixelColor(p, 0 , 0 , 0 );
           }
           strip.show();
           delay(wait); // milliseconds
       __enable_irq();     // Enable Interrupts
   }

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Closing sequence - calls LED detonate and then closes the petals by reversing the motors
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

void closeSeq(){
 int close_ct = 20;
 ledDetonate(r,g,b, 30, 200, 10, 10); //ledDetonate(r,g,b, wait, interval, mult, flashes)
 Serial.println("Closing counter is " + String(close_ct));
 while (close_ct>0)
 {
   digitalWrite(MOTOR_DIR, LOW);         // Set motor DIR to close
   digitalWrite(MOTOR_EN, HIGH);       // motor on
   // Close motor in 3 sec increments
   if (counter >= MOTOR_INC){ // after motor "open increment" (1/10 seconds)
       digitalWrite(MOTOR_EN, LOW);       // motor off for a sec
       close_ct-=1;
       counter = 0;          //Resets timer count to 0
       Serial.println("Counter is at "+ String(close_ct));
   }
       // Done closing after 15 cycles of closing
   if(close_ct < 5){
           digitalWrite(MOTOR_EN, LOW);       // motor off for a sec
           close_ct = 0;        //Reset open count
           counter = 0;
           g = 180;
           r = 0;
           }
   }
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// LED Detonate - counts down visually from a slow pulse to faster, then ends with a full white color
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void ledDetonate(unsigned char r, unsigned char g, unsigned char b, unsigned int wait, unsigned int interval, int mult, int flashes){
       for (int i = flashes; i > 0; i--){
         for( int p=0; p<strip.numPixels(); p++ ) { // Turn all lights completely off
                 strip.setPixelColor(p, r , g , b );
             }
             strip.show();
             delay(interval); // milliseconds
           //ledBreathLinear(r,g,b,(wait/i),mult);
           for( int p=0; p<strip.numPixels(); p++ ) { // Turn all lights completely off
                   strip.setPixelColor(p, 0 , 0 , 0 );
               }
               strip.show();
               delay(interval*i); // milliseconds
       }
       for (int i = flashes; i >= 0; i--){
         for( int p=0; p<strip.numPixels(); p++ ) { // Turn all lights completely off
                 strip.setPixelColor(p, r , g , b );
             }
             strip.show();
             delay(interval); // milliseconds
           //ledBreathLinear(r,g,b,(wait/i),mult);
           for( int p=0; p<strip.numPixels(); p++ ) { // Turn all lights completely off
                   strip.setPixelColor(p, 0 , 0 , 0 );
               }
               strip.show();
               delay(interval); // milliseconds
       }
       delay(5000); // milliseconds
       for( int p=0; p<strip.numPixels(); p++ ) { // Turn all lights completely off
               strip.setPixelColor(p, 150 , 150 , 150 );
           }
           strip.show();
   }

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void blinkCount() {     // called based on timer, currently every 0.1ms
    counter++;		// increase
    if (counter > 100) // 10 sec ?
    {
        counter = 0;
    }
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// string to Int - conversion to only read the first number in a string
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
int stringToInt(String input)
    {
    // An optimized int parse method.
    //char pString = input;
    int result = 0;
    for (int i = 0; i < input.length(); i++)
    {
        if (input[i] != ' ')
            result = 10 * result + (input[i] - 48);
        else break;
    }
    return result;
    }
