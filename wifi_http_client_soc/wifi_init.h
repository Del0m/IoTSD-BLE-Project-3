// wifi_init.h
#ifndef WIFI_INIT_H
#define WIFI_INIT_H

extern const osThreadAttr_t thread_attributes;
extern const sl_wifi_device_configuration_t http_client_configuration;

void wifi_application_start(void *argument);

#define FIREBASE_CERT_INDEX  0   // maps to SL_HTTPS_CLIENT_CERTIFICATE_INDEX_0
#define MQTT_CERT_INDEX      1   // maps to SL_HTTPS_CLIENT_CERTIFICATE_INDEX_1

#endif
