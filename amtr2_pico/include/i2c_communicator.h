//Copyright [2025] [Robert Fudge]
//SPDX-FileCopyrightText: © 2025 Robert Fudge <rnfudge@mun.ca>
//SPDX-License-Identifier: {Apache-2.0}

#ifndef __I2C_COMMUNICATOR_H__
#define __I2C_COMMUNICATOR_H__

#include "pico/stdlib.h"
#include "hardware/i2c.h"

typedef struct {
    float accel_x;  // Acceleration in G's
    float accel_y;
    float accel_z;
    float temp;     // Temperature in Celsius
    float gyro_x;   // Gyroscope in degrees/sec
    float gyro_y;
    float gyro_z;
} mpu6050_data_t;

// MPU6050 configuration structure
typedef struct {
    i2c_inst_t *i2c_port;
    uint8_t address;
    uint sda_pin;
    uint scl_pin;
    float accel_scale;
    float gyro_scale;
} mpu6050_t;

// Function prototypes
bool mpu6050_init(mpu6050_t*, i2c_inst_t*, uint8_t, uint, uint, uint, uint);
bool mpu6050_read(mpu6050_t*, mpu6050_data_t*);

#endif