
static char hiscore_table[10][28]= {
  "1: 0123456789ABCDE: 1234567",
  "2: 0123456789ABCDE: 1234567",
  "3: 0123456789ABCDE: 1234567",
  "4: 0123456789ABCDE: 1234567",
  "5: 0123456789ABCDE: 1234567",
  "6: 0123456789ABCDE: 1234567",
  "7: 0123456789ABCDE: 1234567",
  "8: 0123456789ABCDE: 1234567",
  "9: 0123456789ABCDE: 1234567",
  "10 0123456789ABCDE: 1234567"
};

static uint16_t menu_entry;
uint8_t visible_entry;
int16_t hi_score_scroll, hi_score_scroll_len;

void draw_hiscore_table() {

  if (menu_entry >= visible_entry) {
    visible_entry = menu_entry + 1;
  } else if (menu_entry < visible_entry - 3) {
    visible_entry = menu_entry + 3;
  }

  for (uint8_t i = visible_entry - 3; i < visible_entry; i++) {
    if (i == menu_entry) {
      for(uint8_t k=0;k<W;k++) {
        for(uint8_t j = 19 - (6 * (i - (visible_entry - 3))) - 5; j < 19 - (6 * (i - (visible_entry - 3))); j++) {
          LED(k,j) = 0x202020;
        }
      }
      text_scroll(hiscore_table[i], hi_score_scroll>>1, 0, W, 19 - (6 * (i - (visible_entry - 3))) - 5, CRGB::White);
    } else {
      text_str(hiscore_table[i], 0, 19 - (6 * (i - (visible_entry - 3))) - 5, 0, W, CRGB::White);
    }
  }
  
}

void load_hiscore_entity(uint8_t score_position) {
  uint16_t eeprom_position = 100 + (score_position * 20);
  // check if there's a user name in eeprom for this position
  uint8_t isNameSet = 0;

  EEPROM.get(eeprom_position + 1, isNameSet);
  if(isNameSet != 0) {
    uint8_t i = 0;

    // Load user name from eeprom
    do { 
      EEPROM.get(eeprom_position + 1 + i, hiscore_table[score_position][i + 3]);
      i++;
    } while((i < 16) && (hiscore_table[score_position][i + 3 - 1]));
    strcat(hiscore_table[score_position], ": ");
  } else {
     // no name in eeprom: attach value directly
    strcpy(hiscore_table[score_position]+3, "");
  }

  // load hi score for current position
  uint32_t hiscore = 0;
  if(EEPROM.read(eeprom_position + 16) == 0x42)
    EEPROM.get(eeprom_position + 17, hiscore);
    
  ltoa(hiscore, hiscore_table[score_position]+strlen(hiscore_table[score_position]), 10);
}

/*
 * Initialize hiscore menu.
 */
void hiscore_init(uint8_t sel_entry) {
  LEDS.clear();
  LEDS.setBrightness(config_brightness);
  menu_entry = sel_entry;
  visible_entry = 3;
  hi_score_scroll = -W;
  hi_score_scroll_len = text_str_len(hiscore_table[0]);

  for(int8_t i=0; i<10; i++) {
    load_hiscore_entity(i);  
  }
  draw_hiscore_table();
}

uint8_t hiscore_process(uint8_t keys) {

    if((keys & KEY_DROP) && (menu_entry > 0)) {
      menu_entry--;
      keys_lock();      // prevent auto repeat
    }
    
    if((keys & KEY_DOWN) && (menu_entry < 9)) {
      menu_entry++;
      keys_lock();      // prevent auto repeat
    }
    
   if(keys) {
    hi_score_scroll = -W;
    hi_score_scroll_len = text_str_len(hiscore_table[menu_entry]);
   }
   
   if(++hi_score_scroll > 2*(hi_score_scroll_len))
    hi_score_scroll = -2*W;

   LEDS.clear();
   draw_hiscore_table();
    
   return keys;
}

uint32_t get_score_from_position(uint8_t score_position) {
  uint32_t score = 0;

  if(EEPROM.read(100 + (score_position * 20) + 16) == 0x42) {
      EEPROM.get(100 + (score_position * 20) + 17, score);
  }

  return score;
}

uint32_t get_lowest_score() {
  uint32_t lowest_hiscore = 0;

  for (uint8_t i = 9; i > 0; i--) {
    if(EEPROM.read(100 + (i * 20) + 16) == 0x42) {
      EEPROM.get(100 + (i * 20) + 17, lowest_hiscore);
      break;
    }
  }
  return lowest_hiscore;
}

uint8_t store_new_hiscore(uint32_t new_hiscore) {

  uint8_t score_position = 0;
  uint32_t score = 0;

  do {
    if(EEPROM.read(100 + (score_position * 20) + 16) == 0x42) {
      EEPROM.get(100 + (score_position * 20) + 17, score);
    }
    score_position++;
  } while(score >= new_hiscore);
  score_position--;

  save_newscore_at(new_hiscore, score_position);
  return score_position;
}

void save_newscore_at(uint32_t new_score, uint8_t score_position) {
  uint16_t eeprom_position = 0;
  uint32_t temp_score = 0;
  char temp_name[16];
  char new_name[16];

  boolean set_new_name = false;
  
  for (uint8_t i = score_position; i < 10; i++) {

    eeprom_position = 100 + (i * 20);

    // load old hiscore at this position
    if(EEPROM.read(eeprom_position + 16) == 0x42) {
      EEPROM.get(eeprom_position + 17, temp_score);
    }

    // load username at this position
    for(uint8_t j=0; j<15; j++) {
      EEPROM.get(eeprom_position + 1 + j, temp_name[j]);
      if (set_new_name) {
        EEPROM.put(eeprom_position + 1 + j, new_name[j]);
      }
      new_name[j] = temp_name[j];
    }
    set_new_name = true;
    EEPROM.write(eeprom_position + 16, 0x42);   // write magic marker
    EEPROM.put(eeprom_position + 17, new_score);

    new_score = temp_score;
  }
}

/**
 * Username must not be bigger then 15 characters.
 */
void save_name_at(char *username, uint8_t score_position) {
  uint8_t i = 0;

  // remove old username
  for(uint8_t j=0; j<15; j++) {
     EEPROM.put(100 + (score_position * 20) + 1 + j, "");
  }

  while(*username) {
    EEPROM.put(100 + (score_position * 20) + 1 + i, *username);
    *username++;
    i++;
  }

}

