//Copyright [2025] [Robert Fudge]
//SPDX-FileCopyrightText: © 2025 Robert Fudge <rnfudge@mun.ca>
//SPDX-License-Identifier: {Apache-2.0}

#ifndef __PICO_W_INTERFACE_H__
#define __PICO_W_INTERFACE_H__

#include <stdint.h>
#include <stdbool.h>

//Wi-Fi configuration
#define SERVER_SSID "MPU6050-Server"
#define SERVER_PASSWORD "sensorpass"
#define TCP_PORT 4242

const uint8_t SERVER_IP[4] = {192, 168, 4, 1};

extern mpu6050_data_t current_sensor_data;

void pico_w_web_init(bool);
void pico_w_heartbeat_init(uint32_t);
void __heartbeat_callback(void);

#endif