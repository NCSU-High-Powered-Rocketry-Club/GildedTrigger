#include "trigger.h"

#define PERIOD_MS 100 //TODO: find actual period of stm32
#define WINDOW_MS 5000 //5 seconds
#define N         (WINDOW_MS / PERIOD_MS)   /* amount of samples to average */
#define AIRBORN_THRESHOLD 5 // > 5 g's of accel = airborn
#define LANDED_THRESHOLD 2 // = 1 g's of accel = grounded 


typedef enum {
    STANDBY,
    AIRBORN,
    LANDED
} State;

/**
 * @brief gets the average acceration on a WINDOW_MS ms window
 * 
 * @retval average accel maginitude
 */
static float get_average_accel(void){
    static float      buf[N];    
    static double sum = 0;
    static int count = 0;
    static int i = 0;
    float a = getAcceleration();

    if (count == N){ //if we have N samples
        sum -= buf[i]; // subtract oldest value
    }
    else{
        count++;
    }

    buf[i] = a;
    sum += a;
    i = (i + 1) % N; //increase i but have it wrap around so buf always stays the same size

    if (count < N){
        return -1;
    }
    else {
        return sum / N;
    }
}


/*
maybe some kinda struct for data storage
typedef struct{
    float latest_baro;
    float highest_baro;
    float accel;
    float vel; //integrate ?
    int tick;
    State state; //lwokey dont known how enums work
} Data;
*/





/* function update_data
    update bro

*/

/* function battery_on()   
    hal_write_pin smth //Turns battery on
*/

/*
while true
    if data->state == LANDED
        battery_on()
        program exit

    function update Data

    switch data->state
        case STANDBY:
            if get_baro() > highest_baro + 10
                data->state = MOTOR_BURN
        case MOTOR_BURN:
            if get_accel() < 0.5
                data->state = COAST
        case COAST:
            if get_accel() < 0.1
                data->state = FREEFALL
        case FREEFALL:
            if get_baro() > highest_baro - 10
                data->state = LANDED

*/

