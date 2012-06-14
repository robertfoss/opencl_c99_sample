#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "opencl.h"
#include "util.h"


void
setup_opencl(const char* cl_source_filename, const char* cl_source_main, cl_device_id* device_id,
                  cl_kernel* kernel, cl_context* context, cl_command_queue* queue)
{
        cl_int err;                            // error code returned from api calls

        cl_platform_id platform_id;				// compute device id
        cl_program program;						// compute program
        cl_device_id devices[MAX_RESOURCES];
        cl_platform_id platforms[MAX_RESOURCES];


        unsigned int best_platform = 0;
        unsigned int best_device = 0;
        print_devices(0);

        if(!get_best_device(&best_platform, &best_device)) {
                printf("No suitable device was found! Try using an OpenCL1.1 compatible device.\n");
                exit(1);
        }
        printf("Initiating platform-%d device-%d.\n", best_platform, best_device);



        // Platform
        err = clGetPlatformIDs(MAX_RESOURCES, platforms, NULL);
	ocl_error("Getting platform id", err);

        platform_id = platforms[best_platform];

        // Device
        err = clGetDeviceIDs(platform_id, CL_DEVICE_TYPE_ALL, sizeof(devices), devices, NULL); //NULL, ignore number returned devices.
	ocl_error("Getting device ids", err);

        *device_id = devices[best_device];

        // Context
        *context = clCreateContext(0, 1, device_id, NULL, NULL, &err);
	ocl_error("Creating context", err);

        // Command-queue
        *queue = clCreateCommandQueue(*context, *device_id, 0, &err);
	ocl_error("Creating command queue", err);


        // Read .cl source into memory
        int cl_source_len = 0;
        char* cl_source = file_contents(cl_source_filename, &cl_source_len);


        // Create thes compute program from the source buffer
        program = clCreateProgramWithSource(*context, 1, (const char **) &cl_source, NULL, &err);
	ocl_error("Failed to create compute program", err);


        // Build the program executable
        err = clBuildProgram(program, 0, NULL, NULL, NULL, NULL);
        if (err != CL_SUCCESS) {
                char* build_log;
                size_t log_size;
                // First call to know the proper size
                clGetProgramBuildInfo(program, *device_id, CL_PROGRAM_BUILD_LOG, 0, NULL, &log_size);
                build_log = malloc(sizeof(char)*(log_size+1));
                if(log_size > 0 && build_log != NULL) {
	                // Second call to get the log
	                clGetProgramBuildInfo(program, *device_id, CL_PROGRAM_BUILD_LOG, log_size, build_log, NULL);
	                build_log[log_size] = '\0';
	                printf("%s\n", build_log);
	                free(build_log);
                }

                exit(err);
        }


        // Create the compute kernel in the program we wish to run
        *kernel = clCreateKernel(program, cl_source_main, &err);
	ocl_error("Failed to create compute kernel", err);
}

/*
 * Free allocated OpenCL resources. Note: cl_mem objects are NOT free'd.
 */
void
destroy_opencl(cl_program* program, cl_kernel* kernel, cl_context* context, cl_command_queue* queue)
{
        // Shutdown and cleanup
        clReleaseProgram(*program);
        clReleaseKernel(*kernel);
        clReleaseCommandQueue(*queue);
        clReleaseContext(*context);
}


/*
 * Returns 0 if no suitable device was found.
 */
int
get_best_device(unsigned int *ret_platform, unsigned int *ret_device)
{
	cl_int err = CL_SUCCESS;
        unsigned long long best_score = 0;
	unsigned long long score;

        cl_platform_id platform[MAX_RESOURCES];
        cl_uint num_platform = MAX_RESOURCES;
        cl_device_id devices[MAX_RESOURCES];
        cl_uint num_devices;
        cl_uint numberOfCores;
        cl_long amountOfMemory;
        cl_uint clockFreq;
        cl_ulong maxAllocatableMem;

        err = clGetPlatformIDs(MAX_RESOURCES, platform, &num_platform);
	ocl_error("Getting platform ids", err);

        for(unsigned int i = 0; i < num_platform; i++) {
                err = clGetDeviceIDs(platform[i], CL_DEVICE_TYPE_ALL, sizeof(devices), devices, &num_devices);
		ocl_error("Getting device ids", err);

                for(unsigned int j = 0; j < num_devices; ++j) {
                        err  = clGetDeviceInfo(devices[j], CL_DEVICE_MAX_COMPUTE_UNITS, sizeof(numberOfCores), &numberOfCores, NULL);
                        err |= clGetDeviceInfo(devices[j], CL_DEVICE_GLOBAL_MEM_SIZE, sizeof(amountOfMemory), &amountOfMemory, NULL);
                        err |= clGetDeviceInfo(devices[j], CL_DEVICE_MAX_CLOCK_FREQUENCY, sizeof(clockFreq), &clockFreq, NULL);
                        err |= clGetDeviceInfo(devices[j], CL_DEVICE_MAX_MEM_ALLOC_SIZE, sizeof(maxAllocatableMem), &maxAllocatableMem, NULL);
			ocl_error("Unable to get device info", err);

			score = clockFreq*numberOfCores + amountOfMemory;
		        if(score>best_score) {
		                *ret_platform = i;
		                *ret_device = j;
		        }
                }

        }
        return ((best_score != 0) ? 0 : 1);
}


void
print_devices(int print_extensions)
{
	cl_int err = CL_SUCCESS;

        cl_platform_id platform[MAX_RESOURCES];
        cl_uint num_platform = MAX_RESOURCES;
        char vendor[1024];
        cl_device_id devices[MAX_RESOURCES];
        cl_uint num_devices;
        char deviceName[1024];
        cl_uint numberOfCores;
        cl_long amountOfMemory;
        cl_uint clockFreq;
        cl_ulong maxAllocatableMem;
        cl_ulong localMem;
        cl_bool available;
        char extensions[4096];
        size_t extensions_len = 0;

        err = clGetPlatformIDs(MAX_RESOURCES, platform, &num_platform);
	ocl_error("Getting platform ids", err);

        for(unsigned int i = 0; i < num_platform; i++) {
                err = clGetPlatformInfo (platform[i], CL_PLATFORM_VENDOR, sizeof(vendor), vendor, NULL);
		ocl_error("Getting platform info", err);

                err = clGetDeviceIDs(platform[i], CL_DEVICE_TYPE_ALL, sizeof(devices), devices, &num_devices);
		ocl_error("Getting device ids", err);
                for(unsigned int j = 0; j < num_devices; ++j) {
                        err  = clGetDeviceInfo(devices[j], CL_DEVICE_NAME, sizeof(deviceName), deviceName, NULL);
                        err |= clGetDeviceInfo(devices[j], CL_DEVICE_VENDOR, sizeof(vendor), vendor, NULL);
                        err |= clGetDeviceInfo(devices[j], CL_DEVICE_MAX_COMPUTE_UNITS, sizeof(numberOfCores), &numberOfCores, NULL);
                        err |= clGetDeviceInfo(devices[j], CL_DEVICE_GLOBAL_MEM_SIZE, sizeof(amountOfMemory), &amountOfMemory, NULL);
                        err |= clGetDeviceInfo(devices[j], CL_DEVICE_MAX_CLOCK_FREQUENCY, sizeof(clockFreq), &clockFreq, NULL);
                        err |= clGetDeviceInfo(devices[j], CL_DEVICE_MAX_MEM_ALLOC_SIZE, sizeof(maxAllocatableMem), &maxAllocatableMem, NULL);
                        err |= clGetDeviceInfo(devices[j], CL_DEVICE_LOCAL_MEM_SIZE, sizeof(localMem), &localMem, NULL);
                        err |= clGetDeviceInfo(devices[j], CL_DEVICE_AVAILABLE, sizeof(available), &available, NULL);
                        err |= clGetDeviceInfo(devices[j], CL_DEVICE_EXTENSIONS, sizeof(extensions), &extensions, &extensions_len);
			ocl_error("Unable to get device info", err);

                        printf("Platform-%d Device-%d\t%s - %s\tCores: %d\tMemory: %ldMB\tAvailable: %s\n",
                               i, j, vendor, deviceName, numberOfCores, (maxAllocatableMem/(1024*1024)), (available ? "Yes" : "No"));
                        char* an_extension;
                        if (extensions_len > 0 && print_extensions) {

                                printf("\t\tExtensions: \t");
                                an_extension = strtok(extensions, " ");
                                while (an_extension != NULL) {
                                        printf("%s\n\t\t\t\t", an_extension);
                                        an_extension = strtok(NULL, " ");
                                }
                        }
                        printf("\n");
                }

        }
}
