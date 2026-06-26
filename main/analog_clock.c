#include "analog_clock.h"
#include <math.h>
#include <stdlib.h>

// External dependencies from main_clock_toggle.c
extern void draw_pixel(int x, int y, int color);
extern void draw_text(int x, int y, const char *str, int scale);

// Standard Bresenham's line algorithm for drawing the hands
static void draw_line(int x0, int y0, int x1, int y1) {
    int dx = abs(x1 - x0), sx = x0 < x1 ? 1 : -1;
    int dy = -abs(y1 - y0), sy = y0 < y1 ? 1 : -1; 
    int err = dx + dy, e2; 

    for (;;) {  
        draw_pixel(x0, y0, 1);
        if (x0 == x1 && y0 == y1) break;
        e2 = 2 * err;
        if (e2 >= dy) { err += dy; x0 += sx; }
        if (e2 <= dx) { err += dx; y0 += sy; }
    }
}

void draw_analog_clock(int h, int m, int s) {
    // Screen is 128x64, center point is (64, 32)
    int cx = 64;
    int cy = 32;
    
    // Draw the rectangular outer face
    draw_line(0, 0, 127, 0);
    draw_line(127, 0, 127, 63);
    draw_line(127, 63, 0, 63);
    draw_line(0, 63, 0, 0);
    
    // Draw the cardinal numbers larger (scale = 2)
    draw_text(52, 2, "12", 2);
    draw_text(113, 24, "3", 2);
    draw_text(58, 46, "6", 2);
    draw_text(4, 24, "9", 2);
    
    // Draw the remaining numbers smaller (scale = 1)
    draw_text(88, 6, "1", 1);
    draw_text(108, 16, "2", 1);
    draw_text(108, 42, "4", 1);
    draw_text(88, 50, "5", 1);
    draw_text(34, 50, "7", 1);
    draw_text(14, 42, "8", 1);
    draw_text(10, 16, "10", 1);
    draw_text(28, 6, "11", 1);
    
    float pi = 3.14159265f;
    
    // Calculate polar angles for the hands based on time
    float sec_angle = s * (pi / 30.0f);
    float min_angle = m * (pi / 30.0f) + (s * pi / 1800.0f);
    float hr_angle = (h % 12) * (pi / 6.0f) + (m * pi / 360.0f);
    
    int sec_len = 26;
    int min_len = 22;
    int hr_len = 14;
    
    // Apply elliptical scaling to make the hands stretch out 
    // horizontally, fitting the 128x64 aspect ratio beautifully
    float x_scale = 1.9f; 
    float y_scale = 0.9f;
    
    // Draw the 3 hands (convert polar to cartesian)
    draw_line(cx, cy, cx + (hr_len * x_scale) * sin(hr_angle), cy - (hr_len * y_scale) * cos(hr_angle));     // Hour
    draw_line(cx, cy, cx + (min_len * x_scale) * sin(min_angle), cy - (min_len * y_scale) * cos(min_angle)); // Minute
    draw_line(cx, cy, cx + (sec_len * x_scale) * sin(sec_angle), cy - (sec_len * y_scale) * cos(sec_angle)); // Second
}