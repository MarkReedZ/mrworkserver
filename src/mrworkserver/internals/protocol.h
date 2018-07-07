
#pragma once
#include "Python.h"
#include <stdbool.h>

#define DBG if(0)

typedef struct {
  PyObject_HEAD
  PyObject* app;
  bool closed;

  PyObject* async_func;
  PyObject* task_done;
  PyObject* create_task;
  PyObject* task;

  PyObject* transport;
  PyObject* write;
  double start_time;

  char *buf, *bufp;

} Protocol;


PyObject * Protocol_new(PyTypeObject *type, PyObject *args, PyObject *kwds);
int Protocol_init(Protocol* self, PyObject *args, PyObject *kw);
void Protocol_dealloc(Protocol* self);


PyObject* Protocol_connection_made(Protocol* self, PyObject* transport);
void* Protocol_close(Protocol* self);
PyObject* Protocol_connection_lost(Protocol* self, PyObject* args);
PyObject* Protocol_data_received(Protocol* self, PyObject* data);
PyObject* Protocol_eof_received(Protocol* self);

PyObject* Protocol_get_transport(Protocol* self);
PyObject* Protocol_task_done(Protocol* self, PyObject* task);

PyObject* protocol_process_messages(Protocol* self);
