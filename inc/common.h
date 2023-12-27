#ifndef __COMMON_H
#define __COMMON_H

#include <stdlib.h>

#include "log.h"

#define ERROR_CHECK(error_cond, error_action, log_handle, log_level, message, ...) \
do { \
if (error_cond) { \
    log__write(log_handle, log_level, "%s:%u: (%s) "message,__FILE__,__LINE__,#error_cond,##__VA_ARGS__); \
    error_action; \
}} while(0)

#endif /* __COMMON_H */
