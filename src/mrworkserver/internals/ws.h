
#pragma once

#include <Python.h>
#include <stdbool.h>

typedef struct {
  PyObject_HEAD

  PyObject* async_func;
  PyObject* fetch_func;
  PyObject* task_done;
  PyObject* create_task;
  PyObject* task;

  PyObject *list, *list2;
  int gather_seconds;
  unsigned long last_time;

  bool collect_stats;
  PyObject *async_times;
  double async_start_time;


} WorkServer;

PyObject *WorkServer_new    (PyTypeObject* self, PyObject *args, PyObject *kwargs);
int       WorkServer_init   (WorkServer* self,    PyObject *args, PyObject *kwargs);
void      WorkServer_dealloc(WorkServer* self);

PyObject *WorkServer_cinit(WorkServer* self);

PyObject* WorkServer_process_messages(WorkServer* self, int force);
PyObject* WorkServer_fetch           (WorkServer* self, PyObject *j);
PyObject* WorkServer_task_done(WorkServer* self, PyObject* task);
PyObject *WorkServer_hot(WorkServer *self, PyObject *args); 
