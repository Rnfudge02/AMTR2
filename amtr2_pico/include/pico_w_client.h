//Copyright [2025] [Robert Fudge]
//SPDX-FileCopyrightText: © 2025 Robert Fudge <rnfudge@mun.ca>
//SPDX-License-Identifier: {Apache-2.0}

#ifndef __PICO_W_CLIENT_H__
#define __PICO_W_CLIENT_H__

#include "pico/cyw43_arch.h"
#include "lwip/pbuf.h"
#include "lwip/tcp.h"

#define SERVER_IP "192.168.4.1"  // Default AP IP

void client_init();
bool connect_to_server();
void get_sensor_data(float*, float*, float*, float*);

#endif