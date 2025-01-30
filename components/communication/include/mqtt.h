#include "mqtt_client.h"
#include <string>

// TODO: Implement error handling
class MQTT {
public:
  MQTT();

  void start();
  void publish(const char *topic, const char *data, size_t len);
  void subscribe(const std::string &topic);

private:
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
  static void event_handler(void *handler_args, esp_event_base_t base,
                            int32_t event_id, void *event_data);
  static void log_error_if_nonzero(const char *message, int error_code);

  esp_mqtt_client_config_t _config;
  esp_mqtt_client_handle_t _client = nullptr;
  std::string _config_topic;
  std::string _ota_topic;
  int _qos;
};