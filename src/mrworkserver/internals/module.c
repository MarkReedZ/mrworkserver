#include "module.h"
#include "protocol.h"
#include "ws.h"


static PyModuleDef internals_module = {
  PyModuleDef_HEAD_INIT,
  "internals",
  "C internals",
  -1,
  NULL, //mod_methods,
  NULL, NULL, NULL, NULL
};

PyMODINIT_FUNC
PyInit_internals(void)
{

  PyObject* m = NULL;
  if (PyType_Ready(&ProtocolType) < 0) return NULL;
  if (PyType_Ready(&WorkServerType) < 0) return NULL;

  m = PyModule_Create(&internals_module);
  if(!m) return NULL;

  Py_INCREF(&ProtocolType);
  PyModule_AddObject(m, "Protocol", (PyObject*)&ProtocolType);
  Py_INCREF(&WorkServerType);
  PyModule_AddObject(m, "WorkServer", (PyObject*)&WorkServerType);

  return m;
}

