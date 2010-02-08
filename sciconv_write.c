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

#include "sciconv_write.h"
#include "util.h"

struct sciconv_write_struct *sciconv_write_list = NULL ;

/** list of string
 *
 * WARNING: This will not work with Python 3.0
 */
static int write_listofstring(char *name, PyObject *obj)
{
    int i, n ;
    int tot_size = 0;
    PyObject *item ;
    char *str_item, *buff, *ptr ;
    
    if (!PyList_Check(obj))
        obj = create_list(obj) ;
    
    n = PyList_Size(obj) ; 
    
    for (i = 0; i < n; ++i)
    {
        item = PyList_GetItem(obj, i) ;
        str_item = PyString_AsString(item) ;
        tot_size += strlen(str_item) ;
    }

    buff = (char *) malloc((strlen(name) + tot_size + 3*n + 3)*sizeof(char)) ;
    ptr = buff ;
    strcpy(ptr, name) ;
    ptr += strlen(name) ;

    strcpy(ptr, "=[") ;
    ptr += 2 ;
    
    for (i = 0; i < n; ++i)
    {
        item = PyList_GetItem(obj, i) ;
        str_item = PyString_AsString(item) ;
        strcpy(ptr, "'") ;
        ptr++ ;
        strcpy(ptr, str_item) ;
        ptr += strlen(str_item) ;
        strcpy(ptr, "'") ;
        ptr++ ;
        if (i != n-1)
        {
            strcpy(ptr, ",") ;
            ptr++ ;
        }
    }

    strcpy(ptr, "]") ;
	SendScilabJob(buff);	

    return 1 ;
}

/**
 */
static int test_listofstring(PyObject *obj)
{
	PyObject *item ;
	int n ;

    if (PyString_Check(obj))
    {	
	    sci_debug("[sciconv_write] Match for list of string\n") ;
        return 1 ;
    }

	if (PyList_Check(obj))
	{
		n = PyList_Size(obj) ;
		if (n == 0)
			return -1 ;

		item = PyList_GetItem(obj, 0) ;

	    if (PyString_Check(item))
		{
			sci_debug("[sciconv_write] Match for list of string\n") ;
			return 1 ;
		}
	}
	
	return -1 ;
}

/** 
 * @param name: the name of the scilab variable we want to create
 * @return: negative on failure 
*/
static int write_listoflist(char *name, PyObject *obj)
{
	int i, j, m, n ;
	double *new_vec ;
	PyObject *item, *element ;
    int is_complex_found = 0 ;

	m = PyList_Size(obj) ;
	item = PyList_GetItem(obj, 0) ;
	n = PyList_Size(item) ;

	new_vec = (double*) calloc(sizeof(double)*2*m*n, 1) ;
	
	if (!new_vec)
		return -1 ;

    for (i = 0; i < m; i++)
	{	
        item = PyList_GetItem(obj, i) ;
        
	    for (j = 0; j < n; j++)
		{
            element = PyList_GetItem(item, j) ;

            if (PyComplex_Check(element))
            {
			    is_complex_found = 1 ;
                new_vec[j*m+i] = PyComplex_RealAsDouble(element) ;
                new_vec[m*n + j*m+i] = PyComplex_ImagAsDouble(element) ;
                continue ;
            }

            if (PyFloat_Check(element) || PyLong_Check(element) || PyInt_Check(element))
            {
			    new_vec[j*m+i] = PyFloat_AsDouble(element) ;
                continue ;
            }

            sci_debug("[write_listoflist] something found" \
                                "that is not real or complex") ;
            free(new_vec) ;
            return -1 ;
		}
    }

    if (is_complex_found)
        C2F(cwritecmat)(name, &m, &n, new_vec, strlen(name)) ;
    else
	    WriteMatrix4py(name, &m, &n, new_vec) ;
	
    free(new_vec) ;
	
	return 1 ;
}

static int test_listoflist(PyObject *obj)
{
	int n ;
	PyObject *item, *el ;


	if (!PyList_Check(obj)) 
		return -1 ;
	n = PyList_Size(obj) ;
	if (n ==0)
		return -1 ;
	
	item = PyList_GetItem(obj, 0) ;
	
	if (!PyList_Check(item) || PyList_Size(item)==0)  
		return -1 ;

	el = PyList_GetItem(item, 0) ;

    /* Only the first element is checked, the converter
        will fail later on if all items are not real or 
        complex (This is for performance)
    */
	if (PyFloat_Check(el) || PyLong_Check(el) || PyComplex_Check(el) || PyInt_Check(el))
	{
		sci_debug("[sciconv_write] Match for list of list\n") ;
		return 1 ;
	}
	else
		return -1 ;
}

static int write_listofdouble(char *name, PyObject *obj)
{
	int i, m ;
    int n = 1 ;
	double *new_vec ;
	PyObject *element ;
    int is_complex_found = 0 ;
    
    if (!PyList_Check(obj))
        obj = create_list(obj) ;

	m = PyList_Size(obj) ;

	new_vec = (double*) calloc(sizeof(double)*2*m, 1) ;
	
	if (!new_vec)
		return -1 ;

    for (i = 0; i < m; i++)
	{	
        element = PyList_GetItem(obj, i) ;

        if (PyComplex_Check(element))
        {
	        is_complex_found = 1 ;
            new_vec[i] = PyComplex_RealAsDouble(element) ;
            new_vec[m + i] = PyComplex_ImagAsDouble(element) ;
            continue ;
        }

        if (PyFloat_Check(element) || PyLong_Check(element) || PyInt_Check(element))
        {
	        new_vec[i] = PyFloat_AsDouble(element) ;
            continue ;
        }

        sci_debug("[write_listofdouble] something found" \
                                "that is not real or complex") ;
        free(new_vec) ;
        return -1 ;
    }

    if (is_complex_found)
        C2F(cwritecmat)(name, &m, &n, new_vec, strlen(name)) ;
    else
	    WriteMatrix4py(name, &m, &n, new_vec) ;
	
    free(new_vec) ;
	
	return 1 ;

}

static int test_listofdouble(PyObject *obj)
{
	PyObject *item ;
	int n ;

    if (PyFloat_Check(obj) || PyLong_Check(obj) || PyComplex_Check(obj) || PyInt_Check(obj))
    {
        sci_debug("[sciconv_write] Match for list of double\n") ;
        return 1 ;
    }

	if (PyList_Check(obj))
	{
		n = PyList_Size(obj) ;
		if (n == 0)
			return -1 ;

		item = PyList_GetItem(obj, 0) ;

	    if (PyFloat_Check(item) || PyLong_Check(item) || PyComplex_Check(item) || PyInt_Check(item))
		{
			sci_debug("[sciconv_write] Match for list of double\n") ;
			return 1 ;
		}
	}
	
	return -1 ;
}

#if NUMPY == 1
static int write_numpy(char *name, PyObject *obj)
{

    PyArrayObject * array = (PyArrayObject *) obj ;
    double * data ;    
    int i, j, m, n ;
    complex * item ;

    // TODO: add support for 1D array

    if (array->nd != 1 && array->nd != 2)
    {
        sci_debug("[sciconv_write] Only 1D and 2D array are supported\n") ;
        return -1 ;
    }        
   
    if (array->nd == 1)
    {
        m = array->dimensions[0] ;
        n = 1 ; 
    }
    else
    { 
        m = array->dimensions[0] ;
        n = array->dimensions[1] ;
    }

    if ((array->descr->type_num == PyArray_DOUBLE) || \
        (array->descr->type_num == PyArray_INT) ) 
    {
        
        data = (double*) malloc(m*n*sizeof(double)) ;
        
        if (!data)
        {
            sci_error("[sciconv_write] out of memory\n") ;
            return -1 ;
        }

        for (i=0; i < m ; i++)
            for (j=0; j < n ; j++)
                data[j*m + i] = *(double*)(array->data + i*array->strides[0] + \
                                            j*array->strides[1]) ;
	
        WriteMatrix4py(name, &m, &n, data) ;
        free(data) ;

        return 1 ;
    } 
    
    if (array->descr->type_num == PyArray_CDOUBLE) 
    {
        data = (double*) malloc(2*m*n*sizeof(double)) ;

        if (!data)
        {
            sci_error("[sciconv_write] out of memory\n") ;
            return -1 ;
        }

        for (i=0; i < m ; i++)
            for (j=0; j < n ; j++)
            {
    
                item = (complex*)(array->data + i*array->strides[0] + \
                                            j*array->strides[1]) ;
                data[j*m + i] = (*item)[0] ;
                data[m*n + j*m + i] = (*item)[1] ;	
        }

        C2F(cwritecmat)(name, &m, &n, data, strlen(name)) ;
    
        free(data) ;
        return 1 ;
    }

    sci_debug("[sciconv_write] Array type not supported\n") ;
    return -1 ;
}

static int test_numpy(PyObject *obj)
{
    if (PyArray_Check(obj))
    {
        sci_debug("[sciconv_write] Match for numpy array\n") ;
        return 1 ;
    }
    else
        return -1 ;
}
#endif

/**
 * Add a new converter to the list
 * @param new_type: A scilab type number
 * @param func: The converter function
*/
static void sciconv_write_add(int (*test_func)(PyObject*), int(*func)(char*, PyObject *))
{
	struct sciconv_write_struct *new_conv = \
		(struct sciconv_write_struct*) malloc(sizeof(struct sciconv_write_struct)) ;

	new_conv->test_func = test_func ;
	new_conv->conv_func = func ;

	if (sciconv_write_list == NULL)
	{
		sciconv_write_list = new_conv ;
		new_conv->next = NULL ;
		return ;
	}
	
	new_conv->next = sciconv_write_list->next ;
	sciconv_write_list->next = new_conv ; 
}

/**
 * Initialization
 * Add all the known converter to the list
*/
void sciconv_write_init(void)
{
	// The one added first is the one tested first
	// so the order can be important
#if NUMPY == 1 
	sciconv_write_add(test_numpy, write_numpy) ;
#endif
	sciconv_write_add(test_listoflist, write_listoflist) ;
	sciconv_write_add(test_listofdouble, write_listofdouble) ;
	sciconv_write_add(test_listofstring, write_listofstring) ;
}

