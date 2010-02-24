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

/* 
  Types defined in scilab:

  1  : real or complex constant matrix.
  2  : polynomial matrix.
  4  : boolean matrix.
  5  : sparse matrix.
  6  : sparse boolean matrix.
  8  : matrix of integers stored on 1 2 or 4 bytes
  9  : matrix of graphic handles
  10 : matrix of character strings.
  11 : un-compiled function.
  13 : compiled function.
  14 : function library.
  15 : list.
  16 : typed list (tlist)
  17 : mlist
  128 : pointer


 NOTE: the way numpy vectors and matrices are created leads
        to a memory leak.
*/

#include "sciconv_read.h"
#include "util.h"

struct sciconv_read_struct *sciconv_read_list = NULL;

#if NUMPY == 1
static PyObject * create_numpyarray(double *cxtmp, int m, int n)
{
    PyObject * array ;
    npy_intp dim[2], mn;

    if (m == 1 || n == 1)
    {
        mn = m*n ;

	array = PyArray_NewFromDescr(&PyArray_Type, \
				     PyArray_DescrFromType(PyArray_DOUBLE),\
				     1,\
				     &mn,\
				     NULL,\
				     (void *) cxtmp,\
				     NPY_OWNDATA & NPY_FARRAY,\
				     NULL			     
				    ) ;

        return array ;
    }
    
    dim[0] = n ;
    dim[1] = m ;

	array = PyArray_NewFromDescr(&PyArray_Type, \
      			     PyArray_DescrFromType(PyArray_DOUBLE),\
      			     2,\
      			     dim,\
      			     NULL,\
      			     (void *) cxtmp,\
      			     NPY_OWNDATA & NPY_FARRAY,\
      			     NULL			     
      			    ) ;


    return PyArray_Transpose((PyArrayObject*) array, NULL) ;
}

static PyObject * create_cnumpyarray(double *cxtmp, int m, int n)
{
    PyObject * array ;
    int i, j  ;
    npy_intp dim[2], mn ;
    complex * cxtmp_transpose ;

 	cxtmp_transpose = (complex*) malloc(2*m*n*sizeof(complex));

    dim[0] = m ;
    dim[1] = n ;

 	if (!cxtmp_transpose)
 	{
 		PyErr_SetString(PyExc_MemoryError, "out of memory") ;
 		return NULL ;
 	}

    for (i=0; i<m; ++i)
        for (j=0; j<n; ++j)
        {
            cxtmp_transpose[i*n+j][0] = cxtmp[j*m+i] ;
            cxtmp_transpose[i*n+j][1] = cxtmp[m*n + j*m+i] ;
        }

    if (m == 1 || n == 1)
    {
        mn = m*n ;


		array = PyArray_NewFromDescr(&PyArray_Type, \
				     PyArray_DescrFromType(PyArray_CDOUBLE),\
				     1,\
				     &mn,\
				     NULL,\
				     (void *) cxtmp_transpose,\
				     NPY_OWNDATA & NPY_CARRAY,\
				     NULL			     
				    ) ;
    }
    else

		array = PyArray_NewFromDescr(&PyArray_Type, \
      			     PyArray_DescrFromType(PyArray_CDOUBLE),\
      			     2,\
      			     dim,\
      			     NULL,\
      			     (void *) cxtmp_transpose,\
      			     NPY_OWNDATA & NPY_CARRAY,\
      			     NULL			     
      			    ) ;

    free(cxtmp) ;
    return array ;
}
#else

static PyObject * create_listmatrix(double *cxtmp, int m, int n, int is_complex)
{
	int i,j ;
	PyObject *new_list, *new_line ;
    Py_complex new_complex ;
	

	if (m==1 || n==1)
	{
		new_list = PyList_New(m*n) ;
	
		for (i = 0 ; i < m*n ; i++)
		{
            if (is_complex)
            {
                new_complex.real = cxtmp[i] ;
                new_complex.imag = cxtmp[m*n + i] ;
			    PyList_SET_ITEM(new_list, i, Py_BuildValue("D", &new_complex)) ;	
            }
            else
			    PyList_SET_ITEM(new_list, i, Py_BuildValue("d", cxtmp[i])) ;	
		}
	}	
	else
	{
		new_list = PyList_New(m) ;
		for (i = 0 ; i < m ; i++)
		{
			new_line = PyList_New(n) ;
			for (j = 0 ; j < n ; j++)
                if (is_complex)
                {
                    new_complex.real = cxtmp[j*m + i] ;
                    new_complex.imag = cxtmp[m*n + j*m + i] ;
			        PyList_SET_ITEM(new_line, j, Py_BuildValue("D", &new_complex)) ;	
                }
                else
			        PyList_SET_ITEM(new_line, j, Py_BuildValue("d", cxtmp[j*m + i])) ;	

			PyList_SET_ITEM(new_list, i, Py_BuildValue("O", new_line)) ;
		}
	}

	free(cxtmp) ;
	return Py_BuildValue ("O", new_list) ;
};

#endif

/** 
 * Type 1 : real or complex constant matrix. 
 * @param name: the name of the scilab variable we want to read
 * @return: A list of list 
*/
static PyObject * read_matrix(char *name)
{	

    int m, n, lp ;
    int mult ;
 	double *cxtmp ;
 	PyObject * matrix ;
    
    if (is_real(name)) 
    {
        mult = 1 ;
 	    GetMatrixptr(name, &m, &n, &lp) ; 
    } 
    else
    {
        mult = 2 ;
        C2F(cmatcptr)(name, &m, &n, &lp, strlen(name));
    }

 	cxtmp = (double*) malloc(mult*m*n*sizeof(double));
 	
 	if (!cxtmp)
 	{
 		PyErr_SetString(PyExc_MemoryError, "out of memory") ;
 		return NULL ;
 	}
   
    if (mult == 1) 
    {
	    ReadMatrix4py(name, &m, &n, cxtmp) ;
#if NUMPY == 1
        matrix = create_numpyarray(cxtmp, m, n) ;
#else
        matrix = create_listmatrix(cxtmp, m, n, 0) ;
#endif

    }
    else
    {
        C2F(creadcmat)(name, &m, &n, cxtmp, strlen(name)) ;
#if NUMPY == 1
        matrix = create_cnumpyarray(cxtmp, m, n) ;
#else
        matrix = create_listmatrix(cxtmp, m, n, 1) ;
#endif
    }

    return matrix ;
}


/** 
 * Type 10 : Matrix of string. 
 * @param name: the name of the scilab variable we want to read
 * @return: A list of string
*/
static PyObject * read_string(char *name)
{	

    int m = 0, n = 0 ;
    int i = 0 ;
    int x = 0, y = 0 ;

    char ** variable_from_scilab = NULL ;
    int * lengthOfB = GetLengthStringMatrixByName(name, &m, &n) ;

    PyObject *new_list ;

    variable_from_scilab = (char **) malloc(sizeof(char*)* (m*n)) ;

    for (i = 0; i < m * n; i++)
    {
        variable_from_scilab[i] = (char*) malloc(sizeof(char)*(lengthOfB[i])) ;
    }

    i = 0;
	new_list = PyList_New(m*n) ;
    
    for (x = 1; x <= m; x++)
    {
        for (y = 1; y <= n; y++)
        {
            int nlr = lengthOfB[i] ;
            char *tmpStr = NULL ;
            tmpStr = variable_from_scilab[i] ;
            C2F(creadchains)(name, &x, &y, &nlr, tmpStr, \
                                (unsigned long) strlen(name), \
                                (unsigned long) strlen(tmpStr)) ;
			PyList_SET_ITEM(new_list, i, Py_BuildValue("s", tmpStr)) ;
            free(tmpStr) ;	
            i++;
        }
    }

    return new_list ;
}


/**
 * Add a new converter to the list
 * @param new_type: A scilab type number
 * @param func: The converter function
*/
static void sciconv_read_add(int new_type, PyObject*(*func)(char*))
{
	struct sciconv_read_struct *new_conv = \
		(struct sciconv_read_struct*) malloc(sizeof(struct sciconv_read_struct)) ;

	new_conv->scitype = new_type ;
	new_conv->conv_func = func ;

	if (sciconv_read_list == NULL)
	{
		sciconv_read_list = new_conv ;
		new_conv->next = NULL ;
		return ;
	}
	
	new_conv->next = sciconv_read_list->next ;
	sciconv_read_list->next = new_conv ; 
}

/**
 * Initialization
 * Add all the known converters to the list
*/
void sciconv_read_init(void)
{
	// Most used should come last
	sciconv_read_add(10, read_string) ;
	sciconv_read_add(1, read_matrix) ;

}