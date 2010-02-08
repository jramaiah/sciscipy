#include "Python.h"
#include "util.h"
#include <stdarg.h>

/** Return the scilab type 
 *
 * Returns the scilab type of the scilab variable name
 * 
 * */
int read_sci_type(char *name)
{
	char job[BUFSIZE] ;
	int m, n, lp ;
	double type[1] ;
	
	snprintf(job, BUFSIZE, "_tmp_value_ = type(%s);", name) ;
	SendScilabJob(job) ;
	GetMatrixptr("_tmp_value_", &m, &n, &lp) ; 
	
	if (m*n != 1)
		return -1 ;
		
	ReadMatrix4py("_tmp_value_", &m, &n, &type[0]);

	return (int) type[0] ;
} ;

/** Check if a matrix is real or not
 *
 *  Returns 1 if the matrix is real
 *
 */
int is_real(char *name)
{
    int ret, rowA_, colA_ ;
	char job[BUFSIZE] ;

	snprintf(job, BUFSIZE, "_tmp_value_ = isreal(%s);", name) ;
    
	SendScilabJob("_tmp_value_ = 0;") ;
	SendScilabJob(job) ;
    C2F(creadbmat)("_tmp_value_", &rowA_, &colA_, &ret, \
                                        strlen("_tmp_value_"));
    return ret ;
    
}

void sci_debug(const char *format, ...)
{
#if SCIDEBUG
    va_list argp ;
    va_start(argp, format) ;
    vprintf(format, argp) ;
    va_end(argp) ;
#endif
}

void sci_error(const char *format, ...)
{
    va_list argp ;
    va_start(argp, format) ;
    vprintf(format, argp) ;
    va_end(argp) ;
}

/** Put a Python object in a list
 */
PyObject* create_list(PyObject *obj)
{
    PyObject* new_list ;

    new_list = PyList_New(1) ;
	PyList_SET_ITEM(new_list, 0, obj) ;	
    return new_list ;
} ;


