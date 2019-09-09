#include <Arduino.h>
#include <FastLED.h>

#include "characters.h"

FASTLED_USING_NAMESPACE

#if defined(FASTLED_VERSION) && (FASTLED_VERSION < 3001000)
#warning "Requires FastLED 3.1 or later; check github for latest code."
#endif

#define DATA_PIN    D6
//#define CLK_PIN   4
#define LED_TYPE    WS2811
#define COLOR_ORDER GRB
#define NUM_PANELS  6 
#define NUM_LEDS_PER_PANEL  16 
#define NUM_LEDS    (NUM_LEDS_PER_PANEL*NUM_PANELS)+NUM_PANELS+ 50 
CRGB leds[NUM_LEDS];

uint8_t gHue = 0 ; 

#define BRIGHTNESS          96
#define FRAMES_PER_SECOND   10

uint8_t panelStartOffset[NUM_PANELS] = {0, 16+1, 32+2, 48+3,64+4,80+5}; 

void setup() {
  delay(3000); // 3 second delay for recovery

  Serial.begin(115200); // open the serial port at 9600 bps:
  
  // tell FastLED about the LED strip configuration
  FastLED.addLeds<LED_TYPE,DATA_PIN,COLOR_ORDER>(leds, NUM_LEDS).setCorrection(TypicalLEDStrip);
  //FastLED.addLeds<LED_TYPE,DATA_PIN,CLK_PIN,COLOR_ORDER>(leds, NUM_LEDS).setCorrection(TypicalLEDStrip);

  // set master brightness control
  FastLED.setBrightness(BRIGHTNESS);
}


void segment16(uint8_t panelOffset, int letter, uint8_t hue)
{
  uint8_t startLEDOffset = panelStartOffset[panelOffset];
  uint16_t data = SixteenSegmentASCII[letter-32];

  Serial.println("");
  Serial.print("Letter: ");
  Serial.print((char) letter);
  Serial.print(" (");
  Serial.print(letter-32);
  Serial.print("), LED panel offset: ");
  Serial.print(startLEDOffset);
  
  for (byte i=0; i<NUM_LEDS_PER_PANEL; i++) 
  { 
    uint8_t pos = NUM_LEDS_PER_PANEL-i-1 ;
    boolean show_digit = (data>>(NUM_LEDS_PER_PANEL-1-i)) & 1;  

    Serial.print(", [");
    Serial.print(startLEDOffset + pos);
    Serial.print("] == ");
    Serial.print(show_digit);
    Serial.print(", ");

    if (show_digit)
    {
      leds[ startLEDOffset + pos ] = CHSV( hue, 255, 192);
    }
    else
    {
      leds[ startLEDOffset + pos ] = CHSV( hue, 0, 0);
    }
  }
  Serial.println("");
}


void Text(String text, uint8_t hue)
{  
  for (int i=0; i<text.length(); i++) 
  {
    if(i < NUM_PANELS ) {

      // segment16(i, text[i], hue + (i * 100) );
      segment16(0, text[i], hue + (i * 100) );

      // send the 'leds' array out to the actual LED strip
      FastLED.show();  
      // insert a delay to keep the framerate modest
      FastLED.delay(1000/FRAMES_PER_SECOND); 

      delay(1000 * 1 );
    }
  }
}


void loop()
{

  // Text("HE*LO", gHue); 
  Text("HELLO", gHue); 

  /*
  // send the 'leds' array out to the actual LED strip
  FastLED.show();  
  // insert a delay to keep the framerate modest
  FastLED.delay(1000/FRAMES_PER_SECOND); 
  */
 
  // do some periodic updates
  EVERY_N_MILLISECONDS( 20 ) { gHue++; } // slowly cycle the "base color" through the rainbow

}
