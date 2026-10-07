# include "led.h"

void flashLED(int led){
    
        // GPIOA is the port for both LED's

        // Turn LED on
        HAL_GPIO_TogglePin(GPIOA, (led == 1) ? LED1_Pin : LED2_Pin); 

        HAL_Delay(1000); //Wait 1s

        // Turn LED off
        HAL_GPIO_TogglePin(GPIOA, (led == 1) ? LED1_Pin : LED2_Pin);
}