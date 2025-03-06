#include "esp32_sam.h"

#include <ctype.h>
#include <stdio.h>
#include <string.h>

#include "debug.h"
#include "reciter.h"
#include "sam.h"

int debug = 0;

void strcat_s(char* dest, int size, char* str) {
  unsigned int dlen = strlen(dest);
  if (dlen >= size - 1) return;
  strncat(dest + dlen, str, size - dlen - 1);
}

int sam_set_text(const char* text, int is_phonetic) {
  unsigned char input[256];
  memset(input, 0, 256);

  strncpy((char*)input, text, 256);

  for (int i = 0; input[i] != 0; i++)
    input[i] = (unsigned char)toupper((int)input[i]);

  if (debug) {
    if (is_phonetic)
      printf("phonetic input: %s\n", input);
    else
      printf("text input: %s\n", input);
  }

  if (!is_phonetic) {
    strcat_s((char*)input, 256, "[");
    if (!TextToPhonemes(input)) return 1;
    if (debug) printf("phonetic input: %s\n", input);
  } else {
    strcat_s((char*)input, 256, "\x9b");
  }

  SetInput(input);
  return SAMMain();
}

void sam_set_speed(unsigned char speed) { SetSpeed(speed); }

void sam_set_pitch(unsigned char pitch) { SetPitch(pitch); }

void sam_set_throat(unsigned char throat) { SetThroat(throat); }

void sam_set_mouth(unsigned char mouth) { SetMouth(mouth); }

void sam_enable_sing_mode(int enable) { EnableSingmode(); }

void sam_enable_debug(int enable) { debug = enable; }

char* sam_get_buffer(void) { return GetBuffer(); }

int sam_get_buffer_length(void) { return GetBufferLength() / 50; }
