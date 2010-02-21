/*
    This file is part of Sciscipy.

    Sciscipy is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    Sciscipy is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Foobar.  If not, see <http://www.gnu.org/licenses/>.
    
    Copyright (c) 2009, Vincent Guffens.
*/
#include <Python.h>

#include "sciconv_read.h"
#include "sciconv_write.h"
#include "util.h"


extern int StartScilab(char *SCIpath, char *ScilabStartup,int *Stacksize);
extern int SendScilabJob(char *job); 


static int Initialize(void)  
{
	int res ;
#ifdef _MSC_VER
    res = StartScilab(NULL, NULL, NULL) == FALSE
#else
	if (getenv("SCI") != NULL)
    	res = StartScilab(getenv("SCI"), NULL, NULL) ;
	else
		{
			char sci[sci_max_len] ;
			res = StartScilab(get_SCI(sci), NULL, NULL) ;
		}
#endif

	if (res == FALSE)
        return -1;
	else
		return 0 ;
}

/* Python interface */

static PyObject *
sciscipy_read (PyObject *self, PyObject *args)
{
	char *name ;
	char er_msg[BUFSIZE] ;

	struct sciconv_read_struct *conv = sciconv_read_list ;
	int var_type ;
	

	if ( !PyArg_ParseTuple (args, "s", &name) )
		return NULL ;
		
	var_type = read_sci_type(name) ;
	
	while (conv)
	{
		if (conv->scitype == var_type)
			return conv->conv_func(name) ;
		conv = conv->next ;
	}

	snprintf(er_msg, BUFSIZE, "Type %i not supported", var_type) ;
	PyErr_SetString(PyExc_TypeError, er_msg) ;
	return NULL ;


} ;

static PyObject *
sciscipy_write (PyObject *self, PyObject *args)
{
	char *name ;
	PyObject *obj ;
	int er ;
	struct sciconv_write_struct *conv ; 

	if (!PyArg_ParseTuple (args, "sO", &name, &obj))
		return NULL ;

	Py_INCREF(Py_None) ;

	conv = sciconv_write_list ;

	while (conv)
	{
		if (conv->test_func(obj) > 0)
		{
			er = conv->conv_func(name, obj) ;
			if (er > 0) // success
				return Py_None ;
		}
	
		conv = conv->next ;
	}

	return Py_None ;	  		
} ;

static PyObject *
sciscipy_eval (PyObject *self, PyObject *args)
{

	char *name ;
	if ( !PyArg_ParseTuple (args, "s", &name) )
		return NULL ;

	SendScilabJob(name);	
	while( ScilabHaveAGraph() )
	{
		ScilabDoOneEvent() ;
	}
	
	Py_INCREF(Py_None);
	return Py_None;

} ;

static void numpy_init(void)
{
#if NUMPY == 1
    import_array() ;
#endif
}

static PyMethodDef SciscipyMethods[] = {
    	{"eval",  sciscipy_eval, METH_VARARGS, "eval (cmd) : Execute the Scilab command cmd."},
		{"read", sciscipy_read, METH_VARARGS, "read (sci_name): read a Scilab variable."},
    	{"write", sciscipy_write, METH_VARARGS, "write (sci_name, py_var): Write a Scilab variable."},
    	{NULL, NULL, 0, NULL}        /* Sentinel */
} ;

#ifdef PYTHON3
static struct PyModuleDef sciscipy = {
	PyModuleDef_HEAD_INIT,
   	"sciscipy",   	/* name of module */
   	NULL, 		    /* module documentation, may be NULL */
   	-1,       	    /* size of per-interpreter state of the module,
                     	   or -1 if the module keeps state in global variables. */
   	SciscipyMethods
} ;
#endif

PyMODINIT_FUNC
#ifdef PYTHON3
PyInit_sciscipy(void)
#else
initsciscipy(void)
#endif
{

	int er = Initialize() ;
	if (er != 0)
	{
		PyErr_SetString(PyExc_TypeError, "Can not initialize scilab") ;

#ifdef PYTHON3
		return NULL ;
#endif

	}
	else
	{
        numpy_init() ;
		sciconv_read_init() ;
		sciconv_write_init() ;

#ifdef PYTHON3
		return PyModule_Create(&sciscipy) ;
#else
        Py_InitModule("sciscipy", SciscipyMethods) ;
#endif

	}
} ;
