

#include "protocol.h"
#include "module.h"
#include "dec.h"
#include "unpack.h"

#include "Python.h"
#include <errno.h>
#include <string.h>

#include <netinet/in.h>
#include <netinet/tcp.h>

#define CMD_PUSH   0x1
#define CMD_PUSHJ  0x2
#define CMD_FLUSH  0xA
#define CMD_GET    0xB
#define CMD_SET    0xC

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
  if ( value ) {
    PyObject_Print( type, stdout, 0 ); printf("\n");
    PyObject_Print( value, stdout, 0 ); printf("\n");
  }
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

  finally:
  return (PyObject*)self;
}

void Protocol_dealloc(Protocol* self)
{
  free(self->buf);
  Py_XDECREF(self->app);
  Py_XDECREF(self->transport);
  Py_XDECREF(self->write);
  Py_TYPE(self)->tp_free((PyObject*)self);
}

int Protocol_init(Protocol* self, PyObject *args, PyObject *kw)
{
  DBG printf("protocol  init\n");
  self->closed = true;

  if(!PyArg_ParseTuple(args, "O", &self->app)) return -1;
  Py_INCREF(self->app);

  self->bufp = NULL;
  self->buf = malloc(2*1024*1024);

  //PyObject* loop = NULL;
  //if(!(loop = PyObject_GetAttrString(self->app, "_loop"  ))) return -1;

  return 0;
}

PyObject* Protocol_connection_made(Protocol* self, PyObject* transport)
{
  PyObject* connections = NULL;
  
  self->transport = transport; Py_INCREF(self->transport);
  if(!(self->write = PyObject_GetAttrString(transport, "write"))) return NULL;

  // Is this necessary? Or just inc/dec?
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
  WorkServer_process_messages((WorkServer*)self->app, 1);
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
  DBG printf("protocol data recvd %ld\n", Py_SIZE(py_data));

  char* p;
  Py_ssize_t psz;
  int i;

  if(PyBytes_AsStringAndSize(py_data, &p, &psz) == -1) {
    printf("WARNING py bytes as string failed\n");
    return NULL; //TODO set error
  }
  //if ( psz > 32 ) print_buffer( p, 32 );

  int data_left = psz;

  // If we have buffered data append ourselves to it
  if ( self->bufp ) {
    //printf(" bufp!\n");
    //print_buffer( self->buf, self->bufp-self->buf);
    memcpy(self->bufp, p, psz);
    data_left = data_left + self->bufp - self->buf;
    p = self->buf;
    self->bufp = NULL;
  }

  while ( data_left > 0 ) {
    DBG printf(" top  while dl %d\n",data_left);

    if ( data_left < 4 ) {
      //printf("Received partial data need 4\n");
      if ( self->bufp == NULL ) self->bufp = self->buf;
      memcpy(self->bufp, p, data_left);
      self->bufp += data_left;
      Py_RETURN_NONE;
    }

    int cmd   = (unsigned char)p[1];
    //int topic = (unsigned char)p[2];
    //int slot  = (unsigned char)p[3];

    DBG printf(" dl %d cmd %d\n", data_left, cmd );

    //if ( cmd != 1 ) exit(0);
    //if ( data_left > 32 ) print_buffer( p, 32 );
    //else print_buffer( p, data_left );

    if ( cmd == CMD_FLUSH ) {
      WorkServer_process_messages((WorkServer*)self->app, 1);
      p += 2;
      data_left -= 2;
    }
    else if ( cmd == CMD_SET ) {
      int len   = p[2]<<8 | p[3];

      if ( data_left < len ) {
        DBG printf("Received partial data dl %d need %d\n",data_left,len);
        if ( self->bufp == NULL ) self->bufp = self->buf;
        memcpy(self->bufp, p, data_left);
        self->bufp += data_left;
        //print_buffer( self->buf, data_left );
        //printf(" set buf to %.*s\n", self->bufp-self->buf,self->buf);
        Py_RETURN_NONE;
      }

      p += 4;
      data_left -= len + 4;
      if ( len > 0 ) {
      
        PyObject *o = unpackc( p, len ); 
        p += len;

        PyObject *ret = WorkServer_set((WorkServer*)self->app, PyList_GetItem(o, 0), PyList_GetItem(o, 1));

      }

    }
    else if ( cmd == CMD_GET ) {
      int len   = p[2]<<8 | p[3];
      DBG printf("cmd dl %d len %d\n",data_left,len);

      if ( data_left < len ) {
        DBG printf("Received partial data dl %d need %d\n",data_left,len);
        if ( self->bufp == NULL ) self->bufp = self->buf;
        memcpy(self->bufp, p, data_left);
        self->bufp += data_left;
        //print_buffer( self->buf, data_left );
        //printf(" set buf to %.*s\n", self->bufp-self->buf,self->buf);
        Py_RETURN_NONE;
      }

      p += 4;
      data_left -= len + 4;
      if ( len > 0 ) {
        char *endptr;
        PyObject *o;
        o = unpackc( p, len ); 
        p += len;
//#ifdef __AVX2__
        //o = (PyObject*)jParse(p, &endptr, len);
//#else
        //o = (PyObject*)jsonParse(p, &endptr, len);
//#endif
        PyObject *ret = WorkServer_fetch((WorkServer*)self->app, o);

        if ( !ret ) {
          PyObject *type, *value, *traceback;
          PyErr_Fetch(&type, &value, &traceback);
          Py_XDECREF(traceback); Py_XDECREF(type);
          PyErr_Restore(type, value, traceback);
          printf("Unhandled exception in fetch callback:\n");
          PyObject_Print( type, stdout, 0 ); printf("\n");
          PyObject_Print( value, stdout, 0 ); printf("\n");
          // TODO write error response back
          return NULL;
        } 
        Py_XDECREF(o);

        // If the user returned None do not send a response TODO doc and test
        if ( ret != Py_None ) {

          if ( !PyBytes_Check( ret ) ) {
            PyErr_SetString(PyExc_ValueError, "Fetch callback must return bytes");
            return NULL;
          }
          if(!(o = PyObject_CallFunctionObjArgs(self->write, ret, NULL))) return NULL;
          Py_DECREF(o);

        }

      }
    }
    else if ( cmd == CMD_PUSH || cmd == CMD_PUSHJ ) {

      if ( data_left < 6 ) {
        DBG printf("Received partial data need 8\n");
        if ( self->bufp == NULL ) self->bufp = self->buf;
        memcpy(self->bufp, p, data_left);
        self->bufp += data_left;
        Py_RETURN_NONE;
      }
      int len   = *((int*)(p+2));
      DBG printf("cmd dl %d len %d\n",data_left,len);

      if ( data_left < len+6 ) {
        DBG printf("Received partial data dl %d need %d\n",data_left,len);
        if ( self->bufp == NULL ) self->bufp = self->buf;
        memcpy(self->bufp, p, data_left);
        self->bufp += data_left;
        //print_buffer( self->buf, data_left );
        //printf(" set buf to %.*s\n", self->bufp-self->buf,self->buf);
        Py_RETURN_NONE;
      }

      p += 6;
      data_left -= len + 6;


      if ( len > 0 ) {
        char *endptr;
        PyObject *o;
        //print_buffer( p, len );

        if ( cmd == CMD_PUSH ) {
          o = unpackc( p, len ); 
          p += len;
        } else {
#ifdef __AVX2__
          o = (PyObject*)jParse(p, &endptr, len);
#else
          o = (PyObject*)jsonParse(p, &endptr, len);
#endif
          p = endptr;
        }

        // TODO what to do if bad json? Add error callback? Return error to client?
        if ( o != NULL ) {
          PyList_Append( ((WorkServer*)self->app)->list, o );
          Py_DECREF(o);
        } else {
          if ( PyErr_Occurred() ) PyErr_Print(); // Prints exception and clears the error
        }
      }

    }
    else if ( cmd == 9 ) {
      p += 2;
      data_left -= 2;
    }
    else {
      printf("ERROR unrecognized cmd %d\n",cmd);
      //TODO drop conn
      //print_buffer(p, data_left);
      Py_RETURN_NONE;
    }
  }

  return WorkServer_process_messages((WorkServer*)self->app, 0);

/*
  // If we have enough items or enough time has passed and aren't waiting on a callback 
  if ( self->gather_seconds ) {
    unsigned long cur_time = time(NULL);
    if ( ((cur_time-self->last_time)>self->gather_seconds) && self->task == NULL ) {
      self->last_time = cur_time;
      return protocol_process_messages(self);
    }
  } else {
    if ( PyList_GET_SIZE(self->app->list) > 2 && self->task == NULL ) {
      return protocol_process_messages(self);
    }
  }
*/
  Py_RETURN_NONE;
}

PyObject* Protocol_get_transport(Protocol* self)
{
  Py_INCREF(self->transport);
  return self->transport;
}


