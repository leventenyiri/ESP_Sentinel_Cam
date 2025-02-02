#include "esp_camera.h"

#define CAM_PIN_PWDN -1          // GPIO_NUM_38
#define CAM_PIN_RESET -1         // GPIO_NUM_39
#define CAM_PIN_XCLK GPIO_NUM_4  // XC
#define CAM_PIN_SIOD GPIO_NUM_15 // SDA
#define CAM_PIN_SIOC GPIO_NUM_16 // SCL

#define CAM_PIN_D7 GPIO_NUM_21   // D9
#define CAM_PIN_D6 GPIO_NUM_47   // D8
#define CAM_PIN_D5 GPIO_NUM_48   // D7
#define CAM_PIN_D4 GPIO_NUM_45   // D6
#define CAM_PIN_D3 GPIO_NUM_0    // D5
#define CAM_PIN_D2 GPIO_NUM_17   // D4
#define CAM_PIN_D1 GPIO_NUM_18   // D3
#define CAM_PIN_D0 GPIO_NUM_8    // D2
#define CAM_PIN_VSYNC GPIO_NUM_7 // VS
#define CAM_PIN_HREF GPIO_NUM_6  // HS
#define CAM_PIN_PCLK GPIO_NUM_5  // PC

class Camera {
public:
  Camera();
  esp_err_t take_image();
  const char *get_image_data() {
    return reinterpret_cast<const char *>(_fb->buf);
  }
  size_t get_image_size() { return _fb->len; }
  void return_fb() { esp_camera_fb_return(_fb); }

private:
  camera_config_t _config;
  camera_fb_t *_fb;
};