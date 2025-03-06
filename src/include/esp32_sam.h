#ifndef __ESP32_SAM__
#define __ESP32_SAM__

#ifdef __cplusplus
extern "C" {
#endif

int sam_set_text(const char* text, int is_phonetic);
void sam_set_speed(unsigned char speed);
void sam_set_pitch(unsigned char pitch);
void sam_set_throat(unsigned char throat);
void sam_set_mouth(unsigned char mouth);
void sam_enable_sing_mode(int enable);
void sam_enable_debug(int enable);
char* sam_get_buffer(void);
int sam_get_buffer_length(void);

void strcat_s(char* dest, int size, char* str);

#ifdef __cplusplus
}
#endif

#endif  // __ESP32_SAM__
