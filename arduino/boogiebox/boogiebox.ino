#include <JC_Button.h>
#include <SPI.h>
#include <Adafruit_VS1053.h>
#include <SD.h>

// These are the pins used
#define VS1053_RESET   -1     // VS1053 reset pin (not used!)

// Feather ESP8266
#if defined(ESP8266)
  #define VS1053_CS      16     // VS1053 chip select pin (output)
  #define VS1053_DCS     15     // VS1053 Data/command select pin (output)
  #define CARDCS          2     // Card chip select pin
  #define VS1053_DREQ     0     // VS1053 Data request, ideally an Interrupt pin

// Feather M4 M0, 328, or 32u4
#else
  #define VS1053_CS       6     // VS1053 chip select pin (output)
  #define VS1053_DCS     10     // VS1053 Data/command select pin (output)
  #define CARDCS          5     // Card chip select pin
  // DREQ should be an Int pin *if possible* (not possible on 32u4)
  #define VS1053_DREQ     9     // VS1053 Data request, ideally an Interrupt pin

#endif

#define NEOPIXEL_PIN 8
#define BTN_1_PIN 3
#define BTN_2_PIN 4
#define BTN_3_PIN 5
#define BTN_4_PIN 6
#define BTN_5_PIN 7
#define PULLUP true        //To keep things simple, we use the Arduino's internal pullup resistor.
#define INVERT false       //Since the pullup resistor will keep the pin high unless the
                           //switch is closed, this is negative logic, i.e. a high state
                           //means the button is NOT pressed. (Assuming a normally open switch.)
#define DEBOUNCE_MS 20     //A debounce time of 20 milliseconds usually works well for tactile button switches.
#define LONG_PRESS 1000    //We define a "long press" to be 500 milliseconds.

Adafruit_VS1053_FilePlayer musicPlayer =
Adafruit_VS1053_FilePlayer(VS1053_RESET, VS1053_CS, VS1053_DCS, VS1053_DREQ, CARDCS);

Button btn1(BTN_1_PIN, PULLUP, INVERT, DEBOUNCE_MS);
Button btn2(BTN_2_PIN, PULLUP, INVERT, DEBOUNCE_MS);
Button btn3(BTN_3_PIN, PULLUP, INVERT, DEBOUNCE_MS);
Button btn4(BTN_4_PIN, PULLUP, INVERT, DEBOUNCE_MS);
Button btn5(BTN_5_PIN, PULLUP, INVERT, DEBOUNCE_MS);

// Parameter 1 = number of pixels in strip
// Parameter 2 = Arduino pin number (most are valid)
// Parameter 3 = pixel type flags, add together as needed:
//   NEO_KHZ800  800 KHz bitstream (most NeoPixel products w/WS2812 LEDs)
//   NEO_KHZ400  400 KHz (classic 'v1' (not v2) FLORA pixels, WS2811 drivers)
//   NEO_GRB     Pixels are wired for GRB bitstream (most NeoPixel products)
//   NEO_RGB     Pixels are wired for RGB bitstream (v1 FLORA pixels, not v2)
//   NEO_RGBW    Pixels are wired for RGBW bitstream (NeoPixel RGBW products)
Adafruit_NeoPixel neopixel = Adafruit_NeoPixel(5, NEOPIXEL_PIN, NEO_GRB + NEO_KHZ800);
// IMPORTANT: To reduce NeoPixel burnout risk, add 1000 uF capacitor across
// pixel power leads, add 300 - 500 Ohm resistor on first pixel's data input
// and minimize distance between Arduino and first pixel.  Avoid connecting
// on a live circuit...if you must, connect GND first.

// 0 = select, 1 = player, 2 = volume adjust
//The list of possible states for the state machine.
enum STATES {
  STARTUP,     // startup lightshow & sound fx?
  TO_SELECT,   // lights/fx transition
  SELECT,      // color coded song selection mode
  TO_PLAYER,   // lights/fx transition
  PLAYER,      // music player mode
  TO_VOLUME,   // lights/fx transition
  VOLUME,      // volume adjust mode
  TO_SLEEP,    // lights/fx transition
  SLEEP        // shutdown routine
};
uint8_t STATE;

uint8_t curVol;
uint8_t lowVol = 40;
uint8_t regVol = 20;
char selectBuffer[4];
uint8_t selectBufferIndex = 0;
char lastSelection[4];

unsigned long lastTransitionMs;

void setup()
{
  Serial.begin(115200);

  neopixel.begin();
  neopixel.show(); // Initialize all pixels to 'off'

  curVol = regVol;

  if (! musicPlayer.begin()) { // initialise the music player
     Serial.println(F("Couldn't find VS1053, do you have the right pins defined?"));
     while (1);
  }

  Serial.println(F("VS1053 found"));

  musicPlayer.sineTest(0x44, 500);    // Make a tone to indicate VS1053 is working

  if (!SD.begin(CARDCS)) {
    Serial.println(F("SD failed, or not present"));
    while (1);  // don't do anything more
  }
  Serial.println("SD OK!");

  // list files
  printDirectory(SD.open("/"), 0);

  // Set volume for left, right channels. lower numbers == louder volume!
  musicPlayer.setVolume(10,10);

  #if defined(__AVR_ATmega32U4__)
  // Timer interrupts are not suggested, better to use DREQ interrupt!
  // but we don't have them on the 32u4 feather...
  musicPlayer.useInterrupt(VS1053_FILEPLAYER_TIMER0_INT); // timer int
  #elif defined(ESP32)
  // no IRQ! doesn't work yet :/
  #else
  // If DREQ is on an interrupt pin we can do background
  // audio playing
  musicPlayer.useInterrupt(VS1053_FILEPLAYER_PIN_INT);  // DREQ int
  #endif

  btn1.begin(); // mode button
  btn2.begin(); // prev track / vol low
  btn3.begin(); // pause/play track / vol normal
  btn4.begin(); // next track / vol down
  btn5.begin(); // stop track / vol up

  STATE = STARTUP;
}

void loop()
{
  btn1.read();
  btn2.read();
  btn3.read();
  btn4.read();
  btn5.read();

  switch(STATE) {

    case STARTUP:
      Serial.println(F("Transition: STARTUP"));
      // TODO: do startup lightshow & audio fx
      transition(TO_SELECT);
      break;

    case TO_SELECT:
      Serial.println(F("Transition: TO_SELECT"));
      // TODO: do  lightshow & audio fx
      transition(SELECT);
      break;

    case SELECT:
      if (!musicPlayer.stopped() && transitionTimeout(10)) {
        Serial.println(F("Transition: TO_PLAYER (selection timeout)"));
        transition(TO_PLAYER);
      } else if (btn2.wasReleased() || btn3.wasReleased() || btn4.wasReleased() || btn5.wasReleased()) {
        if (btn2.wasReleased()) {
          selectBuffer[selectBufferIndex++] = F("0");
        } else if (btn3.wasReleased()) {
          selectBuffer[selectBufferIndex++] = F("1");
        } else if (btn4.wasReleased()) {
          selectBuffer[selectBufferIndex++] = F("2");
        } else if (btn5.wasReleased()) {
          selectBuffer[selectBufferIndex++] = F("3");
        }

        // color code completed
        if (selectBufferIndex == 4) {
          selectBufferIndex = 0;
          lastSelection = selectBuffer;
          String track = F("song") + str(selectBuffer) + F(".mp3");
          Serial.println(F("Playing track ") + track);
          musicPlayer.startPlayingFile(track);
          transition(TO_PLAYER);
        } else {
          Serial.print(F("Select: code buffer contains "));
          for (int i = 0; i < selectBufferIndex; i++) {
            Serial.print(selectBuffer[i]);
          }
          Serial.println("");
        }
      }

      // TODO: set lights

      break;

    case TO_PLAYER:
      Serial.println(F("Transition: TO_PLAYER"));
      // TODO: do lightshow & audio fx
      transition(PLAYER);
      break;

    case PLAYER:
      // Serial.println("Transition: PLAYER");
      if (btn1.pressedFor(LONG_PRESS)) {
        transition(TO_VOLUME);
      } else if (btn1.wasReleased()) {
        transition(TO_SELECT);
      } else if (btn2.wasReleased()) {
        // prev track
      } else if (btn3.wasReleased()) {
        // pause/play/shuffle
        if (!musicPlayer.paused()) {
          Serial.println(F("Player: paused"));
          musicPlayer.pausePlaying(true);
        } else if (musicPlayer.paused()) {
          Serial.println(F("Player: play"));
          musicPlayer.pausePlaying(false);
        } else {
          Serial.println(F("Player: shuffle"));
          // TODO: play random song
        }
      } else if (btn4.wasReleased()) {
        // next track
      } else if (btn5.wasReleased()) {
        // stop track
        musicPlayer.stopPlaying();
      }

      // TODO: set playerIdle=true after autoplay stops

      break;

    case TO_VOLUME:
      Serial.println(F("Transition: TO_VOLUME"));
      // TODO: do lightshow & audio fx
      transition(VOLUME);
      break;

    case VOLUME:
      // Serial.println("Transition: VOLUME");
      // exit volume mode once btn1 is released
      if (btn1.isPressed()) {
        if (btn2.wasReleased()) {
          Serial.println(F("Vol: set low"));
          changeVolume(lowVol, true, true);
        } else if (btn3.wasReleased()) {
          Serial.println(F("Vol: set normal"));
          changeVolume(lowReg, true, true);
        } else if (btn4.wasReleased()) {
          Serial.println(F("Vol: down"));
          changeVolume(5, false, false);
        } else if (btn5.wasReleased()) {
          Serial.println(F("Vol: up"));
          changeVolume(5, true, false);
        }
      } else {
        transition(TO_PLAYER);
      }

      break;

    case TO_SLEEP:
      Serial.println(F("Transition: TO_SLEEP"));
      // TODO: do  lightshow & audio fx
      transition(SLEEP);
      break;

    case SLEEP:
      // Serial.println("Transition: SLEEP");
      break;
  }

  // Some example procedures showing how to display to the pixels:
  colorWipe(strip.Color(255, 0, 0), 50); // Red
  colorWipe(strip.Color(0, 255, 0), 50); // Green
  colorWipe(strip.Color(0, 0, 255), 50); // Blue
  //colorWipe(strip.Color(0, 0, 0, 255), 50); // White RGBW
  // Send a theater pixel chase in...
  theaterChase(strip.Color(127, 127, 127), 50); // White
  theaterChase(strip.Color(127, 0, 0), 50); // Red
  theaterChase(strip.Color(0, 0, 127), 50); // Blue

  rainbow(20);
  rainbowCycle(20);
  theaterChaseRainbow(50);
}


void transition(toState) {
  lastTransitionMs = millis();
  STATE = toState;
}

bool transitionTimeout(timeoutInSeconds) {
  return millis() - lastTransitionMs >= timeoutInSeconds*1000;
}

void changeVolume(uint8_t delta, bool positive, bool overwrite) {
  if (overwrite) {
    curVol = delta;
  } else if (positive) {
    curVol += delta;
  } else {
    curVol -= delta;
  }

  if (curVol > 60) {
    curVol = 60;
  } else if (curVol < 0) {
    curVol = 0;
  }
  musicPlayer.setVolume(curVol,curVol);

  Serial.println("Vol: currently " + curVol);
}

/// File listing helper
void printDirectory(File dir, int numTabs) {
  // TODO: save tracks to memory for lookups and shuffling
   while(true) {
     File entry =  dir.openNextFile();
     if (! entry) {
       // no more files
       //Serial.println("**nomorefiles**");
       break;
     }
     for (uint8_t i=0; i<numTabs; i++) {
       Serial.print('\t');
     }
     Serial.print(entry.name());
     if (entry.isDirectory()) {
       Serial.println("/");
       printDirectory(entry, numTabs+1);
     } else {
       // files have sizes, directories do not
       Serial.print("\t\t");
       Serial.println(entry.size(), DEC);
     }
     entry.close();
   }
}


// Fill the dots one after the other with a color
void colorWipe(uint32_t c, uint8_t wait) {
  for(uint16_t i=0; i<neopixel.numPixels(); i++) {
    neopixel.setPixelColor(i, c);
    neopixel.show();
    delay(wait);
  }
}

void rainbow(uint8_t wait) {
  uint16_t i, j;

  for(j=0; j<256; j++) {
    for(i=0; i<neopixel.numPixels(); i++) {
      neopixel.setPixelColor(i, Wheel((i+j) & 255));
    }
    neopixel.show();
    delay(wait);
  }
}

// Slightly different, this makes the rainbow equally distributed throughout
void rainbowCycle(uint8_t wait) {
  uint16_t i, j;

  for(j=0; j<256*5; j++) { // 5 cycles of all colors on wheel
    for(i=0; i< neopixel.numPixels(); i++) {
      neopixel.setPixelColor(i, Wheel(((i * 256 / neopixel.numPixels()) + j) & 255));
    }
    neopixel.show();
    delay(wait);
  }
}

//Theatre-style crawling lights.
void theaterChase(uint32_t c, uint8_t wait) {
  for (int j=0; j<10; j++) {  //do 10 cycles of chasing
    for (int q=0; q < 3; q++) {
      for (uint16_t i=0; i < neopixel.numPixels(); i=i+3) {
        neopixel.setPixelColor(i+q, c);    //turn every third pixel on
      }
      neopixel.show();

      delay(wait);

      for (uint16_t i=0; i < neopixel.numPixels(); i=i+3) {
        neopixel.setPixelColor(i+q, 0);        //turn every third pixel off
      }
    }
  }
}

//Theatre-style crawling lights with rainbow effect
void theaterChaseRainbow(uint8_t wait) {
  for (int j=0; j < 256; j++) {     // cycle all 256 colors in the wheel
    for (int q=0; q < 3; q++) {
      for (uint16_t i=0; i < neopixel.numPixels(); i=i+3) {
        neopixel.setPixelColor(i+q, Wheel( (i+j) % 255));    //turn every third pixel on
      }
      neopixel.show();

      delay(wait);

      for (uint16_t i=0; i < neopixel.numPixels(); i=i+3) {
        neopixel.setPixelColor(i+q, 0);        //turn every third pixel off
      }
    }
  }
}

// Input a value 0 to 255 to get a color value.
// The colours are a transition r - g - b - back to r.
uint32_t Wheel(byte WheelPos) {
  WheelPos = 255 - WheelPos;
  if(WheelPos < 85) {
    return neopixel.Color(255 - WheelPos * 3, 0, WheelPos * 3);
  }
  if(WheelPos < 170) {
    WheelPos -= 85;
    return neopixel.Color(0, WheelPos * 3, 255 - WheelPos * 3);
  }
  WheelPos -= 170;
  return neopixel.Color(WheelPos * 3, 255 - WheelPos * 3, 0);
}
