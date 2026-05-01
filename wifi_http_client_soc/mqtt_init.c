/***************************************************************************/ /**
 * @file
 * @brief Embedded MQTT Client Example Application
 *******************************************************************************
 * # License
 * <b>Copyright 2022 Silicon Laboratories Inc. www.silabs.com</b>
 *******************************************************************************
 *
 * SPDX-License-Identifier: Zlib
 *
 * The licensor of this software is Silicon Laboratories Inc.
 *
 * This software is provided 'as-is', without any express or implied
 * warranty. In no event will the authors be held liable for any damages
 * arising from the use of this software.
 *
 * Permission is granted to anyone to use this software for any purpose,
 * including commercial applications, and to alter it and redistribute it
 * freely, subject to the following restrictions:
 *
 * 1. The origin of this software must not be misrepresented; you must not
 *    claim that you wrote the original software. If you use this software
 *    in a product, an acknowledgment in the product documentation would be
 *    appreciated but is not required.
 * 2. Altered source versions must be plainly marked as such, and must not be
 *    misrepresented as being the original software.
 * 3. This notice may not be removed or altered from any source distribution.
 *
 ******************************************************************************/
#include "sl_net.h"
#include "sl_utility.h"
#include "cmsis_os2.h"
#include "sl_constants.h"
#include "sl_mqtt_client.h"
#include "string.h"
#include "wifi_init.h"
#include "mqtt_init.h"
#include "app.h"
// specific to the board, meant to resolve hostnames
#include "sl_net_dns.h"
#include "sl_si91x_types.h"
#include "sl_si91x_driver.h"
#include "sl_si91x_types.h"
#include "sl_si91x_driver.h"

#include "mqtt_cacert.pem.h"

/******************************************************
 *                    Constants
 ******************************************************/

#define MQTT_HOSTNAME "broker.hivemq.com"

#define MQTT_BROKER_PORT 8883

#define CLIENT_PORT 1

#define CLIENT_ID "WISECONNECT-SDK-MQTT-CLIENT-ID"

#define TOPIC_TO_BE_SUBSCRIBED "WebToMesh\0"
#define QOS_OF_SUBSCRIPTION    SL_MQTT_QOS_LEVEL_1

#define PUBLISH_TOPIC          "MeshToWeb\0"
#define PUBLISH_MESSAGE        "Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do"
#define QOS_OF_PUBLISH_MESSAGE 0

#define IS_DUPLICATE_MESSAGE 0
#define IS_MESSAGE_RETAINED  1
#define IS_CLEAN_SESSION     1

#define LAST_WILL_TOPIC       "MeshToWeb\0"
#define LAST_WILL_MESSAGE     "MQTT Client disconnecting to handle HTTP Post"
#define QOS_OF_LAST_WILL      1
#define IS_LAST_WILL_RETAINED 1

#define ENCRYPT_CONNECTION     1
#define KEEP_ALIVE_INTERVAL    10000
#define MQTT_CONNECT_TIMEOUT   10000
#define MQTT_KEEPALIVE_RETRIES 0


const osThreadAttr_t mqtt_thread_attributes = {
  .name       = "mqtt",
  .attr_bits  = 0,
  .cb_mem     = 0,
  .cb_size    = 0,
  .stack_mem  = 0,
  .stack_size = 8192,
  .priority   = osPriorityNormal,
  .tz_module  = 0,
  .reserved   = 0,
};
sl_mqtt_client_t client = { 0 };

uint8_t is_execution_completed = 0;

sl_mqtt_client_credentials_t *client_credentails = NULL;

sl_mqtt_broker_t mqtt_broker_configuration = {
  .port                    = MQTT_BROKER_PORT,
  .is_connection_encrypted = ENCRYPT_CONNECTION,
  .connect_timeout         = MQTT_CONNECT_TIMEOUT,
  .keep_alive_interval     = KEEP_ALIVE_INTERVAL,
  .keep_alive_retries      = MQTT_KEEPALIVE_RETRIES,
};
sl_mqtt_client_configuration_t mqtt_client_configuration = {
  .is_clean_session = IS_CLEAN_SESSION,
  .client_id        = (uint8_t *)CLIENT_ID,
  .client_id_length = strlen(CLIENT_ID),
#if ENCRYPT_CONNECTION
  .tls_flags = SL_MQTT_TLS_ENABLE | SL_MQTT_TLS_TLSV_1_2 | SL_MQTT_TLS_CERT_INDEX_1, // MQTT_CERT_INDEX
#endif
  .client_port = CLIENT_PORT
};

sl_mqtt_client_message_t message_to_be_published = {
  .qos_level            = QOS_OF_PUBLISH_MESSAGE,
  .is_retained          = IS_MESSAGE_RETAINED,
  .is_duplicate_message = IS_DUPLICATE_MESSAGE,
  .topic                = (uint8_t *)PUBLISH_TOPIC,
  .topic_length         = strlen(PUBLISH_TOPIC),
  .content              = (uint8_t *)PUBLISH_MESSAGE,
  .content_length       = strlen(PUBLISH_MESSAGE),
};

sl_mqtt_client_last_will_message_t last_will_message = {
  .is_retained         = IS_LAST_WILL_RETAINED,
  .will_qos_level      = QOS_OF_LAST_WILL,
  .will_topic          = (uint8_t *)LAST_WILL_TOPIC,
  .will_topic_length   = strlen(LAST_WILL_TOPIC),
  .will_message        = (uint8_t *)LAST_WILL_MESSAGE,
  .will_message_length = strlen(LAST_WILL_MESSAGE),
};

/******************************************************
 *               Function Declarations
 ******************************************************/
void mqtt_client_message_handler(void *client, sl_mqtt_client_message_t *message, void *context);
void mqtt_client_event_handler(void *client, sl_mqtt_client_event_t event, void *event_data, void *context);
void mqtt_client_error_event_handler(void *client, sl_mqtt_client_error_status_t *error);
void print_char_buffer(char *buffer, uint32_t buffer_length);
sl_status_t mqtt_example();

/******************************************************
 *               Function Definitions
 ******************************************************/

void handle_mqtt_reconnect(void) {
  is_execution_completed = 0;
  printf("\r\nAttempting MQTT reconnection\r\n");
  sl_status_t status;

  // reinit the client
  status = sl_mqtt_client_init(&client, mqtt_client_event_handler);

  if (status != SL_STATUS_OK) {
    printf("Failed to init mqtt client: 0x%lx\r\n", status);
    mqtt_client_cleanup();
    return;
  }
  printf("\r\nAttempting connection for initiating mqtt client\r\n");

  // actually reconnect
  status = sl_mqtt_client_connect(&client, &mqtt_broker_configuration, &last_will_message, &mqtt_client_configuration, 0);
  if (status != SL_STATUS_IN_PROGRESS) {
      printf("MQTT reconnect failed: 0x%lx\r\n", status);
      return;
  }
  printf("\r\nAttempting mqtt client connection\r\n");

  while(!is_execution_completed) {
      osThreadYield();
  }
  printf("\r\nSucessfully connected!");
}
/*
void mqtt_app_init(const void *unused)
{
  UNUSED_PARAMETER(unused);
  osThreadNew((osThreadFunc_t)mqtt_application_start, NULL, &thread_attributes);
}
*/
void mqtt_application_start(void *argument)
{
  UNUSED_PARAMETER(argument);
  mqtt_example();

  while (1) {
#if defined(SL_CATALOG_POWER_MANAGER_PRESENT)
    // Let the CPU go to sleep if the system allows it.
    sl_power_manager_sleep();
#else
    osDelay(osWaitForever);
#endif
  }
}
void mqtt_disconnect() {
  if(client.state == SL_MQTT_CLIENT_CONNECTED) {
        sl_status_t status = sl_mqtt_client_disconnect(&client, 5000);

        if(status == SL_STATUS_OK)
          printf("MQTT disconnected.");
        else if(status == SL_STATUS_IN_PROGRESS)
          printf("MQTT attempting disconnect.");
        else
          printf("MQTT disconnection failed");
    }
}

void led0_toggle(int enable) {

}
void mqtt_client_cleanup()
{
  SL_CLEANUP_MALLOC(client_credentails);
  is_execution_completed = 1;
}

void mqtt_client_message_handler(void *client, sl_mqtt_client_message_t *message, void *context)
{
  UNUSED_PARAMETER(context);
  UNUSED_PARAMETER(client);
  printf("Message Received on Topic: ");

  print_char_buffer((char *)message->topic, message->topic_length);
  print_char_buffer((char *)message->content, message->content_length);

  // send to firebase
  // tertiary to ensure that we don't have too big of a message
  uint32_t len = message->content_length < (MSG_LEN - 1) ? message->content_length : (MSG_LEN - 1);
  memcpy(http_message, message->content, len);

  // ensure null termination
  http_message[len] = '\0';

  http_post_request = 1;

  // handle led toggling

}

void print_char_buffer(char *buffer, uint32_t buffer_length)
{
  for (uint32_t index = 0; index < buffer_length; index++) {
    printf("%c", buffer[index]);
  }

  printf("\r\n");
}

void mqtt_client_error_event_handler(void *client, sl_mqtt_client_error_status_t *error)
{
  UNUSED_PARAMETER(client);
  printf("Terminating program, Error: %d\r\n", *error);
  mqtt_client_cleanup();
}

void send_mqtt_message(void * client, char * message) {

  // keep one message object, change out length and string
  message_to_be_published.content = (uint8_t * )message;
  message_to_be_published.content_length = strlen(message);

  sl_status_t status;

  status = sl_mqtt_client_publish(client, &message_to_be_published, 0, &message_to_be_published);
  if (status != SL_STATUS_IN_PROGRESS) {
    printf("Failed to publish message: 0x%lx\r\n", status);

    mqtt_client_cleanup();
    return;
  }
  printf("\r\nAttempting to publish message.\r\n");
}
void mqtt_client_event_handler(void *client, sl_mqtt_client_event_t event, void *event_data, void *context)
{
  switch (event) {
    case SL_MQTT_CLIENT_CONNECTED_EVENT: {
      sl_status_t status;

      status = sl_mqtt_client_subscribe(client,
                                        (uint8_t *)TOPIC_TO_BE_SUBSCRIBED,
                                        strlen(TOPIC_TO_BE_SUBSCRIBED),
                                        QOS_OF_SUBSCRIPTION,
                                        0,
                                        mqtt_client_message_handler,
                                        TOPIC_TO_BE_SUBSCRIBED);
      if (status != SL_STATUS_IN_PROGRESS) {
        printf("Failed to subscribe : 0x%lx\r\n", status);

        mqtt_client_cleanup();
        return;
      }
      printf("Attempting to subscribe to topic: %s", TOPIC_TO_BE_SUBSCRIBED);
      /*
      status = sl_mqtt_client_publish(client, &message_to_be_published, 0, &message_to_be_published);
      if (status != SL_STATUS_IN_PROGRESS) {
        printf("Failed to publish message: 0x%lx\r\n", status);

        mqtt_client_cleanup();
        return;
      }
      */
      break;
    }

    case SL_MQTT_CLIENT_MESSAGE_PUBLISHED_EVENT: {
      sl_mqtt_client_message_t *published_message = (sl_mqtt_client_message_t *)context;

      printf("Published message successfully on topic: ");
      print_char_buffer((char *)published_message->topic, published_message->topic_length);

      break;
    }

    case SL_MQTT_CLIENT_SUBSCRIBED_EVENT: {
      char *subscribed_topic = (char *)context;

      printf("Subscribed to Topic: %s\r\n", subscribed_topic);
      //send_mqtt_message(client, "SI91x has subscribed to the topic.");
      is_execution_completed = 1;

      break;
    }

    case SL_MQTT_CLIENT_UNSUBSCRIBED_EVENT: {
      char *unsubscribed_topic = (char *)context;

      printf("Unsubscribed from topic: %s\r\n", unsubscribed_topic);

      sl_mqtt_client_disconnect(client, 0);
      break;
    }

    case SL_MQTT_CLIENT_DISCONNECTED_EVENT: {
      printf("Disconnected from MQTT broker\r\n");

      //mqtt_client_cleanup();
      break;
    }

    case SL_MQTT_CLIENT_ERROR_EVENT: {
      mqtt_client_error_event_handler(client, (sl_mqtt_client_error_status_t *)event_data);
      break;
    }
    default:
      break;
  }
}

sl_status_t mqtt_example()
{
  sl_status_t status;

  // Load MQTT broker CA certificate at index 2
  status = sl_net_set_credential(SL_NET_TLS_SERVER_CREDENTIAL_ID(MQTT_CERT_INDEX),
                                 SL_NET_SIGNING_CERTIFICATE,
                                 mqtt_cacert,
                                 sizeof(mqtt_cacert) - 1);
  if (status != SL_STATUS_OK) {
    printf("Failed to load MQTT CA cert: 0x%lX\r\n", status);
    return status;
  }
  printf("MQTT CA cert loaded\r\n");

  status = sl_mqtt_client_init(&client, mqtt_client_event_handler);

  if (status != SL_STATUS_OK) {
    printf("Failed to init mqtt client: 0x%lx\r\n", status);
    mqtt_client_cleanup();
    return status;
  }
  // DNS resolution
  sl_ip_address_t resolved_ip  = { 0 };

  osDelay(2000);

  // resolving dns to handle dynamic ips
  printf("\r\nResolving DNS for %s...\r\n", MQTT_HOSTNAME);
  status = sl_net_dns_resolve_hostname(MQTT_HOSTNAME, 10000, SL_NET_DNS_TYPE_IPV4, &resolved_ip);
  printf("\r\nDNS resolve returned: 0x%lX\r\n", status);
  if (status != SL_STATUS_OK) {
    printf("\r\nDNS Resolution Failed: 0x%lX\r\n", status);
    mqtt_client_cleanup();
    return status;
  }
  // set broker ip
  mqtt_broker_configuration.ip.ip.v4.value = resolved_ip.ip.v4.value;
  mqtt_broker_configuration.ip.type = SL_IPV4;


  status =
    sl_mqtt_client_connect(&client, &mqtt_broker_configuration, &last_will_message, &mqtt_client_configuration, 0);
  if (status != SL_STATUS_IN_PROGRESS) {
    printf("Failed to connect to mqtt broker: 0x%lx\r\n", status);

    mqtt_client_cleanup();
    return status;
  }
  printf("Connect to mqtt broker Success \r\n");

  while (1) {
    osThreadYield();
  }

  printf("Example execution completed \r\n");

  return SL_STATUS_OK;
}
