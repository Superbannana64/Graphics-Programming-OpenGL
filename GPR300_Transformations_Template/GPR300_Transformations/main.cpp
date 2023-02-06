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

#include <time.h>

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
float orbitRadius = 5.0f;
float orbitSpeed = 1.0f;

struct Transform //This is for the cubes
{
	glm::vec3 pos;
	glm::quat rot;
	glm::vec3 scale;
	glm::mat4 getModelMatrix()
	{
		glm::mat4 modelMatrix(1);
		glm::mat4 trans = glm::mat4 (
			1.0f, 0.0f, 0.0f, 0.0f,
			0.0f, 1.0f, 0.0f, 0.0f,
			0.0f, 0.0f, 1.0f, 0.0f,
			pos.x,pos.y,pos.z,1.0f
		);
		glm::mat4 scaling = glm::mat4(
			scale.x, 0.0f, 0.0f, 0.0f,
			0.0f, scale.y, 0.0f, 0.0f,
			0.0f, 0.0f, scale.z, 0.0f,
			0.0f, 0.0f, 0.0f, 1.0f
		);
		//glm::mat4 scaling = glm::scale(modelMatrix, scale);
		glm::mat3 rotateX = glm::mat3(
			1.0f, 0.0f, 0.0f,
			0.0f, cos(rot.x), sin(rot.x),
			0.0f, -sin(rot.x), cos(rot.x)
		);
		glm::mat3 rotateY = glm::mat3(
			cos(rot.x), 0.0f, -sin(rot.y),
			0.0f, 1.0f, 0.0f,
			sin(rot.y), 0.0f, cos(rot.x)
		);
		glm::mat3 rotateZ = glm::mat3(
			cos(rot.z), sin(rot.z), 0.0f,
			-sin(rot.z), cos(rot.z), 0.0f,
			0.0f, 0.0f, 1.0f
		);
		
		glm::mat4 rotate = rotateX * rotateY * rotateZ;

		modelMatrix = trans*rotate*scaling;

		return modelMatrix;
	}
};
Transform transform[5];
struct Camera
{
	glm::vec3 pos = glm::vec3(0, 0, 3);
	glm::vec3 target = glm::vec3(0, 0, 0);
	glm::quat rotation;
	float  fov = 90.0f;								//Gui
	float orthSize = 10.0f;							//Gui
	bool orthographic = true;						//Gui
	glm::mat4 getViewMatrix()
	{
		float camX = sin(glfwGetTime() * orbitSpeed) *orbitRadius;
		float camZ = cos(glfwGetTime() * orbitSpeed) *orbitRadius;

		pos = glm::vec3(camX, 0, camZ);

		glm::mat4 viewMatrix = glm::lookAt(pos, target, glm::vec3(0, 1, 0));

		return viewMatrix;
	}
	glm::mat4 getProjectionMatrix()
	{
		if (orthographic)
		{
			float width = orthSize;
			float height = orthSize;
			return ortho(width, height, 0.1f, 100.0f);
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
		/*glm::mat4 orth = glm::frustum(left, right, bottom, top, nearPlane, farPlane); */
		
		glm::mat4 orth = glm::mat4(
			2 / (right - left), 0.0f, 0.0f, 0.0f,
			0.0f, 2 / (top - bottom), 0.0f, 0.0f,
			0.0f, 0.0f, -2 / (farPlane - nearPlane), 0.0f,
			-(right + left) / (right - left), -(top + bottom) / (top - bottom), -(farPlane + nearPlane) / (farPlane - nearPlane), 1.0f
		);

		return orth;
	}
	glm::mat4 perspective(float fov, float a, float n, float f)
	{
		float g = (glm::pi<float>()/180);
		float t = n * tan((g*fov) / 2);
		float b = -t;
		float r = t * a;
		float l = -r;
		glm::mat4 persp = glm::mat4(
			(2*n)/(r-l), 0.0f, (r+l)/(r-l), 0.0f,
			0.0f, (2 * n) / (t - b), (t+b)/(t-b), 0.0f,
			0.0f, 0.0f, -((f + n) / (f - n)), -1.0f,
			0.0f, 0.0f, -((2 * f * n) / (f - n)), 0.0f
		);
		persp = glm::inverse(persp);
		return persp;
	}

};

Camera camera;

int i = 0;

int main() {
	if (!glfwInit()) {
		printf("glfw failed to init");
		return 1;
	}

	srand(time(NULL));

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

	for (i = 0; i < 5; i++)
	{
		float xPos = (-1) + static_cast <float> (rand()) / (static_cast <float> (RAND_MAX / (1 - (-1))));
		float yPos = (-1) + static_cast <float> (rand()) / (static_cast <float> (RAND_MAX / (1 - (-1))));
		float zPos = (-1) + static_cast <float> (rand()) / (static_cast <float> (RAND_MAX / (1 - (-1))));
		int xRot = rand() % 361;
		int yRot = rand() % 361;
		int zRot = rand() % 361;
		float xSca = rand() % 1 + 0.1;
		float ySca = rand() % 1 + 0.1;
		float zSca = rand() % 1 + 0.1;

		transform[i].pos = glm::vec3(xPos, yPos, zPos);
		transform[i].rot = glm::quat(glm::vec3(xRot, yRot, zRot));
		transform[i].scale = glm::vec3(xSca, ySca, zSca);

		shader.setMat4("_Model", transform[i].getModelMatrix());
		shader.setMat4("_View", camera.getViewMatrix());
		shader.setMat4("_Proj", camera.getProjectionMatrix());
	}

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
		shader.setVec3("vPos", glm::vec3(1.0, 1.0, 1.0));
		//glm::mat4 modelMatrix = transform->getModelMatrix(); //Identity
		//USE glm::vec3 EulerAngles(60, 45, 30) in the loop
		//For loop here that spawns multiple cubes

		shader.setMat4("_Model", transform[0].getModelMatrix());
		shader.setMat4("_View", camera.getViewMatrix());
		shader.setMat4("_Proj", camera.getProjectionMatrix());
		cubeMesh.draw();

		shader.setMat4("_Model", transform[1].getModelMatrix());
		cubeMesh.draw();

		shader.setMat4("_Model", transform[2].getModelMatrix());
		cubeMesh.draw();

		shader.setMat4("_Model", transform[3].getModelMatrix());
		cubeMesh.draw();

		shader.setMat4("_Model", transform[4].getModelMatrix());
		cubeMesh.draw();


		//Draw UI
		//Implement orbiting speed and radius
		ImGui::Begin("Settings");
		ImGui::SliderFloat("Orbit Radius", &orbitRadius, 0.0f, 10.0f);//Cam Pos
		ImGui::SliderFloat("Orbit Speed", &orbitSpeed, 0.0f, 10.0f);
		ImGui::SliderFloat("FOV", &camera.fov, 10.0f, 170.0f);
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