//Copyright [2025] [Robert Fudge]
//SPDX-FileCopyrightText: © 2025 Robert Fudge <rnfudge@mun.ca>
//SPDX-License-Identifier: {Apache-2.0}

#include "i2c_communicator.h"

int main() {
    // Initialize Pico W (from previous code)
    datetime_t init_time;
    // ... initialize time ...
    pico_w_init(init_time);

    // Initialize MPU6050
    mpu6050_t imu;
    mpu6050_data_t sensor_data;
    
    if (!mpu6050_init(&imu, i2c0, MPU6050_ADDR, 4, 5, 0, 0)) {
        printf("MPU6050 initialization failed!\n");
        return 1;
    }

    while(true) {
        if (mpu6050_read(&imu, &sensor_data)) {
            printf("Accel: X=%.2fg, Y=%.2fg, Z=%.2fg\n",
                   sensor_data.accel_x,
                   sensor_data.accel_y,
                   sensor_data.accel_z);

            current_sensor_data = sensor_data;
        }
        sleep_ms(100);
    }

    return 0;
}

if (mpu6050_read(&imu, &sensor_data)) {
    printf(...);
    current_sensor_data = sensor_data;
}