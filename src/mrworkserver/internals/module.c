#include "module.h"
#include "protocol.h"


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

  m = PyModule_Create(&internals_module);
  if(!m) return NULL;

  Py_INCREF(&ProtocolType);
  PyModule_AddObject(m, "Protocol", (PyObject*)&ProtocolType);

  return m;
}

