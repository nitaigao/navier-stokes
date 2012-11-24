#define IX(i, j) ((i) + (NW + 2) * (j))
#define SWAP(x0, x) {__global float* tmp = x0; x0 = x; x = tmp;}

void addSource(int NW, int NH, __global float* x, __global float* source, float dt) {
	int size = (NW + 2) * (NH + 2);
	for (int i = 0 ; i < size ; i++) {
    x[i] += dt * source[i];
  }
}

void setBoundary(int NW, int NH, int b, __global float* x) {
  // can be optimized into 1 for loop if we are using a square grid
  for (int i = 1 ; i <= NH; i++) {
    // along the left wall
    x[IX(0,    i)]        = b == 1 ? -x[IX(1, i)]  : x[IX(1, i)];
    
    // along the right wall
    x[IX(NW + 1, i)]      = b == 1 ? -x[IX(NW, i)] : x[IX(NW, i)];
  }
 
  for (int i = 1 ; i <= NW; i++) {
    // along the top wall
    x[IX(i,    0)]      = b == 2 ? -x[IX(i, 1)]  : x[IX(i, 1)];
    
    // along the bottom wall
    x[IX(i,    NH + 1)] = b == 2 ? -x[IX(i, NH)] : x[IX(i, NH)];
  }
  
  x[IX(0,           0)] = 0.5f * (x[IX(1,  0 )]     + x[IX(0,     1)]);
  x[IX(0,      NH + 1)] = 0.5f * (x[IX(1,  NH + 1)] + x[IX(0,     NH)]);
  x[IX(NW + 1,      0)] = 0.5f * (x[IX(NW, 0)]      + x[IX(NW + 1, 1)]);
  x[IX(NW + 1, NH + 1)] = 0.5f * (x[IX(NW, NH + 1)] + x[IX(NW + 1, NH)]);
}

void linearSolve(int NW, int NH, int b, __global float* x, __global float* x0, float a, float c) {
  for (int solveIteration = 0 ; solveIteration < 2; solveIteration++) {
    for (int i = 1 ; i <= NW; i++) {
      for (int j = 1 ; j <= NH; j++) {
        x[IX(i, j)] = x0[IX(i, j)];//(x0[IX(i, j)] + a * (x[IX(i - 1, j)] + x[IX(i + 1, j)] + x[IX(i, j - 1)] + x[IX(i, j + 1)])) / c;
      }
    }
    // setBoundary(NW, NH, b, x);
  }
}

void diffuse(int NW, int NH, int b, __global float* x, __global float* x0, float diff, float dt) {
  float a = dt * diff * NW * NH;
  linearSolve(NW, NH, b, x, x0, a, 1 + 4 * a);
}

void advect(int NW, int NH, int b, __global float* d, __global float* d0, __global float* u, __global float* v, float dt) {
  float dt0 = dt * NW;

  for (int i = 1; i <= NW; i++) {
    for (int j = 1; j <= NH; j++) {
      float x = i - dt0 * u[IX(i,j)];
      float y = j - dt0 * v[IX(i,j)];
      
      if (x < 0.5f) {
        x = 0.5f;
      }
      
      if (x > NW + 0.5f) {
        x = NW + 0.5f;
      }
      
      int i0 = (int)x;
      int i1 = i0 + 1;
      
      if (y < 0.5f) {
        y = 0.5f;
      }
      
      if (y > NH + 0.5f) {
        y = NH + 0.5f;
      }
      
      int j0 = (int)y;
      int j1 = j0 + 1;
      
      float s1 = x - i0;
      float s0 = 1 - s1;
      float t1 = y - j0;
      float t0 = 1 - t1;
      
      d[IX(i,j)] = s0 * (t0 * d0[IX(i0,j0)] + t1 * d0[IX(i0,j1)]) + s1 * (t0 * d0[IX(i1,j0)] + t1 * d0[IX(i1,j1)]);
    }
  }
  //setBoundary(NW, NH, b, d);
}

__kernel void stepDensity(
  const unsigned int NW, 
  const unsigned int NH, 
  __global float* u, 
  __global float* v, 
  __global float* u_prev, 
  __global float* v_prev, 
  const float diff, 
  const float dt) {

  addSource(NW, NH, u, v, dt);
  // SWAP(v, u);
  // diffuse(NW, NH, 0, v, u, diff, dt);
  // SWAP(v, u);
  // advect(NW, NH, 0, u, v, u_prev, v_prev, dt); 
}