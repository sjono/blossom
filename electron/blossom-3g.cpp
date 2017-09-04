
#include <SparkIntervalTimer.h>
#include <neopixel.h>

/*
 Name: Blossom Electron (blossom-3g.ino)
 # Author: Jono + Phil
 # Date: Aug 4 2017
 # Description: Control the Blossom sculpture for LED outputs and motor drivers
 # Gets tweet & instagram counts from particle subscription and responds accordingly
 # 08/08 - Added code for ledChase tail >> this connects and reads gotCount, runs thru line 82 - add printLn after whiles?
 # Next - test with a motor output, add colorWipe, add Detonate code
*/


int checkCount(String command);
void gotCount(const char *name, const char *data);
void ledChaseTail (unsigned char r, unsigned char g, unsigned char b, unsigned int wait, int tail);
void blinkCount(void);

#define PIXEL_COUNT 15 // !!!!!!!!!!!!!!!! BE SURE TO UPDATE THIS !!!!!!!!!!!!!!!!
#define PIXEL_PIN A5
#define MOTOR_DIR A4
#define MOTOR_EN A3
#define MOTOR_INC 30 // motor increment is 3 sec (units are 1/10 seconds)
#define PIXEL_TYPE WS2812B
Adafruit_NeoPixel strip(PIXEL_COUNT, PIXEL_PIN, PIXEL_TYPE);
IntervalTimer myTimer;		// 3 for the Core

int counter = 0;

void setup() {
    pinMode(MOTOR_EN, OUTPUT);
    pinMode(MOTOR_DIR, OUTPUT);

  // AUTO allocate blinkLED to run every 100ms using hmSec timescale (1000 * 0.1ms period)
    myTimer.begin(blinkCount, 00, hmSec);

    strip.begin();
    strip.show();
    Particle.function("count", checkCount); // Begin serial communication
    Serial.begin(9600);
    Serial.println("Initialization >> setting up stuff!");
    // Listen for the webhook response, and call gotCount()
    Particle.subscribe("hook-response/getFeedBlossomCount", gotCount, MY_DEVICES);

    Particle.publish("getFeedBlossomCount");  // check the count ourselves once

    // Close petals during initialization to be sure they are all at zero
    digitalWrite(MOTOR_DIR, LOW);  // Set motor DIR to close
    digitalWrite(MOTOR_EN, HIGH); //Set motor EN to on
    // for(int i=0;i<16;i++) { // comment out during LED testing ~~~~ ADD THIS BACK IN !!!!!!!!! ~~~~~~~~~
    //     Serial.println("closing the petals, " + String(16-i) + " sec remaining");
    //     delay(1000);
    //     //molding motor pin HIGH
    // }
    digitalWrite(MOTOR_EN, LOW); //Set motor EN to off
    Serial.println("Finishing main loop");
}

void loop() {
    int open_ct = 0; // for counting of openings
    int water = 0;
    int change = 0;
    int r = 0, g = 160, b = 0;
    Serial.println("Starting loop - will led breath and then chase");

    //OPTIONAL publish events to trigger webhook (if we want to check server w a specific freq)
    //Particle.publish("getFeedBlossomCount");

    strip.setPixelColor(0, 50,50,50);
    strip.show();
    delay(1000);
    //rainbow(20);

    ledChaseTail(r,g,b,50,6);

    // code from Blossom2016

    // ~~~~~~~~~~~~~~~~~~ run initial "connecting pattern"~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    // Run intro LED pattern to show initializations
    //ledBreathLinear(0,70,80,100,6); // run this (100 means a little longer)
    //colorWipe(0,70,80,50); // turqoise means entered main lo op

    // Run chase tail to indicate "searching for tweets/instas"
    //ledChaseTail(r,g,b,50,6);
    //ledBreathLinear(r,g,b,100,6); // run this (100 means a little longer)
    // ~~~~~~~~~~~~~~~~~~ end connecting pattern"~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

    // ~~~~~~~~~~~~~~~~~~ open based on watering count (make its own function)~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    while(water >= 1)
        {
        change = 10;
        if (water+open_ct > 20)    //To make sure it doesnt over-open
            water = 20 - open_ct;
        if (g > 8)
            g -= change;
        else g = 0;
        if (r < 172)
            r += change;
        else r = 180;
        ledChaseTail(r,g,b,50,6);
        Serial.println("Timer is " + String(counter*100) + ", water is " + String(water)); //debug
        Serial.println("Opening: Counter is at" + String(open_ct) + ", Water count is at " +String(water) + " R is " +r+ ", G is " + String(g));
        digitalWrite(MOTOR_DIR, HIGH);  // Set motor DIR to open
        digitalWrite(MOTOR_EN, HIGH); //Set motor EN to on
        delay(80); //
        digitalWrite(MOTOR_EN, LOW);
        open_ct ++;
        water = water-1;  // Use up one water count
        }
    // ~~~~~~~~~~~~~~~~~~~~ End watering sequence ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

    // ~~~~~~~~~~~~~~ Closing sequence - call if fully open ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    while (open_ct >= 20)
        {
        if (open_ct == 20)
            //ledDetonate(r,g,b, 30, 200, 10, 10); // ~~~~~~~~~~~~~~~~~ ADD THIS FUNCTION !!!!!! ~~~~~~~~~~~~~~~~~~~~ !!!!!!!!!!!!
            //ledDetonate(r,g,b, wait, interval, mult, flashes)
        Serial.println("Closing counter is " + String(open_ct));
        digitalWrite(MOTOR_DIR, LOW);         // Set motor DIR to close
        digitalWrite(MOTOR_EN, HIGH);       // motor on
        // Close motor in 3 sec increments
        if (counter >= MOTOR_INC){ // after motor "open increment" (1/10 seconds)
            digitalWrite(MOTOR_EN, LOW);       // motor off for a sec
            open_ct+=1;
            counter = 0;          //Resets timer count to 0
            Serial.println("Counter is at "+ String(open_ct));
        }
        // Done closing after 15 cycles of closing
        if(open_ct > 30){
            digitalWrite(MOTOR_EN, LOW);       // motor off for a sec
            open_ct = 0;        //Reset open count
            counter = 0;
            g = 180;
            r = 0;
            }
        }
      // ~~~~~~~~~~~~~~~~~~~~ End closing sequence ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

      Serial.println("Watering or closing complete, restarting loop");
}

// This function will get called when weather data comes in
void gotCount(const char *name, const char *data) {
    String str = String(data);

    if (str != NULL) {
        Serial.println("Data received was: ");
        Serial.println(str);
    }
    else {
        Serial.println("Data read but nothing");
    }

    //TO DO! add code to parse the incoming string and store counts, ref below

    // for (std::vector<mts::Cellular::Sms>::iterator it = msgs.begin(); it != msgs.end(); it++) {
    //     sms = it->message.c_str();
    //     incomingNum = it->phoneNumber.c_str();
    //     logInfo("[%s][%s]\r\n%s\r\n", incomingNum, it->timestamp.c_str(), sms);
    //     logInfo("Are there quotes on number: %s and twilio: %s", incomingNum, twilioNum);
    //     if (twilioNum.compare(incomingNum)==0){
    //         logInfo("Text from twilio!");
    //         temp_count = feed_count; // Store previous count
    //         feed_count = StringParse(sms);
    //         if (feed_count > temp_count)
    //             water+= (feed_count - temp_count); // Increment water 2x for every SMS received
    //             logInfo("New watering! old count: %d, new count: %d", temp_count, feed_count);
    //         }
    //     else {
    //         logInfo("text from other, will open");
    //         water+=2;
    //     }
}

// this function automagically gets called upon a matching POST request
int checkCount(String command)
{
  // look for the matching argument "coffee" <-- max of 64 characters long
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

void rainbow(uint8_t wait) {
  uint16_t i, j;
  for(j=0; j<256; j++) {
    for(i=0; i<strip.numPixels(); i++) {
      strip.setPixelColor(i, Wheel((i+j) & 255));
    }
    strip.show();
    delay(wait);
  }
}

// Input a value 0 to 255 to get a color value.
// The colours are a transition r - g - b - back to r.
uint32_t Wheel(byte WheelPos) {
  if(WheelPos < 85) {
   return strip.Color(WheelPos * 3, 255 - WheelPos * 3, 0);
  } else if(WheelPos < 170) {
   WheelPos -= 85;
   return strip.Color(255 - WheelPos * 3, 0, WheelPos * 3);
  }
}

//Ref call: strip.setPixelColor(pixel, r, g, b);
// Jono attempts to rewrite this function
void ledChaseTail (unsigned char r, unsigned char g, unsigned char b, unsigned int wait, int tail){
      Serial.println("Starting LEDChaseTail!");
     int pixelNum = 0;
     int tail_curr = 0;
     int tail_comp = tail + 1;
     __disable_irq();    // Disable Interrupts
     for (int j = 0; j < PIXEL_COUNT; j++) // walks thru the whole string
     {
        // First, update the color on the next pixel
        strip.setPixelColor(j, r, g, b);
        // Then update the tail section to remove the Color
        if((j-tail)>0){
          strip.setPixelColor((j-tail), 0, 0, 0);
        }
        delay(wait); // milliseconds
     }
     for (int j = (PIXEL_COUNT-tail); j < PIXEL_COUNT; j++) // remove the ending tail colors
     {
         strip.setPixelColor((j-tail), 0, 0, 0);
         delay(wait); // milliseconds
     }
   }


// Original function from Phil
// void ledChaseTail (unsigned char r, unsigned char g, unsigned char b, unsigned int wait, int tail){
//      int pixelNum = 0;
//      int tail_curr = 0;
//      int tail_comp = tail + 1;
//      __disable_irq();    // Disable Interrupts
//      for (int c = 0; c < (PIXEL_COUNT+tail); c++){
//        if (pixelNum < PIXEL_COUNT+1){
//            for (int i = 0; i < (pixelNum-tail_curr); i++){
//                 strip.setPixelColor(pixelNum, r, g, b);
//                //sendPixel(0,0,0);
//            }
//            for (int i = 0; i < tail_curr; i++){
//                strip.setPixelColor(pixelNum, (r*(i+1)/(tail_comp)), (g*(i+1)/(tail_comp)), (b*(i+1)/(tail_comp)));
//                //sendPixel((r*(i+1)/(tail_comp)), (g*(i+1)/(tail_comp)), (b*(i+1)/(tail_comp)));
//            }
//            strip.setPixelColor(pixelNum, r, g, b);
//            //sendPixel(r,g,b);
//            for (int i = pixelNum; i < PIXEL_COUNT; i++){
//                strip.setPixelColor(pixelNum, 0, 0, 0);
//                //sendPixel(0,0,0);
//            }
//            strip.show();
//            pixelNum++;
//            if (tail_curr < tail){
//                tail_curr++;
//            }
//        }
//        else {
//            for (int i = 0; i < (pixelNum-tail_curr); i++){
//                strip.setPixelColor(pixelNum, 0, 0, 0);
//               //  sendPixel(0,0,0);
//            }
//            for (int i = 0; i < tail_curr; i++){
//                strip.setPixelColor(pixelNum, (r*(i+1)/(tail_comp)), (g*(i+1)/(tail_comp)), (b*(i+1)/(tail_comp)));
//               //  sendPixel((r*(i+1)/(tail_comp)), (g*(i+1)/(tail_comp)), (b*(i+1)/(tail_comp)));
//            }
//            strip.show();
//            //pixelNum++;
//            tail_curr--;
//        }
//        delay(wait); // milliseconds
//      }
//    }

void blinkCount() {     // called based on timer, currently every 0.1ms
    counter++;		// increase
    if (counter > 100) // 10 sec
    {
        counter = 0;
    }
}
