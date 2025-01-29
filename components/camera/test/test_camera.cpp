// components/camera/test/test_camera.cpp
#include "camera.h"
#include "unity.h"

static Camera *test_camera = nullptr;

void Camera_image_data_test() {
  test_camera = new Camera();
  TEST_ASSERT_NOT_NULL(test_camera);
  TEST_ASSERT_EQUAL(ESP_OK, test_camera->take_image());
  TEST_ASSERT_NOT_NULL(test_camera->get_image_data());
  TEST_ASSERT_GREATER_THAN(0, test_camera->get_image_size());
  test_camera->return_fb();
  delete test_camera;
  test_camera = nullptr;
}

extern "C" void app_main(void) {
  UNITY_BEGIN();
  RUN_TEST(Camera_image_data_test);
  UNITY_END();
}