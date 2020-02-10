#include <Arduino.h>
#include <FastLED.h>
#include <NTPClient.h>
// change next line to use with another board/shield
#include <ESP8266WiFi.h>
//#include <WiFi.h> // for WiFi shield
//#include <WiFi101.h> // for WiFi 101 shield or MKR1000
#include <WiFiUdp.h>



#include "characters.h"

FASTLED_USING_NAMESPACE

#if defined(FASTLED_VERSION) && (FASTLED_VERSION < 3001000)
#warning "Requires FastLED 3.1 or later; check github for latest code."
#endif

#define DATA_PIN            D6
#define LED_TYPE            WS2811
#define COLOR_ORDER         RGB
#define NUM_PANELS          5 
#define NUM_LEDS_PER_PANEL  16 
#define NUM_LEDS            (NUM_LEDS_PER_PANEL*NUM_PANELS) + 1 
#define BRIGHTNESS          255
#define FRAMES_PER_SECOND   10



CRGB leds[NUM_LEDS];



const char *ssid     = "NightVisionCameras";
const char *password = "DogCatBirdTurtle4";

WiFiUDP ntpUDP;

// You can specify the time server pool and the offset (in seconds, can be
// changed later with setTimeOffset() ). Additionaly you can specify the
// update interval (in milliseconds, can be changed using setUpdateInterval() ).
NTPClient timeClient(ntpUDP, "pool.ntp.org", 3600 * 16, 60000);

void Clocks();

uint8_t gHue = 0 ;

void setup() {
  Serial.begin(115200);

  
  
  delay(3000); // 3 second delay for recovery

  Serial.begin(115200); // open the serial port at 9600 bps:
  
  // tell FastLED about the LED strip configuration
  FastLED.addLeds<LED_TYPE,DATA_PIN,COLOR_ORDER>(leds, NUM_LEDS).setCorrection(TypicalLEDStrip);

  // set master brightness control
  FastLED.setBrightness(BRIGHTNESS);



  WiFi.begin(ssid, password);

  while ( WiFi.status() != WL_CONNECTED ) {
    delay ( 500 );
    Serial.print ( "." );
  }

  timeClient.begin();


}


void segment16(uint8_t panelOffset, int letter, uint8_t hue)
{
  if( letter < 32 || letter > ('z'-32) ) {
    // out of range 
    return ; 
  }


  uint8_t startLEDOffset = panelOffset * NUM_LEDS_PER_PANEL;
  uint16_t data = SixteenSegmentASCII[letter-32];
  
  for (byte i=0; i<NUM_LEDS_PER_PANEL; i++) 
  { 
    uint8_t pos = i ;
    boolean show_digit = (data>>(i)) & 1;  
    if (show_digit)
    {
      leds[ startLEDOffset + pos ] = CHSV( hue, 255, 192);
    }
    else
    {
      leds[ startLEDOffset + pos ] = CHSV( hue, 0, 0);
    }
  }
}


void Text(String text, uint8_t hue)
{  
  Serial.print("Text: ");
  Serial.println(text);

  for (int i=0; i<text.length(); i++) 
  {
    if(i < NUM_PANELS ) {
      segment16(i, text[i], hue + (i * 100) );
    }
  }
}

void Alphabet() {

  static const uint8_t START = 'A';
  static const uint8_t END = 'Z';
  static const uint8_t INTERVAL = 1; // Time in seconds before switching.

  static uint8_t offset = START ; 
  static long nextUpdate = 0 ; 

  if( millis() > nextUpdate  ) {
    nextUpdate = millis() + (1000 * INTERVAL) ; 
    offset++;

    if( offset > END ) {
      offset = START ; 
    }    

    static char output[NUM_PANELS+1] ; 
    for( uint8_t letterOffset = 0 ; letterOffset < NUM_PANELS ; letterOffset++ ) {
      output[letterOffset] = offset + letterOffset; 
    }
    output[NUM_PANELS] = 0 ;

    
    Text(String(output), gHue); 
  }
}


void Clocks() {

  static long updateTime = 0 ;
  if( updateTime < millis() ) {
    updateTime = millis() + 3000 ; 
    timeClient.update();
  }


  Serial.print(timeClient.getHours());
  Serial.print(":");
  Serial.print(timeClient.getMinutes());
  Serial.print(":");
  Serial.println(timeClient.getSeconds());

  String output;
  if( timeClient.getHours() < 10 ) {
    output += "0" ; 
  }
  output += timeClient.getHours();
  
  if( timeClient.getMinutes() < 10 ) {
    output += "0" ; 
  }
  output += timeClient.getMinutes();
  
  Text(output, gHue); 
    

  // Text(timeClient.getFormattedTime(), gHue);

  
}

// ========================



// ========================

void loop()
{
 
  // send the 'leds' array out to the actual LED strip
  FastLED.show();  
  // insert a delay to keep the framerate modest
  FastLED.delay(1000/FRAMES_PER_SECOND); 

  Clocks(); 
 
  // do some periodic updates
  EVERY_N_MILLISECONDS( 20 ) { gHue++; } // slowly cycle the "base color" through the rainbow
  
}
