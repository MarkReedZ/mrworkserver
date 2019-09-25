
#pragma once
#include "Python.h"
#include "protocol.h"
#include "ws.h"

//static PyMethodDef mod_methods[] = {
  //{"randint", (PyCFunction)myrandint, METH_VARARGS, "Generate a random integer in the interval [0,range]"},
  //{NULL}
//};

static PyMethodDef WorkServer_methods[] = {
  //{"cinit",      (PyCFunction)WorkServer_cinit,       METH_NOARGS,  ""},
  {"task_done",    (PyCFunction)WorkServer_task_done,   METH_O,       ""},
  {"hot",          (PyCFunction)WorkServer_hot,         METH_VARARGS, ""},
  {NULL}
};


static PyMethodDef Protocol_methods[] = {
  {"connection_made", (PyCFunction)Protocol_connection_made, METH_O,       ""},
  {"connection_lost", (PyCFunction)Protocol_connection_lost, METH_VARARGS, ""},
  {"eof_received",    (PyCFunction)Protocol_eof_received,    METH_NOARGS,  ""},
  {"data_received",   (PyCFunction)Protocol_data_received,   METH_O,       ""},
  //{"reply",           (PyCFunction)Protocol_reply, METH_VARARGS, ""},
  {NULL}
};
static PyGetSetDef Protocol_getset[] = {
  {"transport", (getter)Protocol_get_transport, NULL, "", NULL},
  {NULL}
};

static PyTypeObject ProtocolType = {
  PyVarObject_HEAD_INIT(NULL, 0)
  "protocol.Protocol",      /* tp_name */
  sizeof(Protocol),          /* tp_basicsize */
  0,                         /* tp_itemsize */
  (destructor)Protocol_dealloc, /* tp_dealloc */
  0,                         /* tp_print */
  0,                         /* tp_getattr */
  0,                         /* tp_setattr */
  0,                         /* tp_reserved */
  0,                         /* tp_repr */
  0,                         /* tp_as_number */
  0,                         /* tp_as_sequence */
  0,                         /* tp_as_mapping */
  0,                         /* tp_hash  */
  0,                         /* tp_call */
  0,                         /* tp_str */
  0,                         /* tp_getattro */
  0,                         /* tp_setattro */
  0,                         /* tp_as_buffer */
  Py_TPFLAGS_DEFAULT,        /* tp_flags */
  "Protocol",                /* tp_doc */
  0,                         /* tp_traverse */
  0,                         /* tp_clear */
  0,                         /* tp_richcompare */
  0,                         /* tp_weaklistoffset */
  0,                         /* tp_iter */
  0,                         /* tp_iternext */
  Protocol_methods,          /* tp_methods */
  0,                         /* tp_members */
  Protocol_getset,           /* tp_getset */
  0,                         /* tp_base */
  0,                         /* tp_dict */
  0,                         /* tp_descr_get */
  0,                         /* tp_descr_set */
  0,                         /* tp_dictoffset */
  (initproc)Protocol_init,   /* tp_init */
  0,                         /* tp_alloc */
  Protocol_new,              /* tp_new */
};

static PyTypeObject WorkServerType = {
  PyVarObject_HEAD_INIT(NULL, 0)
  "WorkServer",              /* tp_name */
  sizeof(WorkServer),          /* tp_basicsize */
  0,                         /* tp_itemsize */
  (destructor)WorkServer_dealloc, /* tp_dealloc */
  0,                         /* tp_print */
  0,                         /* tp_getattr */
  0,                         /* tp_setattr */
  0,                         /* tp_reserved */
  0,                         /* tp_repr */
  0,                         /* tp_as_number */
  0,                         /* tp_as_sequence */
  0,                         /* tp_as_mapping */
  0,                         /* tp_hash  */
  0,                         /* tp_call */
  0,                         /* tp_str */
  0,                         /* tp_getattro */
  0,                         /* tp_setattro */
  0,                         /* tp_as_buffer */
  Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,        /* tp_flags */
  "WorkServer",                /* tp_doc */
  0,                         /* tp_traverse */
  0,                         /* tp_clear */
  0,                         /* tp_richcompare */
  0,                         /* tp_weaklistoffset */
  0,                         /* tp_iter */
  0,                         /* tp_iternext */
  WorkServer_methods,          /* tp_methods */
  0,                         /* tp_members */
  0,           /* tp_getset */
  0,                         /* tp_base */
  0,                         /* tp_dict */
  0,                         /* tp_descr_get */
  0,                         /* tp_descr_set */
  0,                         /* tp_dictoffset */
  (initproc)WorkServer_init,   /* tp_init */
  0,                         /* tp_alloc */
  WorkServer_new,              /* tp_new */
};

