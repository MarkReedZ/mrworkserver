
#include <Python.h>
#include <stdbool.h>
#include <time.h>

#include "ws.h"
#include "module.h"
#include "unpack.h"

PyObject *WorkServer_new(PyTypeObject* type, PyObject *args, PyObject *kwargs) {
  WorkServer* self = NULL;
  self = (WorkServer*)type->tp_alloc(type, 0);
  return (PyObject*)self;
}


void WorkServer_dealloc(WorkServer* self) {
  Py_XDECREF(self->async_func);
  Py_TYPE(self)->tp_free((PyObject*)self);

}

int WorkServer_init(WorkServer* self, PyObject *args, PyObject *kwargs) {
  self->list = PyList_New(0);

  PyObject* loop = NULL;
  if(!(loop = PyObject_GetAttrString((PyObject*)self, "_loop"  ))) return -1;
  if(!(self->create_task = PyObject_GetAttrString(loop, "create_task"))) return -1;

  self->last_time = 0;
  if(!PyArg_ParseTuple(args, "Oi", &self->async_func, &self->gather_seconds)) return -1;
  self->task  = NULL;
  if(!(self->task_done  = PyObject_GetAttrString((PyObject*)self, "task_done"))) return -1;
  if(!(self->fetch_func = PyObject_GetAttrString((PyObject*)self, "prefetch" ))) return -1;
  self->collect_stats = false;
  if(!(self->async_times = PyObject_GetAttrString((PyObject*)self, "async_times"  ))) return -1;
  if ( self->async_times != Py_None ) { self->collect_stats = true; }


  //printf("init packer\n");
  //initmrpacker();

  return 0;
}

PyObject *WorkServer_cinit(WorkServer* self) {
 Py_RETURN_NONE;
}

PyObject* WorkServer_fetch(WorkServer* self, PyObject *j) {
  return PyObject_CallFunctionObjArgs(self->fetch_func, j, NULL);
}

PyObject* WorkServer_process_messages(WorkServer* self, int force) {

  //printf( "process %d\n",PyList_GET_SIZE(self->list));

  if ( !force ) {

    // Don't call the async func until the previous call completes
    if ( self->task != NULL ) Py_RETURN_NONE;

    // If we have enough items or enough time has passed 
    //TODO if gather is not set we're going to process every 100 messages make an option?
    if ( self->gather_seconds ) {
      unsigned long cur_time = time(NULL);
      
      if ( (cur_time-self->last_time)>self->gather_seconds ) {
        self->last_time = cur_time;
      } else {
        Py_RETURN_NONE;
      }

    } else {
      if ( PyList_GET_SIZE(self->list) < 100 ) {
        Py_RETURN_NONE;
      }
    }
  }


  if ( self->collect_stats ) {
    static struct timespec ts; clock_gettime(CLOCK_MONOTONIC, &ts);
    self->async_start_time = ts.tv_sec*1000 + (double)ts.tv_nsec/1000000;
  }
  self->list2 = self->list;
  self->list = PyList_New(0);

  //printf( "Calling async with %d items\n",PyList_GET_SIZE(self->list2));

  PyObject* tmp = NULL;
  PyObject* add_done_callback = NULL;

  // Calling an async function returns a coroutine so create a task for it and a done callback
  if(!(tmp        = PyObject_CallFunctionObjArgs(self->async_func,  self, self->list2, NULL))) return NULL;
  if(!(self->task = PyObject_CallFunctionObjArgs(self->create_task, tmp,  NULL))) goto error;
  Py_DECREF(tmp);

  if(!(add_done_callback = PyObject_GetAttrString(self->task, "add_done_callback"))) goto error;
  if(!(tmp = PyObject_CallFunctionObjArgs(add_done_callback, self->task_done, NULL))) goto error;
  Py_DECREF(tmp);
  Py_DECREF(add_done_callback);

  Py_RETURN_NONE;

error:
  Py_XDECREF(self->task); self->task = NULL;
  Py_XDECREF(tmp);
  Py_XDECREF(add_done_callback);
  return NULL;

/*
  TODO Allow a sync callback as well?
    PyObject* tmp = PyObject_CallFunctionObjArgs(self->async_func, list, NULL);
    if ( !tmp ) {
      //TODO add test
      DBG printf("Callback failed with an exception\n");
      PyObject *type, *value, *traceback;
      PyErr_Fetch(&type, &value, &traceback);
      PyErr_NormalizeException(&type, &value, &traceback);
      //if (value) {
      printf("Unhandled exception :\n");
      PyObject_Print( type, stdout, 0 ); printf("\n");
      if ( value ) { PyObject_Print( value, stdout, 0 ); printf("\n"); }
      PyErr_Clear();
      //PyObject_Print( traceback, stdout, 0 ); printf("\n");
    
      Py_XDECREF(traceback);
      Py_XDECREF(type);
      Py_XDECREF(value);
    }

    Py_XDECREF(list);
    list = PyList_New(0);
*/
}


PyObject* WorkServer_task_done(WorkServer* self, PyObject* task)
{
  Py_XDECREF(self->list2); self->list2 = NULL;
  Py_XDECREF(self->task); self->task = NULL;

  if ( self->collect_stats ) {
    static struct timespec ts; clock_gettime(CLOCK_MONOTONIC, &ts);
    double ms = (ts.tv_sec*1000 + (double)ts.tv_nsec/1000000) - self->async_start_time;
    PyObject *pyms = PyFloat_FromDouble(ms);
    PyList_Append( self->async_times, pyms );
    Py_DECREF(pyms);
  }

  Py_RETURN_NONE;
}

PyObject *WorkServer_hot(WorkServer *self, PyObject *args) {

  int64_t rtg;
  uint64_t ts;
  if(!PyArg_ParseTuple(args, "LK", &rtg, &ts)) return NULL;
 
  // Every 50,000 seconds a new timestamp gets + 1 ( a day is 86400 )
  // log10(10,100,1000,etc) is 1,2,3... 
  uint64_t r;
  if ( rtg < 0 ) {
    r = (long) ((((ts-1530000000ull) / 50000.0) - log10(-rtg))  *10000);
  } else if ( rtg > 0 ) {
    r = (long) ((((ts-1530000000) / 50000.0) + log10(rtg) )  *10000);
  } else {
    //r = (long) ((ts-1530000000) / 50000) * 10000;
    r = (long) (ts-1530000000) / 5.0;
  }
  
  return PyLong_FromUnsignedLong(r);

}

