#include "mqtt.h"
#include "esp_log.h"

constexpr auto *TAG = "MQTT";

MQTT::MQTT() {
  // These should be read from NVS
  _config = {
      .broker =
          {
              .address =
                  {
                      .hostname = "192.168.0.232",
                      .transport = MQTT_TRANSPORT_OVER_TCP,
                      .port = 1883,
                  },
          },
      .credentials = {.username = "user",
                      .authentication =
                          {
                              .password = "password",
                          }},
      .session =
          {
              // might help with status messages
              .last_will =
                  {
                      .topic = nullptr,
                      .msg = nullptr,
                      .msg_len = 0,
                      .qos = 0,
                      .retain = false,
                  },
              .protocol_ver = MQTT_PROTOCOL_V_5,
          },
  };
  _qos = 2;
}

void MQTT::log_error_if_nonzero(const char *message, int error_code) {
  if (error_code != 0) {
    ESP_LOGE(TAG, "%s: %d", message, error_code);
  }
}

void MQTT::event_handler(void *handler_args, esp_event_base_t base,
                         int32_t event_id, void *event_data) {
  esp_mqtt_event_handle_t event = (esp_mqtt_event_handle_t)event_data;
  switch (event_id) {
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

void MQTT::start() {
  _client = esp_mqtt_client_init(&_config);
  esp_mqtt_client_register_event(_client, (esp_mqtt_event_id_t)ESP_EVENT_ANY_ID,
                                 event_handler, NULL);
  esp_mqtt_client_start(_client);
}

void MQTT::publish(const char *topic, const char *data, size_t len) {
  esp_mqtt_client_publish(_client, topic, data, len, _qos, false);
}

void MQTT::subscribe(const std::string &topic) {
  esp_mqtt_client_subscribe(_client, topic.c_str(), _qos);
}