#include "sdk_module.h"
#include "wasm_shim.h"
#include "utils.h"

#include <string.h>

#define __FILENAME__ (strrchr(__FILE__, '/') ? strrchr(__FILE__, '/') + 1 : __FILE__)

static PyObject *
sdk_log_info(PyObject *self, PyObject *args)
{
    char *message;
    Py_ssize_t message_len;
    i32 ident;

    if (!PyArg_ParseTuple(args, "s#i:" SDK_LOG_INFO, &message, &message_len, &ident))
    {
        if (PyErr_Occurred())
            PyErr_Print();
        return NULL;
    }

    env_log_msg((u8 *)message, message_len, SDK_LOG_LEVEL_INFO, ident);
    Py_RETURN_NONE;
}

static PyObject *
sdk_http_get(PyObject *self, PyObject *args)
{
    char *url;
    Py_ssize_t url_len;
    i32 ident;

    if (!PyArg_ParseTuple(args, "s#i:" SDK_HTTP_GET, &url, &url_len, &ident))
    {
        if (PyErr_Occurred())
            PyErr_Print();
        return NULL;
    }

    i32 response_size;
    response_size = env_fetch_url(SDK_HTTP_METHOD_GET, (u8 *)url, url_len, NULL, 0, ident);

    // The above gives us the size of the response,
    // we need to allocate memory that will store the response,
    // and request the FFI result to be copied into that reservation
    u8 *resultptr;
    resultptr = allocate(response_size);
    env_get_ffi_result(resultptr, ident);

    PyObject *pResult = PyBytes_FromStringAndSize(resultptr, response_size);
    if (!pResult)
    {
        if (PyErr_Occurred())
            PyErr_Print();
        LOG_MSG(__FILENAME__, "id=%d | sdk_http_get: failed to produce a result object pointer=%p, size=%d, ident=%d!",
                ident, resultptr, response_size, ident);
        return NULL;
    }
    // TODO: after this we need to clear all allocations,
    // Py_BuildValue copies data when creating the Python object

    // Required before returning an object
    Py_INCREF(pResult);

    return pResult;
}

static PyObject *
sdk_return_result(PyObject *self, PyObject *args)
{
    // Arg parsing rabbit hole here - https://docs.python.org/3/c-api/arg.html
    char *result;
    Py_ssize_t result_len;
    i32 ident;

    if (!PyArg_ParseTuple(args, "s#i:" SDK_RETURN_RESULT, &result, &result_len, &ident))
    {
        if (PyErr_Occurred())
            PyErr_Print();
        return NULL;
    }

    LOG_MSG(__FILENAME__, "id=%d | called return_result(%p, %zd, %d).",
            ident, result, result_len, ident);
    env_return_result((u8 *)result, result_len, ident);
    Py_RETURN_NONE;
}

static PyObject *
sdk_return_error(PyObject *self, PyObject *args)
{
    // Arg parsing rabbit hole here - https://docs.python.org/3/c-api/arg.html
    i32 error_code;
    char *msg;
    Py_ssize_t msg_len;
    i32 ident;

    if (!PyArg_ParseTuple(args, "is#i:" SDK_RETURN_RESULT, &error_code, &msg, &msg_len, &ident))
    {
        if (PyErr_Occurred())
            PyErr_Print();
        return NULL;
    }

    LOG_MSG(__FILENAME__, "id=%d | called return_error(%d, %p, %zd, %d).",
            ident, error_code, msg, msg_len, ident);

    env_return_error(error_code, (u8 *)msg, msg_len, ident);
    Py_RETURN_NONE;
}

static PyMethodDef SdkMethods[] = {
    {SDK_RETURN_RESULT, sdk_return_result, METH_VARARGS,
     "Return a result from the plugin execution."},
    {SDK_RETURN_ERROR, sdk_return_error, METH_VARARGS,
     "Return an error from the plugin execution."},
    {SDK_LOG_INFO, sdk_log_info, METH_VARARGS,
     "Send a string message to the log with INFO loglevel."},
    {SDK_HTTP_GET, sdk_http_get, METH_VARARGS,
     "Performs a HTTP GET request and returns the response body."},
    {NULL, NULL, 0, NULL}};

static PyModuleDef SdkModule = {
    PyModuleDef_HEAD_INIT, SDK_MODULE, NULL, -1, SdkMethods,
    NULL, NULL, NULL, NULL};

static PyObject *
PyInit_SdkModule(void)
{
    return PyModule_Create(&SdkModule);
}

int init_sdk_module()
{
    return PyImport_AppendInittab(SDK_MODULE, &PyInit_SdkModule);
}
