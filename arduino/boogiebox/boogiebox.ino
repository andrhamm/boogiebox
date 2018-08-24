#include <JC_Button.h>

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
#define LONG_PRESS 500     //We define a "long press" to be 500 milliseconds.

Button btn1(BTN_1_PIN, PULLUP, INVERT, DEBOUNCE_MS);
Button btn2(BTN_2_PIN, PULLUP, INVERT, DEBOUNCE_MS);
Button btn3(BTN_3_PIN, PULLUP, INVERT, DEBOUNCE_MS);
Button btn4(BTN_4_PIN, PULLUP, INVERT, DEBOUNCE_MS);
Button btn5(BTN_5_PIN, PULLUP, INVERT, DEBOUNCE_MS);

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

unsigned long lastTransitionMs;

void setup()
{
  Serial.begin(115200);
  
  btn1.begin();
  btn2.begin();
  btn3.begin();
  btn4.begin();
  btn5.begin();
  
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
      Serial.println("Transition: STARTUP");
      // TODO: do startup lightshow & audio fx
      transition(TO_SELECT);
      break;
    
    case TO_SELECT:
      Serial.println("Transition: TO_SELECT");
      // TODO: do  lightshow & audio fx
      transition(SELECT);
      break;
      
    case SELECT:
      // Serial.println("Transition: SELECT");
      btn2
      btn3
      btn4
      btn5
    
      if () {
        
      }
      
      break;
      
    case TO_PLAYER:
      Serial.println("Transition: TO_PLAYER");
      // TODO: do lightshow & audio fx
      transition(PLAYER);
      break;
      
    case PLAYER:
      // Serial.println("Transition: PLAYER");
      
      break;
      
    case TO_VOLUME:
      Serial.println("Transition: TO_VOLUME");
      // TODO: do lightshow & audio fx
      transition(VOLUME);
      break;
      
    case VOLUME:
      // Serial.println("Transition: VOLUME");
      break;
    
    case TO_SLEEP:
      Serial.println("Transition: TO_SLEEP");
      // TODO: do  lightshow & audio fx
      transition(SLEEP);
      break;
      
    case SLEEP:
      // Serial.println("Transition: SLEEP");
      break;
  }
  
  if (mode == 0) {
    // select mode
    
    // btn1 = reset color buffer [long press to switch to player mode]
    // btn2 = color1
    // btn3 = color2
    // btn4 = color3
    // btn5 = color4
  } else if (mode == 1) {
    // player mode
    
    // btn1 = switch to select mode [long press to enter volume adjust mode]
    // btn2 = prev track
    // btn3 = pause/play
    // btn4 = next track
    // btn5 = stop
    if (btn1.getSingleDebouncedPress()) {
      
      mode = 1;
    }
    } else if (btn2.getSingleDebouncedRelease()) {
      // btn2 = volume low (quiet but audible)
    } else if (btn3.getSingleDebouncedRelease()) {
      // btn3 = volume normal (default volume, tested & reasonable)
    } else if (btn4.getSingleDebouncedRelease()) {
      // btn4 = volume down
    } else if (btn5.getSingleDebouncedRelease()) {
      // btn5 = volume up
    }
  } else {
    // volume adjust mode
    
    if (btn1.isPressed()) {
      // btn1 [always held down] else return to player mode on release
      
      if (btn2.getSingleDebouncedRelease()) {
      // btn2 = volume low (quiet but audible)
      } else if (btn3.getSingleDebouncedRelease()) {
        // btn3 = volume normal (default volume, tested & reasonable)
      } else if (btn4.getSingleDebouncedRelease()) {
        // btn4 = volume down
      } else if (btn5.getSingleDebouncedRelease()) {
        // btn5 = volume up
      }
    } else {
      // button is released, return to player mode
      mode = 1;
      delay(20);
    }
  }
}


void transition(toState) {
  lastTransitionMs = millis();
  STATE = toState;
}

bool transitionTimeout(timeout) {
  return millis() - lastTransitionMs >= timeout;
}
