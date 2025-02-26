//Copyright [2025] [Robert Fudge]
//SPDX-FileCopyrightText: © 2025 Robert Fudge <rnfudge@mun.ca>
//SPDX-License-Identifier: {Apache-2.0}

#include "pico_w_interface.h"

#include <stdbool.h>
#include <string.h>

#include "pico/stdlib.h"
#include "pico/cyw43_arch.h"
#include "pico/util/datetime.h"

#include "lwip/pbuf.h"
#include "lwip/tcp.h"

//
static bool pico_w_initialized = false;
static volatile bool heartbeat_toggle = false;
static uint32_t heartbeat_interval_seconds = 0;

//
static mpu6050_data_t current_sensor_data;
static struct tcp_pcb *tcp_server_pcb = NULL;

//Linear Acceleration request format
static const char* mpu6050_response_format = 
    "HTTP/1.1 200 OK\r\n"
    "Content-Type: application/json\r\n\r\n"
    "{\"accel_x\":%.2f,\"accel_y\":%.2f,\"accel_z\":%.2f}";

static const char* welcome_response_format = 
        "HTTP/1.1 200 OK\r\n"
        "Content-Type: text/html\r\n\r\n"
        "<html><body><h1>Pico W Web Server</h1></body></html>";

//Forward declarations
static void __heartbeat_callback(void);
static err_t __http_accept_callback(void *arg, struct tcp_pcb *pcb, err_t err);
static err_t __http_recv_callback(void *arg, struct tcp_pcb *pcb, struct pbuf *p, err_t err);

// Function to turn off the heartbeat functionality
void pico_w_heartbeat_disable(void) {
    rtc_disable_alarm();
    heartbeat_interval_seconds = 0;
}

// Function to turn on the heartbeat functionality
void pico_w_heartbeat_init(uint32_t interval_seconds) {
    heartbeat_interval_seconds = interval_seconds;

    datetime_t current_time;
    if (rtc_get_datetime(&current_time)) {
        datetime_t alarm_time = current_time;
        datetime_add_seconds(&alarm_time, interval_seconds);
        rtc_set_alarm(&alarm_time, __heartbeat_callback);
    }
}

// Callback to toggle the LED GPIO pin
static void __heartbeat_callback(void) {
    cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, heartbeat_toggle);
    heartbeat_toggle = !heartbeat_toggle;

    if (heartbeat_interval_seconds > 0) {
        datetime_t current_time;
        if (rtc_get_datetime(&current_time)) {
            datetime_t alarm_time = current_time;
            datetime_add_seconds(&alarm_time, heartbeat_interval_seconds);
            rtc_set_alarm(&alarm_time, __heartbeat_callback);
        }
    }
}

// Function to Initialize Pico W interfaces
void pico_w_init(datetime_t system_init_time) {
    if (!pico_w_initialized) {
        stdio_init_all();

        i2c_init(i2c0, 400 * 1000);

        if (cyw43_arch_init() != 0) {
            printf("Wi-Fi init failed\n");
            return;
        }

        rtc_init();
        rtc_set_datetime(&system_init_time);

        pico_w_initialized = true;
    }
}

//Function to initialize and setup basic HTTP webpage
void pico_w_web_init(bool enable_tcp_server) {
    if (cyw43_arch_enable_ap_mode(SERVER_SSID, SERVER_PASSWORD, CYW43_AUTH_WPA2_AES_PSK) != 0) {
        printf("Failed to enable AP mode\n");
        return;
    }

    ip4_addr_t ip, netmask, gw;
    IP4_ADDR(&ip, SERVER_IP[0], SERVER_IP[1], SERVER_IP[2], SERVER_IP[3]);
    IP4_ADDR(&netmask, 255, 255, 255, 0);
    IP4_ADDR(&gw, SERVER_IP[0], SERVER_IP[1], SERVER_IP[2], SERVER_IP[3]);
    netif_set_addr(&cyw43_state.netif, &ip, &netmask, &gw);

    struct tcp_pcb *pcb = tcp_new_ip_type(IPADDR_TYPE_ANY);
    if (!pcb) {
        printf("Failed to create PCB\n");
        return;
    }

    if (tcp_bind(pcb, IP_ANY_TYPE, 80) != ERR_OK) {
        printf("Failed to bind to port 80\n");
        tcp_close(pcb);
        return;
    }

    pcb = tcp_listen(pcb);
    if (!pcb) {
        printf("Failed to listen\n");
        return;
    }

    tcp_accept(pcb, __http_accept_callback);

    //Setup TCP server if enabled
    if (enable_tcp_server) {
        struct tcp_pcb *pcb = tcp_new_ip_type(IPADDR_TYPE_ANY);
        if (!pcb) {
            printf("Failed to create TCP PCB\n");
            return;
        }

        if (tcp_bind(pcb, IP_ANY_TYPE, TCP_PORT) != ERR_OK) {
            printf("Failed to bind TCP port\n");
            tcp_close(pcb);
            return;
        }

        tcp_server_pcb = tcp_listen(pcb);
        if (!tcp_server_pcb) {
            printf("Failed to listen\n");
            return;
        }

        tcp_accept(tcp_server_pcb, __tcp_server_accept_callback);
        printf("TCP server started on port %d\n", TCP_PORT);
    }
}

//HTTP accept callback
static err_t __http_accept_callback(void *arg, struct tcp_pcb *pcb, err_t err) {
    if (err != ERR_OK || pcb == NULL) {
        return ERR_VAL;
    }
    tcp_recv(pcb, __http_recv_callback);
    return ERR_OK;
}

//HTTP receive callback
static err_t __http_recv_callback(void *arg, struct tcp_pcb *pcb, struct pbuf *p, err_t err) {
    if (p != NULL) {
        tcp_recved(pcb, p->tot_len);
        pbuf_free(p);
    } else if (err == ERR_OK) {
        tcp_close(pcb);
        return ERR_OK;
    }

    char response[128];
    snprintf(response, sizeof(response), "%s", welcome_response_format);
    tcp_write(pcb, response, strlen(response), TCP_WRITE_FLAG_COPY);

    if (tcp_sndbuf(pcb) >= strlen(response)) {
        tcp_write(pcb, response, strlen(response), TCP_WRITE_FLAG_COPY);
    }
    tcp_close(pcb);
    return ERR_OK;
}

//Add new TCP server callbacks
static err_t __tcp_server_accept_callback(void *arg, struct tcp_pcb *client_pcb, err_t err) {
    if (err != ERR_OK || client_pcb == NULL) return ERR_VAL;
    
    printf("Client connected\n");
    tcp_recv(client_pcb, __tcp_server_recv_callback);
    return ERR_OK;
}

static err_t __tcp_server_recv_callback(void *arg, struct tcp_pcb *pcb, struct pbuf *p, err_t err) {
    if (p) {
        //When receiving any data, send sensor data back
        char response[128];
        snprintf(response, sizeof(response),
            "%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f",
            current_sensor_data.accel_x,
            current_sensor_data.accel_y,
            current_sensor_data.accel_z,
            current_sensor_data.gyro_x,
            current_sensor_data.gyro_y,
            current_sensor_data.gyro_z,
            current_sensor_data.temp);

        tcp_write(pcb, response, strlen(response), TCP_WRITE_FLAG_COPY);
        tcp_close(pcb);
        pbuf_free(p);
    }
    return ERR_OK;
}