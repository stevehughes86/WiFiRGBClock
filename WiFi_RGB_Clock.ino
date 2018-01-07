/*
 * Wifi Clock, based on project found here: https://github.com/G6EJD/ESP8266-NTP-time-on-an-OLED-with-WiFi-Manager
 * Clock will display time via RGB LEDs (either smart LEDs or individual RGB LEDs)
 * Created by Steve Hughes
 */ 

String   clock_version = "0.5b";

 /*
  * TO DO list:
  * Add pots to control sat and brightness
  * Add way to control hue speed
  * Nice to have a option of solid colour & solid colour change - maybe a toggle switch
*/

/*****************
 * WiFi Settings *
 *****************/

#include <ESP8266WiFi.h>
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <WiFiManager.h>
#include <WiFiUdp.h>

/****************
 * LED settings *
 ****************/

#include <FastLED.h>

#define LED_PIN     5
#define NUM_LEDS    30
#define BRIGHTNESS  64
#define LED_TYPE    WS2811
#define COLOR_ORDER GRB
CRGB leds[NUM_LEDS];

const int ledDelay = 1; //delay at end of each While cycle

//HSV Settings
float hueVal = 0.00;
const float hueInc = 10.00;
const float hueSpeed = 0.05;
uint8_t satVal = 150; // control saturation
uint8_t valVal = 127; //control brightness


/***********************
 * NTP Server settings *
 ***********************/

#include <NTPClient.h>

//NTP update time - every 60s
#define UPDATE_INTERVAL 60000
//NTP time offset
#define TIME_OFFSET 0

WiFiUDP ntpUDP;

//Set NTP client to uk pool, then offset time, then update interval
NTPClient timeClient(ntpUDP, "uk.pool.ntp.org", TIME_OFFSET, UPDATE_INTERVAL);

/*********************
 * Numbers as arrays *
 *********************/
 
byte digits[10][7] = {
  {1,1,1,0,1,1,1},   // Digit 0
  {1,1,0,0,0,0,0},   // Digit 1
  {0,1,1,1,1,1,0},   // Digit 2
  {1,1,1,1,1,0,0},   // Digit 3
  {1,1,0,1,0,0,1},   // Digit 4
  {1,0,1,1,1,0,1},   // Digit 5
  {1,0,1,1,1,1,1},   // Digit 6
  {1,1,0,0,1,0,0},   // Digit 7
  {1,1,1,1,1,1,1},   // Digit 8
  {1,1,1,1,1,0,1}    // Digit 9 | 2D Array for numbers on 7 segment
};  

/*********
 * Setup *
 *********/
 
void setup(){
  Serial.begin(115200);

//Setup WiFi and WiFi manager
  WiFiManager wifiManager;
//  wifiManager.resetSettings();
  wifiManager.autoConnect();
  
  while (WiFi.status() != WL_CONNECTED) {
    delay (500);
    Serial.print (".");
  }

//Start NTP client
  timeClient.begin();

//Setup FastLED
  FastLED.addLeds<LED_TYPE, LED_PIN, COLOR_ORDER>(leds, NUM_LEDS).setCorrection( TypicalLEDStrip );
  FastLED.setBrightness(BRIGHTNESS);
}

/*************
 * Main Loop *
 *************/
uint8_t startUp = 0;

void loop() {
//  uint16_t satRead = analogRead(A0);
//  satVal = map(satRead, 0, 1023, 0, 255);
//
//  uint16_t valRead = analogRead(A1);
//  valVal = map(satRead, 0, 1023, 0, 255);

//Set time counter
  const unsigned long startTime = millis(); //unsinged long is a 32 bit number +ve only (0 - c.4.3 billion), const as wont change from this value
  unsigned long timeNow = millis();
  uint16_t loopTimer = 0;
  timeClient.update();

//get hour and minute ints
  uint8_t hour = timeClient.getHours();
  uint8_t minute = timeClient.getMinutes();

//get individual digits
  int hourDigitOne = hour / 10; // 10 unit
  int hourDigitTwo = hour % 10; // 01 unit
  int minuteDigitOne = minute / 10; // 10 unit
  int minuteDigitTwo = minute % 10; // 01 unit
  
//Check if just started up
  if (startUp == 0) {
    for (uint8_t i = 0 ; i < NUM_LEDS ; i++) {
      leds[i].setHSV(255,255,255);
    }
    updateDisplay();
    delay(1000);
    startUp++;
  }

  while (loopTimer <= UPDATE_INTERVAL) {
    blankDisplay(); //clear any old data on the display
    
    uint8_t ledPosition = 0;
    uint8_t ledOnCount = 0;

//work through LEDs for digit one
    for (uint8_t i = 0 ; i < 7 ; i++) {
      if (digits[minuteDigitTwo][i] == 1){
        float hueShow = hueVal + (hueInc * ledOnCount);
        leds[ledPosition].setHSV(hueShow, satVal, valVal);
        ledOnCount++;
      }
      else {
        leds[ledPosition].setHSV(0,0,0);
      }
      ledPosition++;
    }

//work through LEDs for digit two
    for (uint8_t i = 0 ; i < 7 ; i++) {
      if (digits[minuteDigitOne][i] == 1){
        float hueShow = hueVal + (hueInc * ledOnCount);
        leds[ledPosition].setHSV(hueShow, satVal, valVal);
        ledOnCount++;
      }
      else {
        leds[ledPosition].setHSV(0,0,0);
      }
      ledPosition++;
    }

//Centre DOTS
    float hueShow = hueVal + (hueInc * ledOnCount);
    leds[ledPosition].setHSV(hueShow, satVal, valVal);
    ledOnCount++; 
    ledPosition++;
    
    hueShow = hueVal + (hueInc * ledOnCount);
    leds[ledPosition].setHSV(hueShow, satVal, valVal);
    ledOnCount++;
    ledPosition++;

//work through LEDs for digit three 
    for (uint8_t i = 0 ; i < 7 ; i++) {
      if (digits[hourDigitTwo][i] == 1){
        float hueShow = hueVal + (hueInc * ledOnCount);
        leds[ledPosition].setHSV(hueShow, satVal, valVal);
        ledOnCount++;
      }
      else {
        leds[ledPosition].setHSV(0,0,0);
      }
      ledPosition++;
    }

//work through LEDs for digit four
    for (uint8_t i = 0 ; i < 7 ; i++) {
      if (digits[hourDigitOne][i] == 1){
        float hueShow = hueVal + (hueInc * ledOnCount);
        leds[ledPosition].setHSV(hueShow, satVal, valVal);
        ledOnCount++;
      }
      else {
        leds[ledPosition].setHSV(0,0,0);
      }
      ledPosition++;
    }

//show LEDs on display
    updateDisplay();
    
    delay(ledDelay);
    timeNow = millis();
    loopTimer = timeNow - startTime;

    hueVal = hueVal + hueSpeed;
    if (hueVal > 255) {
      hueVal = 0.00;
    }
  }
}

/************************
 * Additional Functions *
 ************************/

inline void blankDisplay() {
  for (uint8_t i = 0 ; i < NUM_LEDS ; i++) {
    leds[i].setHSV(0,0,0);
  }
}

inline void updateDisplay(){
  FastLED.show();
}
