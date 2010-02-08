#ifndef STACK_C_4PY
#define STACK_C_4PY

/*
The macros below are defined in stack-c.h 
but they have to be modified. They must 
set the appropriate python error 
code and then return NULL
*/

#define ReadMatrix4py(ct,mx,nx,w)  if (! C2F(creadmat)(ct,mx,nx,w,(unsigned long)strlen(ct) )) \
{	PyErr_SetString(PyExc_TypeError, "Error in readmatrix") ; return 0; }
#define WriteMatrix4py(ct,mx,nx,w)  if (! C2F(cwritemat)(ct,mx,nx,w,strlen(ct) )) \
{	PyErr_SetString(PyExc_TypeError, "Error in Writematrix") ; return 0; }


#endif
