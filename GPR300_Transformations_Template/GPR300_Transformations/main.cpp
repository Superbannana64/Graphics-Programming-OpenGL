#include "GL/glew.h"
#include "GLFW/glfw3.h"

#include <glm/glm.hpp>
#include <glm/matrix.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <stdio.h>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include "imgui/imgui.h"
#include "imgui/imgui_impl_glfw.h"
#include "imgui/imgui_impl_opengl3.h"

#include "EW/Shader.h"
#include "EW/ShapeGen.h"

void resizeFrameBufferCallback(GLFWwindow* window, int width, int height);
void keyboardCallback(GLFWwindow* window, int keycode, int scancode, int action, int mods);

float lastFrameTime;
float deltaTime;

int SCREEN_WIDTH = 1080;
int SCREEN_HEIGHT = 720;

double prevMouseX;
double prevMouseY;
bool firstMouseInput = false;

/* Button to lock / unlock mouse
* 1 = right, 2 = middle
* Mouse will start locked. Unlock it to use UI
* */
const int MOUSE_TOGGLE_BUTTON = 1;
const float MOUSE_SENSITIVITY = 0.1f;

glm::vec3 bgColor = glm::vec3(0);
float exampleSliderFloat = 0.0f;

struct Transform //This is for the cubes
{
	glm::vec4 pos;
	glm::quat rot;
	glm::vec4 scale;
	glm::mat4 getModelMatrix()
	{
		glm::mat4 modelMatrix(1);
		//modelMatrix = modelMatrix * position;

		return modelMatrix;
	}
};
Transform transform[1];
struct Camera
{
	glm::vec3 position;
	glm::quat rotation;
	float fov;								//Gui
	float orthSize;							//Gui
	bool orthographic;						//Gui
	glm::mat4 getViewMatrix()
	{
		return glm::mat4(1);
	}
	glm::mat4 getProjectionMatrix()
	{
		if (orthographic)
		{
			return ortho();
		}
		else
		{
			return perspective();
		}
	}
	glm::mat4 ortho(float height, float aspectRatio, float nearPlane, float farPlane)
	{
		return glm::mat4(1);
	}
	glm::mat4 perspective(float fov, float aspectRatio, float nearPlane, float farPlane)
	{
		glm::mat4 per = glm::perspective(fov, aspectRatio, nearPlane, farPlane);
		return per;
	}
};

Camera camera;

int main() {
	if (!glfwInit()) {
		printf("glfw failed to init");
		return 1;
	}

	GLFWwindow* window = glfwCreateWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Transformations", 0, 0);
	glfwMakeContextCurrent(window);

	if (glewInit() != GLEW_OK) {
		printf("glew failed to init");
		return 1;
	}

	glfwSetFramebufferSizeCallback(window, resizeFrameBufferCallback);
	glfwSetKeyCallback(window, keyboardCallback);

	// Setup UI Platform/Renderer backends
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGui_ImplGlfw_InitForOpenGL(window, true);
	ImGui_ImplOpenGL3_Init();

	//Dark UI theme.
	ImGui::StyleColorsDark();

	Shader shader("shaders/vertexShader.vert", "shaders/fragmentShader.frag"); //Where the shaders are

	MeshData cubeMeshData;
	createCube(1.0f, 1.0f, 1.0f, cubeMeshData);

	Mesh cubeMesh(&cubeMeshData);

	//Enable back face culling
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);

	//Enable blending
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	//Enable depth testing
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);

	while (!glfwWindowShouldClose(window)) {
		glClearColor(bgColor.r,bgColor.g,bgColor.b, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();

		float time = (float)glfwGetTime();
		deltaTime = time - lastFrameTime;
		lastFrameTime = time;

		//Draw
		shader.use();
		shader.setFloat("_Time", time); //the unit thing with time in last program
		
		//glm::mat4 modelMatrix = transform->getModelMatrix(); //Identity
		
		//For loop here that spawns multiple cubes
		//put the cubeMesh.draw() and setMat4 (model, transform[i].getModelMatrix());
		transform[1].pos = glm::vec4(-.75, 0.0, 0.0, 1.0);
		transform[1].rot = glm::quat(0.707107, 0.707107, 0.00, 0.000);
		transform[1].scale = glm::vec4(1.0, 1.0, 1.0, 1.0);

		shader.setMat4("_Model", transform[1].getModelMatrix());

		cubeMesh.draw();

		//Draw UI
		ImGui::Begin("Settings"); //Need orbit rad, speed, fov, orth h, orth toggle
		ImGui::SliderFloat("Orbit Radius", &exampleSliderFloat, 0.0f, 10.0f);
		ImGui::SliderFloat("Orbit Speed", &exampleSliderFloat, 0.0f, 10.0f);
		ImGui::SliderFloat("FOV", &camera.fov, 10.0f, 120.0f);
		ImGui::SliderFloat("Orthographic Height", &camera.orthSize, 10.0f, 50.0f);
		ImGui::Checkbox("Orthographic", &camera.orthographic);
		ImGui::End();

		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
		glfwPollEvents();

		glfwSwapBuffers(window);
	}

	glfwTerminate();
	return 0;
}

void resizeFrameBufferCallback(GLFWwindow* window, int width, int height)
{
	glViewport(0, 0, width, height);
	SCREEN_WIDTH = width;
	SCREEN_HEIGHT = height;
}

void keyboardCallback(GLFWwindow* window, int keycode, int scancode, int action, int mods)
{
	if (keycode == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
		glfwSetWindowShouldClose(window, true);
	}
}