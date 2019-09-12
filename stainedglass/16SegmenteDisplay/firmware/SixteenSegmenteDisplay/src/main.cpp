/**
 * Stainedglass 16-Segmente-Display
 * More information: https://github.com/funvill/segmented-display
 * 
 */ 
#include <Arduino.h>
#include <FastLED.h>

#include "characters.h"

FASTLED_USING_NAMESPACE

#if defined(FASTLED_VERSION) && (FASTLED_VERSION < 3001000)
#warning "Requires FastLED 3.1 or later; check github for latest code."
#endif

#define DATA_PIN            D6
#define LED_TYPE            WS2811
#define COLOR_ORDER         RGB
#define NUM_PANELS          10 
#define NUM_LEDS_PER_PANEL  16 
#define NUM_LEDS            (NUM_LEDS_PER_PANEL*NUM_PANELS) + 1 
#define BRIGHTNESS          255
#define FRAMES_PER_SECOND   10



CRGB leds[NUM_LEDS];


/*
String WordsToUse[] = {
  "ABLUESTAR", 
  "BLINK", 
  "NEAT",
  "AWESOME",
  };
*/

void Abluestar();
void rainbow(); 
void rainbowWithGlitter(); 
void addGlitter( fract8 chanceOfGlitter);
void confetti(); 
void sinelon(); 
void juggle();
void bpm();

uint8_t gHue = 0 ; 
uint8_t gCurrentPatternNumber = 0; // Index number of which pattern is current
// List of patterns to cycle through.  Each is defined as a separate function below.
typedef void (*SimplePatternList[])();
SimplePatternList gPatterns = { Abluestar, rainbow, Abluestar, confetti, Abluestar, sinelon, Abluestar, juggle };




void setup() {
  delay(3000); // 3 second delay for recovery

  Serial.begin(115200); // open the serial port at 9600 bps:
  
  // tell FastLED about the LED strip configuration
  FastLED.addLeds<LED_TYPE,DATA_PIN,COLOR_ORDER>(leds, NUM_LEDS).setCorrection(TypicalLEDStrip);

  // set master brightness control
  FastLED.setBrightness(BRIGHTNESS);
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

// Patterns 
#define ARRAY_SIZE(A) (sizeof(A) / sizeof((A)[0]))

void nextPattern()
{
	// add one to the current pattern number, and wrap around at the end
	gCurrentPatternNumber = (gCurrentPatternNumber + 1) % ARRAY_SIZE( gPatterns);
}


void rainbow()
{
	// FastLED's built-in rainbow generator
	fill_rainbow( leds, NUM_LEDS, gHue, 7);
}

void rainbowWithGlitter()
{
	// built-in FastLED rainbow, plus some random sparkly glitter
	rainbow();
	addGlitter(80);
}

void addGlitter( fract8 chanceOfGlitter)
{
	if( random8() < chanceOfGlitter)
	{
		leds[ random16(NUM_LEDS) ] += CRGB::White;
	}
}

void confetti()
{
	// random colored speckles that blink in and fade smoothly
	fadeToBlackBy( leds, NUM_LEDS, 10);
	int pos = random16(NUM_LEDS);
	leds[pos] += CHSV( gHue + random8(64), 200, 255);
}

void sinelon()
{
	// a colored dot sweeping back and forth, with fading trails
	fadeToBlackBy( leds, NUM_LEDS, 20);
	int pos = beatsin16(13, 0, NUM_LEDS);
	leds[pos] += CHSV( gHue, 255, 192);
}

void bpm()
{
	// colored stripes pulsing at a defined Beats-Per-Minute (BPM)
	uint8_t BeatsPerMinute = 62;
	CRGBPalette16 palette = PartyColors_p;
	uint8_t beat = beatsin8( BeatsPerMinute, 64, 255);
	for( int i = 0; i < NUM_LEDS; i++)   //9948
	{
		leds[i] = ColorFromPalette(palette, gHue + (i * 2), beat - gHue + (i * 10));
	}
}

void juggle()
{
	// eight colored dots, weaving in and out of sync with each other
	fadeToBlackBy( leds, NUM_LEDS, 20);
	byte dothue = 0;
	for( int i = 0; i < 8; i++)
	{
		leds[beatsin16(i + 7, 0, NUM_LEDS)] |= CHSV(dothue, 200, 255);
		dothue += 32;
	}
}

void Abluestar() {
  Text("ABLUESTAR", gHue); 
}

// ========================



// ========================

void loop()
{
  // Call the current pattern function once, updating the 'leds' array
	gPatterns[gCurrentPatternNumber]();

  static uint8_t lastPattern = 0 ; 
  if( gCurrentPatternNumber != lastPattern ) {
    lastPattern = gCurrentPatternNumber ; 
    Serial.print("Pattern: ");
    Serial.println(lastPattern);    
  }


  // send the 'leds' array out to the actual LED strip
  FastLED.show();  
  // insert a delay to keep the framerate modest
  FastLED.delay(1000/FRAMES_PER_SECOND); 
 
  // do some periodic updates
  EVERY_N_MILLISECONDS( 20 ) { gHue++; } // slowly cycle the "base color" through the rainbow
  EVERY_N_SECONDS( 10 ) { nextPattern(); } // change patterns periodically
	
}
