#include "trigger.h"

#define PERIOD_MS 100 //TODO: find actual period of stm32
#define WINDOW_MS 1000 
#define N         (WINDOW_MS / PERIOD_MS)   /* amount of samples to average */
#define LANDED_THRESHOLD 1 // = 1 g's of accel = grounded 


void detect_landing(void){
    static int count = 0;
    float a = getAcceleration();
    if ((a - LANDED_THRESHOLD) < 0.1f) { // check if accel is close to 1 g
        count++;
    }
    else {
        count = 0; // reset count if accel is not close to 1 g
    }
    
    if (count >= N){
        // We have landed
        // Perform any necessary actions upon detecting a landing
        exit(0);
    }
}
