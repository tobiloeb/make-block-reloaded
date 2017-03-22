#include "tetris.h"
#include "SNESpaduino.h"

// bit 0 = last state
// bit 1 = repeat state
// bit 2-7 = repeat counter

uint8_t key_state[7];
SNESpaduino pad(5, 6, 4);

static const uint16_t key_pins[] = {
  BTN_LEFT, BTN_RIGHT, BTN_B, BTN_DOWN, BTN_UP, BTN_START, BTN_SELECT };

void keys_init() {
  for(uint8_t i=0;i<7;i++) {
    // enable internal pullups
    // pinMode(key_pins[i], INPUT_PULLUP);
    key_state[i] = 0;
  }
}

// lock the auto repeat
void keys_lock() {
  for(uint8_t i=0;i<7;i++)
    key_state[i] |= 0xfe;
}

// return whether any button is currently pressed
uint8_t keys_any_down() {
  if (pad.getButtons() > 61440) {
    return 1;
  }
  return 0;
}

uint8_t button_status(uint16_t button) {
  if (pad.getButtons() & button) {
    return 1;
  }
  return 0;
}

// mode 0 = game, 1 = config, 2 = initials
uint8_t keys_get(uint8_t mode) {
  uint8_t ret = 0;

  // rotate key does not repeat
  for(uint8_t i=0;i<7;i++) {
    // has key state changed?
    if(button_status(key_pins[i]) != (key_state[i]&1)) {
      key_state[i] &= 1;     // clear all counter bits
      key_state[i] ^= 1;     // toggle state bit 
      if(key_state[i] & 1) { /* key has just been pressed */
      	ret |= 1<<i;
      
      	// check if down is pressed while up (drop) was already pressed
      	// or vice versa
      	if((((1<<i)==KEY_DROP) && button_status(KEY_DOWN_PIN)) ||
      	   (((1<<i)==KEY_DOWN) && button_status(KEY_DROP_PIN))) 
      	  ret |= KEY_PAUSE;
      }
    } else if(key_state[i] & 1) {
      // key is kept pressed. This will cause some repeat on some keys

      // increase counter value in bits 2..7, saturate counter
      if((key_state[i] >> 2) < 63)
	      key_state[i] = key_state[i]+4; 

      uint8_t counter = key_state[i]>>2;

      // repeat for horizontal movement
      if(((1<<i) == KEY_LEFT) || ((1<<i)==KEY_RIGHT)) {
      	// "DAS" delay of 24 Frames according to tetris concept
      	// afterwards 1/9G
      	if((!(key_state[i] & 2) && (counter == 24)) ||
      	   ( (key_state[i] & 2) && (counter == 9) && (mode != 1)) ||
      	   ( (key_state[i] & 2) && (counter == 1) && (mode == 1))) {
      	  key_state[i] = 3;   // restart counter for continous repeat
      	  ret |= 1<<i;        // report key
      	}
      }

      // mode = 0/1 = normal game mode
      if(mode != 2) {
      	// "soft drop"
      	if((1<<i) == KEY_DOWN) {
      	  if(counter == 3) {    // 1/3G
      	    key_state[i] = 3;   // restart counter for continous repeat
      	    ret |= 1<<i;        // report key
      	  }
      	}
      } else {
      	// key repear for "initials" enter dialog
      	if(((1<<i) == KEY_DOWN) || ((1<<i) == KEY_DROP)) {
      	  if((!(key_state[i] & 2) && (counter == 24)) ||
      	     ( (key_state[i] & 2) && (counter == 9) )) {
      	    key_state[i] = 3;   // restart counter for continous repeat
      	    ret |= 1<<i;        // report key
      	  }
      	}
      }
    }
  }
        
  return ret;
}

