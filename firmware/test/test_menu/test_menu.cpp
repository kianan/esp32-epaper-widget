#include <unity.h>
#include "menu_logic.h"

void setUp() {}
void tearDown() {}

void test_page0_quadrants() {
    TEST_ASSERT_EQUAL(SCREEN_CLOCK,    menuScreenForTap(50,  40, 0));  // top-left
    TEST_ASSERT_EQUAL(SCREEN_POMODORO, menuScreenForTap(150, 40, 0));  // top-right
    TEST_ASSERT_EQUAL(SCREEN_PHOTOS,   menuScreenForTap(50,  120, 0)); // bottom-left
    TEST_ASSERT_EQUAL(SCREEN_JESSIE,   menuScreenForTap(150, 120, 0)); // bottom-right
}

void test_page1_quadrants() {
    TEST_ASSERT_EQUAL(SCREEN_5, menuScreenForTap(50,  40,  1));
    TEST_ASSERT_EQUAL(SCREEN_6, menuScreenForTap(150, 40,  1));
    TEST_ASSERT_EQUAL(SCREEN_7, menuScreenForTap(50,  120, 1));
    TEST_ASSERT_EQUAL(SCREEN_8, menuScreenForTap(150, 120, 1));
}

void test_vertical_boundary() {
    // y < 95 → top, y >= 95 → bottom
    TEST_ASSERT_EQUAL(SCREEN_CLOCK,  menuScreenForTap(50, 94, 0));  // last top pixel
    TEST_ASSERT_EQUAL(SCREEN_PHOTOS, menuScreenForTap(50, 95, 0));  // first bottom pixel
}

void test_horizontal_boundary() {
    // x < 100 → left, x >= 100 → right
    TEST_ASSERT_EQUAL(SCREEN_CLOCK,    menuScreenForTap(99,  40, 0));  // last left pixel
    TEST_ASSERT_EQUAL(SCREEN_POMODORO, menuScreenForTap(100, 40, 0));  // first right pixel
}

void test_corners() {
    TEST_ASSERT_EQUAL(SCREEN_CLOCK,    menuScreenForTap(0,   0,   0));
    TEST_ASSERT_EQUAL(SCREEN_POMODORO, menuScreenForTap(199, 0,   0));
    TEST_ASSERT_EQUAL(SCREEN_PHOTOS,   menuScreenForTap(0,   199, 0));
    TEST_ASSERT_EQUAL(SCREEN_JESSIE,   menuScreenForTap(199, 199, 0));
}

int main() {
    UNITY_BEGIN();
    RUN_TEST(test_page0_quadrants);
    RUN_TEST(test_page1_quadrants);
    RUN_TEST(test_vertical_boundary);
    RUN_TEST(test_horizontal_boundary);
    RUN_TEST(test_corners);
    return UNITY_END();
}
