#include "buttons.h"
#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

// Define the pins we connected the buttons to
#define BTN1_PIN 18 // 12H/24H mode
#define BTN2_PIN 19 // Analog/Digital mode

// State variables to keep track of current modes
static bool mode_12h = false;
static bool mode_analog = false;

// This task runs continuously in the background to monitor button states
static void button_task(void *arg) {
    // Keep track of previous states to detect falling edges (button presses)
    bool btn1_last = true;
    bool btn2_last = true;

    while (1) {
        // Read current states (false = button pressed because they pull to GND)
        bool btn1_state = gpio_get_level(BTN1_PIN);
        bool btn2_state = gpio_get_level(BTN2_PIN);

        // Detect if Button 1 was JUST pressed
        if (btn1_last == true && btn1_state == false) {
            if (mode_analog) {
                mode_analog = false; // Switch back to digital without changing 12/24H mode
            } else {
                mode_12h = !mode_12h; // Toggle 12H/24H mode
            }
        }
        
        // Detect if Button 2 was JUST pressed
        if (btn2_last == true && btn2_state == false) {
            mode_analog = !mode_analog; // Toggle Analog/Digital mode
        }

        // Save current state for next loop
        btn1_last = btn1_state;
        btn2_last = btn2_state;

        // 50ms delay acts as a software debounce so a single press isn't counted multiple times
        vTaskDelay(50 / portTICK_PERIOD_MS); 
    }
}

void buttons_init(void) {
    // Configure the GPIO pins as inputs with internal pull-up resistors
    gpio_config_t io_conf = {
        .intr_type = GPIO_INTR_DISABLE,
        .mode = GPIO_MODE_INPUT,
        .pin_bit_mask = (1ULL << BTN1_PIN) | (1ULL << BTN2_PIN),
        .pull_down_en = 0,
        .pull_up_en = 1 // Enable internal pull-ups
    };
    gpio_config(&io_conf);
    
    // Spawn the FreeRTOS task to monitor the buttons asynchronously
    xTaskCreate(button_task, "button_task", 2048, NULL, 10, NULL);
}

bool get_12h_mode(void) {
    return mode_12h;
}

bool get_analog_mode(void) {
    return mode_analog;
}