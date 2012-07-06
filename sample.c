#include <stdio.h>
#include <math.h>

#include "CL/cl.h"

#include "opencl.h"
#include "util.h"

#define DATA_SIZE (1024)

int main()
{
        cl_int err;                         // error code returned from api calls
        size_t global;                      // global domain size for our calculation
        size_t local;                       // local domain size for our calculation
        cl_device_id device_id;             // device id running computation
        cl_context context;                 // compute context
        cl_command_queue queue;             // compute command queue
        cl_kernel kernel;                   // compute kernel

        cl_mem input;                       // device memory used for the input array
        cl_mem output;                      // device memory used for the output array

        float data[DATA_SIZE];              // original data set given to device
        float results[DATA_SIZE];           // results returned from device
        unsigned int correct;               // number of correct results returned


        // Fill our data set with random values
        unsigned int i = 0;
		unsigned int count = DATA_SIZE;
		for(i = 0; i < count; i++)
		    data[i] = (float) (rand() / (float)RAND_MAX);

        setup_opencl("square.cl", "square", &device_id, &kernel, &context, &queue);

        // Get the maximum work group size for executing the kernel on the device
        err = clGetKernelWorkGroupInfo(kernel, device_id, CL_KERNEL_WORK_GROUP_SIZE, sizeof(local), &local, NULL);
        if (err != CL_SUCCESS) {
                printf("Error: Failed to retrieve kernel work group info: %s\n", ocl_error_string(err));
                exit(1);
        }

        // Create the input and output arrays in device memory for our calculation
        input = clCreateBuffer(context,  CL_MEM_READ_ONLY,  sizeof(float) * count, NULL, &err);
        if (err != CL_SUCCESS) {
                printf("Error: Failed to allocate device READ memory: %s\n", ocl_error_string(err));
                exit(1);
        }
        output = clCreateBuffer(context, CL_MEM_WRITE_ONLY, sizeof(float) * count, NULL, &err);
        if (err != CL_SUCCESS) {
                printf("Error: Failed to allocate device WRITE memory: %s\n", ocl_error_string(err));
                exit(1);
        }

	// Write our data set into the input array in device memory 
	err = clEnqueueWriteBuffer(queue, input, CL_TRUE, 0, sizeof(float) * count, data, 0, NULL, NULL);
	if (err != CL_SUCCESS) {
		printf("Error: Failed to write to source array: %s\n", ocl_error_string(err));
		exit(1);
	}

        // Set the arguments to our compute kernel
        err  = clSetKernelArg(kernel, 0, sizeof(cl_mem), &input);
        err |= clSetKernelArg(kernel, 1, sizeof(cl_mem), &output);
        err |= clSetKernelArg(kernel, 2, sizeof(unsigned int), &count);
        if (err != CL_SUCCESS) {
                printf("Error: Failed to set kernel arguments: %s\n", ocl_error_string(err));
                exit(1);
        }

        // Execute the kernel over the entire range of our 1d input data set
        // using the maximum number of work group items for this device
        global = count;
        err = clEnqueueNDRangeKernel(queue, kernel, 1, NULL, &global, &local, 0, NULL, NULL);
        if (err != CL_SUCCESS) {
                printf("Error: Failed to execute kernel: %s\n", ocl_error_string(err));
                return EXIT_FAILURE;
        }

        // Wait for the command commands to get serviced before reading back results
        clFinish(queue);

        // Read back the results from the device to verify the output
        err = clEnqueueReadBuffer(queue, output, CL_TRUE, 0, sizeof(float) * count, results, 0, NULL, NULL );
        if (err != CL_SUCCESS) {
                printf("Error: Failed to read output array: %s\n", ocl_error_string(err));
                exit(1);
        }

        // Validate our results
        correct = 0;
        for(i = 0; i < count; i++) {
                if(results[i] == data[i] * data[i])
                        correct++;
                else
                        printf("[%d]: %f^2 == %f, != %f\n", i, data[i], data[i] * data[i], results[i]);
        }

        // Print a brief summary detailing the results
        printf("Computed '%d/%d' correct values!\n", correct, count);
}
