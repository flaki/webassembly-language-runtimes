#pragma once

// Simple python module that offers python-host functions
#define PY_SSIZE_T_CLEAN
#include <Python.h>

#define SDK_MODULE "sdk"

#define SDK_RETURN_RESULT "return_result"
#define SDK_RETURN_ERROR "return_error"

#define SDK_LOG_INFO "log_info"
#define SDK_LOG_LEVEL_INFO 3

#define SDK_HTTP_GET "http_get"
#define SDK_HTTP_METHOD_GET 0

int init_sdk_module();
