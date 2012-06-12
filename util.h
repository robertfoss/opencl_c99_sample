#ifndef UTIL_H
#define UTIL_H

#include "CL/cl.h"

char *file_contents(const char *filename, int *length);
const char* ocl_error_string(cl_int error);
void ocl_error(const char *descr, cl_int err);


#endif //UTIL_H_OPENSSL