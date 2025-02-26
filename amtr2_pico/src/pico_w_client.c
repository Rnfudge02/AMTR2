//Copyright [2025] [Robert Fudge]
//SPDX-FileCopyrightText: © 2025 Robert Fudge <rnfudge@mun.ca>
//SPDX-License-Identifier: {Apache-2.0}

#include "pico_w_client.h"
#include "pico_w_interface.h"

static struct tcp_pcb *tcp_pcb = NULL;
static volatile bool data_ready = false;
static float sensor_values[7];

void client_init() {
    if (cyw43_arch_init()) {
        printf("Wi-Fi init failed\n");
        return;
    }
    
    cyw43_arch_enable_sta_mode();
    
    // Connect to server's AP
    while(cyw43_arch_wifi_connect_timeout_ms(SERVER_SSID, SERVER_PASSWORD, 
                                           CYW43_AUTH_WPA2_AES_PSK, 30000)) {
        printf("Failed to connect to AP\n");
        sleep_ms(1000);
    }
    printf("Connected to AP\n");
}

static err_t tcp_client_recv(void *arg, struct tcp_pcb *pcb, struct pbuf *p, err_t err) {
    if (p) {
        // Parse received data
        char *data = (char *)p->payload;
        sscanf(data, "%f,%f,%f,%f,%f,%f,%f", &sensor_values[0], &sensor_values[1], &sensor_values[2],
            &sensor_values[3], &sensor_values[4], &sensor_values[5], &sensor_values[6]);
               
        data_ready = true;
        pbuf_free(p);
    }
    return ERR_OK;
}

bool connect_to_server() {
    ip_addr_t server_addr;
    ipaddr_aton(SERVER_IP, &server_addr);
    
    tcp_pcb = tcp_new_ip_type(IPADDR_TYPE_V4);
    if (!tcp_pcb) return false;

    if (tcp_connect(tcp_pcb, &server_addr, TCP_PORT, __tcp_client_connected) != ERR_OK) {
        tcp_close(tcp_pcb);
        return false;
    }
    
    return true;
}

void get_sensor_data(float* accel_x, float* accel_y, float* accel_z, float* temp) {
    data_ready = false;
    tcp_write(tcp_pcb, "DATA", 4, TCP_WRITE_FLAG_COPY);
    
    // Wait for data (with timeout)
    absolute_time_t timeout = make_timeout_time_ms(1000);
    while (!data_ready && absolute_time_diff_us(get_absolute_time(), timeout) > 0) {
        cyw43_arch_poll();
        sleep_ms(10);
    }
    
    if (data_ready) {
        *accel_x = sensor_values[0];
        *accel_y = sensor_values[1];
        *accel_z = sensor_values[2];
        *temp = sensor_values[6];
    }
}

static err_t __tcp_client_connected(void *arg, struct tcp_pcb *pcb, err_t err) {
    if (err != ERR_OK) return err;
    tcp_recv(pcb, tcp_client_recv);
    return ERR_OK;
}