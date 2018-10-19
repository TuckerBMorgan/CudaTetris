
#include "cuda_runtime.h"
#include "device_launch_parameters.h"
/*
#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>
*/
#include <stdio.h>
#include <stdint.h>

cudaError_t addWithCuda(int *c, const int *a, const int *b, unsigned int size);

//__global__ lua_State* luaL_newstate();


__global__ void addKernel(int *c, const int *a, const int *b)
{
	//lua_State* L = luaL_newstate();
    int i = threadIdx.x;
    c[i] = a[i] + b[i];
}

void fill_frame(uint8_t* frame) {
}


int try_stuff(int* check)
{
    const int arraySize = 5;
    const int a[arraySize] = { 1, 2, 3, 4, 5 };
    const int b[arraySize] = { 11, 20, 30, 40, 50 };
    int c[arraySize] = { 0 };
//	lua_State *L = 
	

    // Add vectors in parallel.
    cudaError_t cudaStatus = addWithCuda(c, a, b, arraySize);
    if (cudaStatus != cudaSuccess) {
        fprintf(stderr, "addWithCuda failed!");
        return 1;
    }
	printf("hllogddffge world\n");

    printf("{1,2,3,4,5} + {10,20,30,40,50} = {%d,   %d,%d,%d,%d}\n",
        c[0], c[1], c[2], c[3], c[4]);

	check[0] = c[0];
	check[1] = c[1];
	check[2] = c[2];
	printf("hlloe world\n");

    // cudaDeviceReset must be called before exiting in order for profiling and
    // tracing tools such as Nsight and Visual Profiler to show complete traces.
    cudaStatus = cudaDeviceReset();
    if (cudaStatus != cudaSuccess) {
        fprintf(stderr, "cudaDeviceReset failed!");
        return 1;
    }

    return 0;
}

__global__ void color_in_array(uint8_t* resultFrame, size_t screen_height) {
	int x = threadIdx.x;
	int y = threadIdx.y;

	resultFrame[x + y * screen_height] = x % 255;
}

cudaError_t render_screen(uint8_t* resultFrame, size_t width, size_t height) {
	cudaError_t result;
	uint8_t* screenBuffer = 0;
	
	size_t value = sizeof(uint8_t) * width * height;
	result = cudaMalloc((void**)&screenBuffer, value);
	if (result != cudaSuccess) {
		fprintf(stderr, "Error with the backbuffer malloc");
		cudaFree(screenBuffer);//attempt to free it atleast 
		return result;
	}

	color_in_array <<<1, width * height>>> (screenBuffer, height);
	result = cudaGetLastError();

	if (result != cudaSuccess) {
		fprintf(stderr, "Error with the backbuffer malloc");
		cudaFree(screenBuffer);//attempt to free it atleast 
		return result;
	}
	
	result = cudaMemcpy(resultFrame, screenBuffer, width * height * sizeof(uint8_t), cudaMemcpyDeviceToHost);

	cudaFree(screenBuffer);
	return result;
	
}

// Helper function for using CUDA to add vectors in parallel.
cudaError_t addWithCuda(int *c, const int *a, const int *b, unsigned int size)
{
    int *dev_a = 0;
    int *dev_b = 0;
    int *dev_c = 0;
    cudaError_t cudaStatus;

    // Choose which GPU to run on, change this on a multi-GPU system.
    cudaStatus = cudaSetDevice(0);
    if (cudaStatus != cudaSuccess) {
        fprintf(stderr, "cudaSetDevice failed!  Do you have a CUDA-capable GPU installed?");
        goto Error;
    }

    // Allocate GPU buffers for three vectors (two input, one output)    .
    cudaStatus = cudaMalloc((void**)&dev_c, size * sizeof(int));
    if (cudaStatus != cudaSuccess) {
        fprintf(stderr, "cudaMalloc failed!");
        goto Error;
    }

    cudaStatus = cudaMalloc((void**)&dev_a, size * sizeof(int));
    if (cudaStatus != cudaSuccess) {
        fprintf(stderr, "cudaMalloc failed!");
        goto Error;
    }

    cudaStatus = cudaMalloc((void**)&dev_b, size * sizeof(int));
    if (cudaStatus != cudaSuccess) {
        fprintf(stderr, "cudaMalloc failed!");
        goto Error;
    }

    // Copy input vectors from host memory to GPU buffers.
    cudaStatus = cudaMemcpy(dev_a, a, size * sizeof(int), cudaMemcpyHostToDevice);
    if (cudaStatus != cudaSuccess) {
        fprintf(stderr, "cudaMemcpy failed!");
        goto Error;
    }

    cudaStatus = cudaMemcpy(dev_b, b, size * sizeof(int), cudaMemcpyHostToDevice);
    if (cudaStatus != cudaSuccess) {
        fprintf(stderr, "cudaMemcpy failed!");
        goto Error;
    }

    // Launch a kernel on the GPU with one thread for each element.
    addKernel<<<1, size>>>(dev_c, dev_a, dev_b);

    // Check for any errors launching the kernel
    cudaStatus = cudaGetLastError();
    if (cudaStatus != cudaSuccess) {
        fprintf(stderr, "addKernel launch failed: %s\n", cudaGetErrorString(cudaStatus));
        goto Error;
    }
    
    // cudaDeviceSynchronize waits for the kernel to finish, and returns
    // any errors encountered during the launch.
    cudaStatus = cudaDeviceSynchronize();
    if (cudaStatus != cudaSuccess) {
        fprintf(stderr, "cudaDeviceSynchronize returned error code %d after launching addKernel!\n", cudaStatus);
        goto Error;
    }

    // Copy output vector from GPU buffer to host memory.
    cudaStatus = cudaMemcpy(c, dev_c, size * sizeof(int), cudaMemcpyDeviceToHost);
    if (cudaStatus != cudaSuccess) {
        fprintf(stderr, "cudaMemcpy failed!");
        goto Error;
    }

Error:
    cudaFree(dev_c);
    cudaFree(dev_a);
    cudaFree(dev_b);
    
    return cudaStatus;
}
