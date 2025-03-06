#include <ctype.h>
#include <stdio.h>
#include <string.h>

#include "driver/i2s_std.h"
#include "esp32_sam.h"
#include "esp_console.h"
#include "esp_err.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"

#define PROMPT_STR CONFIG_IDF_TARGET

#define I2S_DOUT 4
#define I2S_CLK 5
#define I2S_WS 6

const char *TAG = "SAM-ESP32";

static i2s_chan_handle_t tx_chan;  // I2S tx channel handler
static esp_console_repl_t *repl = NULL;

static int console_configure_sam(int argc, char **argv);
static int console_say_text(int argc, char **argv);

const esp_console_cmd_t cmds[] = {
    {
        .command = "configure",
        .help = "Configure SAM",
        .hint = NULL,
        .func = &console_configure_sam,
    },
    {
        .command = "say",
        .help = "say a phrase",
        .hint = NULL,
        .func = &console_say_text,
    },
};

static int min(int l, int r) { return l < r ? l : r; }

void setup_i2s(int ws_pin, int dout_pin, int clk_pin, uint32_t sample_rate_hz) {
  i2s_std_config_t tx_std_cfg = {
      .clk_cfg = I2S_STD_CLK_DEFAULT_CONFIG(sample_rate_hz),
      .slot_cfg = I2S_STD_MSB_SLOT_DEFAULT_CONFIG(I2S_DATA_BIT_WIDTH_16BIT,
                                                  I2S_SLOT_MODE_MONO),
      .gpio_cfg =
          {
              .mclk = I2S_GPIO_UNUSED,
              .bclk = (gpio_num_t)clk_pin,
              .ws = (gpio_num_t)ws_pin,
              .dout = (gpio_num_t)dout_pin,
              .din = I2S_GPIO_UNUSED,
              .invert_flags =
                  {
                      .mclk_inv = false,
                      .bclk_inv = false,
                      .ws_inv = false,
                  },
          },
  };

  i2s_chan_config_t tx_chan_cfg =
      I2S_CHANNEL_DEFAULT_CONFIG(I2S_NUM_AUTO, I2S_ROLE_MASTER);
  ESP_ERROR_CHECK(i2s_new_channel(&tx_chan_cfg, &tx_chan, NULL));
  ESP_ERROR_CHECK(i2s_channel_init_std_mode(tx_chan, &tx_std_cfg));
}

void sam_speak() {
  char *buf = sam_get_buffer();
  int buf_len = sam_get_buffer_length();
  ESP_LOGI(TAG, "Buffer: %p, Length: %d", buf, buf_len);

  int16_t *output_buf = (int16_t *)malloc(buf_len * 2);
  if (output_buf == NULL) {
    ESP_LOGE(TAG, "Failed to allocate output buffer");
    return;
  }

  // convert 8 bit unsigned to 16 bit signed and attenuate
  for (int i = 0; i < buf_len; i++) {
    output_buf[i] = ((buf[i] * 32767 / 255) - 16384) >> 2;
  }

  buf_len = buf_len * 2;
  size_t total_bytes_written = 0;
  size_t bytes_written = 0;
  ESP_ERROR_CHECK(i2s_channel_enable(tx_chan));
  while (total_bytes_written < buf_len) {
    i2s_channel_write(tx_chan, output_buf + total_bytes_written,
                      buf_len - total_bytes_written, &bytes_written,
                      portMAX_DELAY);
    total_bytes_written += bytes_written;
  }

  free(output_buf);
  ESP_ERROR_CHECK(i2s_channel_disable(tx_chan));
}

void print_config_usage() {
  printf("usage: configure [options]\n");
  printf("options\n");
  printf("	-pitch number		set pitch value (default=64)\n");
  printf("	-speed number		set speed value (default=72)\n");
  printf("	-throat number		set throat value (default=128)\n");
  printf("	-mouth number		set mouth value (default=128)\n");
  printf("	-sing			special treatment of pitch\n");
  printf("	-debug			print additional debug messages\n");
  printf("\n");
}

void print_say_usage() {
  printf("usage: say [options] Word1 Word2 ....\n");
  printf("options\n");
  printf("	-phonetic 		enters phonetic mode. (see below)\n");
  printf("\n");

  printf("     VOWELS                            VOICED CONSONANTS	\n");
  printf("IY           f(ee)t                    R        red		\n");
  printf("IH           p(i)n                     L        allow		\n");
  printf("EH           beg                       W        away		\n");
  printf("AE           Sam                       W        whale		\n");
  printf("AA           pot                       Y        you		\n");
  printf("AH           b(u)dget                  M        Sam		\n");
  printf("AO           t(al)k                    N        man		\n");
  printf(
      "OH           cone                      NX       so(ng)	"
      "	\n");
  printf("UH           book                      B        bad		\n");
  printf("UX           l(oo)t                    D        dog		\n");
  printf("ER           bird                      G        again		\n");
  printf("AX           gall(o)n                  J        judge		\n");
  printf("IX           dig(i)t                   Z        zoo		\n");
  printf("				       ZH       plea(s)ure	\n");
  printf("   DIPHTHONGS                          V        seven		\n");
  printf(
      "EY           m(a)de                    DH       (th)en	"
      "	\n");
  printf("AY           h(igh)						\n");
  printf("OY           boy						\n");
  printf("AW           h(ow)                     UNVOICED CONSONANTS	\n");
  printf("OW           slow                      S         Sam		\n");
  printf("UW           crew                      Sh        fish		\n");
  printf("                                       F         fish		\n");
  printf("                                       TH        thin		\n");
  printf(" SPECIAL PHONEMES                      P         poke		\n");
  printf("UL           sett(le) (=AXL)           T         talk		\n");
  printf("UM           astron(omy) (=AXM)        K         cake		\n");
  printf(
      "UN           functi(on) (=AXN)         CH        speech	"
      "	\n");
  printf("Q            kitt-en (glottal stop)    /H        a(h)ead	\n");
}

static int console_configure_sam(int argc, char **argv) {
  if (argc <= 1) {
    print_config_usage();
    return 1;
  }

  unsigned char input[256];
  memset(input, 0, 256);

  int i = 1;
  while (i < argc) {
    if (argv[i][0] != '-') {
      strcat_s((char *)input, 256, argv[i]);
      strcat_s((char *)input, 256, " ");
    } else {
      if (strcmp(&argv[i][1], "sing") == 0) {
        sam_enable_sing_mode(1);
        i++;
      } else if (strcmp(&argv[i][1], "debug") == 0) {
        sam_enable_debug(1);
        i++;
      } else if (strcmp(&argv[i][1], "pitch") == 0) {
        sam_set_pitch((unsigned char)min(atoi(argv[i + 1]), 255));
        i++;
      } else if (strcmp(&argv[i][1], "speed") == 0) {
        sam_set_speed((unsigned char)min(atoi(argv[i + 1]), 255));
        i++;
      } else if (strcmp(&argv[i][1], "mouth") == 0) {
        sam_set_mouth((unsigned char)min(atoi(argv[i + 1]), 255));
        i++;
      } else if (strcmp(&argv[i][1], "throat") == 0) {
        sam_set_throat((unsigned char)min(atoi(argv[i + 1]), 255));
        i++;
      } else {
        print_config_usage();
        return 1;
      }
    }

    i++;
  }

  return 0;
}

static int console_say_text(int argc, char **argv) {
  if (argc <= 1) {
    print_say_usage();
    return 1;
  }

  char input[256];
  memset(input, 0, 256);

  int phonetic = 0;
  int i = 1;
  while (i < argc) {
    if (argv[i][0] != '-') {
      strcat_s(input, 256, argv[i]);
      strcat_s(input, 256, " ");
    } else {
      if (strcmp(&argv[i][1], "phonetic") == 0) {
        phonetic = 1;
      } else {
        print_say_usage();
        return 1;
      }
    }

    i++;
  }

  for (i = 0; input[i] != 0; i++) input[i] = toupper((int)input[i]);

  sam_set_text(input, phonetic);
  sam_speak();

  return 0;
}

void app_main(void) {
  setup_i2s(I2S_WS, I2S_DOUT, I2S_CLK, 22050);

  esp_console_repl_config_t repl_config = ESP_CONSOLE_REPL_CONFIG_DEFAULT();
  /* Prompt to be printed before each line.
   * This can be customized, made dynamic, etc.
   */
  repl_config.prompt = PROMPT_STR ">";
  repl_config.max_cmdline_length = 128;

// Init console based on menuconfig settings
#if defined(CONFIG_ESP_CONSOLE_UART_DEFAULT) || \
    defined(CONFIG_ESP_CONSOLE_UART_CUSTOM)
  esp_console_dev_uart_config_t hw_config =
      ESP_CONSOLE_DEV_UART_CONFIG_DEFAULT();
  ESP_ERROR_CHECK(esp_console_new_repl_uart(&hw_config, &repl_config, &repl));

// USJ console can be set only on esp32p4, having separate USB PHYs for
// USB_OTG and USJ
#elif defined(CONFIG_ESP_CONSOLE_USB_SERIAL_JTAG)
  esp_console_dev_usb_serial_jtag_config_t hw_config =
      ESP_CONSOLE_DEV_USB_SERIAL_JTAG_CONFIG_DEFAULT();
  ESP_ERROR_CHECK(
      esp_console_new_repl_usb_serial_jtag(&hw_config, &repl_config, &repl));

#else
#error Unsupported console type
#endif

  for (int count = 0; count < sizeof(cmds) / sizeof(esp_console_cmd_t);
       count++) {
    ESP_ERROR_CHECK(esp_console_cmd_register(&cmds[count]));
  }

  ESP_ERROR_CHECK(esp_console_start_repl(repl));
}
