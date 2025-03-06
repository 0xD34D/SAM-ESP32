
#ifndef __ESP32_SAM__
#define __ESP32_SAM__

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Sets the text to be spoken by the SAM.
 *
 * @param text The text to be spoken.
 * @param is_phonetic Flag indicating if the text is phonetic.
 *                    1 for phonetic, 0 for regular text.
 * @return 0 on success, non-zero on failure.
 */
int sam_set_text(const char* text, int is_phonetic);

/**
 * @brief Sets the speaking speed.
 *
 * @param speed The desired speaking speed (0-255).
 */
void sam_set_speed(unsigned char speed);

/**
 * @brief Sets the speaking pitch.
 *
 * @param pitch The desired speaking pitch (0-255).
 */
void sam_set_pitch(unsigned char pitch);

/**
 * @brief Sets the speaking throat parameter.
 *
 * @param throat The desired throat parameter (0-255).
 */
void sam_set_throat(unsigned char throat);

/**
 * @brief Sets the speaking mouth parameter.
 *
 * @param mouth The desired mouth parameter (0-255).
 */
void sam_set_mouth(unsigned char mouth);

/**
 * @brief Enables or disables the sing mode.
 *        Sing mode modifies the pitch.
 *
 * @param enable 1 to enable, 0 to disable.
 */
void sam_enable_sing_mode(int enable);

/**
 * @brief Enables or disables debug messages.
 *
 * @param enable 1 to enable, 0 to disable.
 */
void sam_enable_debug(int enable);

/**
 * @brief Gets the pointer to the audio buffer.
 *
 * @return A pointer to the audio buffer, which is unsigned 8-bit PCM.
 */
char* sam_get_buffer(void);

/**
 * @brief Gets the length of the audio buffer.
 *
 * @return The length of the audio buffer in bytes.
 */
int sam_get_buffer_length(void);

/**
 * @brief Concatenates a string to another string safely.
 *
 * @param dest The destination string.
 * @param size The maximum size of the destination string.
 * @param str The source string to be concatenated.
 */
void strcat_s(char* dest, int size, const char* str);

#ifdef __cplusplus
}
#endif

#endif  // __ESP32_SAM__
