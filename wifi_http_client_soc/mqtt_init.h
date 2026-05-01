#ifndef MQTT_INIT_H
#define MQTT_INIT_H

#include "cmsis_os2.h"
#include "sl_mqtt_client.h"

extern const osThreadAttr_t mqtt_thread_attributes;

// Function declarations for messaging / handling http requests
void send_mqtt_message(void *client, char * message);
void mqtt_application_start(void *argument);
void handle_mqtt_reconnect(void);
void mqtt_client_cleanup();
void mqtt_disconnect();

#endif
