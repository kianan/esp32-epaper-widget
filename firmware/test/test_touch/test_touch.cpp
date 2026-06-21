#include <unity.h>
#include "touch_logic.h"

void setUp() {}
void tearDown() {}

void test_tap_no_movement() {
    TEST_ASSERT_EQUAL(TOUCH_TAP, classifyGesture(0, 0));
}

void test_tap_small_movement() {
    TEST_ASSERT_EQUAL(TOUCH_TAP, classifyGesture(10, 5));
    TEST_ASSERT_EQUAL(TOUCH_TAP, classifyGesture(24, 19));  // just under both thresholds
}

void test_swipe_right() {
    TEST_ASSERT_EQUAL(SWIPE_RIGHT, classifyGesture(25, 0));  // exactly at threshold
    TEST_ASSERT_EQUAL(SWIPE_RIGHT, classifyGesture(80, 10));
}

void test_swipe_left() {
    TEST_ASSERT_EQUAL(SWIPE_LEFT, classifyGesture(-25, 0));
    TEST_ASSERT_EQUAL(SWIPE_LEFT, classifyGesture(-80, 10));
}

void test_swipe_down() {
    TEST_ASSERT_EQUAL(SWIPE_DOWN, classifyGesture(0, 20));   // exactly at threshold
    TEST_ASSERT_EQUAL(SWIPE_DOWN, classifyGesture(5, 60));
}

void test_swipe_up() {
    TEST_ASSERT_EQUAL(SWIPE_UP, classifyGesture(0, -20));
    TEST_ASSERT_EQUAL(SWIPE_UP, classifyGesture(5, -60));
}

void test_diagonal_dominant_axis_wins() {
    TEST_ASSERT_EQUAL(SWIPE_RIGHT, classifyGesture(50, 30));  // more horizontal
    TEST_ASSERT_EQUAL(SWIPE_DOWN,  classifyGesture(30, 50));  // more vertical
    TEST_ASSERT_EQUAL(SWIPE_LEFT,  classifyGesture(-50, 30));
    TEST_ASSERT_EQUAL(SWIPE_UP,    classifyGesture(10, -40));
}

void test_equal_displacement_prefers_horizontal() {
    // adx >= ady branch → horizontal wins on tie
    TEST_ASSERT_EQUAL(SWIPE_RIGHT, classifyGesture(30, 30));
    TEST_ASSERT_EQUAL(SWIPE_LEFT,  classifyGesture(-30, 30));
}

int main() {
    UNITY_BEGIN();
    RUN_TEST(test_tap_no_movement);
    RUN_TEST(test_tap_small_movement);
    RUN_TEST(test_swipe_right);
    RUN_TEST(test_swipe_left);
    RUN_TEST(test_swipe_down);
    RUN_TEST(test_swipe_up);
    RUN_TEST(test_diagonal_dominant_axis_wins);
    RUN_TEST(test_equal_displacement_prefers_horizontal);
    return UNITY_END();
}
