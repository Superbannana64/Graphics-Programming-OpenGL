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
	glm::vec3 pos;
	glm::quat rot;
	glm::vec3 scale;
	glm::mat4 getModelMatrix()
	{
		glm::mat4 modelMatrix(1);
		glm::mat4 trans = glm::translate(modelMatrix, pos);
		glm::mat4 scaling = glm::scale(modelMatrix, scale);
		glm::mat4 rotate = glm::mat4_cast(rot);
		
		modelMatrix = trans*rotate*scaling;

		return modelMatrix;
	}
};
Transform transform[1];
struct Camera
{
	glm::vec3 pos = glm::vec3(0, 0, -1);
	glm::quat rotation;
	float  fov = 10.0f;								//Gui
	float orthSize = 1.0f;							//Gui
	bool orthographic = true;						//Gui
	glm::mat4 getViewMatrix()
	{
		glm::mat4 viewMatrix = glm::lookAt(pos, glm::vec3(0, 0, 0), glm::vec3(0, 1, 0));
		return viewMatrix;
	}
	glm::mat4 getProjectionMatrix()
	{
		if (orthographic)
		{
			float right = orthSize/2;
			float top = orthSize / 2;
			return ortho(right, top, 0.1f, 100.0f);
		}
		else
		{
			float aspectRatio = SCREEN_WIDTH / SCREEN_HEIGHT;
			//return glm::mat4(1);
			return perspective(fov, aspectRatio, 0.1f, 100.0f);
		}
		
	}
	glm::mat4 ortho(float right, float top, float nearPlane, float farPlane)
	{
		float bottom = -top;
		float left = -right;
		glm::mat4 orth = glm::frustum(left, right, bottom, top, nearPlane, farPlane);
		
		return orth;
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
		shader.setVec3("vPos", glm::vec3(1.0, 2.0, 2.0));
		//glm::mat4 modelMatrix = transform->getModelMatrix(); //Identity
		
		//For loop here that spawns multiple cubes
		//put the cubeMesh.draw() and setMat4 (model, transform[i].getModelMatrix());
		transform[0].pos = glm::vec3(-1.0, 0.0, 0.0);
		glm::vec3 EulerAngles(90, 45, 30);
		transform[0].rot = glm::quat(EulerAngles);
		transform[0].scale = glm::vec3(0.5, 0.5, 0.5);

		shader.setMat4("_Model", transform[0].getModelMatrix());
		shader.setMat4("_View", camera.getViewMatrix());
		shader.setMat4("_Proj", camera.getProjectionMatrix());

		cubeMesh.draw();

		//Draw UI
		ImGui::Begin("Settings"); //Need orbit rad, speed, fov, orth h, orth toggle
		ImGui::SliderFloat("Orbit Radius", &exampleSliderFloat, 0.0f, 10.0f);//Cam Pos
		ImGui::SliderFloat("Orbit Speed", &exampleSliderFloat, 0.0f, 10.0f);
		ImGui::SliderFloat("FOV", &camera.fov, 15.5f, 18.0f);
		ImGui::SliderFloat("Orthographic Height", &camera.orthSize, 1.0f, 50.0f);
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