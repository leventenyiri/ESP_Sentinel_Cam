#include "esp_netif.h"
#include "protocol_examples_common.h"
#include <esp_log.h>
#include <esp_system.h>
#include <nvs_flash.h>
#include <string.h>
#include <sys/param.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#ifndef portTICK_RATE_MS
#define portTICK_RATE_MS portTICK_PERIOD_MS
#endif

#include "driver/i2c_master.h"
#include "driver/temperature_sensor.h"
#include "esp_event.h"
#include "esp_wifi.h"
#include "mqtt_client.h"
#include "nvs_flash.h"

#include "camera.h"
#include "secret.h"

static const char *TAG = "main_app";
static EventGroupHandle_t wifi_event_group;
const int WIFI_CONNECTED_BIT = BIT0;

// Initialize Wi-Fi
static void event_handler(void *arg, esp_event_base_t event_base,
                          int32_t event_id, void *event_data) {
  if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
    esp_wifi_connect();
  } else if (event_base == WIFI_EVENT &&
             event_id == WIFI_EVENT_STA_DISCONNECTED) {
    ESP_LOGI(TAG, "Retry connecting to the AP");
    esp_wifi_connect();
    xEventGroupClearBits(wifi_event_group, WIFI_CONNECTED_BIT);
  } else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
    ip_event_got_ip_t *event = (ip_event_got_ip_t *)event_data;
    ESP_LOGI(TAG, "Got IP:" IPSTR, IP2STR(&event->ip_info.ip));
    xEventGroupSetBits(wifi_event_group, WIFI_CONNECTED_BIT);
  }
}

static void wifi_init(void) {
  wifi_event_group = xEventGroupCreate();

  esp_netif_init();
  esp_event_loop_create_default();
  esp_netif_create_default_wifi_sta();
  wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
  esp_wifi_init(&cfg);

  esp_event_handler_instance_register(WIFI_EVENT, ESP_EVENT_ANY_ID,
                                      &event_handler, NULL, NULL);
  esp_event_handler_instance_register(IP_EVENT, IP_EVENT_STA_GOT_IP,
                                      &event_handler, NULL, NULL);

  wifi_config_t wifi_config = {};
  strncpy((char *)wifi_config.sta.ssid, WIFI_SSID,
          sizeof(wifi_config.sta.ssid));
  strncpy((char *)wifi_config.sta.password, WIFI_PASS,
          sizeof(wifi_config.sta.password));

  esp_wifi_set_mode(WIFI_MODE_STA);
  esp_wifi_set_config(WIFI_IF_STA, &wifi_config);
  esp_wifi_start();

  ESP_LOGI(TAG, "Waiting for WiFi connection...");
}

static void log_error_if_nonzero(const char *message, int error_code) {
  if (error_code != 0) {
    ESP_LOGE(TAG, "Last error %s: 0x%x", message, error_code);
  }
}

/*
 * @brief Event handler registered to receive MQTT events
 *
 *  This function is called by the MQTT client event loop.
 *
 * @param handler_args user data registered to the event.
 * @param base Event base for the handler(always MQTT Base in this example).
 * @param event_id The id for the received event.
 * @param event_data The data for the event, esp_mqtt_event_handle_t.
 */
static void mqtt_event_handler(void *handler_args, esp_event_base_t base,
                               int32_t event_id, void *event_data) {
  ESP_LOGD(TAG,
           "Event dispatched from event loop base=%s, event_id=%" PRIi32 "",
           base, event_id);
  esp_mqtt_event_handle_t event =
      reinterpret_cast<esp_mqtt_event_handle_t>(event_data);
  switch ((esp_mqtt_event_id_t)event_id) {
  case MQTT_EVENT_CONNECTED:
    ESP_LOGI(TAG, "MQTT_EVENT_CONNECTED");
    break;
  case MQTT_EVENT_DISCONNECTED:
    ESP_LOGI(TAG, "MQTT_EVENT_DISCONNECTED");
    break;

  case MQTT_EVENT_SUBSCRIBED:
    ESP_LOGI(TAG, "MQTT_EVENT_SUBSCRIBED, msg_id=%d", event->msg_id);
    break;
  case MQTT_EVENT_UNSUBSCRIBED:
    ESP_LOGI(TAG, "MQTT_EVENT_UNSUBSCRIBED, msg_id=%d", event->msg_id);
    break;
  case MQTT_EVENT_PUBLISHED:
    ESP_LOGI(TAG, "MQTT_EVENT_PUBLISHED, msg_id=%d", event->msg_id);
    break;
  case MQTT_EVENT_DATA:
    ESP_LOGI(TAG, "MQTT_EVENT_DATA");
    printf("TOPIC=%.*s\r\n", event->topic_len, event->topic);
    printf("DATA=%.*s\r\n", event->data_len, event->data);
    break;
  case MQTT_EVENT_ERROR:
    ESP_LOGI(TAG, "MQTT_EVENT_ERROR");
    if (event->error_handle->error_type == MQTT_ERROR_TYPE_TCP_TRANSPORT) {
      log_error_if_nonzero("reported from esp-tls",
                           event->error_handle->esp_tls_last_esp_err);
      log_error_if_nonzero("reported from tls stack",
                           event->error_handle->esp_tls_stack_err);
      log_error_if_nonzero("captured as transport's socket errno",
                           event->error_handle->esp_transport_sock_errno);
      ESP_LOGI(TAG, "Last errno string (%s)",
               strerror(event->error_handle->esp_transport_sock_errno));
    }
    break;
  default:
    ESP_LOGI(TAG, "Other event id:%d", event->event_id);
    break;
  }
}

extern "C" void app_main(void) {
  esp_err_t ret = nvs_flash_init();
  if (ret == ESP_ERR_NVS_NO_FREE_PAGES ||
      ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
    ESP_ERROR_CHECK(nvs_flash_erase());
    ret = nvs_flash_init();
  }
  ESP_ERROR_CHECK(ret);

  wifi_init();
  // Wait for WiFi connection
  EventBits_t bits = xEventGroupWaitBits(wifi_event_group, WIFI_CONNECTED_BIT,
                                         pdFALSE, pdFALSE, portMAX_DELAY);

  if (bits & WIFI_CONNECTED_BIT) {
    ESP_LOGI(TAG, "WiFi Connected, starting MQTT...");
  }

  esp_mqtt_client_config_t mqtt_cfg = {
      .broker = {.address = {
                     .hostname = "192.168.0.232",
                     .transport = MQTT_TRANSPORT_OVER_TCP,
                     .port = 1883,
                 }}};
  esp_mqtt_client_handle_t client = esp_mqtt_client_init(&mqtt_cfg);
  esp_mqtt_client_register_event(client, (esp_mqtt_event_id_t)ESP_EVENT_ANY_ID,
                                 mqtt_event_handler, NULL);
  esp_mqtt_client_start(client);

  ESP_LOGI(TAG, "[APP] Free memory: %" PRIu32 " bytes",
           esp_get_free_heap_size());

  ESP_LOGI(TAG,
           "Install temperature sensor, expected temp ranger range: 20~80 ℃");
  temperature_sensor_handle_t temp_sensor = NULL;
  temperature_sensor_config_t temp_sensor_config =
      TEMPERATURE_SENSOR_CONFIG_DEFAULT(20, 80);
  ESP_ERROR_CHECK(
      temperature_sensor_install(&temp_sensor_config, &temp_sensor));
  ESP_LOGI(TAG, "Enable temperature sensor");
  ESP_ERROR_CHECK(temperature_sensor_enable(temp_sensor));
  // float tsens_value;

  ESP_LOGI(TAG, "Free PSRAM before init: %d",
           heap_caps_get_free_size(MALLOC_CAP_SPIRAM));
  ESP_LOGI(TAG, "Largest free block in PSRAM: %d",
           heap_caps_get_largest_free_block(MALLOC_CAP_SPIRAM));

  Camera cam;

  while (1) {
    vTaskDelay(1000 / portTICK_PERIOD_MS);

    ESP_LOGI(TAG, "[APP] Free memory before fb: %" PRIu32 " bytes",
             esp_get_free_heap_size());
    const camera_fb_t *image = cam.get_fb();
    ESP_LOGI(TAG, "[APP] Free memory after fb: %" PRIu32 " bytes",
             esp_get_free_heap_size());
    // Publish the image to the MQTT topic
    esp_mqtt_client_publish(client, "mqtt/rpi/image",
                            reinterpret_cast<const char *>(image->buf),
                            image->len, 2, 0);
    ESP_LOGI(TAG, "Message published!");
    cam.return_fb();
  }
}