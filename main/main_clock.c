/*
 * ESP32 Internet Clock using ESP-IDF
 * Hardware: ELEGOO ESP32, 0.96" SSD1306 OLED (I2C)
 * Language: C
 */

#include <stdio.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "esp_sntp.h"
#include "nvs_flash.h"
#include "driver/i2c.h"

/* --- Configuration --- */
#define I2C_MASTER_SCL_IO           22
#define I2C_MASTER_SDA_IO           21
#define I2C_MASTER_NUM              I2C_NUM_0
#define I2C_MASTER_FREQ_HZ          400000
#define OLED_I2C_ADDRESS            0x3C

// Wi-Fi Configuration pulled safely from menuconfig
#define ESP_WIFI_SSID      CONFIG_ESP_WIFI_SSID
#define ESP_WIFI_PASS      CONFIG_ESP_WIFI_PASSWORD

static const char *TAG = "CLOCK";

// FreeRTOS Event Group for Wi-Fi
static EventGroupHandle_t s_wifi_event_group;
#define WIFI_CONNECTED_BIT BIT0

/* --- Minimal 5x8 Font (ASCII 32-90 for time & date display) --- */
static const uint8_t font5x8[][5] = {
    {0x00, 0x00, 0x00, 0x00, 0x00}, // 32: Space
    {0x00, 0x00, 0x4f, 0x00, 0x00}, // 33: !
    {0x00, 0x07, 0x00, 0x07, 0x00}, // 34: "
    {0x14, 0x7f, 0x14, 0x7f, 0x14}, // 35: #
    {0x24, 0x2a, 0x7f, 0x2a, 0x12}, // 36: $
    {0x23, 0x13, 0x08, 0x64, 0x62}, // 37: %
    {0x36, 0x49, 0x55, 0x22, 0x50}, // 38: &
    {0x00, 0x05, 0x03, 0x00, 0x00}, // 39: '
    {0x00, 0x1c, 0x22, 0x41, 0x00}, // 40: (
    {0x00, 0x41, 0x22, 0x1c, 0x00}, // 41: )
    {0x14, 0x08, 0x3E, 0x08, 0x14}, // 42: *
    {0x08, 0x08, 0x3E, 0x08, 0x08}, // 43: +
    {0x00, 0x00, 0x50, 0x30, 0x00}, // 44: ,
    {0x08, 0x08, 0x08, 0x08, 0x08}, // 45: -
    {0x00, 0x60, 0x60, 0x00, 0x00}, // 46: .
    {0x20, 0x10, 0x08, 0x04, 0x02}, // 47: /
    {0x3E, 0x51, 0x49, 0x45, 0x3E}, // 48: 0
    {0x00, 0x42, 0x7F, 0x40, 0x00}, // 49: 1
    {0x42, 0x61, 0x51, 0x49, 0x46}, // 50: 2
    {0x21, 0x41, 0x45, 0x4B, 0x31}, // 51: 3
    {0x18, 0x14, 0x12, 0x7F, 0x10}, // 52: 4
    {0x27, 0x45, 0x45, 0x45, 0x39}, // 53: 5
    {0x3C, 0x4A, 0x49, 0x49, 0x30}, // 54: 6
    {0x01, 0x71, 0x09, 0x05, 0x03}, // 55: 7
    {0x36, 0x49, 0x49, 0x49, 0x36}, // 56: 8
    {0x06, 0x49, 0x49, 0x29, 0x1E}, // 57: 9
    {0x00, 0x36, 0x36, 0x00, 0x00}, // 58: :
    {0x00, 0x56, 0x36, 0x00, 0x00}, // 59: ;
    {0x08, 0x14, 0x22, 0x41, 0x00}, // 60: <
    {0x14, 0x14, 0x14, 0x14, 0x14}, // 61: =
    {0x00, 0x41, 0x22, 0x14, 0x08}, // 62: >
    {0x02, 0x01, 0x51, 0x09, 0x06}, // 63: ?
    {0x32, 0x49, 0x59, 0x51, 0x3E}, // 64: @
    {0x7E, 0x11, 0x11, 0x11, 0x7E}, // 65: A
    {0x7F, 0x49, 0x49, 0x49, 0x36}, // 66: B
    {0x3E, 0x41, 0x41, 0x41, 0x22}, // 67: C
    {0x7F, 0x41, 0x41, 0x22, 0x1C}, // 68: D
    {0x7F, 0x49, 0x49, 0x49, 0x41}, // 69: E
    {0x7F, 0x09, 0x09, 0x09, 0x01}, // 70: F
    {0x3E, 0x41, 0x49, 0x49, 0x7A}, // 71: G
    {0x7F, 0x08, 0x08, 0x08, 0x7F}, // 72: H
    {0x00, 0x41, 0x7F, 0x41, 0x00}, // 73: I
    {0x20, 0x40, 0x41, 0x3F, 0x01}, // 74: J
    {0x7F, 0x08, 0x14, 0x22, 0x41}, // 75: K
    {0x7F, 0x40, 0x40, 0x40, 0x40}, // 76: L
    {0x7F, 0x02, 0x0C, 0x02, 0x7F}, // 77: M
    {0x7F, 0x04, 0x08, 0x10, 0x7F}, // 78: N
    {0x3E, 0x41, 0x41, 0x41, 0x3E}, // 79: O
    {0x7F, 0x09, 0x09, 0x09, 0x06}, // 80: P
    {0x3E, 0x41, 0x51, 0x21, 0x5E}, // 81: Q
    {0x7F, 0x09, 0x19, 0x29, 0x46}, // 82: R
    {0x46, 0x49, 0x49, 0x49, 0x31}, // 83: S
    {0x01, 0x01, 0x7F, 0x01, 0x01}, // 84: T
    {0x3F, 0x40, 0x40, 0x40, 0x3F}, // 85: U
    {0x1F, 0x20, 0x40, 0x20, 0x1F}, // 86: V
    {0x3F, 0x40, 0x38, 0x40, 0x3F}, // 87: W
    {0x63, 0x14, 0x08, 0x14, 0x63}, // 88: X
    {0x07, 0x08, 0x70, 0x08, 0x07}, // 89: Y
    {0x61, 0x51, 0x49, 0x45, 0x43}  // 90: Z
};

/* --- I2C / SSD1306 Display Driver --- */
static void i2c_master_init() {
    i2c_config_t conf = {
        .mode = I2C_MODE_MASTER,
        .sda_io_num = I2C_MASTER_SDA_IO,
        .sda_pullup_en = GPIO_PULLUP_ENABLE,
        .scl_io_num = I2C_MASTER_SCL_IO,
        .scl_pullup_en = GPIO_PULLUP_ENABLE,
        .master.clk_speed = I2C_MASTER_FREQ_HZ,
    };
    i2c_param_config(I2C_MASTER_NUM, &conf);
    i2c_driver_install(I2C_MASTER_NUM, conf.mode, 0, 0, 0);
}

static void ssd1306_send_command(uint8_t cmd) {
    i2c_cmd_handle_t cmd_handle = i2c_cmd_link_create();
    i2c_master_start(cmd_handle);
    i2c_master_write_byte(cmd_handle, (OLED_I2C_ADDRESS << 1) | I2C_MASTER_WRITE, true);
    i2c_master_write_byte(cmd_handle, 0x00, true); // Control byte: Command
    i2c_master_write_byte(cmd_handle, cmd, true);
    i2c_master_stop(cmd_handle);
    i2c_master_cmd_begin(I2C_MASTER_NUM, cmd_handle, 1000 / portTICK_PERIOD_MS);
    i2c_cmd_link_delete(cmd_handle);
}

static void ssd1306_init() {
    uint8_t init_cmds[] = {
        0xAE, // Display off
        0x20, 0x00, // Set Memory Addressing Mode to Horizontal
        0x21, 0x00, 0x7F, // Set Column Address (0-127)
        0x22, 0x00, 0x07, // Set Page Address (0-7)
        0xC8, // COM Output Scan Direction (flip vert)
        0xA1, // Segment Re-map (flip horiz)
        0xA8, 0x3F, // Multiplex Ratio (for 128x64)
        0xD3, 0x00, // Display Offset
        0x40, // Start Line 0
        0x8D, 0x14, // Enable charge pump (critical for standard modules)
        0xAF  // Display ON
    };
    for(int i=0; i < sizeof(init_cmds); i++) {
        ssd1306_send_command(init_cmds[i]);
    }
}

// Global framebuffer
static uint8_t fb[8][128];

static void ssd1306_clear() {
    memset(fb, 0, sizeof(fb));
}

static void ssd1306_flush() {
    for (int page = 0; page < 8; page++) {
        ssd1306_send_command(0xB0 + page); // Set Page Start Address
        ssd1306_send_command(0x00);        // Set Lower Column Start Address
        ssd1306_send_command(0x10);        // Set Higher Column Start Address

        i2c_cmd_handle_t cmd_handle = i2c_cmd_link_create();
        i2c_master_start(cmd_handle);
        i2c_master_write_byte(cmd_handle, (OLED_I2C_ADDRESS << 1) | I2C_MASTER_WRITE, true);
        i2c_master_write_byte(cmd_handle, 0x40, true); // Control byte: Data
        i2c_master_write(cmd_handle, fb[page], 128, true);
        i2c_master_stop(cmd_handle);
        i2c_master_cmd_begin(I2C_MASTER_NUM, cmd_handle, 1000 / portTICK_PERIOD_MS);
        i2c_cmd_link_delete(cmd_handle);
    }
}

// Basic text drawing function. scale=2 draws characters at 10x16 pixels.
static void draw_text(int x, int y, const char *str, int scale) {
    while (*str) {
        char c = *str;
        if (c >= 32 && c <= 90) {
            int font_idx = c - 32;
            for (int col = 0; col < 5; col++) {
                uint8_t line = font5x8[font_idx][col];
                for (int row = 0; row < 8; row++) {
                    if (line & (1 << row)) {
                        // Draw scaled pixel
                        for(int sx=0; sx<scale; sx++) {
                            for(int sy=0; sy<scale; sy++) {
                                int px = x + (col * scale) + sx;
                                int py = y + (row * scale) + sy;
                                if(px >= 0 && px < 128 && py >= 0 && py < 64) {
                                    fb[py / 8][px] |= (1 << (py % 8));
                                }
                            }
                        }
                    }
                }
            }
        }
        x += (6 * scale); // 5 cols + 1 spacing space
        str++;
    }
}

/* --- Wi-Fi and SNTP --- */
static void wifi_event_handler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data) {
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
        esp_wifi_connect();
    } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
        esp_wifi_connect();
        ESP_LOGI(TAG, "Retrying Wi-Fi connection...");
    } else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        xEventGroupSetBits(s_wifi_event_group, WIFI_CONNECTED_BIT);
        ESP_LOGI(TAG, "Connected to Wi-Fi!");
    }
}

static void wifi_init_sta(void) {
    s_wifi_event_group = xEventGroupCreate();
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_netif_create_default_wifi_sta();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    esp_event_handler_instance_t instance_any_id;
    esp_event_handler_instance_t instance_got_ip;
    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &wifi_event_handler, NULL, &instance_any_id));
    ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &wifi_event_handler, NULL, &instance_got_ip));

    wifi_config_t wifi_config = {
        .sta = {
            .ssid = ESP_WIFI_SSID,
            .password = ESP_WIFI_PASS,
            .threshold.authmode = WIFI_AUTH_WPA2_PSK,
        },
    };
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());

    ESP_LOGI(TAG, "Wi-Fi initialization complete. Waiting for connection...");
    xEventGroupWaitBits(s_wifi_event_group, WIFI_CONNECTED_BIT, pdFALSE, pdFALSE, portMAX_DELAY);
}

static void initialize_sntp(void) {
    ESP_LOGI(TAG, "Initializing SNTP");
    esp_sntp_setoperatingmode(SNTP_OPMODE_POLL);
    esp_sntp_setservername(0, "pool.ntp.org");
    esp_sntp_init();

    // Wait for time to be set
    time_t now = 0;
    struct tm timeinfo = { 0 };
    int retry = 0;
    while (timeinfo.tm_year < (2020 - 1900) && ++retry < 15) {
        ESP_LOGI(TAG, "Waiting for system time to be set... (%d/15)", retry);
        vTaskDelay(2000 / portTICK_PERIOD_MS);
        time(&now);
        localtime_r(&now, &timeinfo);
    }
}

/* --- Main Application --- */
void app_main(void) {
    // Initialize NVS (required for Wi-Fi)
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
      ESP_ERROR_CHECK(nvs_flash_erase());
      ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    // Hardware Init
    i2c_master_init();
    ssd1306_init();
    ssd1306_clear();
    draw_text(10, 24, "CONNECTING...", 1);
    ssd1306_flush();

    // Network & Time Init
    wifi_init_sta();
    initialize_sntp();

    // Set timezone (Example: Eastern Time). You can adjust this POSIX string for your location.
    setenv("TZ", "EST5EDT,M3.2.0,M11.1.0", 1);
    tzset();

    char strftime_buf[32];
    char date_buf[32];

    while (1) {
        time_t now;
        struct tm timeinfo;
        time(&now);
        localtime_r(&now, &timeinfo);

        // Format Time (HH:MM:SS)
        strftime(strftime_buf, sizeof(strftime_buf), "%H:%M:%S", &timeinfo);
        
        // Format Date (MMM DD YYYY). Force upper-case as our small font only has uppercase
        strftime(date_buf, sizeof(date_buf), "%b %d %Y", &timeinfo);
        for(int i=0; date_buf[i]; i++){
            if(date_buf[i] >= 'a' && date_buf[i] <= 'z') date_buf[i] -= 32;
        }

        ssd1306_clear();
        
        // Draw Time (Large text, scale = 2)
        draw_text(16, 16, strftime_buf, 2);
        
        // Draw Date (Small text, scale = 1)
        draw_text(28, 45, date_buf, 1);
        
        ssd1306_flush();

        vTaskDelay(1000 / portTICK_PERIOD_MS); // Update every 1 second
    }
}