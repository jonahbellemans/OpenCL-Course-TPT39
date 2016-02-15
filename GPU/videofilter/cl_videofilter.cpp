#include <stdio.h>
#include <stdlib.h>
#include <iostream> // for standard I/O
#include <fstream>
#include <time.h>
#include "opencv2/opencv.hpp"
#include <math.h>
#include <CL/cl.h>
#include <CL/cl_ext.h>
#include <unistd.h>

#define STRING_BUFFER_LEN 1024
#define ROWS 360
#define COLS 640
using namespace std;
using namespace cv;

 //#define SHOW


// All following code taken from the provided snippets
void print_clbuild_errors(cl_program program,cl_device_id device)
	{
		cout<<"Program Build failed\n";
		size_t length;
		char buffer[2048];
		clGetProgramBuildInfo(program, device, CL_PROGRAM_BUILD_LOG, sizeof(buffer), buffer, &length);
		cout<<"--- Build log ---\n "<<buffer<<endl;
		exit(1);
	}

unsigned char ** read_file(const char *name) {
  size_t size;
  unsigned char **output=(unsigned char **)malloc(sizeof(unsigned char *));
  FILE* fp = fopen(name, "rb");
  if (!fp) {
    printf("no such file:%s",name);
    exit(-1);
  }

  fseek(fp, 0, SEEK_END);
  size = ftell(fp);
  fseek(fp, 0, SEEK_SET);

  *output = (unsigned char *)malloc(size);
  unsigned char **outputstr=(unsigned char **)malloc(sizeof(unsigned char *));
  *outputstr= (unsigned char *)malloc(size);
  if (!*output) {
    fclose(fp);
    printf("mem allocate failure:%s",name);
    exit(-1);
  }

  if(!fread(*output, size, 1, fp)) printf("failed to read file\n");
  fclose(fp);
  printf("file size %d\n",size);
  printf("-------------------------------------------\n");
  snprintf((char *)*outputstr,size,"%s\n",*output);
  printf("%s\n",*outputstr);
  printf("-------------------------------------------\n");
  return outputstr;
}
void callback(const char *buffer, size_t length, size_t final, void *user_data)
{
     fwrite(buffer, 1, length, stdout);
}
// Thanks to https://stackoverflow.com/questions/24326432/convenient-way-to-show-opencl-error-codes
const char *getErrorString(cl_int error)
{
switch(error){
    // run-time and JIT compiler errors
    case 0: return "CL_SUCCESS";
    case -1: return "CL_DEVICE_NOT_FOUND";
    case -2: return "CL_DEVICE_NOT_AVAILABLE";
    case -3: return "CL_COMPILER_NOT_AVAILABLE";
    case -4: return "CL_MEM_OBJECT_ALLOCATION_FAILURE";
    case -5: return "CL_OUT_OF_RESOURCES";
    case -6: return "CL_OUT_OF_HOST_MEMORY";
    case -7: return "CL_PROFILING_INFO_NOT_AVAILABLE";
    case -8: return "CL_MEM_COPY_OVERLAP";
    case -9: return "CL_IMAGE_FORMAT_MISMATCH";
    case -10: return "CL_IMAGE_FORMAT_NOT_SUPPORTED";
    case -11: return "CL_BUILD_PROGRAM_FAILURE";
    case -12: return "CL_MAP_FAILURE";
    case -13: return "CL_MISALIGNED_SUB_BUFFER_OFFSET";
    case -14: return "CL_EXEC_STATUS_ERROR_FOR_EVENTS_IN_WAIT_LIST";
    case -15: return "CL_COMPILE_PROGRAM_FAILURE";
    case -16: return "CL_LINKER_NOT_AVAILABLE";
    case -17: return "CL_LINK_PROGRAM_FAILURE";
    case -18: return "CL_DEVICE_PARTITION_FAILED";
    case -19: return "CL_KERNEL_ARG_INFO_NOT_AVAILABLE";

    // compile-time errors
    case -30: return "CL_INVALID_VALUE";
    case -31: return "CL_INVALID_DEVICE_TYPE";
    case -32: return "CL_INVALID_PLATFORM";
    case -33: return "CL_INVALID_DEVICE";
    case -34: return "CL_INVALID_CONTEXT";
    case -35: return "CL_INVALID_QUEUE_PROPERTIES";
    case -36: return "CL_INVALID_COMMAND_QUEUE";
    case -37: return "CL_INVALID_HOST_PTR";
    case -38: return "CL_INVALID_MEM_OBJECT";
    case -39: return "CL_INVALID_IMAGE_FORMAT_DESCRIPTOR";
    case -40: return "CL_INVALID_IMAGE_SIZE";
    case -41: return "CL_INVALID_SAMPLER";
    case -42: return "CL_INVALID_BINARY";
    case -43: return "CL_INVALID_BUILD_OPTIONS";
    case -44: return "CL_INVALID_PROGRAM";
    case -45: return "CL_INVALID_PROGRAM_EXECUTABLE";
    case -46: return "CL_INVALID_KERNEL_NAME";
    case -47: return "CL_INVALID_KERNEL_DEFINITION";
    case -48: return "CL_INVALID_KERNEL";
    case -49: return "CL_INVALID_ARG_INDEX";
    case -50: return "CL_INVALID_ARG_VALUE";
    case -51: return "CL_INVALID_ARG_SIZE";
    case -52: return "CL_INVALID_KERNEL_ARGS";
    case -53: return "CL_INVALID_WORK_DIMENSION";
    case -54: return "CL_INVALID_WORK_GROUP_SIZE";
    case -55: return "CL_INVALID_WORK_ITEM_SIZE";
    case -56: return "CL_INVALID_GLOBAL_OFFSET";
    case -57: return "CL_INVALID_EVENT_WAIT_LIST";
    case -58: return "CL_INVALID_EVENT";
    case -59: return "CL_INVALID_OPERATION";
    case -60: return "CL_INVALID_GL_OBJECT";
    case -61: return "CL_INVALID_BUFFER_SIZE";
    case -62: return "CL_INVALID_MIP_LEVEL";
    case -63: return "CL_INVALID_GLOBAL_WORK_SIZE";
    case -64: return "CL_INVALID_PROPERTY";
    case -65: return "CL_INVALID_IMAGE_DESCRIPTOR";
    case -66: return "CL_INVALID_COMPILER_OPTIONS";
    case -67: return "CL_INVALID_LINKER_OPTIONS";
    case -68: return "CL_INVALID_DEVICE_PARTITION_COUNT";

    // extension errors
    case -1000: return "CL_INVALID_GL_SHAREGROUP_REFERENCE_KHR";
    case -1001: return "CL_PLATFORM_NOT_FOUND_KHR";
    case -1002: return "CL_INVALID_D3D10_DEVICE_KHR";
    case -1003: return "CL_INVALID_D3D10_RESOURCE_KHR";
    case -1004: return "CL_D3D10_RESOURCE_ALREADY_ACQUIRED_KHR";
    case -1005: return "CL_D3D10_RESOURCE_NOT_ACQUIRED_KHR";
    default: return "Unknown OpenCL error";
    }
}


void checkError(int status, const char *msg) {
    if(status!=CL_SUCCESS)
        printf("%s: %s\n",msg,getErrorString(status));
}

// Main method and own code starts here
int main(int, char**)
{
    VideoCapture camera("./bourne.mp4");
    if(!camera.isOpened())  // check if we succeeded
        return -1;

    const string NAME = "./cl_output.avi";   // Form the new name with container
    int ex = static_cast<int>(CV_FOURCC('M','J','P','G'));
    Size S = Size((int) camera.get(CV_CAP_PROP_FRAME_WIDTH),    // Acquire input size
                  (int) camera.get(CV_CAP_PROP_FRAME_HEIGHT));
	//Size S =Size(1280,720);
	cout << "SIZE:" << S << endl;

    VideoWriter outputVideo;                                        // Open the output
        outputVideo.open(NAME, ex, 25, S, true);

    if (!outputVideo.isOpened())
    {
        cout  << "Could not open the output video for write: " << NAME << endl;
        return -1;
    }
	time_t start,end;
	double diff,tot;
	int count=0;
	const char *windowName = "filter";   // Name shown in the GUI window.
    #ifdef SHOW
    namedWindow(windowName); // Resizable window, might not work on Windows.
    #endif

    // Initialize OpenCL variables, code reused from vector_add.cpp
    char char_buffer[STRING_BUFFER_LEN];
    cl_platform_id platform;
    cl_device_id device;
    cl_context context;
    cl_context_properties context_properties[] =
    {
         CL_CONTEXT_PLATFORM, 0,
         CL_PRINTF_CALLBACK_ARM, (cl_context_properties)callback,
         CL_PRINTF_BUFFERSIZE_ARM, 0x1000,
         0
    };
    cl_command_queue queue;
    cl_program program;
    cl_kernel kernel;
    int status;


    // Declaring necessary OpenCL buffer variables
    cl_mem input_buf, output_buf, gauss_buf, sobelx_buf, sobely_buf;

    // OpenCL Configuration
    clGetPlatformIDs(1, &platform, NULL);

    clGetPlatformInfo(platform, CL_PLATFORM_NAME, STRING_BUFFER_LEN, char_buffer, NULL);
    printf("%-40s = %s\n", "CL_PLATFORM_NAME", char_buffer);

    clGetPlatformInfo(platform, CL_PLATFORM_VENDOR, STRING_BUFFER_LEN, char_buffer, NULL);
    printf("%-40s = %s\n", "CL_PLATFORM_VENDOR ", char_buffer);

    clGetPlatformInfo(platform, CL_PLATFORM_VERSION, STRING_BUFFER_LEN, char_buffer, NULL);
    printf("%-40s = %s\n\n", "CL_PLATFORM_VERSION ", char_buffer);
    context_properties[1] = (cl_context_properties)platform;

    clGetDeviceIDs(platform, CL_DEVICE_TYPE_GPU, 1, &device, NULL);
    context = clCreateContext(context_properties, 1, &device, NULL, NULL, NULL);

    queue = clCreateCommandQueue(context, device, CL_QUEUE_PROFILING_ENABLE, &status);
    checkError(status, "Failed to create command queue");

    // Create necessary OpenCL Events
    cl_event gauss1, gauss2, gauss3,
							sobelx, sobely,
							map_event_g1, unmap_event_g1,
							map_event_g2, unmap_event_g2,
							map_event_g3, unmap_event_g3,
							map_event_sobel, unmap_event_sobel,
							mapoutput_event_g1, unmapoutput_event_g1,
							mapoutput_event_g2, unmapoutput_event_g2,
							mapoutput_event_g3, unmapoutput_event_g3,
							mapoutput_event_sobelx, unmapoutput_event_sobelx,
							mapoutput_event_sobely, unmapoutput_event_sobely,
							write_gauss_buf, write_sobelx_buf, write_sobely_buf;

    // Define Guassian Convolutional Kernel

		const float gauss3x3[9] = {
      0.0625, 0.125, 0.0625,
      0.125, 0.25, 0.125,
      0.0625, 0.125, 0.0625
    };

/*
		float gauss3x3[9];

		Mat gaussKern = getGaussianKernel(9, 0, CV_32F);
		memcpy(gauss3x3, gaussKern.data, 9*sizeof(float));
		for (int i = 0; i < 9; ++i) {
			printf("Gauss coefficient: %f\n", gauss3x3[i]);
		}*/

    // Define Sobel Convolutional Kernels
    const float dx_sobel3x3[9] = {
      -2, 0, 2,
      -4, 0, 4,
      -2, 0, 2
    };

    const float dy_sobel3x3[9] = {
      2, 4, 2,
      0, 0, 0,
      -2, -4, -2
    };

    // Create necessary OpenCL Buffers
    gauss_buf = clCreateBuffer(context, CL_MEM_READ_ONLY, 3*3*sizeof(float), NULL, &status);
    checkError(status, "Failed to create buffer for Gauss");

		status = clEnqueueWriteBuffer(queue, gauss_buf,  CL_TRUE, 0, 3*3*sizeof(float), gauss3x3, 0, NULL, &write_gauss_buf);
		checkError(status, "Failed to write buffer for Gauss");

		sobelx_buf = clCreateBuffer(context, CL_MEM_READ_ONLY, 3*3*sizeof(float), NULL, &status);
    checkError(status, "Failed to create buffer for SobelX");

		status = clEnqueueWriteBuffer(queue, sobelx_buf,  CL_TRUE, 0, 3*3*sizeof(float), dx_sobel3x3, 0, NULL, &write_sobelx_buf);
		checkError(status, "Failed to write buffer for SobelX");

		sobely_buf = clCreateBuffer(context, CL_MEM_READ_ONLY, 3*3*sizeof(float), NULL, &status);
    checkError(status, "Failed to create buffer for SobelY");

		status = clEnqueueWriteBuffer(queue, sobely_buf,  CL_TRUE, 0, 3*3*sizeof(float), dy_sobel3x3, 0, NULL, &write_sobely_buf);
		checkError(status, "Failed to write buffer for SobelY");

    input_buf = clCreateBuffer(context, CL_MEM_ALLOC_HOST_PTR, (2+ROWS)*(2+COLS)*sizeof(float), NULL, &status);
    checkError(status, "Failed to create buffer for input");

    output_buf = clCreateBuffer(context, CL_MEM_ALLOC_HOST_PTR, ROWS*COLS*sizeof(float), NULL, &status);
    checkError(status, "Failed to create buffer for output");

    unsigned char **opencl_program=read_file("cl_videofilter.cl");
    program = clCreateProgramWithSource(context, 1, (const char **)opencl_program, NULL, NULL);
    if (program == NULL) {
      printf("Program creation failed\n");
      return 1;
    }
    int success=clBuildProgram(program, 0, NULL, NULL, NULL, NULL);
    if(success!=CL_SUCCESS) print_clbuild_errors(program,device);
    kernel = clCreateKernel(program, "apply_3x3_convolution", NULL);
    const size_t global_work_size[2] = {ROWS, COLS};

    while (true) {

		Mat cameraFrame,displayframe;
		count=count+1;
		if(count > 299) break;
        camera >> cameraFrame;
        Mat filterframe = Mat(cameraFrame.size(), CV_8UC3);
        Mat grayframe, paddedframe, edge_x,edge_y,edge,edge_inv;
    		cvtColor(cameraFrame, grayframe, CV_BGR2GRAY);
				//Initialize edge, edge_x, edge_y
				edge_x = grayframe.clone();
				edge_y = grayframe.clone();
				edge = grayframe.clone();

			// Convert grayframe, edge, edge_x and edge_y to floats instead of chars
			grayframe.convertTo(grayframe, CV_32FC1);
			edge.convertTo(edge, CV_32FC1);
			edge_x.convertTo(edge_x, CV_32FC1);
			edge_y.convertTo(edge_y, CV_32FC1);

      // Start timer
      time (&start);

      // Apply Gauss Convolutions

            // Gauss Convolution 1


						// Add padding to input
						copyMakeBorder(grayframe, paddedframe, 0, 2, 0, 2, BORDER_CONSTANT, 0);


						// Map input array
						float* input = (float *)clEnqueueMapBuffer(queue, input_buf, CL_TRUE,
								CL_MAP_WRITE,0, (2+ROWS)*(2+COLS)*sizeof(float), 0, NULL, &map_event_g1, &status);
						checkError(status, "Failed to map input for G1");

						// Copy grayframe into memory
						memcpy(input, paddedframe.data, (2+ROWS)*(2+COLS)*sizeof(float));

						// UnMap input array
						status = clEnqueueUnmapMemObject(queue,input_buf,input,1, &map_event_g1, &unmap_event_g1);
						checkError(status, "Failed to unmap input for G1");


            // Set kernel arguments.
            unsigned argi = 0;

            status = clSetKernelArg(kernel, argi++, sizeof(cl_mem), &input_buf);
            checkError(status, "G1: Failed to set argument 1");

            status = clSetKernelArg(kernel, argi++, sizeof(cl_mem), &gauss_buf);
            checkError(status, "G1: Failed to set argument 2");

            status = clSetKernelArg(kernel, argi++, sizeof(cl_mem), &output_buf);
            checkError(status, "G1: Failed to set argument 3");

            status = clEnqueueNDRangeKernel(queue, kernel, 2, NULL, global_work_size, NULL, 1, &unmap_event_g1, &gauss1);
            checkError(status, "G1: Failed to launch kernel");

						// Map output array
			      float* output = (float *)clEnqueueMapBuffer(queue, output_buf, CL_TRUE,
			          CL_MAP_READ, 0, ROWS*COLS*sizeof(float),  1, &gauss1, &mapoutput_event_g1, &status);
			      checkError(status, "Failed to map output for G1");

			      // Reassign output to the frame
			      memcpy(grayframe.data, output, ROWS*COLS*sizeof(float));

						// UnMap Output array
						status = clEnqueueUnmapMemObject(queue,output_buf,output,1, &mapoutput_event_g1, &unmapoutput_event_g1);
						checkError(status, "Failed to unmap output for G1");



            // Gauss Convolution 2

						// Add padding to input
			      copyMakeBorder(grayframe, paddedframe, 0, 2, 0, 2, BORDER_CONSTANT, 0);


			      // Map input array
			      input = (float *)clEnqueueMapBuffer(queue, input_buf, CL_TRUE,
			          CL_MAP_WRITE,0, (2+ROWS)*(2+COLS)*sizeof(float), 1, &unmapoutput_event_g1, &map_event_g2, &status);
			      checkError(status, "Failed to map input for G2");

			      // Copy grayframe into memory
			      memcpy(input, paddedframe.data, (2+ROWS)*(2+COLS)*sizeof(float));

			      // UnMap input array
			      status = clEnqueueUnmapMemObject(queue,input_buf,input,1, &map_event_g2, &unmap_event_g2);
						checkError(status, "Failed to unmap input for G2");



						// Set kernel arguments.
            argi = 0;

            status = clSetKernelArg(kernel, argi++, sizeof(cl_mem), &input_buf);
            checkError(status, "G2: Failed to set argument 1");

            status = clSetKernelArg(kernel, argi++, sizeof(cl_mem), &gauss_buf);
            checkError(status, "G2: Failed to set argument 2");

            status = clSetKernelArg(kernel, argi++, sizeof(cl_mem), &output_buf);
            checkError(status, "G2: Failed to set argument 3");

            status = clEnqueueNDRangeKernel(queue, kernel, 2, NULL, global_work_size, NULL, 1, &unmap_event_g2, &gauss2);
            checkError(status, "G2: Failed to launch kernel");


						// Map output array
			      output = (float *)clEnqueueMapBuffer(queue, output_buf, CL_TRUE,
			          CL_MAP_READ, 0, ROWS*COLS*sizeof(float),  1, &gauss2, &mapoutput_event_g2, &status);
			      checkError(status, "Failed to map output for G2");

			      // Reassign output to the frame
			      memcpy(grayframe.data, output, ROWS*COLS*sizeof(float));

						// UnMap Output array
						status = clEnqueueUnmapMemObject(queue,output_buf,output,1, &mapoutput_event_g2, &unmapoutput_event_g2);
						checkError(status, "Failed to unmap output for G2");

            // Gauss Convolution 3

						// Add padding to input
			      copyMakeBorder(grayframe, paddedframe, 0, 2, 0, 2, BORDER_CONSTANT, 0);


			      // Map input array
			      input = (float *)clEnqueueMapBuffer(queue, input_buf, CL_TRUE,
			          CL_MAP_WRITE,0, (2+ROWS)*(2+COLS)*sizeof(float), 1, &unmapoutput_event_g2, &map_event_g3, &status);
			      checkError(status, "Failed to map input for G3");

			      // Copy grayframe into memory
			      memcpy(input, paddedframe.data, (2+ROWS)*(2+COLS)*sizeof(float));

			      // UnMap input array
			      status = clEnqueueUnmapMemObject(queue,input_buf,input,1, &map_event_g3, &unmap_event_g3);
						checkError(status, "Failed to unmap input for G3");

						// Set kernel arguments.
            argi = 0;

            status = clSetKernelArg(kernel, argi++, sizeof(cl_mem), &input_buf);
            checkError(status, "G3: Failed to set argument 1");

            status = clSetKernelArg(kernel, argi++, sizeof(cl_mem), &gauss_buf);
            checkError(status, "G3: Failed to set argument 2");

            status = clSetKernelArg(kernel, argi++, sizeof(cl_mem), &output_buf);
            checkError(status, "G3: Failed to set argument 3");

            status = clEnqueueNDRangeKernel(queue, kernel, 2, NULL, global_work_size, NULL, 1, &unmap_event_g3, &gauss3);
            checkError(status, "G3: Failed to launch kernel");

						// Map output array
			      output = (float *)clEnqueueMapBuffer(queue, output_buf, CL_TRUE,
			          CL_MAP_READ, 0, ROWS*COLS*sizeof(float),  1, &gauss3, &mapoutput_event_g3, &status);
			      checkError(status, "Failed to map output for G3");

			      // Reassign output to the frame
			      memcpy(grayframe.data, output, ROWS*COLS*sizeof(float));

						// UnMap Output array
						status = clEnqueueUnmapMemObject(queue,output_buf,output,1, &mapoutput_event_g3, &unmapoutput_event_g3);
						checkError(status, "Failed to unmap output for G3");


			// Apply Sobel Edge Convolution
			// Sobel X
					// Add padding to input
		      copyMakeBorder(grayframe, paddedframe, 0, 2, 0, 2, BORDER_CONSTANT, 0);


		      // Map input array
		      input = (float *)clEnqueueMapBuffer(queue, input_buf, CL_TRUE,
		          CL_MAP_WRITE,0, (2+ROWS)*(2+COLS)*sizeof(float), 1, &unmapoutput_event_g3, &map_event_sobel, &status);
		      checkError(status, "Failed to map input for Sobel");

		      // Copy grayframe into memory
		      memcpy(input, paddedframe.data, (2+ROWS)*(2+COLS)*sizeof(float));

		      // UnMap input array
		      status = clEnqueueUnmapMemObject(queue,input_buf,input,1, &map_event_sobel, &unmap_event_sobel);
					checkError(status, "Failed to unmap input for Sobel");

					// Set kernel arguments.
					argi = 0;

					status = clSetKernelArg(kernel, argi++, sizeof(cl_mem), &input_buf);
					checkError(status, "SX: Failed to set argument 1");

					status = clSetKernelArg(kernel, argi++, sizeof(cl_mem), &sobelx_buf);
					checkError(status, "SX: Failed to set argument 2");

					status = clSetKernelArg(kernel, argi++, sizeof(cl_mem), &output_buf);
					checkError(status, "SX: Failed to set argument 3");

					status = clEnqueueNDRangeKernel(queue, kernel, 2, NULL, global_work_size, NULL, 1, &unmap_event_sobel, &sobelx);
					checkError(status, "SX: Failed to launch kernel");

					// Map output array
		      output = (float *)clEnqueueMapBuffer(queue, output_buf, CL_TRUE,
		          CL_MAP_READ, 0, ROWS*COLS*sizeof(float),  1, &sobelx, &mapoutput_event_sobelx, &status);
		      checkError(status, "Failed to map output");

		      // Reassign output to the frame
		      memcpy(edge_x.data, output, ROWS*COLS*sizeof(float));

					// UnMap Output array
					status = clEnqueueUnmapMemObject(queue,output_buf,output,1, &mapoutput_event_sobelx, &unmapoutput_event_sobelx);
					checkError(status, "Failed to unmap output");


		  	// Sobel Y
					// Set kernel arguments.
					argi = 0;

					status = clSetKernelArg(kernel, argi++, sizeof(cl_mem), &input_buf);
					checkError(status, "SY: Failed to set argument 1");

					status = clSetKernelArg(kernel, argi++, sizeof(cl_mem), &sobely_buf);
					checkError(status, "SY: Failed to set argument 2");

					status = clSetKernelArg(kernel, argi++, sizeof(cl_mem), &output_buf);
					checkError(status, "SY: Failed to set argument 3");

					status = clEnqueueNDRangeKernel(queue, kernel, 2, NULL, global_work_size, NULL, 1, &unmapoutput_event_sobelx, &sobely);
					checkError(status, "SY: Failed to launch kernel");

					// Map output array
		     output = (float *)clEnqueueMapBuffer(queue, output_buf, CL_TRUE,
		          CL_MAP_READ, 0, ROWS*COLS*sizeof(float),  1, &sobely, &mapoutput_event_sobely, &status);
		      checkError(status, "Failed to map output");

		      // Reassign output to the frame
		      memcpy(edge_y.data, output, ROWS*COLS*sizeof(float));

					// UnMap Output array
					status = clEnqueueUnmapMemObject(queue,output_buf,output,1, &mapoutput_event_sobely, &unmapoutput_event_sobely);
					checkError(status, "Failed to unmap output");



					addWeighted( edge_x, 0.5, edge_y, 0.5, 0, edge );
			    threshold(edge, edge, 80, 255, THRESH_BINARY_INV);


					// Convert frames back to char
					grayframe.convertTo(grayframe, CV_8UC1);
					edge.convertTo(edge, CV_8UC1);
					edge_x.convertTo(edge_x, CV_8UC1);
					edge_y.convertTo(edge_y, CV_8UC1);

		//Scharr(grayframe, edge_x, CV_8U, 0, 1, 1, 0, BORDER_DEFAULT );
		//Scharr(grayframe, edge_y, CV_8U, 1, 0, 1, 0, BORDER_DEFAULT );






    time (&end);

		cvtColor(edge, edge_inv, CV_GRAY2BGR);
    // Clear the output image to black, so that the cartoon line drawings will be black (ie: not drawn).
  	memset((char*)displayframe.data, 0, displayframe.step * displayframe.rows);
		grayframe.copyTo(displayframe,edge);
    cvtColor(displayframe, displayframe, CV_GRAY2BGR);
    outputVideo << displayframe;
	#ifdef SHOW
        imshow(windowName, grayframe);
	#endif
		diff = difftime (end,start);
		tot+=diff;
	}
	outputVideo.release();
	camera.release();
  printf ("FPS %.2lf .\n", 299.0/tot );

    return EXIT_SUCCESS;
}
