#include <iostream>
#include <cmath>
#include <fstream>
#include <vector>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/matrix_transform.hpp>

using namespace std;

struct VAO {
    GLuint VertexArrayID;
    GLuint VertexBuffer;
    GLuint ColorBuffer;

    GLenum PrimitiveMode;
    GLenum FillMode;
    int NumVertices;
};
typedef struct VAO VAO;

struct GLMatrices {
	glm::mat4 projection;
	glm::mat4 model;
	glm::mat4 view;
	GLuint MatrixID;
} Matrices;

GLuint programID;

/* Function to load Shaders - Use it as it is */
GLuint LoadShaders(const char * vertex_file_path,const char * fragment_file_path) {

	// Create the shaders
	GLuint VertexShaderID = glCreateShader(GL_VERTEX_SHADER);
	GLuint FragmentShaderID = glCreateShader(GL_FRAGMENT_SHADER);

	// Read the Vertex Shader code from the file
	std::string VertexShaderCode;
	std::ifstream VertexShaderStream(vertex_file_path, std::ios::in);
	if(VertexShaderStream.is_open())
	{
		std::string Line = "";
		while(getline(VertexShaderStream, Line))
			VertexShaderCode += "\n" + Line;
		VertexShaderStream.close();
	}

	// Read the Fragment Shader code from the file
	std::string FragmentShaderCode;
	std::ifstream FragmentShaderStream(fragment_file_path, std::ios::in);
	if(FragmentShaderStream.is_open()){
		std::string Line = "";
		while(getline(FragmentShaderStream, Line))
			FragmentShaderCode += "\n" + Line;
		FragmentShaderStream.close();
	}

	GLint Result = GL_FALSE;
	int InfoLogLength;

	// Compile Vertex Shader
	printf("Compiling shader : %s\n", vertex_file_path);
	char const * VertexSourcePointer = VertexShaderCode.c_str();
	glShaderSource(VertexShaderID, 1, &VertexSourcePointer , NULL);
	glCompileShader(VertexShaderID);

	// Check Vertex Shader
	glGetShaderiv(VertexShaderID, GL_COMPILE_STATUS, &Result);
	glGetShaderiv(VertexShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
	std::vector<char> VertexShaderErrorMessage(InfoLogLength);
	glGetShaderInfoLog(VertexShaderID, InfoLogLength, NULL, &VertexShaderErrorMessage[0]);
	fprintf(stdout, "%s\n", &VertexShaderErrorMessage[0]);

	// Compile Fragment Shader
	printf("Compiling shader : %s\n", fragment_file_path);
	char const * FragmentSourcePointer = FragmentShaderCode.c_str();
	glShaderSource(FragmentShaderID, 1, &FragmentSourcePointer , NULL);
	glCompileShader(FragmentShaderID);

	// Check Fragment Shader
	glGetShaderiv(FragmentShaderID, GL_COMPILE_STATUS, &Result);
	glGetShaderiv(FragmentShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
	std::vector<char> FragmentShaderErrorMessage(InfoLogLength);
	glGetShaderInfoLog(FragmentShaderID, InfoLogLength, NULL, &FragmentShaderErrorMessage[0]);
	fprintf(stdout, "%s\n", &FragmentShaderErrorMessage[0]);

	// Link the program
	fprintf(stdout, "Linking program\n");
	GLuint ProgramID = glCreateProgram();
	glAttachShader(ProgramID, VertexShaderID);
	glAttachShader(ProgramID, FragmentShaderID);
	glLinkProgram(ProgramID);

	// Check the program
	glGetProgramiv(ProgramID, GL_LINK_STATUS, &Result);
	glGetProgramiv(ProgramID, GL_INFO_LOG_LENGTH, &InfoLogLength);
	std::vector<char> ProgramErrorMessage( max(InfoLogLength, int(1)) );
	glGetProgramInfoLog(ProgramID, InfoLogLength, NULL, &ProgramErrorMessage[0]);
	fprintf(stdout, "%s\n", &ProgramErrorMessage[0]);

	glDeleteShader(VertexShaderID);
	glDeleteShader(FragmentShaderID);

	return ProgramID;
}

static void error_callback(int error, const char* description)
{
    fprintf(stderr, "Error: %s\n", description);
}

void quit(GLFWwindow *window)
{
    glfwDestroyWindow(window);
    glfwTerminate();
//    exit(EXIT_SUCCESS);
}

/* Generate VAO, VBOs and return VAO handle */
struct VAO* create3DObject (GLenum primitive_mode, int numVertices, const GLfloat* vertex_buffer_data, const GLfloat* color_buffer_data, GLenum fill_mode=GL_FILL)
{
    struct VAO* vao = new struct VAO;
    vao->PrimitiveMode = primitive_mode;
    vao->NumVertices = numVertices;
    vao->FillMode = fill_mode;

    // Create Vertex Array Object
    // Should be done after CreateWindow and before any other GL calls
    glGenVertexArrays(1, &(vao->VertexArrayID)); // VAO
    glGenBuffers (1, &(vao->VertexBuffer)); // VBO - vertices
    glGenBuffers (1, &(vao->ColorBuffer));  // VBO - colors

    glBindVertexArray (vao->VertexArrayID); // Bind the VAO
    glBindBuffer (GL_ARRAY_BUFFER, vao->VertexBuffer); // Bind the VBO vertices
    glBufferData (GL_ARRAY_BUFFER, 3*numVertices*sizeof(GLfloat), vertex_buffer_data, GL_STATIC_DRAW); // Copy the vertices into VBO
    glVertexAttribPointer(
                          0,                  // attribute 0. Vertices
                          3,                  // size (x,y,z)
                          GL_FLOAT,           // type
                          GL_FALSE,           // normalized?
                          0,                  // stride
                          (void*)0            // array buffer offset
                          );

    glBindBuffer (GL_ARRAY_BUFFER, vao->ColorBuffer); // Bind the VBO colors
    glBufferData (GL_ARRAY_BUFFER, 3*numVertices*sizeof(GLfloat), color_buffer_data, GL_STATIC_DRAW);  // Copy the vertex colors
    glVertexAttribPointer(
                          1,                  // attribute 1. Color
                          3,                  // size (r,g,b)
                          GL_FLOAT,           // type
                          GL_FALSE,           // normalized?
                          0,                  // stride
                          (void*)0            // array buffer offset
                          );

    return vao;
}

/* Generate VAO, VBOs and return VAO handle - Common Color for all vertices */
struct VAO* create3DObject (GLenum primitive_mode, int numVertices, const GLfloat* vertex_buffer_data, const GLfloat red, const GLfloat green, const GLfloat blue, GLenum fill_mode=GL_FILL)
{
    GLfloat* color_buffer_data = new GLfloat [3*numVertices];
    for (int i=0; i<numVertices; i++) {
        color_buffer_data [3*i] = red;
        color_buffer_data [3*i + 1] = green;
        color_buffer_data [3*i + 2] = blue;
    }

    return create3DObject(primitive_mode, numVertices, vertex_buffer_data, color_buffer_data, fill_mode);
}

/* Render the VBOs handled by VAO */
void draw3DObject (struct VAO* vao)
{
    // Change the Fill Mode for this object
    glPolygonMode (GL_FRONT_AND_BACK, vao->FillMode);

    // Bind the VAO to use
    glBindVertexArray (vao->VertexArrayID);

    // Enable Vertex Attribute 0 - 3d Vertices
    glEnableVertexAttribArray(0);
    // Bind the VBO to use
    glBindBuffer(GL_ARRAY_BUFFER, vao->VertexBuffer);

    // Enable Vertex Attribute 1 - Color
    glEnableVertexAttribArray(1);
    // Bind the VBO to use
    glBindBuffer(GL_ARRAY_BUFFER, vao->ColorBuffer);

    // Draw the geometry !
    glDrawArrays(vao->PrimitiveMode, 0, vao->NumVertices); // Starting from vertex 0; 3 vertices total -> 1 triangle
}

/**************************
 * Customizable functions *
 **************************/

struct rect{
	float x1;
	float x2;	
	float y1;
	float y2;
	float translation;	
	int c;
	bool alive;
};

struct receptacle{
  float x1;
  float x2;
  float translate;
  int c;
};

struct cannon{
  float x;
  float y;  
  float translate;
  float rotate; 
};

struct reflectors{
  float x1;
  float x2; 
  float y1;
  float y2;
  float m;
  float c;
};

struct rail{
  float x1;
  float x2; 
  float y1;
  float y2;
  float m;
  float c;	
};

int points = 0;
bool gameover = false;
int hit_count = 0;
float speed = 0.1;
float zoomFactor = 1.0;
float panFactor = 0;
double mouseX;
double mouseY;

rect boxes[15];
receptacle bucket[2];
cannon gun[2];
reflectors mirror[3];
rail bullet[10];

bool keystates_pressed[350];
bool mouse_keystates_pressed[8];
bool mouse_keystates_released[8];
bool keystates_released[350];

/* Executed when a regular key is pressed/released/held-down */
/* Prefered for Keyboard events */
void keyboard (GLFWwindow* window, int key, int scancode, int action, int mods)
{
     // Function is called first on GLFW_PRESS.
    if (action == GLFW_PRESS)
    {
        keystates_pressed[key] = true;
        keystates_released[key] = false;      
    }
    else if (action == GLFW_RELEASE)
    {
        keystates_pressed[key] = false;
        keystates_released[key] = true;
    }
}

/* Executed for character input (like in text boxes) */
void keyboardChar (GLFWwindow* window, unsigned int key)
{
	switch (key) {
		case 'Q':
		case 'q':
            quit(window);
            break;
		default:
			break;
	}
}

/* Executed when a mouse button is pressed/released */
void mouseButton (GLFWwindow* window, int button, int action, int mods)
{
  if (action == GLFW_PRESS)
  {
    mouse_keystates_pressed[button] = true;
    mouse_keystates_released[button] = false;
  }
  else
  {
    mouse_keystates_pressed[button] = false;
    mouse_keystates_released[button] = true;    
  }
} 

/* Executed when window is resized to 'width' and 'height' */
/* Modify the bounds of the screen here in glm::ortho or Field of View in glm::Perspective */
void reshapeWindow (GLFWwindow* window, int width, int height)
{
    int fbwidth=width, fbheight=height;
    /* With Retina display on Mac OS X, GLFW's FramebufferSize
     is different from WindowSize */
    glfwGetFramebufferSize(window, &fbwidth, &fbheight);

	GLfloat fov = 90.0f;

	// sets the viewport of openGL renderer
	glViewport (0, 0, (GLsizei) fbwidth, (GLsizei) fbheight);

	// set the projection matrix as perspective
	/* glMatrixMode (GL_PROJECTION);
	   glLoadIdentity ();
	   gluPerspective (fov, (GLfloat) fbwidth / (GLfloat) fbheight, 0.1, 500.0); */
	// Store the projection matrix in a variable for future use
    // Perspective projection for 3D views
    // Matrices.projection = glm::perspective (fov, (GLfloat) fbwidth / (GLfloat) fbheight, 0.1f, 500.0f);

    // Ortho projection for 2D views
    Matrices.projection = glm::ortho(-40.0f, 40.0f, -40.0f, 40.0f, 0.1f, 500.0f);
}

VAO *laser[10], *cannon_t1, *cannon_t2, *cannon_r1, *cannon_r2, *rectangle[40], *basket1, *basket2, *mirror1, *mirror2, *mirror3, *line;

// Creates the triangle object used in this sample code
void createCannon ()
{
  /* Define vertex array as used in glBegin (GL_TRIANGLES) */
  static const GLfloat vertex_buffer_data_t1 [] = {
    -40,6,0, // vertex 1
    -36,0,0, // vertex 2
    -40,-6,0 // vertex 3
  };

  static const GLfloat color_buffer_data_t1 [] = {
    0.3,0.3,0.3, // color 1
    0.3,0.3,0.3, // color 2
    0.3,0.3,0.3  // color 3
  };

  cannon_t1 = create3DObject(GL_TRIANGLES, 3, vertex_buffer_data_t1, color_buffer_data_t1, GL_FILL);

  static const GLfloat vertex_buffer_data_t2 [] = {
    -40,5,0, // vertex 1
    -36.5,0,0, // vertex 2
    -40,-5,0 // vertex 3
  };

  static const GLfloat color_buffer_data_t2 [] = {
    1,1,1, // color 1
    1,1,1, // color 2
    1,1,1  // color 3
  };

  // create3DObject creates and returns a handle to a VAO that can be used later
  cannon_t2 = create3DObject(GL_TRIANGLES, 3, vertex_buffer_data_t2, color_buffer_data_t2, GL_FILL);

  const GLfloat color_buffer_data_r [] = {
    0.2,0.2,0, // color 1
    0.2,0.2,0, // color 2
    0.2,0.2,0, // color 3

    0.2,0.2,0, // color 3
    0.2,0.2,0, // color 4
    0.2,0.2,0  // color 1
  };

  const GLfloat vertex_buffer_data_r1 [] = {
  	-39,2.2,0,  // vertex 1
  	-39,-2.2,0, // vertex 2
  	-34.5,-1.7,0, // vertex 3

  	-39,2.2,0,  // vertex 1
  	-34.5,-1.7,0, // vertex 3
  	-34.5,1.7,0  // vertex 4  	
  };

  gun[0].x = -39;
  gun[0].y = 0;
  gun[0].translate = 0.0;

  cannon_r1 = create3DObject(GL_TRIANGLES, 6, vertex_buffer_data_r1, color_buffer_data_r, GL_FILL);

  const GLfloat vertex_buffer_data_r2 [] = {
  	-35.5,1.2,0,  // vertex 1
  	-35.5,-1.2,0, // vertex 2
  	-31,-0.7,0, // vertex 3

  	-35.5,1.2,0,  // vertex 1
  	-31,-0.7,0, // vertex 3
  	-31,0.7,0  // vertex 4  	
  };

  gun[1].x = -31;
  gun[1].y = 0;
  gun[1].translate = 0.0;

  cannon_r2 = create3DObject(GL_TRIANGLES, 6, vertex_buffer_data_r2, color_buffer_data_r, GL_FILL);
}

// Creates the triangle object used in this sample code
void createLine ()
{
  /* Define vertex array as used in glBegin (GL_TRIANGLES) */
  static const GLfloat vertex_buffer_data [] = {
    -40,-36,0, // vertex 0
    40,-36,0 // vertex 1
  };

  static const GLfloat color_buffer_data [] = {
    0,0,0, // color 0
    0,0,0 // color 1
  };

  line = create3DObject(GL_LINES, 2, vertex_buffer_data, color_buffer_data, GL_LINE);
}

int y = 0;

void createRectangle (int i)
{
  int x,c,r,g,b;

  x = rand() % 50 - 20;
  y += rand() % 20;
  c = rand() % 3;

  // red = 1
  if (c == 1){
  	r = 0; g = -1; b = -1;
  }
  // green = 2
  else if (c == 2){
  	r = -1; g = 0; b = -1;
  }
  // black
  else{
  	r = -1; g = -1; b = -1;
  }

  boxes[i].x1 = x;
  boxes[i].x2 = x+1.5;
  boxes[i].y1 = 42+y;
  boxes[i].y2 = 44.5+y;
  boxes[i].c = c;
  boxes[i].alive = true;
  boxes[i].translation = 0.0f;

  // GL3 accepts only Triangles. Quads are not supported
  const GLfloat vertex_buffer_data [] = {
      boxes[i].x1, boxes[i].y1, 0,
      boxes[i].x2, boxes[i].y1, 0,
      boxes[i].x2, boxes[i].y2, 0,

      boxes[i].x2, boxes[i].y2, 0,
      boxes[i].x1, boxes[i].y2, 0,
      boxes[i].x1, boxes[i].y1, 0
    };

    const GLfloat color_buffer_data [] = {
      1+r,1+g,1+b, // color 1
      1+r,1+g,1+b, // color 2
      1+r,1+g,1+b, // color 3

      1+r,1+g,1+b, // color 3
      1+r,1+g,1+b, // color 4
      1+r,1+g,1+b // color 1
    };

    // create3DObject creates and returns a handle to a VAO that can be used later
    rectangle[i] = create3DObject(GL_TRIANGLES, 6, vertex_buffer_data, color_buffer_data, GL_FILL);    
}

void createLaser (float x1, float y1, float x2, float y2, float m, float c, int i)
{
  float t_x,t_y;

  t_x = 0.25*sin(m);
  t_y = 0.25*cos(m);

  if (x2 == 0 && y2 == 0)
  {
    x2 = x1+100*cos(m);
    y2 = y1+100*sin(m);
  }

  bullet[i].x1 = x1;
  bullet[i].x2 = x2;
  bullet[i].y1 = y1;
  bullet[i].y2 = y2;  
  bullet[i].m = m;
  bullet[i].c = c;  

  const GLfloat color_buffer_data [] = {
    0,0,1, // color 1
    0,0,1, // color 2
    0,0,1, // color 3

    0,0,1, // color 3
    0,0,1, // color 4
    0,0,1  // color 1
  };

  const GLfloat vertex_buffer_data [] = {
    x1-t_x,y1+t_y,0,
    x1+t_x,y1-t_y,0,
    x2+t_x,y2-t_y,0,

    x1-t_x,y1+t_y,0,
    x2-t_x,y2+t_y,0,
    x2+t_x,y2-t_y,0,
  };

  laser[i] = create3DObject(GL_TRIANGLES, 6, vertex_buffer_data, color_buffer_data, GL_FILL);
}

void createMirrors ()
{
  const GLfloat color_buffer_data [] = {
    0,0,0, // color 1
    0,0,0, // color 2
    0,0,0, // color 3

    0,0,0, // color 3
    0,0,0, // color 4
    0,0,0  // color 1
  };

  const GLfloat vertex_buffer_data_m1 [] = {
    -1,-2,0, // vertex 1
    4,-2+5*sqrt(3),0, // vertex 2
    -1-0.2,-2+sqrt(3)/5,0, // vertex 3

    -1-0.2,-2+sqrt(3)/5,0, // vertex 3    
    4-0.2,-2+5*sqrt(3)+sqrt(3)/5,0, // vertex 4
    4,-2+5*sqrt(3),0 // vertex 2
  };

  mirror[0].m = M_PI/3;
  mirror[0].c = -2 + tan(mirror[0].m);
  mirror[0].x1 = -1;
  mirror[0].x2 = 4;
  mirror[0].y1 = -2;
  mirror[0].y2 = -2+5*sqrt(3);

  mirror1 = create3DObject(GL_TRIANGLES, 6, vertex_buffer_data_m1, color_buffer_data, GL_FILL);    

  const GLfloat vertex_buffer_data_m2 [] = {
    28,-25,0, // vertex 1
    36,-25+8/sqrt(3),0, // vertex 2
    28-0.2,-25+sqrt(3)/5,0, // vertex 3

    28-0.2,-25+sqrt(3)/5,0, // vertex 3
    36-0.2,-25+8/sqrt(3)+sqrt(3)/5,0, // vertex 4
    36,-25+8/sqrt(3),0 // vertex 2
  };

  mirror[1].m = M_PI/6;
  mirror[1].c = -25 - 28*tan(mirror[1].m);
  mirror[1].x1 = 28;
  mirror[1].x2 = 36;
  mirror[1].y1 = -25;
  mirror[1].y2 = -25+8/sqrt(3);

  mirror2 = create3DObject(GL_TRIANGLES, 6, vertex_buffer_data_m2, color_buffer_data, GL_FILL);    

  const GLfloat vertex_buffer_data_m3 [] = {
    25,32,0, // vertex 1
    32,25,0, // vertex 2
    32-0.4/sqrt(2),25-0.4/sqrt(2),0, // vertex 3

    32-0.4/sqrt(2),25-0.4/sqrt(2),0, // vertex 3
    25-0.4/sqrt(2),32-0.4/sqrt(2),0, // vertex 4
    25,32,0 // vertex 1
  };

  mirror[2].m = (3*M_PI)/4;
  mirror[2].c = 32 - 25*tan(mirror[2].m);
  mirror[2].x1 = 25;
  mirror[2].x2 = 32;
  mirror[2].y1 = 32;
  mirror[2].y2 = 25;

  mirror3 = create3DObject(GL_TRIANGLES, 6, vertex_buffer_data_m3, color_buffer_data, GL_FILL);    
}

// Creates the rectangle object used in this sample code
void createBasket ()
{
  // GL3 accepts only Triangles. Quads are not supported
  const GLfloat vertex_buffer_data_b1 [] = {
    12.5,-40.0,0, // vertex 1
    19.5,-40.0,0, // vertex 2
    21.5,-36.5,0, // vertex 3

    21.5,-36.5,0, // vertex 3
    10.5,-36.5,0, // vertex 4
    12.5,-40.0,0 // vertex 1
  };

  const GLfloat color_buffer_data_b1 [] = {
    0,1,0, // color 1
    0,1,0, // color 2
    0,1,0, // color 3

    0,1,0, // color 3
    0,1,0, // color 4
    0,1,0, // color 1
  };

  bucket[0].x1 = 10.5;
  bucket[0].x2 = 21.5;
  bucket[0].c = 2;
  bucket[0].translate = 0.0;

  // create3DObject creates and returns a handle to a VAO that can be used later
  basket1 = create3DObject(GL_TRIANGLES, 6, vertex_buffer_data_b1, color_buffer_data_b1, GL_FILL);

  const GLfloat vertex_buffer_data_b2 [] = {
    -12.5,-40.0,0, // vertex 1
    -19.5,-40.0,0, // vertex 2
    -21.5,-36.5,0, // vertex 3

    -21.5,-36.5,0, // vertex 3
    -10.5,-36.5,0, // vertex 4
    -12.5,-40.0,0 // vertex 1
  };

  const GLfloat color_buffer_data_b2 [] = {
    1,0,0, // color 1
    1,0,0, // color 2
    1,0,0, // color 3

    1,0,0, // color 3
    1,0,0, // color 4
    1,0,0, // color 1
  };

  bucket[1].x2 = -10.5;
  bucket[1].x1 = -21.5;
  bucket[1].c = 1;
  bucket[1].translate = 0.0;

  // create3DObject creates and returns a handle to a VAO that can be used later
  basket2 = create3DObject(GL_TRIANGLES, 6, vertex_buffer_data_b2, color_buffer_data_b2, GL_FILL);  
}

void translateBaskets ()
{
  if (keystates_pressed[GLFW_KEY_LEFT_CONTROL] && keystates_pressed[GLFW_KEY_RIGHT] && !keystates_released[GLFW_KEY_RIGHT])
  {
    bucket[1].translate += 0.5;
    bucket[1].x1 += 0.5;
    bucket[1].x2 += 0.5;
  }
  else if (keystates_pressed[GLFW_KEY_LEFT_CONTROL] && keystates_pressed[GLFW_KEY_LEFT] && !keystates_released[GLFW_KEY_LEFT])
  {
    bucket[1].translate -= 0.5;
    bucket[1].x1 -= 0.5;
    bucket[1].x2 -= 0.5;
  }
  else if (keystates_pressed[GLFW_KEY_LEFT_ALT] && keystates_pressed[GLFW_KEY_RIGHT] && !keystates_released[GLFW_KEY_RIGHT])
  {
    bucket[0].translate += 0.5;
    bucket[0].x1 += 0.5;
    bucket[0].x2 += 0.5;
  }
  else if (keystates_pressed[GLFW_KEY_LEFT_ALT] && keystates_pressed[GLFW_KEY_LEFT] && !keystates_released[GLFW_KEY_LEFT])
  {
    bucket[0].translate -= 0.5;
    bucket[0].x1 -= 0.5;
    bucket[0].x2 -= 0.5;
  }
}

void translateCannon ()
{
   if (keystates_pressed[GLFW_KEY_S] && !keystates_released[GLFW_KEY_S])
   {
     gun[0].translate += 0.5;
     gun[0].y += 0.5;

     gun[1].translate += 0.5;
     gun[1].y += 0.5;
   }
   else if (keystates_pressed[GLFW_KEY_F] && !keystates_released[GLFW_KEY_F])
   {
     gun[0].translate -= 0.5;
     gun[0].y -= 0.5;

     gun[1].translate -= 0.5;
     gun[1].y -= 0.5;
   }
}

void rotateCannon ()
{
   if (keystates_pressed[GLFW_KEY_A] && !keystates_released[GLFW_KEY_A])
   {
     gun[0].rotate += 0.01;
     gun[1].rotate += 0.01;
   }
   else if (keystates_pressed[GLFW_KEY_D] && !keystates_released[GLFW_KEY_D])
   {
     gun[0].rotate -= 0.01;
     gun[1].rotate -= 0.01;
   }
}

void score ()
{
  for (int i=0;i<15;i++)
  {
    for (int j=0;j<2;j++)
    {
      if (boxes[i].x1 >= bucket[j].x1 && boxes[i].x2 <= bucket[j].x2 && boxes[i].y2 <= -36)
      {
        if (bucket[j].c == boxes[i].c)
        {
          points += 10;
          cout<<"Nice catch, you earned 10 points"<<endl;
		      cout<<"Score = "<<points<<endl;
        }
        else if (boxes[i].c == 0)
        {
          cout<<"You caught the black brick!"<<endl;
          cout<<"GAMEOVER"<<endl;
          gameover = true;
        }
        else
        {
          points -= 5;
          cout<<"Oops, wrong basket, you lose 5 points"<<endl;
		      cout<<"Score = "<<points<<endl;
        }
      }
    }
  }
}

void shoot(int i)
{
  int j, min;
  float x,y,m,c,x2,y2;

  m = tan(bullet[i].m);
  c = bullet[i].c;

  x2 = bullet[i].x2;
  y2 = bullet[i].y2;  
  min = -1;

  for (j=0;j<15;j++)
  {
 	x = boxes[j].x1;
  	y = m*x + c;
  	if (boxes[j].y1 <= y && boxes[j].y2 >= y && boxes[j].y1 < 40 && boxes[j].y2 > -36)
    {
      if (x < x2)
      {
        x2 = x;
        y2 = y;
        min = j;
      }
    }

  	x = boxes[j].x2;
  	y = m*x + c;
  	if (boxes[j].y1 <= y && boxes[j].y2 >= y && boxes[j].y1 < 40 && boxes[j].y2 > -36)
    {
      if (x < x2)
      {
        x2 = x;
        y2 = y;
        min = j;
      }
    }
  }

  if (min != -1)
  {
      if (boxes[min].c > 0)
      {
        hit_count ++;
        points += 10;
		    cout<<"Nice shot, you earned 10 points"<<endl;
        cout<<"Score = "<<points<<endl;
        if (hit_count >= 500)
        {
          cout<<"This was your 500th hit. Remember next time that you have only limited lasers."<<endl;
          cout<<"GAMEOVER"<<endl;
          gameover = true;
        }
        else if (hit_count >= 400)
          cout<<"Use your lasers wisely. You have only "<<hit_count<<" remaining"<<endl;        
      }
      else if (boxes[min].c == 0)
      {
        hit_count += 5;
        points -= 5;
        cout<<"Whoops you shot a black brick, you lose 5 points and 5 lasers"<<endl;
        cout<<"Score = "<<points<<endl;
        if (hit_count >= 500)
        {
          cout<<"This was your 500th hit. Remember next time that you have only limited lasers."<<endl;
          cout<<"GAMEOVER"<<endl;
          gameover = true;
        }
        else if (hit_count >= 400)
          cout<<"Use your lasers wisely. You have only "<<hit_count<<" remaining"<<endl;        
      }
      createLaser(bullet[i].x1,bullet[i].y1,x2,y2,bullet[i].m,bullet[i].c,i);
      for (int k = i+1;k<10;k++)
        laser[k] = NULL;
      createRectangle(min);
  }
}

void block_speed()
{
  if (keystates_pressed[GLFW_KEY_N])
  {
    speed += 0.1;
    if (speed > 0.5)
      speed = 0.5;
  }
  if (keystates_pressed[GLFW_KEY_M])
  {
    speed -= 0.1;
    if (speed < 0.1)
      speed = 0.1;
  }
}

void zoom()
{
  if (keystates_pressed[GLFW_KEY_UP])
  {
    zoomFactor += 0.1;
    if (zoomFactor > 2)
      zoomFactor = 2;
  }
  if (keystates_pressed[GLFW_KEY_DOWN])
  {
    zoomFactor -= 0.1;
    if (zoomFactor < 1)
      zoomFactor = 1.0;      
    if (40/zoomFactor + panFactor > 40)
      panFactor = 40 - 40/zoomFactor;
    if (-40/zoomFactor + panFactor < -40)
      panFactor = -40 + 40/zoomFactor;
  }
  if (zoomFactor == 1)
    panFactor = 0;
}

void pan()
{
  if (keystates_pressed[GLFW_KEY_RIGHT] && zoomFactor != 1)
  {
    panFactor += 1;
    if (40/zoomFactor + panFactor > 40)
      panFactor = 40 - 40/zoomFactor;
  }
  if (keystates_pressed[GLFW_KEY_LEFT] && zoomFactor != 1)
  {
    panFactor -= 1;
    if (-40/zoomFactor + panFactor < -40)
      panFactor = -40 + 40/zoomFactor;
  }
}

int mouse_basket = -1, mouse_shoot = -1, mouse_cannon = -1;
double m_x,m_y;

void mouse_movement (GLFWwindow* window)
{
  if (mouse_basket == -1 && mouse_cannon == -1 && mouse_shoot == -1 && mouse_keystates_pressed[GLFW_MOUSE_BUTTON_LEFT] && !mouse_keystates_released[GLFW_MOUSE_BUTTON_LEFT])
  {
    glfwGetCursorPos(window, &mouseX, &mouseY);
    m_x = (mouseX*2*40/600) - 40.0;
    m_y = 40.0 - (mouseY*2*40/600);

    for (int i=0; i<2; i++)
    {
      if (m_x >= bucket[i].x1 && m_x <= bucket[i].x2 && m_y >= -40 && m_y <= -36)
      {
        mouse_basket = i;
        m_x = bucket[i].x1 + 5.5;
        break;
      }
    }

    if (m_x >= -40 && m_x <= -39 + 8*cos(gun[0].rotate) && m_y >= gun[0].y - 5 && m_y <= gun[0].y + 5)
    {
      m_y = gun[0].y;
      mouse_cannon = 1;
    }

    if (mouse_cannon == -1 && mouse_basket == -1)
      mouse_shoot = 1;
  }
  else if (mouse_keystates_released[GLFW_MOUSE_BUTTON_LEFT])
  {
    glfwGetCursorPos(window, &mouseX, &mouseY);
    mouseX = (mouseX*2*40/600) - 40.0;
    mouseY = 40.0 - (mouseY*2*40/600);

    if (mouse_cannon != -1 && mouse_shoot == -1 && mouse_basket == -1)
    {
        gun[0].y = mouseY;
        gun[1].y = mouseY;
        gun[0].translate += mouseY - m_y;
        gun[1].translate += mouseY - m_y;
        mouse_cannon = -1;
    }

    if (mouse_shoot != -1 && mouse_basket == -1 && mouse_cannon == -1)
    {
        float angle;
        angle = atan((mouseY - gun[0].y)/(mouseX - gun[0].x));
        gun[0].rotate = angle;
        gun[1].rotate = angle;
        mouse_shoot = -1;
    }

    if (mouse_basket != -1)
    {
      if (mouseX <= 38 && mouseX >= -38 && mouseY <= -36 && mouseY >= -40)
      {
        bucket[mouse_basket].x1 = mouseX - 5.5;
        bucket[mouse_basket].x2 = mouseX + 5.5;       
        bucket[mouse_basket].translate += mouseX - m_x;
      }
      mouse_basket = -1;
    }
  }
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
  if (yoffset == 1)
  {
    zoomFactor += 0.1;
    if (zoomFactor > 2)
      zoomFactor = 2;  
  }
  else if (yoffset == -1)
  {
    zoomFactor -= 0.1;
    if (zoomFactor < 1)
      zoomFactor = 1.0;      
    if (40/zoomFactor + panFactor > 40)
      panFactor = 40 - 40/zoomFactor;
    if (-40/zoomFactor + panFactor < -40)
      panFactor = -40 + 40/zoomFactor;
  }
  if (zoomFactor == 1)
    panFactor = 0;
}

double last_update_time = glfwGetTime(), current_time;

/* Render the scene with openGL */
/* Edit this function according to your assignment */
void draw ()
{
  // clear the color and depth in the frame buffer
  glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  // use the loaded shader program
  // Don't change unless you know what you are doing
  glUseProgram (programID);

  // Eye - Location of camera. Don't change unless you are sure!!
  glm::vec3 eye (0, 0, 1);
  // Target - Where is the camera looking at.  Don't change unless you are sure!!
  glm::vec3 target (0, 0, 0);
  // Up - Up vector defines tilt of camera.  Don't change unless you are sure!!
  glm::vec3 up (0, 1, 0);

  Matrices.projection = glm::ortho(-40.0f/zoomFactor + panFactor, 40.0f/zoomFactor + panFactor, -40.0f/zoomFactor, 40.0f/zoomFactor, 0.1f, 500.0f);
  // Compute Camera matrix (view)
  Matrices.view = glm::lookAt( eye, target, up ); // Rotating Camera for 3D
  //  Don't change unless you are sure!!
  //Matrices.view = glm::lookAt(glm::vec3(0,0,3), glm::vec3(0,0,0), glm::vec3(0,1,0)); // Fixed camera for 2D (ortho) in XY plane

  // Compute ViewProject matrix as view/camera might not be changed for this frame (basic scenario)
  //  Don't change unless you are sure!!
  glm::mat4 VP = Matrices.projection * Matrices.view;

  // Send our transformation to the currently bound shader, in the "MVP" uniform
  // For each model you render, since the MVP will be different (at least the M part)
  //  Don't change unless you are sure!!
  glm::mat4 MVP;	// MVP = Projection * View * Model

  // Load identity to model matrix
  Matrices.model = glm::mat4(1.0f);

  /* Render your scene */

  // //  Don't change unless you are sure!!
  glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);

  // Pop matrix to undo transformations till last push matrix instead of recomputing model matrix
  // glPopMatrix ();

  for (int i=0;i<15;i++)
  {
    // glTranslatef
    Matrices.model = glm::mat4(1.0f);
    glm::mat4 translateRectangle = glm::translate (glm::vec3(0, boxes[i].translation, 0));
    Matrices.model *= translateRectangle;
    MVP = VP * Matrices.model;
    glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);

  	boxes[i].y1 -= speed;
  	boxes[i].y2 -= speed;
    boxes[i].translation -= speed;

    // draw3DObject draws the VAO given to it using current MVP matrix
    draw3DObject(rectangle[i]);
  }

  Matrices.model = glm::mat4(1.0f);
  glm::mat4 translateBasket1 = glm::translate (glm::vec3(bucket[0].translate, 0, 0));
  Matrices.model *= translateBasket1;
  MVP = VP * Matrices.model;
  glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
  draw3DObject(basket1);

  Matrices.model = glm::mat4(1.0f);
  glm::mat4 translateBasket2 = glm::translate (glm::vec3(bucket[1].translate, 0, 0));
  Matrices.model *= translateBasket2;
  MVP = VP * Matrices.model;
  glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);

  draw3DObject(basket2);

  Matrices.model = glm::mat4(1.0f);
  MVP = VP * Matrices.model;
  glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);

  draw3DObject(line);
  draw3DObject(mirror1);  
  draw3DObject(mirror2);
  draw3DObject(mirror3);
  
  Matrices.model = glm::mat4(1.0f);
  glm::mat4 translateCannonTriangles = glm::translate (glm::vec3(0, gun[0].translate, 0));
  Matrices.model *= translateCannonTriangles;
  MVP = VP * Matrices.model;
  glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
  draw3DObject(cannon_t1);
  draw3DObject(cannon_t2);

  Matrices.model = glm::mat4(1.0f);
  glm::mat4 translateCannons = glm::translate (glm::vec3(0 - gun[0].translate*sin(gun[0].rotate), gun[0].translate*cos(gun[0].rotate), 0));
  glm::mat4 translateCannons_to_origin = glm::translate (glm::vec3(-1*gun[0].x,-1*gun[0].y, 0));    
  glm::mat4 rotateCannons = glm::rotate((float)(gun[0].rotate), glm::vec3(0,0,1));  // rotate about vector (1,0,0)
  glm::mat4 translateCannons_back = glm::translate (glm::vec3(gun[0].x, gun[0].y, 0));
  glm::mat4 cannonTransform =  translateCannons * translateCannons_back * rotateCannons * translateCannons_to_origin;
  Matrices.model *= cannonTransform;
  MVP = VP * Matrices.model;
  glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
  draw3DObject(cannon_r1);
  draw3DObject(cannon_r2);

  Matrices.model = glm::mat4(1.0f);
  MVP = VP * Matrices.model;
  glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);

  current_time = glfwGetTime();
  int hit = -1;
  if ((current_time - last_update_time) >= 1)
  {  
	  // atleast 0.5s elapsed since last frame
	  if (keystates_pressed[GLFW_KEY_SPACE])
	  {
    	// do something every 0.5 seconds ..
		  float x1,x2,y1,y2,x,y,m1,m2,c1,c2;
		  int flag = 0, count = 0, j;
		  bool check[3] = {false,false,false};

		  m1 = gun[1].rotate;
		  x1 = gun[0].x + (gun[1].x - gun[0].x)*cos(m1);
		  y1 = gun[1].y + (gun[1].x - gun[0].x)*sin(m1);

	  	while (flag == 0)
	  	{
        for (int i=0;i<3;i++)
        {
          if (check[i] == false)
          {
            c1 = y1 - tan(m1)*x1;
            m2 = mirror[i].m;
            c2 = mirror[i].c;

            x2 = (c2-c1)/(tan(m1)-tan(m2));
            y2 = tan(m1)*x2 + c1;

            if (x2 > mirror[i].x1 && x2 < mirror[i].x2)
            {
              check[i] = true;
              createLaser (x1,y1,x2,y2,m1,c1,count);
              m1 = 2*m2 - m1;
              x1 = x2;
              y1 = y2;
              c1 = y2 - tan(m1)*x2;
              count++;
            }
            else if (count > 0)
            {
              createLaser (x1,y1,0,0,m1,c1,count);    
              flag = 1;
              break;            
            }
          }
        }
	  		if (count == 0 && flag == 0)
	  		{
		        createLaser (x1,y1,0,0,m1,c1,count);
		        flag = 1;		
	  		}
      }
	  }
    last_update_time = current_time;
  }

  current_time = glfwGetTime();
  if (current_time - last_update_time < 0.2)
  {
      for (int i=0;i<10;i++)
      {
        if (laser[i] != NULL)
        {
          shoot(i);
          draw3DObject(laser[i]);
        }
        else
         break;
      }
  }
  else
  {
    for (int i=0;i<10;i++)
      laser[i] = NULL;    
  }

}

/* Initialise glfw window, I/O callbacks and the renderer to use */
/* Nothing to Edit here */
GLFWwindow* initGLFW (int width, int height)
{
    GLFWwindow* window; // window desciptor/handle

    glfwSetErrorCallback(error_callback);
    if (!glfwInit()) {
//        exit(EXIT_FAILURE);
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    window = glfwCreateWindow(width, height, "Sample OpenGL 3.3 Application", NULL, NULL);

    if (!window) {
        glfwTerminate();
//        exit(EXIT_FAILURE);
    }

    glfwMakeContextCurrent(window);
    gladLoadGLLoader((GLADloadproc) glfwGetProcAddress);
    glfwSwapInterval( 1 );

    /* --- register callbacks with GLFW --- */

    /* Register function to handle window resizes */
    /* With Retina display on Mac OS X GLFW's FramebufferSize
     is different from WindowSize */
    glfwSetFramebufferSizeCallback(window, reshapeWindow);
    glfwSetWindowSizeCallback(window, reshapeWindow);

    /* Register function to handle window close */
    glfwSetWindowCloseCallback(window, quit);

    /* Register function to handle keyboard input */
    glfwSetKeyCallback(window, keyboard);      // general keyboard input
    glfwSetCharCallback(window, keyboardChar);  // simpler specific character handling

    /* Register function to handle mouse click */
    glfwSetMouseButtonCallback(window, mouseButton);  // mouse button clicks
    glfwSetScrollCallback(window, scroll_callback);
    return window;
}

/* Initialize the OpenGL rendering properties */
/* Add all the models to be created here */
void initGL (GLFWwindow* window, int width, int height)
{
    /* Objects should be created before any other gl function and shaders */
	// Create the models
  // Generate the VAO, VBOs, vertices data & copy into the array buffer

  createCannon ();
  createBasket ();

  for (int i=0; i<15;i++)
    createRectangle (i);

  createLine ();
  createMirrors ();

	// Create and compile our GLSL program from the shaders
	programID = LoadShaders( "Sample_GL.vert", "Sample_GL.frag" );
	// Get a handle for our "MVP" uniform
	Matrices.MatrixID = glGetUniformLocation(programID, "MVP");

	reshapeWindow (window, width, height);

    // Background color of the scene
	glClearColor (1.0f, 1.0f, 1.0f, 0.0f); // R, G, B, A
	glClearDepth (1.0f);

	glEnable (GL_DEPTH_TEST);
	glDepthFunc (GL_LEQUAL);

    cout << "VENDOR: " << glGetString(GL_VENDOR) << endl;
    cout << "RENDERER: " << glGetString(GL_RENDERER) << endl;
    cout << "VERSION: " << glGetString(GL_VERSION) << endl;
    cout << "GLSL: " << glGetString(GL_SHADING_LANGUAGE_VERSION) << endl;
}

int main (int argc, char** argv)
{
  srand(time(NULL));

	int width = 600;
	int height = 600;
 	int count = 0;

  for (int i=0;i<350;i++)
  {
    keystates_pressed[i] = false;
    keystates_released[i] = false;
  }

  GLFWwindow* window = initGLFW(width, height);

	initGL (window, width, height);

    /* Draw in loop */
  
  cout<<"=========================================="<<endl;
  cout<<"Start playing, best of luck!"<<endl;
  cout<<"Your score is 0"<<endl;

  while (!glfwWindowShouldClose(window) && !gameover) {

    mouse_movement (window);
    translateBaskets ();
    score ();
    translateCannon ();
    rotateCannon ();

    for (int i=0;i<15;i++)
    {
        if (boxes[i].y2 < -36.0 && boxes[i].alive == true)
        {
           count++;
           boxes[i].alive = false;
           createRectangle (i);
           if (count == 15)
           {
              y = 0;
              count = 0;
           }
        }
    }

    block_speed ();
    zoom();
    pan();

     // OpenGL Draw commands
    draw();

      // Swap Frame Buffer in double buffering
    glfwSwapBuffers(window);

      // Poll for Keyboard and mouse events
    glfwPollEvents();

        // Control based on time (Time based transformation like 5 degrees rotation every 0.5s)
    }

    if (points <= 0)
    	cout<<"Be more careful next time"<<endl;
    else if (points <= 100 && points > 0)
    	cout<<"Not bad, try harder next time"<<endl;
    else if (points <= 200 && points > 100)
    	cout<<"Well done. Good job"<<endl;
    else if (points <= 300 && points > 200)
    	cout<<"You're a good player already"<<endl;
    else if (points <= 400 && points > 300)
    	cout<<"Great score! Cheers"<<endl;
    else if (points <= 500 && points > 400)
    	cout<<"Whohoho! Amazing game"<<endl;
    else if (points > 500)
    	cout<<"You're a legend!"<<endl;

	cout<<"=========================================="<<endl;
    cout<<"Your final score is "<<points<<endl;

    glfwTerminate();
//    exit(EXIT_SUCCESS);
}