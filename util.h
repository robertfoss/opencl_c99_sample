#ifndef _UTIL_H_INCLUDED
#define _UTIL_H_INCLUDED

#include "CL/cl.h"

char *file_contents(const char *filename, int *length);

const char* ocl_error_string(cl_int error);


#endif
