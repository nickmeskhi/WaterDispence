#ifndef TEST_PATTERNS_H
#define TEST_PATTERNS_H

#include "draw_func.h"

// Test pattern prototypes
void test_solid_colours(void);
void test_concentric_rings(void);
void test_colour_wedges(void);
void test_grid(void);
void test_clock_marks(void);
void test_checkerboard(void);
void test_greyscale_ramp(void);

// Helper to run all tests in sequence (optional)
void run_all_tests(void);

#endif // TEST_PATTERNS_H
