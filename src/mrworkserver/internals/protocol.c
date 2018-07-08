#include "protocol.h"
#include "module.h"
#include "dec.h"

#include "Python.h"
#include <errno.h>
#include <string.h>

#include <netinet/in.h>
#include <netinet/tcp.h>

static PyObject *list = NULL;

static void print_buffer( char* b, int len ) {
  for ( int z = 0; z < len; z++ ) {
    printf( "%02x ",(int)b[z]);
  }
  printf("\n");
}


void printErr(void) {
  PyObject *type, *value, *traceback;
  PyErr_Fetch(&type, &value, &traceback);
  printf("Unhandled exception :\n");
  PyObject_Print( type, stdout, 0 ); printf("\n");
  PyObject_Print( value, stdout, 0 ); printf("\n");
}

PyObject * Protocol_new(PyTypeObject *type, PyObject *args, PyObject *kwds)
{
  Protocol* self = NULL;
  DBG printf("protocol new\n");

  self = (Protocol*)type->tp_alloc(type, 0);
  if(!self) goto finally;

  self->transport = NULL;
  self->app   = NULL;
  self->write = NULL;
  self->task  = NULL;

  finally:
  return (PyObject*)self;
}

void Protocol_dealloc(Protocol* self)
{
  Py_XDECREF(self->app);
  Py_XDECREF(self->async_func);
  Py_XDECREF(self->transport);
  Py_XDECREF(self->write);
  Py_TYPE(self)->tp_free((PyObject*)self);
}

int Protocol_init(Protocol* self, PyObject *args, PyObject *kw)
{
  DBG printf("protocol init\n");
  self->closed = true;

  if(!PyArg_ParseTuple(args, "O", &self->app)) return -1;
  Py_INCREF(self->app);

  self->bufp = NULL;
  self->buf = malloc(2*1024*1024);

  if ( list == NULL ) list = PyList_New(0);

  if(!(self->task_done   = PyObject_GetAttrString((PyObject*)self, "task_done"  ))) return -1;

  PyObject* loop = NULL;
  if(!(loop = PyObject_GetAttrString(self->app, "_loop"  ))) return -1;
  if(!(self->create_task = PyObject_GetAttrString(loop, "create_task"))) return -1;


  return 0;
}

PyObject* Protocol_connection_made(Protocol* self, PyObject* transport)
{
  PyObject* connections = NULL;

  self->transport = transport; Py_INCREF(self->transport);

  if(!(self->write = PyObject_GetAttrString(transport, "write"))) return NULL;
  if(!(self->async_func  = PyObject_GetAttrString(self->app, "cb"))) return NULL;
  if(!(connections = PyObject_GetAttrString(self->app, "_connections"))) return NULL;

  if(PySet_Add(connections, (PyObject*)self) == -1) { Py_XDECREF(connections); return NULL; }
  DBG printf("connection made num %ld\n", PySet_GET_SIZE(connections));
  Py_XDECREF(connections);

  self->closed = false;
  Py_RETURN_NONE;
}

void* Protocol_close(Protocol* self)
{
  void* result = self;

  PyObject* close = PyObject_GetAttrString(self->transport, "close");
  if(!close) return NULL;
  PyObject* tmp = PyObject_CallFunctionObjArgs(close, NULL);
  Py_XDECREF(close);
  if(!tmp) return NULL;
  Py_DECREF(tmp);
  self->closed = true;

  return result;

}

PyObject* Protocol_eof_received(Protocol* self) {
  DBG printf("eof received\n");
  Py_RETURN_NONE; // Closes the connection and conn lost will be called next
}
PyObject* Protocol_connection_lost(Protocol* self, PyObject* args)
{
  DBG printf("conn lost\n");
  self->closed = true;

  PyObject* connections = NULL;

  // Remove the connection from app.connections
  if(!(connections = PyObject_GetAttrString(self->app, "_connections"))) return NULL;
  int rc = PySet_Discard(connections, (PyObject*)self);
  Py_XDECREF(connections);
  if ( rc == -1 ) return NULL;

  Py_RETURN_NONE;
}

PyObject* Protocol_data_received(Protocol* self, PyObject* py_data)
{
  //self->num_data_received++;
  printf("protocol data recvd %ld\n", Py_SIZE(py_data));

  char* p;
  Py_ssize_t psz;
  int i;

  if(PyBytes_AsStringAndSize(py_data, &p, &psz) == -1) {
    printf("WARNING py bytes as string failed\n");
    return NULL; //TODO set error
  }

  int data_left = psz;
  if ( self->bufp ) {
    memcpy(self->bufp, p, psz);
    data_left = self->bufp - self->buf;
    p = self->bufp;
    self->bufp = NULL;
  }

  int zz = 0; 
  while ( data_left > 0 ) {
    zz += 1;
    if ( zz > 100 ) break;

    if ( data_left < 4 ) {
      //DBG printf("Received partial data %d more bytes need %d\n",data_left, conn->needs);
      if ( self->bufp == NULL ) self->bufp = self->buf;
      memcpy(self->bufp, p, data_left);
      Py_RETURN_NONE;
    }

    int cmd   = (unsigned char)p[1];
    int topic = (unsigned char)p[2];
    int slot  = (unsigned char)p[3];

    if ( cmd == 1 ) {

      if ( data_left < 8 ) {
        //DBG printf("Received partial data %d more bytes need %d\n",data_left, conn->needs);
        if ( self->bufp == NULL ) self->bufp = self->buf;
        memcpy(self->bufp, p, data_left);
        Py_RETURN_NONE;
      }
      int len   = *((int*)(p)+1);
      p += 8;
      data_left -= len + 8;

      //memcpy( s->write, p, len );
      char *endptr;
      PyObject *o;
#ifdef __AVX2__
      o = (PyObject*)jParse(p, &endptr, len);
#else
      o = (PyObject*)jsonParse(p, &endptr, len);
#endif
      //PyObject_Print( o, stdout, 0 );
      PyList_Append( list, o );
      p = endptr;

    }
    else {
      printf("ERROR unrecognized cmd %d\n",cmd);
      //TODO drop conn
      print_buffer(p, data_left);
      Py_RETURN_NONE;
    }
  }

  // If we have enough items and aren't waiting on a callback 
  if ( PyList_GET_SIZE(list) > 2 && self->task == NULL ) {
    return protocol_process_messages(self);
  }

  Py_RETURN_NONE;
}

PyObject* Protocol_get_transport(Protocol* self)
{
  Py_INCREF(self->transport);
  return self->transport;
}

PyObject* protocol_process_messages(Protocol* self) {

  PyObject* ret = NULL;
  PyObject* tmp = NULL;
  PyObject* add_done_callback = NULL;

  // Calling an async function returns a coroutine so create a task for it and a done callback
  if(!(ret        = PyObject_CallFunctionObjArgs(self->async_func,  list, NULL))) return NULL; 
  if(!(self->task = PyObject_CallFunctionObjArgs(self->create_task, ret,  NULL))) return NULL; 
  Py_XDECREF(ret);

  if(!(add_done_callback = PyObject_GetAttrString(self->task, "add_done_callback"))) goto error;
  if(!(tmp = PyObject_CallFunctionObjArgs(add_done_callback, self->task_done, NULL))) goto error;
  Py_DECREF(tmp);
  Py_DECREF(add_done_callback);

  Py_RETURN_NONE;

error:
  Py_XDECREF(self->task); self->task = NULL;
  Py_XDECREF(ret);
  Py_XDECREF(add_done_callback);
  Py_XDECREF(tmp);
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

PyObject* Protocol_task_done(Protocol* self, PyObject* task)
{
  Py_XDECREF(list);
  list = PyList_New(0);

  Py_XDECREF(self->task); self->task = NULL;

  Py_RETURN_NONE;
}


