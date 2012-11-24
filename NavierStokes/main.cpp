#if defined(__APPLE__)
  #include <GL/glew.h>

#undef glGenVertexArrays
#undef glBindVertexArray

#define glGenVertexArrays glGenVertexArraysAPPLE
#define glBindVertexArray glBindVertexArrayAPPLE

#elif defined(_WIN32)
  #include <windows.h>
  
  #define GLEW_STATIC
  #include <GL/glew.h> 
  #include <GL/gl.h> 
  #include <GL/glu.h> 
#else
  #include <GL/glew.h>
#endif

#define _USE_MATH_DEFINES 
#include <cmath>

#include <FreeImage.h>

#include <GL/glfw.h>
#include <fstream>
#include <iostream>
#include <sstream>

#include <glm/glm.hpp> //vec3, vec4, ivec4, mat4
#include <glm/gtc/matrix_transform.hpp> //translate, rotate, scale, perspective
#include <glm/gtc/type_ptr.hpp> //value_ptr

#include "solver.h"
#include "solvercl.h"

#include "file2string.h"

static int WINDOW_WIDTH = 1280;
static int WINDOW_HEIGHT = 800;

static bool drawingDensity = true;

static int NW;
static int NH;
static float dt, diff, visc;
static float force, source;

static float *u, *v, *u_prev, *v_prev;
static float *dens, *dens_prev;

static float *color_r, *color_r_prev;
static float *color_r_u, *color_r_v, *color_r_u_prev, *color_r_v_prev;

static float *color_g, *color_g_prev;
static float *color_g_u, *color_g_v, *color_g_u_prev, *color_g_v_prev;

static float *color_b, *color_b_prev;
static float *color_b_u, *color_b_v, *color_b_u_prev, *color_b_v_prev;

static GLuint gridVAO = 0;

static GLuint colorVBO = 0;

static float* colorVertexArray = 0;
static unsigned int vertexSize = 0;
static unsigned int colorSize = 0;

static BYTE* imageData = NULL;

static int size = 0;
static unsigned int bpp = 0;

static SolverCL solverCL;

enum ColorMode {
  RED,
  GREEN,
  BLUE,
  COLOR_MODE_MAX
};

static unsigned int colorMode = RED;

static float forward, right, up = 0.0f;
static float yRotation = 0.0f;

int omx, omy;

void addDensity(float* density, int i, int j, float source) {
	density[IX(i - 1, j)] = source;

	density[IX(i - 1, j - 1)] = source;
	density[IX(i, j - 1)] = source;
	density[IX(i + 1, j - 1)] = source;

	density[IX(i + 1, j)] = source;

	density[IX(i + 1, j + 1)] = source;
	density[IX(i, j + 1)] = source;
	density[IX(i - 1, j + 1)] = source;

	density[IX(i, j)] = source; 
}

void input() {
  
  float speed = -1.0f;
  
  if (glfwGetKey(GLFW_KEY_UP) == GLFW_PRESS) {
    forward += -speed * dt;
  }
  
  if (glfwGetKey(GLFW_KEY_DOWN) == GLFW_PRESS) {
    forward -= -speed * dt;
  }
  
  if (glfwGetKey(GLFW_KEY_LEFT) == GLFW_PRESS) {
    right -= speed * dt;
  }
  
  if (glfwGetKey(GLFW_KEY_RIGHT) == GLFW_PRESS) {
    right += speed * dt;
  }
  
	unsigned int size = (NW+2)*(NH+2);

	memset(dens_prev, 0, sizeof(float) * size);
	memset(v_prev, 0, sizeof(float) * size);
	memset(u_prev, 0, sizeof(float) * size);

	memset(color_r_prev, 0, sizeof(float) * size);
	memset(color_r_u_prev, 0, sizeof(float) * size);
	memset(color_r_v_prev, 0, sizeof(float) * size);

	memset(color_b_prev, 0, sizeof(float) * size);
	memset(color_b_u_prev, 0, sizeof(float) * size);
	memset(color_b_v_prev, 0, sizeof(float) * size);

	memset(color_g_prev, 0, sizeof(float) * size);
	memset(color_g_u_prev, 0, sizeof(float) * size);
	memset(color_g_v_prev, 0, sizeof(float) * size);
  
  int mx, my;
  glfwGetMousePos(&mx, &my);
  int i = (int)((mx /(float)WINDOW_WIDTH)*NW+1);
  int j = (int)(((WINDOW_HEIGHT-my) /(float)WINDOW_HEIGHT)*NH+1);

  if (glfwGetMouseButton(GLFW_MOUSE_BUTTON_LEFT)) {
    u[IX(i,j)] = force * (mx-omx);
    v[IX(i,j)] = force * (omy-my);

    color_r_u[IX(i,j)] = force * (mx-omx);
    color_r_v[IX(i,j)] = force * (omy-my);

    color_g_u[IX(i,j)] = force * (mx-omx);
    color_g_v[IX(i,j)] = force * (omy-my);

    color_b_u[IX(i,j)] = force * (mx-omx);
    color_b_v[IX(i,j)] = force * (omy-my);
  }

  if (glfwGetMouseButton(GLFW_MOUSE_BUTTON_RIGHT) || glfwGetKey('D')) {

		addDensity(dens_prev, i, j, source);

    if (colorMode == RED) {
			addDensity(color_r_prev, i, j, source);     
    }

    if (colorMode == GREEN) {
      addDensity(color_g_prev, i, j, source);     
    }

    if (colorMode == BLUE) {
      addDensity(color_b_prev, i, j, source);     
    }
  }
    
  omx = mx;
	omy = my;
}

void update() {
  stepVelocity(NW, NH, color_r_u, color_r_v, color_r_u_prev, color_r_v_prev, visc, dt);
  solverCL.stepDensity(NW, NH, color_r, color_r_prev, color_r_u, color_r_v, diff, dt, size);
  //stepDensity(NW, NH, color_r, color_r_prev, color_r_u, color_r_v, diff, dt, size);

//  stepVelocity(NW, NH, color_g_u, color_g_v, color_g_u_prev, color_g_v_prev, visc, dt);
//  solverCL.stepDensity(NW, NH, color_g, color_g_prev, color_g_u, color_g_v, diff, dt, size);
//
//  stepVelocity(NW, NH, color_b_u, color_b_v, color_b_u_prev, color_b_v_prev, visc, dt);
//  solverCL.stepDensity(NW, NH, color_b, color_b_prev, color_b_u, color_b_v, diff, dt, size);
}

void drawVelocity() {
  float hw = 1.0f / NW;
	float hh = 1.0f / NH;
  
	glColor3f(1.0f, 1.0f, 1.0f);
	glLineWidth(1.0f);
  
	glBegin (GL_LINES);
  
  for (int i = 1; i <= NW ; i++) {
    
    float x = (i-0.5f) * hw - 0.5;
    
    for (int j=1; j <= NH ; j++) {
 
      float y = (j-0.5f)* hh - 0.5;
      
      glVertex2f(x, y);
      glVertex2f(x+u[IX(i,j)], y+v[IX(i,j)]);
    }
  }
  
	glEnd ();
}

void drawDensityComponent(int index, unsigned int& colorIndex) 
{	
	float cr = color_r[index];
	float cg = color_g[index];
	float cb = color_b[index];

  colorVertexArray[colorIndex++] = cr;
  colorVertexArray[colorIndex++] = cg;
  colorVertexArray[colorIndex++] = cb;
  colorVertexArray[colorIndex++] = 1.0f;
}


void drawDensity() {
  unsigned int colorIndex = 0;

  for (int i = 0 ; i <= NW ; i++) {
   for (int j = 0 ; j <= NH ; j++) {
  		drawDensityComponent(IX(i, j), colorIndex);
  		drawDensityComponent(IX(i + 1, j), colorIndex);
  		drawDensityComponent(IX(i + 1, j + 1), colorIndex);
  		drawDensityComponent(IX(i, j + 1), colorIndex);
     }
   }

  glBindBuffer(GL_ARRAY_BUFFER, colorVBO);
  glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(float) * colorSize, colorVertexArray);

  glDrawArrays(GL_QUADS, 0, vertexSize);
}

void render() {
  glTranslatef(0, 0, forward);
  
  if (drawingDensity) drawDensity();
  else drawVelocity();
}

void printLog(GLuint obj) {
	int infologLength = 0;
	const int maxLength = 256;

	char infoLog[maxLength];
  
	if (glIsShader(obj)) {
		glGetShaderInfoLog(obj, maxLength, &infologLength, infoLog);
  }
	else {
		glGetProgramInfoLog(obj, maxLength, &infologLength, infoLog);
  }
  
	if (infologLength > 0) {
		printf("%s\n",infoLog);
  }
}

GLuint createShaderProgram() {
  std::string vertexFile = file2string("navierstokes.v.glsl");
  GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
  const char *vertexSource = vertexFile.c_str();
  glShaderSource(vertexShader, 1, &vertexSource, NULL);
  glCompileShader(vertexShader);
  printLog(vertexShader);
  
  std::string fragmentFile = file2string("navierstokes.f.glsl");
  GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
  const char *fragmentSource = fragmentFile.c_str();
  glShaderSource(fragmentShader, 1, &fragmentSource, NULL);
  glCompileShader(fragmentShader);
  printLog(fragmentShader);
  
  GLuint shaderProgram = glCreateProgram();
  glAttachShader(shaderProgram, vertexShader);
  glAttachShader(shaderProgram, fragmentShader);

  glBindAttribLocation(shaderProgram, 0, "position");

  glLinkProgram(shaderProgram);
  
  printLog(shaderProgram);
  
  return shaderProgram;
}

BYTE* LoadTexture(const char* filename, unsigned int* width, unsigned int* height, unsigned int *bpp) {
	FREE_IMAGE_FORMAT fif = FreeImage_GetFileType(filename, 0);
	
	if(fif == FIF_UNKNOWN) fif = FreeImage_GetFIFFromFilename(filename);
	if(fif == FIF_UNKNOWN) return 0;
	
	FIBITMAP *dib = 0;
	if(FreeImage_FIFSupportsReading(fif)) dib = FreeImage_Load(fif, filename);

	if(!dib) return 0;

	*bpp = FreeImage_GetBPP(dib);

	FIBITMAP* scaled = FreeImage_Rescale(dib, NW+2, NH+2, FILTER_BILINEAR);

	BYTE* bits = (BYTE*)FreeImage_GetBits(scaled);

	*width = FreeImage_GetWidth(scaled);
	*height = FreeImage_GetHeight(scaled);

	if((bits == 0) || (width == 0) || (height == 0)) return 0;

	FreeImage_Unload(dib);

	return bits;
}

void clear() {
	unsigned int size = (NW+2)*(NH+2);

	memset(dens_prev, 0, sizeof(float) * size);
	memset(v_prev, 0, sizeof(float) * size);
	memset(u_prev, 0, sizeof(float) * size);

	memset(color_r_prev, 0, sizeof(float) * size);
	memset(color_r_u, 0, sizeof(float) * size);
	memset(color_r_v, 0, sizeof(float) * size);
	memset(color_r_u_prev, 0, sizeof(float) * size);
	memset(color_r_v_prev, 0, sizeof(float) * size);

	memset(color_b_prev, 0, sizeof(float) * size);
	memset(color_b_u, 0, sizeof(float) * size);
	memset(color_b_v, 0, sizeof(float) * size);
	memset(color_b_u_prev, 0, sizeof(float) * size);
	memset(color_b_v_prev, 0, sizeof(float) * size);

	memset(color_g_u, 0, sizeof(float) * size);
	memset(color_g_v, 0, sizeof(float) * size);
	memset(color_g_prev, 0, sizeof(float) * size);
	memset(color_g_u_prev, 0, sizeof(float) * size);
	memset(color_g_v_prev, 0, sizeof(float) * size);

	for (unsigned int i = 0; i < size; i++) {
		color_r[i] = 0.0f;
		color_g[i] = 0.0f;
		color_b[i] = 0.0f;
	}

	if (imageData != NULL) {
		for (unsigned int i = 0; i < size; i++) {
			int components = bpp / 8.0f;
			BYTE b = imageData[0+(i*components)];
			BYTE g = imageData[1+(i*components)];
			BYTE r = imageData[2+(i*components)];

			color_r[i] = r / 255.0f;
			color_g[i] = g / 255.0f;
			color_b[i] = b / 255.0f;
		}
	}
}

void printHelp() {
	printf("----------------\n");
	printf("v - Change Viscosity\n");
	printf("f - Change Force\n");
	printf("t - Change Solve Steps\n");
	printf("c - Clear Forces and Reset\n");
	printf("h - Print Help\n");
	printf("b - Change Right Click Color\n");
	printf("----------------\n");
	printf("esc - Quit\n");
	printf("----------------\n");
	printf("left click - Move Velocity\n");
	printf("right click - Add Density\n");
	printf("----------------\n");
}

void keyCallback(int keyCode, int action) {
	if (keyCode == 'M' && action == GLFW_PRESS) {
		drawingDensity = !drawingDensity;
	}

	if (glfwGetKey('V') && action == GLFW_PRESS) {
		if (visc == 0.0f) {
			visc = 0.00001f;
		} else 
		if (visc == 0.00001f) {
			visc = 0.0001f;
		} else 
		if (visc == 0.0001f) {
			visc = 0.001f;
		} else
		if (visc == 0.001f) {
			visc = 0.01f;
		} else
		if (visc == 0.01f) {
			visc = 0.1f;
		} else
		if (visc == 0.1f) {
			visc = 0.0f;
		}

		printf("viscosity %f\n", visc);
	}

	if (glfwGetKey('F') && action == GLFW_PRESS) {
		if (force == 3.0f) {
			force = 0.01f;
		} else
		if (force == 0.01f) {
			force = 0.1f;
		} else
		if (force == 0.1f) {
			force = 1.0f;
		} else 
		if (force == 1.0f) {
			force = 2.0f;
		}
		if (force == 2.0f) {
			force = 3.0f;
		}

		printf("force %f\n", force);
	}

	if (keyCode == 'B' && action == GLFW_PRESS) {
		colorMode++;
		if (colorMode == COLOR_MODE_MAX) {
			colorMode = 0;
		}

		if (colorMode == 0) {
			printf("color mode is RED\n");
		}

		if (colorMode == 1) {
			printf("color mode is GREEN\n");
		}

		if (colorMode == 2) {
			printf("color mode is BLUE\n");
		}
	}

	if (keyCode == 'S' && action == GLFW_PRESS) {
		SOLVE_STEPS += 2;
		if (SOLVE_STEPS >= 12) {
			SOLVE_STEPS = 2;
		}
		printf("solve steps %d\n", SOLVE_STEPS);
	}

	if (keyCode == 'C' && action == GLFW_PRESS) {
		clear();
		printf("clearing\n");
	}

	if (keyCode == 'H' && action == GLFW_PRESS) {
		printHelp();
	}
}

int main(int argc, const char * argv[]) {

  int gridSize = 150;

	NW = gridSize;
	NH = gridSize;
  dt = 0.2f;
  diff = 0.0f;
  visc = 0.0f;//00001f;
  force = 1.0f;
  source = 1.0f;

  printf("%s\n", argv[1]);
  printf("viscosity %f\n", visc);
  printf("force %f\n", force);
 
	printHelp();

  size = (NW+2)*(NH+2);//*(N+2); // cube
  
  solverCL.init(size);
  
	u = (float *)malloc(size * sizeof(float));
	v = (float *)malloc(size * sizeof(float));
	u_prev = (float *)malloc(size * sizeof(float));
	v_prev = (float *)malloc(size * sizeof(float));
	dens = (float *)malloc(size * sizeof(float));
	dens_prev	= (float *)malloc(size * sizeof(float));

  color_r_u = (float *)malloc(size * sizeof(float));
  color_r_v = (float *)malloc(size * sizeof(float));
  color_r_u_prev = (float *)malloc(size * sizeof(float));
  color_r_v_prev = (float *)malloc(size * sizeof(float));
  color_r = (float *)malloc(size * sizeof(float));
  color_r_prev = (float *)malloc(size * sizeof(float));

  color_g_u = (float *)malloc(size * sizeof(float));
  color_g_v = (float *)malloc(size * sizeof(float));
  color_g_u_prev = (float *)malloc(size * sizeof(float));
  color_g_v_prev = (float *)malloc(size * sizeof(float));
  color_g = (float *)malloc(size * sizeof(float));
  color_g_prev = (float *)malloc(size * sizeof(float));

  color_b_u = (float *)malloc(size * sizeof(float));
  color_b_v = (float *)malloc(size * sizeof(float));
  color_b_u_prev = (float *)malloc(size * sizeof(float));
  color_b_v_prev = (float *)malloc(size * sizeof(float));
  color_b = (float *)malloc(size * sizeof(float));
  color_b_prev = (float *)malloc(size * sizeof(float));
  
  GLboolean running;

	clear();

	if (argc > 1) {

		FreeImage_Initialise(true);

		unsigned int width, height = 0;
		imageData = LoadTexture(argv[1], &width, &height, &bpp);


		for (unsigned int i = 0; i < size; i++) {
			int components = bpp / 8.0f;
			BYTE b = imageData[0+(i*components)];
			BYTE g = imageData[1+(i*components)];
			BYTE r = imageData[2+(i*components)];
			
			color_r[i] = r / 255.0f;
			color_g[i] = g / 255.0f;
			color_b[i] = b / 255.0f;
		}
	}

  if(!glfwInit()) {
    fprintf( stderr, "Failed to initialize GLFW\n" );
    return EXIT_FAILURE;
  }
 
  if(!glfwOpenWindow(WINDOW_WIDTH, WINDOW_HEIGHT, 0, 0, 0, 0, 0, 0, GLFW_WINDOW)) {
    fprintf(stderr, "Failed to open GLFW window\n");
    glfwTerminate();
    return EXIT_FAILURE;
  }

  GLenum err = glewInit();
  if (GLEW_OK != err) {
    fprintf(stderr, "Error: %s\n", glewGetErrorString(err));
  }
  
  
  glfwSetKeyCallback(keyCallback);
  glfwSetWindowTitle("Fluid Simulation - Navier Stokes");
  glfwSwapInterval(1);
  
  GLuint shaderProgram = createShaderProgram();

  vertexSize = (NW + 1) * (NH + 1) * 2 * 4;

  float* positionVertexArray = (float *)malloc(vertexSize * sizeof(float));
  memset(positionVertexArray, 0, vertexSize * sizeof(float));

  float hw = 1.0f / NW;
  float hh = 1.0f / NH;

  unsigned int vertexIndex = 0;

  for (float i = 0 ; i <= NW ; i++) {
    float x = (i - 0.5f) * hw - 0.5;
    
    for (float j = 0 ; j <= NH ; j++) {
      float y = (j - 0.5f) * hh - 0.5;

      positionVertexArray[vertexIndex++] = x;
      positionVertexArray[vertexIndex++] = y;

      positionVertexArray[vertexIndex++] = x + hw;
      positionVertexArray[vertexIndex++] = y;

      positionVertexArray[vertexIndex++] = x + hw;
      positionVertexArray[vertexIndex++] = y + hh;

      positionVertexArray[vertexIndex++] = x;
      positionVertexArray[vertexIndex++] = y + hh;
    }
  }

  glGenVertexArrays(1, &gridVAO);
  glBindVertexArray(gridVAO);

  GLuint positionVBO = 0;
  glGenBuffers(1, &positionVBO);
  glBindBuffer(GL_ARRAY_BUFFER, positionVBO);
  glBufferData(GL_ARRAY_BUFFER, sizeof(float) * vertexSize, positionVertexArray, GL_STATIC_DRAW);

  glVertexAttribPointer(0, 2, GL_FLOAT, 0, 0, 0);
  glEnableVertexAttribArray(0);

  colorSize = (NW + 1) * (NH + 1) * 4 * 4;

  colorVertexArray = (float *)malloc(colorSize * sizeof(float));
  memset(colorVertexArray, 0, colorSize * sizeof(float));

  unsigned int colorIndex = 0;

  for (float i = 0 ; i <= NW ; i++) {
    for (float j = 0 ; j <= NH ; j++) {

      colorVertexArray[colorIndex++] = 0;
      colorVertexArray[colorIndex++] = 1;
      colorVertexArray[colorIndex++] = 0;
      colorVertexArray[colorIndex++] = 1;

      colorVertexArray[colorIndex++] = 0;
      colorVertexArray[colorIndex++] = 1;
      colorVertexArray[colorIndex++] = 0;
      colorVertexArray[colorIndex++] = 1;

      colorVertexArray[colorIndex++] = 0;
      colorVertexArray[colorIndex++] = 1;
      colorVertexArray[colorIndex++] = 0;
      colorVertexArray[colorIndex++] = 1;

      colorVertexArray[colorIndex++] = 0;
      colorVertexArray[colorIndex++] = 1;
      colorVertexArray[colorIndex++] = 0;
      colorVertexArray[colorIndex++] = 1;
    }
  }

  colorVBO = 0;
  glGenBuffers(1, &colorVBO);
  glBindBuffer(GL_ARRAY_BUFFER, colorVBO);
  glBufferData(GL_ARRAY_BUFFER, sizeof(float) * colorSize, colorVertexArray, GL_DYNAMIC_DRAW);

  glVertexAttribPointer(1, 4, GL_FLOAT, 0, 0, 0);
  glEnableVertexAttribArray(1);
  
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  forward = -2.0f;
  
  running = GL_TRUE;

  while (running) {
    
    input();
    update();
    
    glClearColor(0, 0, 0, 1);
    glClear(GL_COLOR_BUFFER_BIT);
    glViewport(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);
    
    glUseProgram(shaderProgram);
    
    glm::mat4 projection = glm::perspective(45.0f, (float)WINDOW_WIDTH / (float)WINDOW_HEIGHT, 0.1f, 100.0f);
    GLuint projectionLocation = glGetUniformLocation(shaderProgram, "Projection");
    glUniformMatrix4fv(projectionLocation, 1, false, glm::value_ptr(projection));
    
    glm::mat4 view(1.0f);
    view = glm::rotate(view, (yRotation / (float)M_PI) * 180.0f, glm::vec3(0, 1, 0));
    view = glm::translate(view, glm::vec3(right, up, forward));
  
    GLuint viewLocation = glGetUniformLocation(shaderProgram, "View");
    glUniformMatrix4fv(viewLocation, 1, false, glm::value_ptr(view));

    render();
    
    glfwSwapBuffers();
    
    running = !glfwGetKey(GLFW_KEY_ESC) && glfwGetWindowParam(GLFW_OPENED);
  }
  
  glfwTerminate();
  
  return 0;
}
