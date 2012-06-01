#ifndef _OPENCL_H_INCLUDED
#define _OPENCL_H_INCLUDED

#include "CL/cl.h"

#define MAX_RESOURCES (32)

void setup_opencl(const char* cl_source_filename, const char* cl_source_main, cl_device_id* device_id,
				 cl_kernel* kernel, cl_context* context, cl_command_queue* queue);
void destroy_opencl(cl_program* program, cl_kernel* kernel, cl_context* context, cl_command_queue* queue);
void print_devices();
int get_best_device(unsigned int *ret_platform, unsigned int *ret_device);

#endif
