/***************************************************************************/ /**
 * @file
 * @brief HTTP Client Application
 ******************************************************************************/
#include "sl_board_configuration.h"
#include "cmsis_os2.h"
#include "sl_wifi.h"
#include "sl_net.h"
#include "app.h"
#include "sl_sntp.h"
#include "sl_http_client.h"
#include "sl_net_dns.h"
#include <string.h>
#include "sl_si91x_types.h"
#include "sl_si91x_driver.h"
#include "index.html.h"
#include "sl_si91x_types.h"
#include "wifi_init.h"
#define SNTP_SERVER "time.google.com"
#define SNTP_TIMEOUT_MS 10000
#define SNTP_STOP_TIMEOUT 2000
#include "sl_si91x_driver.h"
#include "mqtt_init.h"


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
#define TLS_VERSION SL_TLS_V_1_3

#define LOAD_CERTIFICATE 1
#endif

// Firestore target
#define HTTP_HOSTNAME  "iotproject3-89f3e-default-rtdb.firebaseio.com"
#define HTTP_PORT      443
#define HTTP_URL       "/readings.json"
#define HTTP_DATA      "{\"Lux\": 11, \"RH\": 12, \"Temp\": 13}"

#if EXTENDED_HEADER_ENABLE
#define KEY1 "Content-Type"
#define VAL1 "application/json"
#define KEY2 "Content-Length"
#define VAL2 "33"
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

extern const osThreadAttr_t mqtt_thread_attributes;
const osThreadAttr_t app_thread_attributes = {
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

const sl_wifi_device_configuration_t http_client_configuration = {
  .boot_option = LOAD_NWP_FW,
  .mac_address = NULL,
  .band        = SL_SI91X_WIFI_BAND_2_4GHZ,
  .boot_config = {
    .oper_mode              = SL_SI91X_CLIENT_MODE,
    .coex_mode              = SL_SI91X_WLAN_ONLY_MODE,
    .feature_bit_map        = (SL_SI91X_FEAT_SECURITY_PSK | SL_SI91X_FEAT_AGGREGATION),
    .tcp_ip_feature_bit_map = (SL_SI91X_TCP_IP_FEAT_DHCPV4_CLIENT
                             | SL_SI91X_TCP_IP_FEAT_HTTP_CLIENT
                             | SL_SI91X_TCP_IP_FEAT_DNS_CLIENT
                             | SL_SI91X_TCP_IP_FEAT_SSL
                             | SL_SI91X_TCP_IP_FEAT_SNTP_CLIENT
                             | SL_SI91X_TCP_IP_FEAT_EXTENSION_VALID),
    .custom_feature_bit_map     = SL_SI91X_CUSTOM_FEAT_EXTENTION_VALID,
    .ext_custom_feature_bit_map = (SL_SI91X_EXT_FEAT_XTAL_CLK | MEMORY_CONFIG
#if defined(SLI_SI917) || defined(SLI_SI915)
                                   | SL_SI91X_EXT_FEAT_FRONT_END_SWITCH_PINS_ULP_GPIO_4_5_0
#endif
                                   ),
    .ext_tcp_ip_feature_bit_map = (SL_SI91X_EXT_TCP_IP_FEAT_SSL_THREE_SOCKETS
                                 //| SL_SI91X_EXT_TCP_IP_FEAT_SSL_MEMORY_CLOUD
                                 | SL_SI91X_EXT_TCP_IP_SSL_16K_RECORD
                                 | SL_SI91X_EXT_EMB_MQTT_ENABLE
                                 | SL_SI91X_EXT_TCP_IP_TOTAL_SELECTS(10)
                                 ),
    .bt_feature_bit_map      = 0,
    .ble_feature_bit_map     = 0,
    .ble_ext_feature_bit_map = 0,
    .config_feature_bit_map  = 0
  }
};

/******************************************************
 *               Function Declarations
 ******************************************************/

//void wifi_init(const void *unused);
static sl_status_t load_certificates(void);

/******************************************************
 *               Function Definitions
 ******************************************************/
/*
void wifi_init(const void *unused)
{
  UNUSED_PARAMETER(unused);
  osThreadNew((osThreadFunc_t)wifi_application_start, NULL, &thread_attributes);
}
*/
static sl_status_t sync_time_with_sntp(void)
{
  sl_status_t status;
  sl_sntp_client_config_t sntp_cfg = { 0 };
  uint8_t date_time[64] = { 0 };

  sl_ip_address_t ip_addr;
  status = sl_net_dns_resolve_hostname(SNTP_SERVER, 15000, SL_NET_DNS_TYPE_IPV4, &ip_addr);

  sntp_cfg.server_host_name = ip_addr.ip.v4.bytes;
  sntp_cfg.sntp_method = SL_SNTP_UNICAST_MODE;
  sntp_cfg.sntp_timeout = 15000;
  sntp_cfg.flags = 0; // IPv4


  status = sl_sntp_client_start(&sntp_cfg, SNTP_TIMEOUT_MS);
  if (status != SL_STATUS_OK) {
    printf("\r\nSNTP start failed: 0x%lX\r\n", status);
    return status;
  }

  status = sl_sntp_client_get_time_date(date_time, sizeof(date_time), SNTP_TIMEOUT_MS);
  if (status != SL_STATUS_OK) {
    printf("\r\nSNTP get time failed: 0x%lX\r\n", status);
    (void)sl_sntp_client_stop(SNTP_STOP_TIMEOUT);
    return status;
  }

  printf("\r\nSNTP time: %s\r\n", date_time);

  status = sl_sntp_client_stop(SNTP_STOP_TIMEOUT);
  if (status != SL_STATUS_OK) {
    printf("\r\nSNTP stop failed: 0x%lX\r\n", status);
    return status;
  }

  load_certificates();
  return SL_STATUS_OK;
}

static sl_status_t load_certificates(void)
{


  return SL_STATUS_OK;
}

extern void wifi_application_start(void *argument)
{
  UNUSED_PARAMETER(argument);
  sl_status_t status;

  // wifi parameters
  status = sl_net_init(SL_NET_WIFI_CLIENT_INTERFACE, &http_client_configuration, NULL, NULL);
  if (status != SL_STATUS_OK) {
    printf("Failed to start Wi-Fi client interface: 0x%lx\r\n", status);
    return;
  }
  printf("\r\nWi-Fi Init Success\r\n");

  // set up wifi connection
  status = sl_net_up(SL_NET_WIFI_CLIENT_INTERFACE, SL_NET_DEFAULT_WIFI_CLIENT_PROFILE_ID);
  if (status != SL_STATUS_OK) {
    printf("Failed to bring Wi-Fi client interface up: 0x%lx\r\n", status);
    return;
  }
  printf("\r\nWi-Fi Client Connected\r\n");

  // load sntp clock
  status = sync_time_with_sntp();
  if (status != SL_STATUS_OK) {
    printf("\r\nSNTP sync failed: 0x%lX\r\n", status);
    return;
  }

  osDelay(2000);

  osThreadId_t wifi_tid = osThreadNew((osThreadFunc_t)application_start, NULL, &app_thread_attributes);
  if (wifi_tid == NULL) {
      printf("Failed to create application_start thread!\n");
  } else {
      printf("application_start thread created successfully\n");
  }

  osDelay(10000);

  osThreadId_t mqtt_tid = osThreadNew((osThreadFunc_t)mqtt_application_start, NULL, &mqtt_thread_attributes);
  if (mqtt_tid == NULL) {
      printf("Failed to create mqtt_application_start thread!\n");
  } else {
      printf("application_start thread created successfully\n");
  }





  // keep the task alive
  while(1) {
     osThreadYield();
  }
}

