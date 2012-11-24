#ifndef SOLVER_H
#define SOLVER_H

#define IX(i, j) ((i) + (NW + 2) * (j))
#define SWAP(x0, x) {float* tmp = x0; x0 = x; x = tmp;}

void stepDensity(int NW, int NH, float* u, float* v, float* u_prev, float* v_prev, float diff, float dt, unsigned int bufferSize);
void stepVelocity(int NW, int NH, float* u, float* v, float* u0, float* v0, float visc, float dt);

extern int SOLVE_STEPS;

#endif
