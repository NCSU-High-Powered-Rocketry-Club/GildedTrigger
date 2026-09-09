/* typedef enum {
    STANDBY,
    MOTOR_BURN,
    COAST,
    FREEFALL,
    LANDED
} State;
*/

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



/* function get_baro
    gets newest barometer data
*/

/* function get_accel
    gets newest accelerometer data
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