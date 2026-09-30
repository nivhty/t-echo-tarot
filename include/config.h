#ifndef CONFIG_H
#define CONFIG_H

// --- Timing ---
#define INACTIVITY_TIMEOUT_MS   60000   // 60 seconds before deep sleep
#define BACKLIGHT_DURATION_MS   5000    // 5 seconds backlight on touch
#define LONG_PRESS_MS           800     // Long press threshold
#define SPLASH_DURATION_MS      2000    // Splash screen display time
#define SHUFFLE_FRAMES          5       // Number of 'shuffle' animation frames
#define SHUFFLE_FRAME_MS        150     // Delay between shuffle frames

// --- Display ---
#define SCREEN_WIDTH            200
#define SCREEN_HEIGHT           200
#define BASE_ROTATION           3       // 0: Portrait(bottom), 1: Landscape(right), 2: Portrait(top), 3: Landscape(left)

// Portrait card bitmap (default, used if bitmaps were generated without --landscape)
#define CARD_IMG_WIDTH          128
#define CARD_IMG_HEIGHT         160
#define CARD_IMG_BYTES          (CARD_IMG_WIDTH / 8 * CARD_IMG_HEIGHT)  // 2560 bytes
// Landscape card bitmap (generated with --landscape flag)
#define CARD_IMG_L_WIDTH        160
#define CARD_IMG_L_HEIGHT       128
#define CARD_IMG_L_BYTES        (CARD_IMG_L_WIDTH / 8 * CARD_IMG_L_HEIGHT)  // 2560 bytes

// --- Battery ---
#define BATTERY_FULL_V          4.2f
#define BATTERY_EMPTY_V         3.3f

#endif // CONFIG_H
