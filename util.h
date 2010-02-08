#ifndef _UTIL_H
#define _UTIL_H

#undef HAVE_STRERROR

#include "stack-c-4py.h"
#include "CallScilab.h"
#include "stack-c.h"

#define BUFSIZE 1024

/*
 * This has to be defined when a numpy extension
 * is split accross multiple files
 */
#define PY_ARRAY_UNIQUE_SYMBOL UNIQUE

typedef double complex[2] ;

int read_sci_type(char *name) ;
int is_real(char *name) ;

void sci_debug(const char *format, ...) ;
void sci_error(const char *format, ...) ;

PyObject* create_list(PyObject *obj) ;

#endif
