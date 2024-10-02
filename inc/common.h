#ifndef __COMMON_H
#define __COMMON_H

#include <glad/gl.h>
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include <stdlib.h>

#ifndef __FILE_NAME__
    #include "libgen.h"
    #define __FILE_NAME__ basename(__FILE__)
#endif

#include "log.h"



extern log_t *log_handle;

#define ERROR_CHECK(error_cond, error_action, log_level, message, ...) \
do { \
if (error_cond) { \
    log__write(log_handle, log_level, "%s:%u: (%s) "message,__FILE_NAME__,__LINE__,#error_cond,##__VA_ARGS__); \
    error_action; \
}} while(0)

#endif /* __COMMON_H */
