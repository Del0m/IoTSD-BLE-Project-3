/***************************************************************************/ /**
 * @file app.h
 * @brief Top level application functions
 *******************************************************************************
 * # License
 * <b>Copyright 2020 Silicon Laboratories Inc. www.silabs.com</b>
 *******************************************************************************
 *
 * The licensor of this software is Silicon Laboratories Inc. Your use of this
 * software is governed by the terms of Silicon Labs Master Software License
 * Agreement (MSLA) available at
 * www.silabs.com/about-us/legal/master-software-license-agreement. This
 * software is distributed to you in Source Code format and is governed by the
 * sections of the MSLA applicable to Source Code.
 *
 ******************************************************************************/

#ifndef APP_H
#define APP_H
#include "sl_http_client.h"

#define MSG_LEN 64
extern volatile uint8_t http_post_requested;
extern uint8_t * http_message;
extern volatile uint8_t http_post_request;
/***************************************************************************/ /**
 * Initialize application.
 ******************************************************************************/
void wifi_init(void);
void mqtt_app_init(void);
void http_post(uint8_t * message);
sl_status_t http_post_json(sl_http_client_t *client_handle,
                           sl_http_client_configuration_t *client_configuration,
                           uint8_t *resolved_ip_str,
                           const char *json_data);

void application_start(void *argument);
/***************************************************************************/ /**
 * App ticking function.
 ******************************************************************************/
void app_process_action(void);

#endif // APP_H
