#if defined(_APPLE_)
  #include <OpenGL/OpenGL.h>
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

#include <GL/glfw.h>
#include <fstream>
#include <iostream>
#include <sstream>

#include <glm/glm.hpp> //vec3, vec4, ivec4, mat4
#include <glm/gtc/matrix_transform.hpp> //translate, rotate, scale, perspective
#include <glm/gtc/type_ptr.hpp> //value_ptr

#include "solver.h"

static int WINDOW_WIDTH = 800;
static int WINDOW_HEIGHT = 600;

static bool drawingDensity = true;

static int N;
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

void input() {
  
  // keys
  
  float speed = -1.0f;
  
  if (glfwGetKey('W') == GLFW_PRESS) {
    forward += -speed * dt;
  }
  
  if (glfwGetKey('S') == GLFW_PRESS) {
    forward -= -speed * dt;
  }
  
  if (glfwGetKey('A') == GLFW_PRESS) {
    right -= speed * dt;
  }
  
  if (glfwGetKey('D') == GLFW_PRESS) {
    right += speed * dt;
  }
  
  if (glfwGetKey('E') == GLFW_PRESS) {
    up += speed * dt;
  }
  
  if (glfwGetKey('Q') == GLFW_PRESS) {
    up -= speed * dt;
  }

  if (glfwGetKey(GLFW_KEY_LEFT)) {
    yRotation -= speed * dt * dt;
  }
  
  if (glfwGetKey(GLFW_KEY_RIGHT)) {
    yRotation += speed * dt * dt;
  }
  
  // mouse

  int i, j, size = (N+2)*(N+2);
  for (int i=0 ; i<size ; i++ ) {
		u_prev[i] = v_prev[i] = dens_prev[i] = 0.0f;
    color_r_u_prev[i] = color_r_v_prev[i] = color_r_prev[i] = 0.0f;
    color_g_u_prev[i] = color_g_v_prev[i] = color_g_prev[i] = 0.0f;
    color_b_u_prev[i] = color_b_v_prev[i] = color_b_prev[i] = 0.0f;
	}
  
  int mx, my;
  glfwGetMousePos(&mx, &my);
  i = (int)((mx /(float)WINDOW_WIDTH)*N+1);
  j = (int)(((WINDOW_HEIGHT-my) /(float)WINDOW_HEIGHT)*N+1);

  if (glfwGetMouseButton(GLFW_MOUSE_BUTTON_LEFT) || glfwGetKey('V')) {
    u[IX(i,j)] = force * (mx-omx);
    v[IX(i,j)] = force * (omy-my);

    color_r_u[IX(i,j)] = force * (mx-omx);
    color_r_v[IX(i,j)] = force * (omy-my);

    color_g_u[IX(i,j)] = force * (mx-omx);
    color_g_v[IX(i,j)] = force * (omy-my);

    color_b_u[IX(i,j)] = force * (mx-omx);
    color_b_v[IX(i,j)] = force * (omy-my);
  //}
  

  //if (glfwGetMouseButton(GLFW_MOUSE_BUTTON_RIGHT) || glfwGetKey('D')) {
    dens_prev[IX(i,j)] = source;
    dens_prev[IX(i+1,j)] = source;
    dens_prev[IX(i-1,j)] = source;
    dens_prev[IX(i,j+1)] = source;
    dens_prev[IX(i,j-1)] = source;

    if (colorMode == RED) {
      color_r[IX(i, j)] = 1.0f;
      color_r[IX(i + 1, j)] = 1.0f;
      color_r[IX(i - 1, j)] = 1.0f;
      color_r[IX(i, j + 1)] = 1.0f;
      color_r[IX(i, j - 1)] = 1.0f;
    }

    if (colorMode == GREEN) {
      color_g[IX(i, j)] = 1.0f;
      color_g[IX(i + 1, j)] = 1.0f;
      color_g[IX(i - 1, j)] = 1.0f;
      color_g[IX(i, j + 1)] = 1.0f;
      color_g[IX(i, j - 1)] = 1.0f;
    }

    if (colorMode == BLUE) {
      color_b[IX(i, j)] = 1.0f;
      color_b[IX(i + 1, j)] = 1.0f;
      color_b[IX(i - 1, j)] = 1.0f;
      color_b[IX(i, j + 1)] = 1.0f;
      color_b[IX(i, j - 1)] = 1.0f;
    }
  }
    
  omx = mx;
	omy = my;
}

void update() {
  vel_step(N, u, v, u_prev, v_prev, visc, dt);
	dens_step(N, dens, dens_prev, u, v, diff, dt);
  
  vel_step(N, color_r_u, color_r_v, color_r_u_prev, color_r_v_prev, visc, dt);
  dens_step(N, color_r, color_r_prev, color_r_u, color_r_v, diff, dt);

  vel_step(N, color_g_u, color_g_v, color_g_u_prev, color_g_v_prev, visc, dt);
  dens_step(N, color_g, color_g_prev, color_g_u, color_g_v, diff, dt);

  vel_step(N, color_b_u, color_b_v, color_b_u_prev, color_b_v_prev, visc, dt);
  dens_step(N, color_b, color_b_prev, color_b_u, color_b_v, diff, dt);
}

void drawVelocity() {
  float h = 1.0f / N;
  
	glColor3f(1.0f, 1.0f, 1.0f);
	glLineWidth(1.0f);
  
	glBegin (GL_LINES);
  
  for (int i = 1; i <= N ; i++) {
    
    float x = (i-0.5f)*h - 0.5;
    
    for (int j=1; j <= N ; j++) {
    
      float y = (j-0.5f)*h - 0.5;
      
      glVertex2f(x, y);
      glVertex2f(x+u[IX(i,j)], y+v[IX(i,j)]);
    }
  }
  
	glEnd ();
}

void drawDensity() {
  float h = 1.0f/N;
  
	glBegin (GL_QUADS);
  
  for (int i = 0 ; i <= N ; i++) {
    float x = (i - 0.5f) * h - 0.5;
    
    for (int j = 0 ; j <= N ; j++) {
      float y = (j - 0.5f) * h - 0.5;

      int i00 = IX(i, j);
      float d00 = dens[i00];
      float c00r = color_r[i00];
      float c00g = color_g[i00];
      float c00b = color_b[i00];
      glColor4f(c00r, c00g, c00b, 1.0f); glVertex2f(x, y);
      
      int i10 = IX(i + 1, j);
      float d10 = dens[i10];
      float c10r = color_r[i10];
      float c10g = color_g[i10];
      float c10b = color_b[i10];
      glColor4f(c10r, c10g, c10b, 1.0f); glVertex2f(x + h, y);
      
      int i11 = IX(i + 1, j + 1);
      float d11 = dens[i11];
      float c11r = color_r[i11];
      float c11g = color_g[i11];
      float c11b = color_b[i11];
      glColor4f(c11r, c11g, c11b, 1.0f); glVertex2f(x + h, y + h);      

      int i01 = IX(i, j + 1);
      float d01 = dens[i01];
      float c01r = color_r[i01];
      float c01g = color_g[i01];
      float c01b = color_b[i01];
      glColor4f(c01r, c01g, c01b, 1.0f); glVertex2f(x, y + h);
    }
  }
	glEnd ();
}

void render() {
  glTranslatef(0, 0, forward);
  
  if (drawingDensity) drawDensity();
  else drawVelocity();
}

std::string file2string(const std::string& filePath) {
  std::ifstream fileStream(filePath.c_str());
  std::stringstream textStream;
  textStream << fileStream.rdbuf();
  return textStream.str();
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
  glLinkProgram(shaderProgram);
  
  printLog(shaderProgram);
  
  return shaderProgram;
}

void keyCallback(int keyCode, int action) {
  if (keyCode == 'V' && action == GLFW_PRESS) {
    drawingDensity = !drawingDensity;
  }
  if (keyCode == 'B' && action == GLFW_PRESS) {
    colorMode++;
    if (colorMode == COLOR_MODE_MAX) {
      colorMode = 0;
    }
  }
  if (keyCode == 'C' && action == GLFW_PRESS) {
    drawingDensity = !drawingDensity;
  }
}

int main(int argc, const char * argv[]) {
  N = 64;
  dt = 0.1f;
  diff = 0.0f;
  visc = 0.0f;
  force = 0.1f;
  source = 1.0f;
  
  int size = (N+2)*(N+2);//*(N+2); // cube
  
	u = (float *)malloc(size * sizeof(float));
	memset(u, 0, size * sizeof(float));

	v = (float *)malloc(size * sizeof(float));
	memset(v, 0, size * sizeof(float));

	u_prev = (float *)malloc(size * sizeof(float));
	memset(u_prev, 0, size * sizeof(float));

	v_prev = (float *)malloc(size * sizeof(float));
	memset(v_prev, 0, size * sizeof(float));

	dens = (float *)malloc(size * sizeof(float));
	memset(dens, 0, size * sizeof(float));

	dens_prev	= (float *)malloc(size * sizeof(float));
	memset(dens_prev, 0, size * sizeof(float));


//--

  color_r_u = (float *)malloc(size * sizeof(float));
  memset(color_r_u, 0, size * sizeof(float));

  color_r_v = (float *)malloc(size * sizeof(float));
  memset(color_r_v, 0, size * sizeof(float));

  color_r_u_prev = (float *)malloc(size * sizeof(float));
  memset(color_r_u_prev, 0, size * sizeof(float));

  color_r_v_prev = (float *)malloc(size * sizeof(float));
  memset(color_r_v_prev, 0, size * sizeof(float));

  color_r = (float *)malloc(size * sizeof(float));
  memset(color_r, 0, size * sizeof(float));

  color_r_prev = (float *)malloc(size * sizeof(float));
  memset(color_r_prev, 0, size * sizeof(float));

//--

  color_g_u = (float *)malloc(size * sizeof(float));
  memset(color_g_u, 0, size * sizeof(float));

  color_g_v = (float *)malloc(size * sizeof(float));
  memset(color_g_v, 0, size * sizeof(float));

  color_g_u_prev = (float *)malloc(size * sizeof(float));
  memset(color_g_u_prev, 0, size * sizeof(float));

  color_g_v_prev = (float *)malloc(size * sizeof(float));
  memset(color_g_v_prev, 0, size * sizeof(float));

  color_g = (float *)malloc(size * sizeof(float));
  memset(color_g, 0, size * sizeof(float));

  color_g_prev = (float *)malloc(size * sizeof(float));
  memset(color_g_prev, 0, size * sizeof(float));

  //--

  color_b_u = (float *)malloc(size * sizeof(float));
  memset(color_b_u, 0, size * sizeof(float));

  color_b_v = (float *)malloc(size * sizeof(float));
  memset(color_b_v, 0, size * sizeof(float));

  color_b_u_prev = (float *)malloc(size * sizeof(float));
  memset(color_b_u_prev, 0, size * sizeof(float));

  color_b_v_prev = (float *)malloc(size * sizeof(float));
  memset(color_b_v_prev, 0, size * sizeof(float));

  color_b = (float *)malloc(size * sizeof(float));
  memset(color_b, 0, size * sizeof(float));

  color_b_prev = (float *)malloc(size * sizeof(float));
  memset(color_b_prev, 0, size * sizeof(float));
  
  GLboolean running;
  
  for (int i = 0; i < size; i++) {
    //if (i > (float)size / 2.0f) {
      dens[i] = 1.0f;
      color_r[i] = 126/255.0f;
      color_g[i] = 11/255.0f;
      color_b[i] = 128/255.0f;
    //}
  }

  // Initialise GLFW
  if(!glfwInit()) {
    fprintf( stderr, "Failed to initialize GLFW\n" );
    return EXIT_FAILURE;
  }
 
  // Open OpenGL window
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
  
  glfwSetWindowTitle("Navier Stokes");
  
  // Enable vertical sync (on cards that support it)
  glfwSwapInterval(1);
  
  GLuint shaderProgram = createShaderProgram();
  
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  forward = -1.0f;
  
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
    
    // Swap buffers
    glfwSwapBuffers();
    
    // Check if the ESC key was pressed or the window was closed
    running = !glfwGetKey(GLFW_KEY_ESC) && glfwGetWindowParam(GLFW_OPENED);
  }
  
  // Close OpenGL window and terminate GLFW
  glfwTerminate();
  
  return 0;
}
