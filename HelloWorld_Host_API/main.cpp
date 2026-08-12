#define CL_HPP_TARGET_OPENCL_VERSION 300
#define CL_HPP_ENABLE_EXCEPTIONS
#include <CL/opencl.hpp>
#include <vector>
#include <iostream>

std::string opencl_c_container() { 
    return R"KERNELRAWSTRING(
        __kernel void add(__global const float *a, __global const float *b,
                        __global float *c) {
            int i = get_global_id(0);
            c[i] = a[i] + b[i];
        }
    )KERNELRAWSTRING";
}

int main () {
    // TO DO - Platforma i device
    std::vector<cl::Platform> platforms;
    cl::Platform::get(&platforms);
    cl::Platform platform = platforms.front(); // pierwsza platfroma

    std::vector<cl::Device> devices;
    if (platform.getDevices(CL_DEVICE_TYPE_GPU, &devices) != CL_SUCCESS) {
        std::cerr << "Error: Failed to get GPU devices\n";
        return 1;
    }

    cl::Device device = devices.front(); // bierzemy piersze deivec
    std::cout << "Device: " << device.getInfo<CL_DEVICE_NAME>() << "\n";

    // TO DO Contex oraz Queue
    cl::Context context(device);
    cl::CommandQueue queue(context, device, CL_QUEUE_PROFILING_ENABLE);

    // To DO Kompilacja prgramu (prosty kernel)
    std::string src = opencl_c_container();
    cl::Program program(context, src);
    try {
        program.build({device});
    } catch (cl::Error& e) {
        std::cerr << "Build error:\n"
                  << program.getBuildInfo<CL_PROGRAM_BUILD_LOG>(device) << "\n";
        return 1;
    }

    // TO DO incalizacja buforów
    const int N = 1024;
    std::vector<float> a(N, 1.0f), b(N, 2.0f), c(N, -1.0f);
    cl::Buffer bufA(context, CL_MEM_READ_ONLY, sizeof(float) * N); // Deklracja bufora o szerokości floata zwielokrtonione o N i mamy cały bufor
    cl::Buffer bufB(context, CL_MEM_READ_ONLY, sizeof(float) * N);
    cl::Buffer bufC(context, CL_MEM_WRITE_ONLY, sizeof(float) * N);

    if (bufA() == 0 || bufB() == 0 || bufC() == 0) {
        std::cerr << "Error: Failed to create bufffers\n";
        return 1;
    }

    try { 
        queue.enqueueWriteBuffer(bufA, CL_TRUE, 0, sizeof(float) * N, a.data());
    } catch (cl::Error& e) {
        std::cerr << "Error: Failed to write buffer A\n";
        return 1;
    }

    try {
        queue.enqueueWriteBuffer(bufB, CL_TRUE, 0,sizeof(float) * N, b.data());
    } catch (cl::Error& e) {
        std::cerr << "Error: Failed to write buffer B\n";
        return 1;
    }
    
    // TO DO: odpalenie kerneli wraz z arg
    cl::Kernel kernel(program, "add");
    kernel.setArg(0, bufA);
    kernel.setArg(1, bufB);
    kernel.setArg(2, bufC);

    // To DO: uruchomienie i odczyt
    try {
        queue.enqueueNDRangeKernel(kernel, cl::NullRange, cl::NDRange(N), cl::NullRange);
        queue.enqueueReadBuffer(bufC, CL_TRUE, 0,sizeof(float) * N, c.data());
    } catch (cl::Error& e) {
        std::cerr << "Error: Failed to execute kernel or read buffer C\n";
        return 1;
    }

    std::cout << "c[0] = " << c[0] << "\n";
    return 0;
}