#include "mechanics.h"

#include "vector.h"
#include "log.h"

#include "unity.h"


#define __SKIP_LOG_DATA

#define STR_BUF_SIZE    256


/* Unused but needs to be defined */
log_t *log_handle;


void setUp(void)
{

}

void tearDown(void)
{

}

void test_componentize_force_2d(void)
{
    const vector2d_t distance_vector[] = {
        {.i = 1,    .j = 0},
        {.i = 1,    .j = 1},
        {.i = 0,    .j = 1},
        {.i = -1,   .j = 1},
        {.i = -1,   .j = 0},
        {.i = -1,   .j = -1},
        {.i = 0,    .j = -1},
        {.i = 1,    .j = -1},
    };
    const vector2d_t F_expected[] = {
        {.i = 1,                    .j = 0},
        {.i = 0.707106781186547,    .j = 0.707106781186547},
        {.i = 6.12323399573677E-17, .j = 1},
        {.i = -0.707106781186547,   .j = 0.707106781186547},
        {.i = -1,                   .j = 0},
        {.i = -0.707106781186547,   .j = -0.707106781186547},
        {.i = 6.12323399573677E-17, .j = -1},
        {.i = 0.707106781186547,    .j = -0.707106781186547},
    };
    const unsigned int test_count = sizeof(F_expected)/sizeof(vector2d_t);
    const double F_scalar = 1;
    char msg_buf[STR_BUF_SIZE];

    for (unsigned int i = 0; i < test_count; ++i) {

        const vector2d_t F_actual = componentize_force_2d(F_scalar, distance_vector[i]);

        snprintf(msg_buf, sizeof(msg_buf), "Failure at %i loop iteration of i component", i);
        TEST_ASSERT_DOUBLE_WITHIN_MESSAGE(10E-15, F_expected[i].i, F_actual.i, msg_buf);
        snprintf(msg_buf, sizeof(msg_buf), "Failure at %i loop iteration of j component", i);
        TEST_ASSERT_EQUAL_DOUBLE_MESSAGE(F_expected[i].j, F_actual.j, msg_buf);

        resetTest();
    }
}

void test_componentize_force_3d(void)
{
    const vector3d_t distance_vector[] = {
        {.i = 1,    .j = 0,     .k = 1},
        {.i = 1,    .j = 1,     .k = 1},
        {.i = 0,    .j = 1,     .k = 1},
        {.i = -1,   .j = 1,     .k = 1},
        {.i = -1,   .j = 0,     .k = 1},
        {.i = -1,   .j = -1,    .k = 1},
        {.i = 0,    .j = -1,    .k = 1},
        {.i = 1,    .j = -1,    .k = 1},
        
        {.i = 1,    .j = 0,     .k = -1},
        {.i = 1,    .j = 1,     .k = -1},
        {.i = 0,    .j = 1,     .k = -1},
        {.i = -1,   .j = 1,     .k = -1},
        {.i = -1,   .j = 0,     .k = -1},
        {.i = -1,   .j = -1,    .k = -1},
        {.i = 0,    .j = -1,    .k = -1},
        {.i = 1,    .j = -1,    .k = -1},
    };
    const vector3d_t F_expected[] = {
        {.i = 0.707106781186547,    .j = 0,                     .k = 0.707106781186547},
        {.i = 0.577350269189626,    .j = 0.577350269189626,     .k = 0.577350269189626},
        {.i = 4.32978028117747E-17, .j = 0.707106781186547,     .k = 0.707106781186547},
        {.i = -0.577350269189626,   .j = 0.577350269189626,     .k = 0.577350269189626},
        {.i = -0.707106781186547,   .j = 0,                     .k = 0.707106781186547},
        {.i = -0.577350269189626,   .j = -0.577350269189626,    .k = 0.577350269189626},
        {.i = 4.32978028117747E-17, .j = -0.707106781186547,    .k = 0.707106781186547},
        {.i = 0.577350269189626,    .j = -0.577350269189626,    .k = 0.577350269189626},

        {.i = 0.707106781186547,    .j = 0,                     .k = -0.707106781186547},
        {.i = 0.577350269189626,    .j = 0.577350269189626,     .k = -0.577350269189626},
        {.i = 4.32978028117747E-17, .j = 0.707106781186547,     .k = -0.707106781186547},
        {.i = -0.577350269189626,   .j = 0.577350269189626,     .k = -0.577350269189626},
        {.i = -0.707106781186547,   .j = 0,                     .k = -0.707106781186547},
        {.i = -0.577350269189626,   .j = -0.577350269189626,    .k = -0.577350269189626},
        {.i = 4.32978028117747E-17, .j = -0.707106781186547,    .k = -0.707106781186547},
        {.i = 0.577350269189626,    .j = -0.577350269189626,    .k = -0.577350269189626},
    };
    const unsigned int test_count = sizeof(F_expected)/sizeof(vector3d_t);
    const double F_scalar = 1;
    char msg_buf[STR_BUF_SIZE];

    for (unsigned int i = 0; i < test_count; ++i) {

        const vector3d_t F_actual = componentize_force_3d(F_scalar, distance_vector[i]);

        snprintf(msg_buf, sizeof(msg_buf), "Failure at %i loop iteration of i component", i);
        TEST_ASSERT_DOUBLE_WITHIN_MESSAGE(10E-15, F_expected[i].i, F_actual.i, msg_buf);
        snprintf(msg_buf, sizeof(msg_buf), "Failure at %i loop iteration of j component", i);
        TEST_ASSERT_EQUAL_DOUBLE_MESSAGE(F_expected[i].j, F_actual.j, msg_buf);
        snprintf(msg_buf, sizeof(msg_buf), "Failure at %i loop iteration of k component", i);
        TEST_ASSERT_EQUAL_DOUBLE_MESSAGE(F_expected[i].k, F_actual.k, msg_buf);

        resetTest();
    }
}
