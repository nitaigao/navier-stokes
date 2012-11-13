#include <iostream>

#ifdef _APPLE_
#include <OpenGL/OpenGL.h>
#else
#include <GL/glew.h>
#endif

#include <GL/glfw.h>
#include <fstream>
#include <iostream>
#include <sstream>

#include "solver.h"

#include <glm/glm.hpp> //vec3, vec4, ivec4, mat4
#include <glm/gtc/matrix_transform.hpp> //translate, rotate, scale, perspective
#include <glm/gtc/type_ptr.hpp> //value_ptr

static int WINDOW_WIDTH = 800;
static int WINDOW_HEIGHT = 600;

static bool drawingDensity = true;

static int N;
static float dt, diff, visc;
static float force, source;

static float *u, *v, *u_prev, *v_prev;
static float *dens, *dens_prev;

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
	}
  
  int mx, my;
  glfwGetMousePos(&mx, &my);
  i = (int)((mx /(float)WINDOW_WIDTH)*N+1);
  j = (int)(((WINDOW_HEIGHT-my) /(float)WINDOW_HEIGHT)*N+1);

  if (glfwGetMouseButton(GLFW_MOUSE_BUTTON_RIGHT) || glfwGetKey('D')) {
    dens_prev[IX(i,j)] = source;
    dens_prev[IX(i+1,j)] = source;
    dens_prev[IX(i-1,j)] = source;
    dens_prev[IX(i,j+1)] = source;
    dens_prev[IX(i,j-1)] = source;
  }
  
  if (glfwGetMouseButton(GLFW_MOUSE_BUTTON_LEFT) || glfwGetKey('V')) {
    u[IX(i,j)] = force * (mx-omx);
    v[IX(i,j)] = force * (omy-my);
  }
    
  omx = mx;
	omy = my;
  
}

void update() {
  vel_step(N, u, v, u_prev, v_prev, visc, dt);
	dens_step(N, dens, dens_prev, u, v, diff, dt);
}

void drawVelocity() {
  int i, j;
	float x, y, h;
  
	h = 1.0f/N;
  
	glColor3f ( 1.0f, 1.0f, 1.0f );
	glLineWidth ( 1.0f );
  
	glBegin (GL_LINES);
  
  for ( i=1 ; i<=N ; i++ ) {
    x = (i-0.5f)*h;
    for ( j=1 ; j<=N ; j++ ) {
      y = (j-0.5f)*h;
      
      glVertex2f ( x, y );
      glVertex2f ( x+u[IX(i,j)], y+v[IX(i,j)] );
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
      
//      for (int k = 0; k <= N; k++) {
//        float z = (k - 0.5f) * h;
      
        float d00 = dens[IX(i,j)];
        float d01 = dens[IX(i,j+1)];
        float d10 = dens[IX(i+1,j)];
        float d11 = dens[IX(i+1,j+1)];
      
        glColor4f(1.0f, 1.0f, 1.0f, d00); glVertex2f (x, y);
        glColor4f(1.0f, 1.0f, 1.0f, d10); glVertex2f (x+h, y);
        glColor4f(1.0f, 1.0f, 1.0f, d11); glVertex2f (x+h, y+h);
        glColor4f(1.0f, 1.0f, 1.0f, d01); glVertex2f (x, y+h);
      

        
//      }
    }
  }

  
	glEnd ();
}

void render() {
  glTranslatef(0, 0, forward);
  
  if (drawingDensity) drawDensity();
  else drawVelocity();
  
//  glBegin(GL_TRIANGLES);
//    glVertex3f(-1.0f, -1.0f, 1.0f);
//    glVertex3f(1.0f, -1.0f, 1.0f);
//    glVertex3f(-1.0f, 1.0f, 1.0f);
//    glVertex3f(1.0f, 1.0f, 1.0f);
//    glVertex3f(-1.0f, 1.0f, 1.0f);
//    glVertex3f(1.0f, -1.0f, 1.0f);
//  glEnd();
}

std::string file2string(const std::string& filePath) {
  std::ifstream fileStream(filePath.c_str());
  std::stringstream textStream;
  textStream << fileStream.rdbuf();
  return textStream.str();
}

void printLog(GLuint obj) {
	int infologLength = 0;
	int maxLength;
  
	if(glIsShader(obj)) {
		glGetShaderiv(obj,GL_INFO_LOG_LENGTH,&maxLength);
  }
	else {
		glGetProgramiv(obj,GL_INFO_LOG_LENGTH,&maxLength);
  }
  
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
}

int main(int argc, const char * argv[]) {
  N = 64;
  dt = 0.1f;
  diff = 0.0f;
  visc = 0.0f;
  force = 1.00f;
  source = 10.0f;
  
  int size = (N+2)*(N+2);//*(N+2); // cube
  
	u = (float *)malloc(size*sizeof(float));
	v = (float *)malloc(size*sizeof(float));
	u_prev = (float *)malloc(size*sizeof(float));
	v_prev = (float *)malloc(size*sizeof(float));
	dens = (float *)malloc(size*sizeof(float));
	dens_prev	= (float *)malloc(size*sizeof(float));
  
  GLboolean running;
  
  for (int i = 0; i < size; i++) {
    if (i > size / 2.0f) {
      dens[i] = 1.0f;
    }
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
    
    glClearColor(1, 0, 0, 1);
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
