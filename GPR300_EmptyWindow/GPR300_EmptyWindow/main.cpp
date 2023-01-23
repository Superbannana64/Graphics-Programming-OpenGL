#include "GL/glew.h"
#include "GLFW/glfw3.h"

#include <stdio.h>

//void keyboardCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
void resizeFrameBufferCallback(GLFWwindow* window, int width, int height);

//TODO: Vertex shader source code
//Where transformations are preformed later
const char* vertexShaderSource =
"#version 450												\n"
"layout (location = 0) in vec3 vPos;						\n"
"layout (location = 1) in vec4 vColor;						\n"
"out vec4 Color;											\n"
"uniform float _Time;										\n"
"uniform int _MoveType;                                     \n"
"void main(){												\n"
"   float time = 0;                                         \n"
"   int moveType = _MoveType;                               \n"
"   if(moveType == 1){time = cos(_Time);}                   \n"
"   else {time = abs(sin(_Time));}                          \n"
"	Color = vColor;											\n"
"	gl_Position = vec4(vPos.x, vPos.y+time, vPos.z,1.0);    \n"
"}															\0";

//TODO: Fragment shader source code
const char* fragmentShaderSource = 
"#version 450												\n"
"out vec4 FragColor;										\n"
"in vec4 Color;												\n"
"uniform float _Time;										\n"
"void main(){												\n"
"   float t = abs(sin(_Time));								\n"
"	FragColor = Color * t;									\n"
"}															\0";

//TODO: Vertex data array
const float vertexData[] = { 
	//x		y		z		//Colors (RGBA)
	//Triangle 1
	-0.75,	-0.75,	+0.0,	1.0, 0.0, 0.0, 1.0,	//Bottom Left
	+0.0,	-0.75,	+0.0,	0.0, 1.0, 0.0, 1.0,	//Bottom Right
	+0.0,	+0.0,	+0.0,	0.0, 0.0, 1.0, 1.0,	//Top Center

	//Triangle 2
	+0.0,	-0.75,	+0.0,	0.0, 1.0, 0.0, 1.0,	//Bottom Left
	+0.75,	-0.75,	+0.0,	1.0, 0.0, 0.0, 1.0,	//Bottom Right
	+0.0,	+0.0,	+0.0,	0.0, 0.0, 1.0, 1.0,	//Top Center
};

int main() {
	if (!glfwInit()) {
		printf("glfw failed to init");
		return 1;
	}

	GLFWwindow* window = glfwCreateWindow(640, 480, "OpenGLExample", 0, 0);
	glfwMakeContextCurrent(window);

	if (glewInit() != GLEW_OK) {
		printf("glew failed to init");
		return 1;
	}

	glfwSetFramebufferSizeCallback(window, resizeFrameBufferCallback);


	//TODO: Create and compile vertex shader
	GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
	glCompileShader(vertexShader);

	//TODO: Get vertex shader compilation status and output info log
	GLint successVertex;
	glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &successVertex);
	if (!successVertex)
	{
		GLchar infoLog[512];
		glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
		printf("Failed to compile vertex shader: %s", infoLog);
	}
	
	//TODO: Create and compile fragment shader
	GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
	glCompileShader(fragmentShader);

	//TODO: Get fragment shader compilation status and output info log
	GLint successFragment;
	glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &successFragment);
	if (!successFragment)
	{
		GLchar infoLog[512];
		glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
		printf("Failed to compile fragment shader: %s", infoLog);
	}

	//TODO: Create shader program
	GLuint shaderProgram = glCreateProgram();

	//TODO: Attach vertex and fragment shaders to shader program
	glAttachShader(shaderProgram, vertexShader);
	glAttachShader(shaderProgram, fragmentShader);

	//TODO: Link shader program
	glLinkProgram(shaderProgram);

	//TODO: Check for link status and output errors
	GLint successProgram;
	glGetProgramiv(shaderProgram, GL_LINK_STATUS, &successProgram);
	if (!successProgram)
	{
		GLchar infoLog[512];
		glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
		printf("Failed to link shader program: %s", infoLog);
	}

	//TODO: Delete vertex + fragment shader objects
	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);

	//TODO: Create and bind Vertex Array Object (VAO)
	// create new vertex array object
	GLuint vertexArrayObject;
	glGenVertexArrays(1, &vertexArrayObject);

	//Bind new VAO so that function calls can modify it
	glBindVertexArray(vertexArrayObject);

	//TODO: Create and bind Vertex Buffer Object (VBO), fill with vertexData
	//Generate buffer with an ID
	GLuint vertexBufferObject;
	glGenBuffers(1, &vertexBufferObject);

	//Binds buffer to the GL_ARRAY_BUFFER target. Bound for vertext attributes
	glBindBuffer(GL_ARRAY_BUFFER, vertexBufferObject);

	//Create new data store.
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertexData), vertexData, GL_STATIC_DRAW);

	//TODO: Define vertex attribute layout
	
	//Position
	//Define hpw vertex data should be interpreted by vertex shader.
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 7, (const void*)0);
	//first 0 is for vertex data id (which is 0 bc we have 1 data set), 3 is for vec size (hense vec3)
	
	//Enable this vertext attribute (this is required)
	glEnableVertexAttribArray(0);
	//number is the data id like above's

	//Color
	glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(float) * 7, (const void*)(sizeof(float)*3));
	glEnableVertexAttribArray(1);

	//the first 0 could be automated if you have multiple things to draw
	//but not 100% automated bc different vertex sizes

	while (!glfwWindowShouldClose(window)) {
		glClearColor(0.2f, 0.3f, 0.6f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);

		//Get time
		float time = (float)glfwGetTime();

		//TODO:Use shader program
		glUseProgram(shaderProgram);
		//This allows to use different shader programs

		//use time in the shader program (pulses color)
		GLint timeLoc = glGetUniformLocation(shaderProgram, "_Time");
		glUniform1f(timeLoc, time);

		GLint moveType = glGetUniformLocation(shaderProgram, "_MoveType");
		glUniform1f(moveType, 0);
		
		//TODO: Draw triangle (3 indices!)
		glDrawArrays(GL_TRIANGLES, 0, 3);

		//Seperate triangle movement
		glUniform1f(moveType, 1);
		glDrawArrays(GL_TRIANGLES, 3, 3);
		//Parameter of what u draw, index of vertex, and num of verticies

		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	glfwTerminate();
	return 0;
}

void resizeFrameBufferCallback(GLFWwindow* window, int width, int height)
{
	glViewport(0, 0, width, height);
}

