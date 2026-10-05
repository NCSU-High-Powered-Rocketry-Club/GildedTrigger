#include "trigger.h"

/*
 * Not an STM32 clock period. The main loop calls detect_landing() with no
 * delay, so the rate is set by the blocking 6-byte I2C read inside
 * getAcceleration(): ~820us on the 100kHz bus, which rounds to 1ms.
 * Speeding up I2C or adding a delay to the main loop changes WINDOW_MS.
 */
#define PERIOD_MS 1
#define WINDOW_MS 1000 
#define N         (WINDOW_MS / PERIOD_MS)   /* amount of samples to average */
#define LANDED_THRESHOLD 1 // = 1 g's of accel = grounded 


void detect_landing(void){
    static int count = 0;
    float a = getAcceleration();
    if (fabsf(a - LANDED_THRESHOLD) < 0.1f) { // check if accel is close to 1 g
        count++;
    }
    else {
        count = 0; // reset count if accel is not close to 1 g
    }
    
    if (count >= N){
        // We have landed
        
        // Perform any necessary actions upon detecting a landing
        
        // Drive SWITCH1 and SWITCH2 High
        HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_1); 
        HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_2);

        HAL_Delay(5); //Wait 5ms

        // Drive SWITCH1 and SWITCH2 Low
        HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_1);
        HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_2);
        exit(0);
    }
}
