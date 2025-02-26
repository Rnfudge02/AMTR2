//Copyright [2025] [Robert Fudge]
//SPDX-FileCopyrightText: © 2025 Robert Fudge <rnfudge@mun.ca>
//SPDX-License-Identifier: {Apache-2.0}

#include "pico_w_client.h"

int main() {
    stdio_init_all();
    client_init();
    
    if (!connect_to_server()) {
        printf("Failed to connect to server\n");
        return 1;
    }
    printf("Connected to server\n");

    while(true) {
        float ax, ay, az, temp;
        get_sensor_data(&ax, &ay, &az, &temp);
        
        printf("Accel: X=%.2fg Y=%.2fg Z=%.2fg Temp=%.1f°C\n",
               ax, ay, az, temp);
        
        // Blink LED on data reception
        cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, 1);
        sleep_ms(100);
        cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, 0);
        
        sleep_ms(900);
    }
}