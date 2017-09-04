/*
 Name: Dragonfly HTTP Full (main.cpp)
 # Author: Jono + Phil
 # Date: Sep 13 2016
 # Description: Code to connect to HTTP for updated tweet counts
 # derived from Dragonfly Cellular HTTP Example
 # 830p - JS added code to send SMS
 # 09/15 11:45am  JS changing opening speed to reflect other timing example, fix open duration
 # 09/18 Run on SMS
*/

#include "mbed.h"
#include "mtsas.h"
#include "led.h"
int StringParse(string input);
int IntParse(char* input);
// This line controls the regulator's battery charger.
// BC_NCE = 0 enables the battery charger
// BC_NCE = 1 disables the battery charger
DigitalOut bc_nce(PB_2);

bool init_mtsas();
char* httpResToStr(HTTPResult res);

// The MTSSerialFlowControl object represents the physical serial link between the processor and the cellular radio.
mts::MTSSerialFlowControl* io;
// The Cellular object represents the cellular radio.
mts::Cellular* radio;

// An APN is required for GSM radios. This works for AT&T CARD!
static const char apn[] = "wap.cingular";
static std::string phone_number = "15039620846";

bool radio_ok = false;
Timer timer1; // For counting the HTTP requests
string twilioNum = "\"+12672140103\"";

int main() {
    //~~~~INITIALIZE VARIABLES~~~~~~

    //LED VARIABLES
    int r, g, b;
    data05 = 0;                            //Initialize direction CLOSE
    data04 = 1;                            //Initialize motor on
    wait(16);         // Close motor for 8 sec
    data04 = 0;         // turn motor off
    int open_ct = 0;

    r = 0;
    g = 160;
    b = 0;
    int change = 10;
    ledChaseTail(160,0,0,50,6); // Run ledChase once at the start
    ledChaseTail(160,0,0,50,6); // Run ledChase once at the start
    ledChaseTail(160,0,0,50,6); // Run ledChase once at the start
    //END LED VARIABLES

    // Disable the battery charger unless a battery is attached.
    bc_nce = 1;
    int http_ct = 0; // Counter for the number of times the board connects to HTTP
    string sms = "0"; // store any incoming SMS message
    string incomingNum = "0"; //store incoming phone number
    int feed_count = 0; //  integer version of web_count
    int temp_count = 0; // Store previous count
    int water = 0; // Count new waters each time
    // Change the baud rate of the debug port from the default 9600 - switch to 115200 later to reduce noise
    Serial debug(USBTX, USBRX);
    debug.baud(9600);

    //Sets the log level to TRACE, for higher outputs
    //Possible levels: NONE, FATAL, ERROR, WARNING, INFO, DEBUG, TRACE
    mts::MTSLog::setLogLevel(mts::MTSLog::TRACE_LEVEL);
    timer1.start();
    logInfo("initializing cellular radio");
    radio_ok = init_mtsas();
    if (! radio_ok) {
        while (true) {
            logError("failed to initialize cellular radio");
            wait(1);
        }
    }

    logInfo("setting APN");
    if (radio->setApn(apn) != MTS_SUCCESS)
        logError("failed to set APN to \"%s\"", apn);

    logInfo("bringing up the link");
    if (! radio->connect()) {
        logError("failed to bring up the link");
    }
    else {
        mts::Cellular::Sms msg; //Cell SMS setup
        msg.phoneNumber = phone_number;

    // HTTPClient object used for HTTP requests.
        HTTPClient http;
        //~~~~~~~~~~~~  END INITIALIZE ~~~~~~~~~
        while(1) //MAIN LOOP FOR READING HTTP & SMS
            {
            ledBreathLinear(0,70,80,100,6); // run this (100 means a little longer)
            colorWipe(0,70,80,50); // turqoise means entered main lo op
            // HTTP GET

//            //~~~~~~~ READ HTTP ~~~~~~~~~~~
//            char http_rx_buf[1024];
//            HTTPResult res;
//
//            // IHTTPDataIn object - will contain data received from server.
//            HTTPText http_rx(http_rx_buf, sizeof(http_rx_buf));
//
//            // Make a HTTP GET request to http://guarded-eyrie-61449.herokuapp.com
//            res = http.get("http://guarded-eyrie-61449.herokuapp.com/", &http_rx);
//            if (res != HTTP_OK)
//                logError("HTTP GET failed [%d][%s]", res, httpResToStr(res));
//            else {
//                logInfo("HTTP GET succeeded [%d]\r\n%s", http.getHTTPResponseCode(), http_rx_buf);
//                http_ct++; // Incremenets http read count when successful
//                if (http_ct >= 10){ // Sends one SMS of http text after 10 successful reads
//                    msg.message = http_rx_buf;
//                    if (radio->sendSMS(msg) != MTS_SUCCESS) //Send HTTP results thru SMS
//                        logError("sending SMS failed");
//                    http_ct = 0;
//                    }
//                //read_count = strtok(http_rx_buf, " "); // Stores feed_count number
//                temp_count = feed_count; // Store previous count
//                feed_count = IntParse(http_rx_buf);
//                if (feed_count > temp_count) // Increment only if the feed_count is larger
//                    water += (feed_count - temp_count)/4; //dampened b/c of counts
//                logInfo("**Total Count from HTTP is: %d", feed_count);
//                }
//            //~~~END READING HTTP~~~~~~~~~

            //~~~~~~~ CHECK FOR SMS ~~~~~
            // Display any received SMS messages.

            std::vector<mts::Cellular::Sms> msgs = radio->getReceivedSms();
            for (std::vector<mts::Cellular::Sms>::iterator it = msgs.begin(); it != msgs.end(); it++) {
                sms = it->message.c_str();
                incomingNum = it->phoneNumber.c_str();
                logInfo("[%s][%s]\r\n%s\r\n", incomingNum, it->timestamp.c_str(), sms);
                logInfo("Are there quotes on number: %s and twilio: %s", incomingNum, twilioNum);
                if (twilioNum.compare(incomingNum)==0){
                    logInfo("Text from twilio!");
                    temp_count = feed_count; // Store previous count
                    feed_count = StringParse(sms);
                    if (feed_count > temp_count)
                        water+= (feed_count - temp_count); // Increment water 2x for every SMS received
                        logInfo("New watering! old count: %d, new count: %d", temp_count, feed_count);
                    }
                else {
                    logInfo("text from other, will open");
                    water+=2;
                }
                }
            radio->deleteOnlyReceivedReadSms();
            // Finish HTTP Get
            logInfo("Running ledChaseTail");
            ledChaseTail(r,g,b,50,6);
            ledBreathLinear(r,g,b,100,6); // run this (100 means a little longer)
            //~~~~~~~INCREMENT MOTORS AND OUTPUT LEDS~~~~~~~~~~
            timer1.reset(); //restart timer
            //~~~~~~~~OPEN THE PETAL!!!~~~~~~~~~~~~~~~~~~~
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
                printf("\n Timer is %d, water is %d", timer1.read_ms(), water); //debug
                printf("\n Opening: Counter is at %d, Water count is at %d R is %d, G is %d", open_ct, water, r, g);
                data05 = 1;         // Set motor DIR to open
                data04 = 1;         //Set motor EN to on
                wait(0.8);
                data04 = 0; // Stop the motors
                open_ct ++;
                water = water-1;  // Use up one water count
                }
            timer1.reset();
            //~~~~~~~~~~~ CLOSE THE PETAL!! ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
            while (open_ct >= 20)   // If fully open, close it before connecting!
                {
                if (open_ct == 20)
                    ledDetonate(r,g,b, 30, 200, 10, 10); // Would this be good?
                    //ledDetonate(r,g,b, wait, interval, mult, flashes)
                printf("\n Closing counter is %d.. R is %d, G is %d", open_ct, r, g);
                data05 = 0;         // Set motor DIR to close
                data04 = 1;         // motor on
                // Close motor in 3 sec increments
                if (timer1.read_ms() >= 3000){
                    data04 = 0;         // Turn motor off for a second
                    open_ct+=1;
                    timer1.reset();          //Resets timer count to 0
                    printf("\n Counter is at %d", open_ct);
                }
                // Done closing after 15 cycles of closing
                if(open_ct > 30){
                    data04 = 0; // Turn off motoro
                    open_ct = 0;        //Reset open count
                    timer1.reset();
                    g = 180;
                    r = 0;
                    }
                }

            //~~~~~~~~END INCREMENT~~~~~~~~~~~~~~~~~~~~~

            ledChaseTail(r,g,b,50,6);
            ledBreathLinear(r,g,b,100,6); // run this (100 means a little longer)
            logInfo("Looping back");

            } // End main While loop

        } // End loop if cell connection worked

    logInfo("finished - bringing down link");
    radio->disconnect();

    return 0;
}

bool init_mtsas() {
    io = new mts::MTSSerialFlowControl(RADIO_TX, RADIO_RX, RADIO_RTS, RADIO_CTS);
    if (! io)
        return false;

    // radio default baud rate is 115200
    io->baud(115200);
    radio = mts::CellularFactory::create(io);
    if (! radio)
        return false;

    // Transport must be set properly before any TCPSocketConnection or UDPSocket objects are created
    Transport::setTransport(radio);

    return true;
}

char* httpResToStr(HTTPResult res) {
    switch(res) {
        case HTTP_PROCESSING:
            return "HTTP_PROCESSING";
        case HTTP_PARSE:
            return "HTTP_PARSE";
        case HTTP_DNS:
            return "HTTP_DNS";
        case HTTP_PRTCL:
            return "HTTP_PRTCL";
        case HTTP_NOTFOUND:
            return "HTTP_NOTFOUND";
        case HTTP_REFUSED:
            return "HTTP_REFUSED";
        case HTTP_ERROR:
            return "HTTP_ERROR";
        case HTTP_TIMEOUT:
            return "HTTP_TIMEOUT";
        case HTTP_CONN:
            return "HTTP_CONN";
        case HTTP_CLOSED:
            return "HTTP_CLOSED";
        case HTTP_REDIRECT:
            return "HTTP_REDIRECT";
        case HTTP_OK:
            return "HTTP_OK";
        default:
            return "HTTP Result unknown";
    }
}

int IntParse(char* input)
    {
    // An optimized int parse method.
    char* pString = input;
    int result = 0;
    for (int i = 0; i < 6; i++)
    {
        if (pString[i] != ' ')
            result = 10 * result + (input[i] - 48);
        else break;
    }
    return result;
    }

int StringParse(string input)
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


void ledChaseTail (unsigned char r, unsigned char g, unsigned char b, unsigned int wait, int tail){
    int pixelNum = 0;
    int tail_curr = 0;
    int tail_comp = tail + 1;
    __disable_irq();    // Disable Interrupts
    for (int c = 0; c < (PIXELS+tail); c++){
      if (pixelNum < PIXELS+1){
          for (int i = 0; i < (pixelNum-tail_curr); i++){
              sendPixel(0,0,0);
          }
          for (int i = 0; i < tail_curr; i++){
              sendPixel((r*(i+1)/(tail_comp)), (g*(i+1)/(tail_comp)), (b*(i+1)/(tail_comp)));
          }
          sendPixel(r,g,b);
          for (int i = pixelNum; i < PIXELS; i++){
              sendPixel(0,0,0);
          }
          show();
          pixelNum++;
          if (tail_curr < tail){
              tail_curr++;
          }
      }
      else {
          for (int i = 0; i < (pixelNum-tail_curr); i++){
              sendPixel(0,0,0);
          }
          for (int i = 0; i < tail_curr; i++){
              sendPixel((r*(i+1)/(tail_comp)), (g*(i+1)/(tail_comp)), (b*(i+1)/(tail_comp)));
          }
          show();
          //pixelNum++;
          tail_curr--;
      }
      wait_ms(wait);
    }
    __enable_irq();     // Enable Interrupts
}

void ledBreathLinear(unsigned char r, unsigned char g, unsigned char b, unsigned int wait, int mult){
    char r_int = r/mult;
    char g_int = g/mult;
    char b_int = b/mult;
    __disable_irq();    // Disable Interrupts
    for(int i = 0; i<mult; i++){
        char r_curr = i * r_int;
        char g_curr = i * g_int;
        char b_curr = i * b_int;
        for( int p=0; p<PIXELS; p++ ) {
            sendPixel( r_curr , g_curr , b_curr );
        }
        show();
        wait_ms(wait);
    }
    for(int i = mult; i>0; i--){
        char r_curr = i * r_int;
        char g_curr = i * g_int;
        char b_curr = i * b_int;
        for( int p=0; p<PIXELS; p++ ) {
            sendPixel( r_curr , g_curr , b_curr );
        }
        show();
        wait_ms(wait);
    }
    __enable_irq();     // Enable Interrupts
}

void ledDetonate(unsigned char r, unsigned char g, unsigned char b, unsigned int wait, unsigned int interval, int mult, int flashes){
    for (int i = flashes; i > 0; i--){
        showColor(r,g,b);
        wait_ms(interval);
        //ledBreathLinear(r,g,b,(wait/i),mult);
        showColor(0,0,0);
        wait_ms(interval*i);
    }
    for (int i = flashes; i >= 0; i--){
        showColor(r,g,b);
        wait_ms(interval);
        showColor(0,0,0);
        wait_ms(interval);
    }
    int r_bright = r*2;
    if (r_bright >= 255){
        r_bright = 255;
    }
    int g_bright = g*2;
    if (g_bright >= 255){
        g_bright = 255;
    }
    int b_bright = b*2;
    if (b_bright >= 255){
        b_bright = 255;
    }
    wait_ms(5000);
    showColor(150,150,150);
}
