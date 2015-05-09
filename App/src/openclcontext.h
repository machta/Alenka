/**
 * @brief The header with the OpenCLContext class definition.
 *
 * @file
 */

#ifndef OPENCLCONTEXT_H
#define OPENCLCONTEXT_H

#include "error.h"

#include <QOpenGLContext>
#include <CL/cl_gl.h>

#include <string>

/**
 * @brief A wrapper for cl_context.
 */
class OpenCLContext
{
public:
	/**
	 * @brief OpenCLContext constructor.
	 * @param platform Used as an index to an array returned by clGetPlatformIDs().
	 * @param device Used as an index to an array returned by clGetDeviceIDs().
	 * @param deviceType Used in the call to clGetDeviceIDs().
	 * @param parentContext If not nullptr, this context will be able to share
	 * buffers with parentContext.
	 *
	 * parentContext is needed to setup proper communication between OpenGL and OpenCL.
	 * This is the only platform dependent code in the whole program and
	 * will probably need to be modified when the code is ported to other platforms.
	 */
	OpenCLContext(unsigned int platform, unsigned int device, cl_device_type deviceType, QOpenGLContext* parentContext = nullptr);

	~OpenCLContext();

	/**
	 * @brief Returns the underlying OpenCL object.
	 */
	cl_context getCLContext() const
	{
		return context;
	}

	/**
	 * @brief Returns the platform id resolved during construction.
	 */
	cl_platform_id getCLPlatform() const
	{
		return platformId;
	}

	/**
	 * @brief Returns the device id resolved during construction.
	 */
	cl_device_id getCLDevice() const
	{
		return deviceId;
	}

	/**
	 * @brief Returns a human-readable string with info about the selected platform.
	 *
	 * clGetPlatformInfo() is used to retrieve this info.
	 */
	std::string getPlatformInfo() const;

	/**
	 * @brief Returns a human-readable string with info about the selected device.
	 *
	 * clGetDeviceInfo() is used to retrieve this info.
	 */
	std::string getDeviceInfo() const;

	/**
	 * @brief A convenience function for using a barrier.
	 */
	static void enqueueBarrier(cl_command_queue commandQueue, cl_event event)
	{
		cl_int err;
#if CL_VERSION_1_2
		err = clEnqueueBarrierWithWaitList(commandQueue, 1, &event, nullptr);
		checkClErrorCode(err, "clEnqueueBarrierWithWaitList()");
#else
		err = clEnqueueWaitForEvents(commandQueue, 1, &event);
		checkClErrorCode(err, "clEnqueueWaitForEvents()");
#endif

		err = clReleaseEvent(event);
		checkClErrorCode(err, "clReleaseEvent()");
	}

private:
	cl_context context;
	cl_platform_id platformId;
	cl_device_id deviceId;
};

/**
 * @brief This macro is used to ensure consistency when creating the same object in different places.
 */
#define OPENCL_CONTEXT_CONSTRUCTOR_PARAMETERS PROGRAM_OPTIONS["clPlatform"].as<int>(), PROGRAM_OPTIONS["clDevice"].as<int>(), CL_DEVICE_TYPE_ALL

#endif // OPENCLCONTEXT_H
