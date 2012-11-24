#ifndef SOLVER_CL_H
#define SOLVER_CL_H

#include <cstdlib>

#ifdef __APPLE__
#include <OpenCL/OpenCL.h>
#elif defined(_WIN32)
#include <CL/cl.h>
#else
#endif

class SolverCL {
  
public:
  
  void init(size_t bufferSize);
  
  void destroy();
  
public:
  
  void stepDensity(int NW, int NH, float* x, float* x0, float* u, float* v, float diff, float dt, size_t bufferSize);
  void stepVelocity(int NW, int NH, float* u, float* v, float* u0, float* v0, float visc, float dt, size_t bufferSize);

  
private:
  
  int err;                            // error code returned from api calls
  
  size_t global;
  size_t local;
  
  cl_device_id device_id;
  cl_context context;
  cl_command_queue commands;
  cl_program program;
  cl_kernel kernel;
  
  cl_mem u_mem;
  cl_mem v_mem;
  
  cl_mem u_prev_mem;
  cl_mem v_prev_mem;

private:
  
  unsigned int NW;
  unsigned int NH;
  
  float diff, dt;

  
};

#endif
