// components/camera/test/test_camera.cpp
#include "camera.h"
#include "unity.h"

static Camera *test_camera = nullptr;

// This can be run only one time
TEST_CASE("Camera get image data test", "[camera]") {
  test_camera = new Camera();
  TEST_ASSERT_NOT_NULL(test_camera);
  TEST_ASSERT_EQUAL(ESP_OK, test_camera->take_image());
  TEST_ASSERT_NOT_NULL(test_camera->get_image_data());
  TEST_ASSERT_GREATER_THAN(0, test_camera->get_image_size());
  test_camera->return_fb();
  delete test_camera;
  test_camera = nullptr;
}

/* TEST_CASE("Camera get image size test", "[camera]") {
test_camera = new Camera();
TEST_ASSERT_NOT_NULL(test_camera);
TEST_ASSERT_EQUAL(ESP_OK, test_camera->take_image());
TEST_ASSERT_GREATER_THAN(0, test_camera->get_image_size());
test_camera->return_fb();
esp_camera_deinit(); causes this error: E (2511) gdma:gdma_disconnect(314):
no peripheral is connected to the channel
delete test_camera; test_camera = nullptr;
} */