#include "camera.h"
#include "unity.h"

void dummy_test() { TEST_ASSERT_TRUE(true); }

extern "C" void app_main(void) {
  UNITY_BEGIN();
  RUN_TEST(dummy_test);
  UNITY_END();
}