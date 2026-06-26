#ifndef BUTTONS_H
#define BUTTONS_H

#include <stdbool.h>

// Initializes the GPIO pins and starts the background button monitoring task
void buttons_init(void);

// Returns true if 12-hour mode is selected, false for 24-hour (military) mode
bool get_12h_mode(void);

// Returns true if analog mode is selected, false for digital mode
bool get_analog_mode(void);

#endif // BUTTONS_H