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
#include "EW/EwMath.h"
#include "EW/Camera.h"
#include "EW/Mesh.h"
#include "EW/Transform.h"
#include "EW/ShapeGen.h"

void processInput(GLFWwindow* window);
void resizeFrameBufferCallback(GLFWwindow* window, int width, int height);
void keyboardCallback(GLFWwindow* window, int keycode, int scancode, int action, int mods);
void mouseScrollCallback(GLFWwindow* window, double xoffset, double yoffset);
void mousePosCallback(GLFWwindow* window, double xpos, double ypos);
void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods);

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
const float CAMERA_MOVE_SPEED = 5.0f;
const float CAMERA_ZOOM_SPEED = 3.0f;

Camera camera((float)SCREEN_WIDTH / (float)SCREEN_HEIGHT);

//Directional
glm::vec3 bgColor = glm::vec3(0);
glm::vec3 dLightColor = glm::vec3(1.0f);
glm::vec3 dLightPosition = glm::vec3(-0.2f, -1.0f, -0.3f);
float dLightIntensity = 1;

//Point
glm::vec3 pLightColor = glm::vec3(1.0f);
glm::vec3 pLightPosition = glm::vec3(-0.2f, -1.0f, -0.3f);
float pLightIntensity = 1;

//Spot
glm::vec3 sLightColor = glm::vec3(1.0f);
glm::vec3 sLightPosition = glm::vec3(-0.2f, -1.0f, -0.3f);
float sLightIntensity = 1;

//Materials
glm::vec3 objectColor = glm::vec3(1.0f);
float matAmbient = 1.0f;
float matDiffuse = 1.0f;
float matSpecular = 1.0f;
float matShiny = 32.0f;

bool wireFrame = false;

int main() {
	if (!glfwInit()) {
		printf("glfw failed to init");
		return 1;
	}

	GLFWwindow* window = glfwCreateWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Lighting", 0, 0);
	glfwMakeContextCurrent(window);

	if (glewInit() != GLEW_OK) {
		printf("glew failed to init");
		return 1;
	}

	glfwSetFramebufferSizeCallback(window, resizeFrameBufferCallback);
	glfwSetKeyCallback(window, keyboardCallback);
	glfwSetScrollCallback(window, mouseScrollCallback);
	glfwSetCursorPosCallback(window, mousePosCallback);
	glfwSetMouseButtonCallback(window, mouseButtonCallback);

	//Hide cursor
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	// Setup UI Platform/Renderer backends
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGui_ImplGlfw_InitForOpenGL(window, true);
	ImGui_ImplOpenGL3_Init();

	//Dark UI theme.
	ImGui::StyleColorsDark();

	//Used to draw shapes. This is the shader you will be completing.
	Shader litShader("shaders/defaultLit.vert", "shaders/defaultLit.frag");

	//Used to draw light sphere
	Shader unlitShader("shaders/defaultLit.vert", "shaders/unlit.frag");

	ew::MeshData cubeMeshData;
	ew::createCube(1.0f, 1.0f, 1.0f, cubeMeshData);
	ew::MeshData sphereMeshData;
	ew::createSphere(0.5f, 64, sphereMeshData);
	ew::MeshData cylinderMeshData;
	ew::createCylinder(1.0f, 0.5f, 64, cylinderMeshData);
	ew::MeshData planeMeshData;
	ew::createPlane(1.0f, 1.0f, planeMeshData);

	ew::Mesh cubeMesh(&cubeMeshData);
	ew::Mesh sphereMesh(&sphereMeshData);
	ew::Mesh planeMesh(&planeMeshData);
	ew::Mesh cylinderMesh(&cylinderMeshData);

	//Enable back face culling
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);

	//Enable blending
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	//Enable depth testing
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);

	//Initialize shape transforms
	ew::Transform cubeTransform;
	ew::Transform sphereTransform;
	ew::Transform planeTransform;
	ew::Transform cylinderTransform;
	ew::Transform lightTransform;

	cubeTransform.position = glm::vec3(-2.0f, 0.0f, 0.0f);
	sphereTransform.position = glm::vec3(0.0f, 0.0f, 0.0f);

	planeTransform.position = glm::vec3(0.0f, -1.0f, 0.0f);
	planeTransform.scale = glm::vec3(10.0f);

	cylinderTransform.position = glm::vec3(2.0f, 0.0f, 0.0f);

	lightTransform.scale = glm::vec3(0.5f);
	lightTransform.position = glm::vec3(0.0f, 5.0f, 0.0f);

	while (!glfwWindowShouldClose(window)) {
		processInput(window);
		glClearColor(bgColor.r,bgColor.g,bgColor.b, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();

		float time = (float)glfwGetTime();
		deltaTime = time - lastFrameTime;
		lastFrameTime = time;

		//Draw
		litShader.use();
		litShader.setMat4("_Projection", camera.getProjectionMatrix());
		litShader.setMat4("_View", camera.getViewMatrix());
		litShader.setVec3("_ViewPos", camera.getPosition());

		//Directional Light
		litShader.setVec3("dLight.direction", dLightPosition);
		litShader.setVec3("dLight.lightColor", dLightColor);
		litShader.setFloat("dLight.intensity", dLightIntensity);
		//Point Light
		litShader.setVec3("pLight.position", pLightPosition);
		litShader.setVec3("pLight.lightColor", pLightColor);
		litShader.setFloat("pLight.intensity", pLightIntensity);
		//Spot Light
		litShader.setVec3("sLight.position", camera.getPosition());
		litShader.setVec3("sLight.direction", camera.getForward());
		litShader.setVec3("sLight.lightColor", sLightColor);
		litShader.setFloat("sLight.intensity", sLightIntensity);
		litShader.setFloat("sLight.cutoff", glm::cos(glm::radians(12.5f)));
		litShader.setFloat("sLight.outerCutOff", glm::cos(glm::radians(17.5f)));


		//Materials Make them all ImGUI interfacable
		litShader.setVec3("material.objectColor", objectColor);
		litShader.setFloat("material.ambient", matAmbient);
		litShader.setFloat("material.diffuse", matDiffuse);
		litShader.setFloat("material.specular", matSpecular);
		litShader.setFloat("material.shininess", matShiny);

		//Directional light
		litShader.setVec3("dLight.ambient", glm::vec3(0.2f, 0.2f, 0.2f));
		litShader.setVec3("dLight.diffuse", glm::vec3(0.5f, 0.5f, 0.5f));
		litShader.setVec3("dLight.specular", glm::vec3(1.0f, 1.0f, 1.0f));
		//Point Light
		litShader.setVec3("pLight.ambient", glm::vec3(0.2f, 0.2f, 0.2f));
		litShader.setVec3("pLight.diffuse", glm::vec3(0.5f, 0.5f, 0.5f));
		litShader.setVec3("pLight.specular", glm::vec3(1.0f, 1.0f, 1.0f));
		litShader.setFloat("pLight.constant", 1.0f);
		litShader.setFloat("pLight.linear", 0.09f);
		litShader.setFloat("pLight.quadratic", 0.032f);
		//Spot Light
		litShader.setVec3("sLight.ambient", glm::vec3(0.2f, 0.2f, 0.2f));
		litShader.setVec3("sLight.diffuse", glm::vec3(0.5f, 0.5f, 0.5f));
		litShader.setVec3("sLight.specular", glm::vec3(1.0f, 1.0f, 1.0f));
		litShader.setFloat("sLight.constant", 1.0f);
		litShader.setFloat("sLight.linear", 0.09f);
		litShader.setFloat("sLight.quadratic", 0.032f);


		//Draw cube
		litShader.setMat4("_Model", cubeTransform.getModelMatrix());
		cubeMesh.draw();

		//Draw sphere
		litShader.setMat4("_Model", sphereTransform.getModelMatrix());
		sphereMesh.draw();

		//Draw cylinder
		litShader.setMat4("_Model", cylinderTransform.getModelMatrix());
		cylinderMesh.draw();

		//Draw plane
		litShader.setMat4("_Model", planeTransform.getModelMatrix());
		planeMesh.draw();

		//Draw light as a small sphere using unlit shader, ironically.
		unlitShader.use();
		unlitShader.setMat4("_Projection", camera.getProjectionMatrix());
		unlitShader.setMat4("_View", camera.getViewMatrix());
		unlitShader.setMat4("_Model", lightTransform.getModelMatrix());
		unlitShader.setVec3("_Color", dLightColor);
		sphereMesh.draw();

		//Draw UI
		ImGui::Begin("Directional Light Settings");

		ImGui::ColorEdit3("Light Color", &dLightColor.r);
		ImGui::DragFloat3("Light Direction", (float*)&dLightPosition);
		ImGui::DragFloat("Intensity", &dLightIntensity, 0.1f, 0.0f, 5.0f);
		ImGui::End();

		ImGui::Begin("Point Light Settings");
		ImGui::ColorEdit3("Light Color", &pLightColor.r);
		ImGui::DragFloat3("Light Direction", (float*)&pLightPosition);
		ImGui::DragFloat("Intensity", &pLightIntensity, 0.1f, 0.0f, 5.0f);
		//Attentuation
		ImGui::End();

		ImGui::Begin("Spot Light Settings");
		ImGui::ColorEdit3("Light Color", &sLightColor.r);
		ImGui::DragFloat3("Light Direction", (float*)&sLightPosition);
		ImGui::DragFloat("Intensity", &sLightIntensity, 0.1f, 0.0f, 5.0f);
		//Attenuation
		//min angle
		//max angle
		ImGui::End();

		//Material Settings
		ImGui::Begin("Matterial Settings");
		ImGui::ColorEdit3("Material Color", &objectColor.r);
		ImGui::DragFloat("AmbientK", &matAmbient, 0.001f, 0.0f, 1.0f);
		ImGui::DragFloat("DiffuseK", &matDiffuse, 0.001f, 0.0f, 1.0f);
		ImGui::DragFloat("SpecularK", &matSpecular, 0.001f, 0.0f, 1.0f);
		ImGui::DragFloat("Shininess", &matShiny, 1.0f, 1.0f, 512.0f);
		ImGui::End();

		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
		glfwPollEvents();

		glfwSwapBuffers(window);
	}

	glfwTerminate();
	return 0;
}
//Author: Eric Winebrenner
void resizeFrameBufferCallback(GLFWwindow* window, int width, int height)
{
	SCREEN_WIDTH = width;
	SCREEN_HEIGHT = height;
	camera.setAspectRatio((float)SCREEN_WIDTH / SCREEN_HEIGHT);
	glViewport(0, 0, width, height);
}
//Author: Eric Winebrenner
void keyboardCallback(GLFWwindow* window, int keycode, int scancode, int action, int mods)
{
	if (keycode == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
		glfwSetWindowShouldClose(window, true);
	}
	//Reset camera
	if (keycode == GLFW_KEY_R && action == GLFW_PRESS) {
		camera.setPosition(glm::vec3(0, 0, 5));
		camera.setYaw(-90.0f);
		camera.setPitch(0.0f);
		firstMouseInput = false;
	}
	if (keycode == GLFW_KEY_1 && action == GLFW_PRESS) {
		wireFrame = !wireFrame;
		glPolygonMode(GL_FRONT_AND_BACK, wireFrame ? GL_LINE : GL_FILL);
	}
}
//Author: Eric Winebrenner
void mouseScrollCallback(GLFWwindow* window, double xoffset, double yoffset)
{
	if (abs(yoffset) > 0) {
		float fov = camera.getFov() - (float)yoffset * CAMERA_ZOOM_SPEED;
		camera.setFov(fov);
	}
}
//Author: Eric Winebrenner
void mousePosCallback(GLFWwindow* window, double xpos, double ypos)
{
	if (glfwGetInputMode(window, GLFW_CURSOR) != GLFW_CURSOR_DISABLED) {
		return;
	}
	if (!firstMouseInput) {
		prevMouseX = xpos;
		prevMouseY = ypos;
		firstMouseInput = true;
	}
	float yaw = camera.getYaw() + (float)(xpos - prevMouseX) * MOUSE_SENSITIVITY;
	camera.setYaw(yaw);
	float pitch = camera.getPitch() - (float)(ypos - prevMouseY) * MOUSE_SENSITIVITY;
	pitch = glm::clamp(pitch, -89.9f, 89.9f);
	camera.setPitch(pitch);
	prevMouseX = xpos;
	prevMouseY = ypos;
}
//Author: Eric Winebrenner
void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods)
{
	//Toggle cursor lock
	if (button == MOUSE_TOGGLE_BUTTON && action == GLFW_PRESS) {
		int inputMode = glfwGetInputMode(window, GLFW_CURSOR) == GLFW_CURSOR_DISABLED ? GLFW_CURSOR_NORMAL : GLFW_CURSOR_DISABLED;
		glfwSetInputMode(window, GLFW_CURSOR, inputMode);
		glfwGetCursorPos(window, &prevMouseX, &prevMouseY);
	}
}

//Author: Eric Winebrenner
//Returns -1, 0, or 1 depending on keys held
float getAxis(GLFWwindow* window, int positiveKey, int negativeKey) {
	float axis = 0.0f;
	if (glfwGetKey(window, positiveKey)) {
		axis++;
	}
	if (glfwGetKey(window, negativeKey)) {
		axis--;
	}
	return axis;
}

//Author: Eric Winebrenner
//Get input every frame
void processInput(GLFWwindow* window) {

	float moveAmnt = CAMERA_MOVE_SPEED * deltaTime;

	//Get camera vectors
	glm::vec3 forward = camera.getForward();
	glm::vec3 right = glm::normalize(glm::cross(forward, glm::vec3(0,1,0)));
	glm::vec3 up = glm::normalize(glm::cross(forward, right));

	glm::vec3 position = camera.getPosition();
	position += forward * getAxis(window, GLFW_KEY_W, GLFW_KEY_S) * moveAmnt;
	position += right * getAxis(window, GLFW_KEY_D, GLFW_KEY_A) * moveAmnt;
	position += up * getAxis(window, GLFW_KEY_Q, GLFW_KEY_E) * moveAmnt;
	camera.setPosition(position);
}
