/***************************************************************************/ /**
 * @file
 * @brief HTTP Client Application
 ******************************************************************************/
#include "sl_board_configuration.h"
#include "cmsis_os2.h"
#include "sl_wifi.h"
#include "sl_net.h"
#include "sl_sntp.h"
#include "sl_http_client.h"
#include "sl_net_dns.h"
#include <string.h>
#include "sl_si91x_types.h"
#include "sl_si91x_driver.h"
#include "sl_si91x_types.h"
#include "wifi_init.h"
#include "mqtt_init.h"
#include "app.h"
#define SNTP_SERVER "time.google.com"
#define SNTP_TIMEOUT_MS 10000
#define SNTP_STOP_TIMEOUT 2000
#include "sl_si91x_driver.h"

#define SNTP_SERVER "time.google.com"
#define SNTP_TIMEOUT_MS 10000
#define SNTP_STOP_TIMEOUT 2000
#include "sl_si91x_driver.h"

#include "firebase_cacert.pem.h"

/******************************************************
 *                      Macros
 ******************************************************/
#define CLEAN_HTTP_CLIENT_IF_FAILED(status, client_handle, is_sync) \
  {                                                                 \
    if (status != SL_STATUS_OK) {                                   \
      sl_http_client_deinit(client_handle);                         \
      return ((is_sync == 0) ? status : callback_status);           \
    }                                                               \
  }

/******************************************************
 *                    Constants
 ******************************************************/
#define HTTP_CLIENT_USERNAME ""
#define HTTP_CLIENT_PASSWORD ""

#define IP_VERSION   SL_IPV4
#define HTTP_VERSION SL_HTTP_V_1_1

#define HTTPS_ENABLE 1
#define EXTENDED_HEADER_ENABLE 1

#if HTTPS_ENABLE
#define TLS_VERSION SL_TLS_V_1_2

#include "cacert.pem.h"

#define LOAD_CERTIFICATE 1
#endif

// Firestore target

#define HTTP_HOSTNAME  "iotproject3-89f3e-default-rtdb.firebaseio.com"
#define HTTP_PORT      443
#define HTTP_URL       "/readings.json"

/*
#define HTTP_HOSTNAME  "tank-sense.com"
#define HTTP_PORT      80
#define HTTP_URL       "/api/iot/"
*/
#define HTTP_DATA      "{\"Lux\": 11, \"RH\": 12, \"Temp\": 13}"

#if EXTENDED_HEADER_ENABLE
#define KEY1 "Content-Type"
#define VAL1 "application/json"

#endif


#define APP_BUFFER_LENGTH    2000
#define HTTP_SUCCESS_RESPONSE 1
#define HTTP_FAILURE_RESPONSE 2
#define HTTP_END_OF_DATA      1
#define HTTP_SYNC_RESPONSE    0
#define HTTP_ASYNC_RESPONSE   1

/******************************************************
 *               Variable Definitions
 ******************************************************/
const osThreadAttr_t thread_attributes = {
  .name       = "app",
  .attr_bits  = 0,
  .cb_mem     = 0,
  .cb_size    = 0,
  .stack_mem  = 0,
  .stack_size = 8192,
  .priority   = osPriorityNormal,
  .tz_module  = 0,
  .reserved   = 0,
};

uint8_t app_buffer[APP_BUFFER_LENGTH] = { 0 };
uint32_t app_buff_index = 0;
volatile uint8_t http_rsp_received = 0;
volatile uint8_t end_of_file       = 0;
sl_status_t callback_status        = SL_STATUS_OK;

sl_http_client_t client_handle                      = 0;
sl_http_client_configuration_t client_configuration = { 0 };

// DNS resolution
sl_ip_address_t resolved_ip  = { 0 };
uint8_t resolved_ip_str[16]  = { 0 };

// message
uint8_t message_buffer[MSG_LEN];
uint8_t * http_message = message_buffer;
volatile uint8_t http_post_request;

/******************************************************
 *               Function Declarations
 ******************************************************/
sl_status_t http_client_application(void);
sl_status_t http_response_status(volatile uint8_t *response);
sl_status_t http_post_response_callback_handler(const sl_http_client_t *client,
                                                sl_http_client_event_t event,
                                                void *data,
                                                void *request_context);
static void reset_http_handles(void);

/******************************************************
 *               Function Definitions
 ******************************************************/
void app_init(const void *unused)
{
  UNUSED_PARAMETER(unused);
  osThreadNew((osThreadFunc_t)wifi_application_start, NULL, &thread_attributes);
}
/*
static void sntp_event_handler(sl_sntp_client_response_t *response, uint8_t *user_data, uint16_t user_data_length)
{
UNUSED_PARAMETER(response);
UNUSED_PARAMETER(user_data);
UNUSED_PARAMETER(user_data_length);
}
*/


void application_start(void *argument)
{
  UNUSED_PARAMETER(argument);
  sl_status_t status;
  printf("\r\nApplication Start Task now attempting to run http_client_application()\r\n");

  // Load Firebase CA certificate at index 1
  status = sl_net_set_credential(SL_NET_TLS_SERVER_CREDENTIAL_ID(FIREBASE_CERT_INDEX),
                                 SL_NET_SIGNING_CERTIFICATE,
                                 firebase_cacert,
                                 sizeof(firebase_cacert) - 1);
  if (status != SL_STATUS_OK) {
    printf("Failed to load Firebase CA cert: 0x%lX\r\n", status);
    return;
  }
  printf("Firebase CA cert loaded\r\n");

  status = http_client_application();
  if (status != SL_STATUS_OK) {
    printf("\r\nUnexpected error while HTTP client operation: 0x%lX\r\n", status);
    return;
  }

  printf("\r\nApplication Demonstration Completed Successfully!\r\n");

  // reopen client
  // handle_mqtt_reconnect();


  while (1) {
    osDelay(100);
    if (http_post_request) {
      http_post_request = 0;
      printf("\r\nMessage Received for HTTP Client.\r\n");
      http_post(http_message);
    }

  }
}
sl_status_t http_post_json(sl_http_client_t *client_handle,
                           sl_http_client_configuration_t *client_configuration,
                           uint8_t *resolved_ip_str,
                           const char *json_data)
{
  sl_status_t status = SL_STATUS_OK;
  sl_http_client_request_t client_request = { 0 };

  client_request.ip_address      = resolved_ip_str;
  client_request.host_name       = (uint8_t *)HTTP_HOSTNAME;
  client_request.port            = HTTP_PORT;
  client_request.resource        = (uint8_t *)HTTP_URL;
  client_request.extended_header = NULL;

  int retries = 0;
  do {
#if EXTENDED_HEADER_ENABLE
    if(client_request.extended_header != NULL)
      sl_http_client_delete_all_headers(&client_request);
    client_request.extended_header = NULL;
    status = sl_http_client_add_header(&client_request, KEY1, VAL1);
    if (status != SL_STATUS_OK) {
      sl_http_client_deinit(client_handle);
      return status;
    }
#endif

    client_request.http_method_type = SL_HTTP_POST;
    client_request.body             = (uint8_t *)json_data;
    client_request.body_length      = strlen(json_data);

    status = sl_http_client_request_init(&client_request, http_post_response_callback_handler, "Firestore POST");
    if (status != SL_STATUS_OK) {
      sl_http_client_deinit(client_handle);
      return status;
    }

    http_rsp_received = 0;
    callback_status   = SL_STATUS_OK;

    status = sl_http_client_send_request(client_handle, &client_request);
    if (status == SL_STATUS_IN_PROGRESS) {
      status = http_response_status(&http_rsp_received);
    }

    if (status == 0x1BBD2 || callback_status == 0x1BBD2) {
      printf("TLS handshake failed (BBD2), retrying... (%d/%d)\r\n", retries + 1, 30);
      osDelay(1000);
      sl_http_client_deinit(client_handle);
      sl_http_client_init(client_configuration, client_handle);
      status = 0x1BBD2;
    }
    retries++;
  } while (status == 0x1BBD2 && retries < 30);

  reset_http_handles();

  if(client_request.extended_header != NULL)
    sl_http_client_delete_all_headers(&client_request);

  return status;
}

// abstracted function for mqtt
void http_post(uint8_t * message) {
  if(http_post_json(&client_handle, &client_configuration, resolved_ip_str, (char *)message) == SL_STATUS_OK) {
      printf("\r\nSent to firebase successfully!");
  } else {
      printf("\r\nFailed to send to firebase.");
  }

  return;
}

sl_status_t http_client_application(void)
{

  // turn off the mqtt client, this is needed as both clients can't persist at the same time
  // mqtt_disconnect();

  sl_status_t status                                  = SL_STATUS_OK;

  // Empty credentials (not used for Firestore API key auth)
  uint16_t username_length = strlen(HTTP_CLIENT_USERNAME);
  uint16_t password_length = strlen(HTTP_CLIENT_PASSWORD);
  uint32_t credential_size = sizeof(sl_http_client_credentials_t) + username_length + password_length;

  sl_http_client_credentials_t *client_credentials = (sl_http_client_credentials_t *)malloc(credential_size);
  SL_VERIFY_POINTER_OR_RETURN(client_credentials, SL_STATUS_ALLOCATION_FAILED);
  memset(client_credentials, 0, credential_size);
  client_credentials->username_length = username_length;
  client_credentials->password_length = password_length;
  memcpy(&client_credentials->data[0], HTTP_CLIENT_USERNAME, username_length);
  memcpy(&client_credentials->data[username_length], HTTP_CLIENT_PASSWORD, password_length);


  // Client configuration
  client_configuration.network_interface = SL_NET_WIFI_CLIENT_INTERFACE;
  client_configuration.ip_version        = IP_VERSION;
  client_configuration.http_version      = HTTP_VERSION;
#if HTTPS_ENABLE
  client_configuration.https_enable      = true;
  client_configuration.https_use_sni     = true;
  client_configuration.tls_version       = TLS_VERSION;
  client_configuration.certificate_index = FIREBASE_CERT_INDEX;
  //client_configuration.
#endif

  status = sl_net_set_credential(SL_NET_HTTP_CLIENT_CREDENTIAL_ID(FIREBASE_CERT_INDEX),
                                 SL_NET_HTTP_CLIENT_CREDENTIAL,
                                 client_credentials,
                                 credential_size);
  if (status != SL_STATUS_OK) {
    free(client_credentials);
    return status;
  }

  osDelay(2000);

  // resolving dns to handle dynamic ips
  printf("\r\nResolving DNS for %s...\r\n", HTTP_HOSTNAME);
  status = sl_net_dns_resolve_hostname(HTTP_HOSTNAME, 10000, SL_NET_DNS_TYPE_IPV4, &resolved_ip);
  printf("\r\nDNS resolve returned: 0x%lX\r\n", status);
  if (status != SL_STATUS_OK) {
    printf("\r\nDNS Resolution Failed: 0x%lX\r\n", status);
    free(client_credentials);
    return status;
  }

  sprintf((char *)resolved_ip_str, "%u.%u.%u.%u",
          resolved_ip.ip.v4.bytes[0], resolved_ip.ip.v4.bytes[1],
          resolved_ip.ip.v4.bytes[2], resolved_ip.ip.v4.bytes[3]);
  printf("DNS Resolved to: %s\r\n", resolved_ip_str);

  sl_http_client_init(&client_configuration, &client_handle);

  status = http_post_json(&client_handle, &client_configuration, resolved_ip_str, HTTP_DATA);

  if (status != SL_STATUS_OK) {
    sl_http_client_deinit(&client_handle);
    free(client_credentials);
    return status;
  }

  printf("\r\nHTTP POST request Success\r\n");

  //status = sl_http_client_deinit(&client_handle);
  //free(client_credentials);

  return status;
}

sl_status_t http_post_response_callback_handler(const sl_http_client_t *client,
                                                sl_http_client_event_t event,
                                                void *data,
                                                void *request_context)
{
  UNUSED_PARAMETER(client);
  UNUSED_PARAMETER(event);
  UNUSED_PARAMETER(request_context);

  sl_http_client_response_t *post_response = (sl_http_client_response_t *)data;
  callback_status                          = post_response->status;

  printf("\r\nPOST HTTP response code: %u\r\n", post_response->http_response_code);
  printf("\r\nPOST status: 0x%lX\r\n", post_response->status);
  if (post_response->data_length > 0) {
    printf("\r\nResponse body: %.*s\r\n", post_response->data_length, post_response->data_buffer);
  }

  if (post_response->status != SL_STATUS_OK
      || (post_response->http_response_code >= 400
          && post_response->http_response_code <= 599
          && post_response->http_response_code != 0)) {
    http_rsp_received = HTTP_FAILURE_RESPONSE;
    return post_response->status;
  }

  if (post_response->end_of_data) {
    http_rsp_received = HTTP_SUCCESS_RESPONSE;
  }

  return SL_STATUS_OK;
}

sl_status_t http_response_status(volatile uint8_t *response)
{
  uint32_t timeout = 30000; // 30 seconds
  uint32_t elapsed = 0;
  while (!(*response)) {
    osDelay(10);
    elapsed += 10;
    if (elapsed >= timeout) {
      printf("\r\nHTTP response timeout!\r\n");
      return SL_STATUS_TIMEOUT;
    }
  }
  if (*response != HTTP_SUCCESS_RESPONSE) {
    return SL_STATUS_FAIL;
  }
  *response = 0;
  return SL_STATUS_OK;
}

static void reset_http_handles(void)
{
  app_buff_index = 0;
  end_of_file    = 0;
}
