//Copyright [2025] [Robert Fudge]
//SPDX-FileCopyrightText: © 2025 Robert Fudge <rnfudge@mun.ca>
//SPDX-License-Identifier: {Apache-2.0}

#include "i2c_communicator.h"

// MPU6050 register definitions
#define MPU6050_ADDR         0x68
#define MPU6050_WHO_AM_I     0x75
#define MPU6050_PWR_MGMT_1   0x6B
#define MPU6050_ACCEL_CONFIG 0x1C
#define MPU6050_GYRO_CONFIG  0x1B
#define MPU6050_ACCEL_XOUT_H 0x3B

// Sensitivity scales from datasheet
static const float ACCEL_SCALE_FACTOR[] = {
    16384.0, // ±2g
    8192.0,  // ±4g
    4096.0,  // ±8g
    2048.0   // ±16g
};

static const float GYRO_SCALE_FACTOR[] = {
    131.0,   // ±250°/s
    65.5,    // ±500°/s
    32.8,    // ±1000°/s
    16.4     // ±2000°/s
};

bool mpu6050_init(mpu6050_t *sensor, i2c_inst_t *i2c_port, uint8_t address, 
                 uint sda_pin, uint scl_pin, uint accel_range, uint gyro_range) {
    // Initialize I2C peripheral
    i2c_init(i2c_port, 400 * 1000); // 400 kHz
    gpio_set_function(sda_pin, GPIO_FUNC_I2C);
    gpio_set_function(scl_pin, GPIO_FUNC_I2C);
    gpio_pull_up(sda_pin);
    gpio_pull_up(scl_pin);

    // Verify device presence
    uint8_t who_am_i;
    i2c_read_blocking(i2c_port, address, &who_am_i, 1, false);
    if (who_am_i != 0x68) {
        return false;
    }

    // Configure device
    uint8_t config_data[3];
    
    // Wake up device
    config_data[0] = MPU6050_PWR_MGMT_1;
    config_data[1] = 0x00;
    i2c_write_blocking(i2c_port, address, config_data, 2, false);

    // Set accelerometer range
    config_data[0] = MPU6050_ACCEL_CONFIG;
    config_data[1] = (accel_range & 0x03) << 3;
    i2c_write_blocking(i2c_port, address, config_data, 2, false);

    // Set gyroscope range
    config_data[0] = MPU6050_GYRO_CONFIG;
    config_data[1] = (gyro_range & 0x03) << 3;
    i2c_write_blocking(i2c_port, address, config_data, 2, false);

    // Store configuration
    sensor->i2c_port = i2c_port;
    sensor->address = address;
    sensor->sda_pin = sda_pin;
    sensor->scl_pin = scl_pin;
    sensor->accel_scale = ACCEL_SCALE_FACTOR[accel_range];
    sensor->gyro_scale = GYRO_SCALE_FACTOR[gyro_range];

    return true;
}

bool mpu6050_read(mpu6050_t *sensor, mpu6050_data_t *data) {
    uint8_t buffer[14];
    
    // Read all sensor data registers
    uint8_t reg = MPU6050_ACCEL_XOUT_H;
    if (i2c_write_blocking(sensor->i2c_port, sensor->address, &reg, 1, true) != 1) {
        return false;
    }
    
    if (i2c_read_blocking(sensor->i2c_port, sensor->address, buffer, 14, false) != 14) {
        return false;
    }

    // Process accelerometer data
    data->accel_x = (int16_t)(buffer[0] << 8 | buffer[1]) / sensor->accel_scale;
    data->accel_y = (int16_t)(buffer[2] << 8 | buffer[3]) / sensor->accel_scale;
    data->accel_z = (int16_t)(buffer[4] << 8 | buffer[5]) / sensor->accel_scale;

    // Process temperature
    data->temp = (int16_t)(buffer[6] << 8 | buffer[7]) / 340.0 + 36.53;

    // Process gyroscope data
    data->gyro_x = (int16_t)(buffer[8] << 8 | buffer[9]) / sensor->gyro_scale;
    data->gyro_y = (int16_t)(buffer[10] << 8 | buffer[11]) / sensor->gyro_scale;
    data->gyro_z = (int16_t)(buffer[12] << 8 | buffer[13]) / sensor->gyro_scale;

    return true;
}