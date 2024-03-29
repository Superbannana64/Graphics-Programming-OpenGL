#include "GL/glew.h"
#include "GLFW/glfw3.h"

#include <glm/glm.hpp>
#include <glm/matrix.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>


#include <iostream>

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
GLuint createTexture(const char* filePath);

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

glm::vec3 bgColor = glm::vec3(0.0f, 1.0f, 0.0f);
glm::vec3 lightColor = glm::vec3(1.0f);
glm::vec3 lightPosition = glm::vec3(0.0f, 3.0f, 0.0f);

glm::vec3 dLightColor = glm::vec3(1.0f);
glm::vec3 dLightPosition = glm::vec3(0.2f, 1.0f, 0.3f);
float dLightIntensity = 1;

glm::vec3 objectColor = glm::vec3(1.0f);
float matAmbient = 1.0f;
float matDiffuse = 1.0f;
float matSpecular = 1.0f;
float matShiny = 32.0f;

//Blur
float blurIntensity = 2.0f;
//DOF
float focusDist = 5.0f;
float farDOF = 1.0f;
float nearDOF = 0.9f;
float falloff = 15.0f;
float nearPlane = 0.1f;
float farPlane = 1000.0f;
float minStrength = 0.07f;


bool wireFrame = false;
float test = 0;

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

	//Another shader for screen
	Shader screenShader("shaders/screenLit.vert","shaders/screenLit.frag");

	Shader blurShader("shaders/screenLit.vert","shaders/blurLit.frag");


	//All Below is the quad for the post processing
	float quadVertices[] = {
		-1.0f,  1.0f,  0.0f, 1.0f,
		-1.0f, -1.0f,  0.0f, 0.0f,
		 1.0f, -1.0f,  1.0f, 0.0f,

		-1.0f,  1.0f,  0.0f, 1.0f,
		 1.0f, -1.0f,  1.0f, 0.0f,
		 1.0f,  1.0f,  1.0f, 1.0f
	};
	unsigned int quadVAO, quadVBO;
	glGenVertexArrays(1, &quadVAO);
	glGenBuffers(1, &quadVBO);
	glBindVertexArray(quadVAO);
	glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));

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

	GLuint conTexture = createTexture("container.jpg");
	GLuint wallTexture = createTexture("wall.jpg");

	screenShader.use();
	screenShader.setInt("screenTexture", 0);

	//create custom FBO for normal texture
	unsigned int fbo, fboBlur;
	unsigned int rbo, rboBlur;

	glGenFramebuffers(1, &fbo);
	glBindFramebuffer(GL_FRAMEBUFFER, fbo);

	//textureNormalBuffer
	//textureBlur
	//textureDepthbuffer

	//Attach depth
	unsigned int textureColorbuffer, textureDepthbuffer;
	glGenTextures(1, &textureColorbuffer);
	glBindTexture(GL_TEXTURE_2D, textureColorbuffer);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, SCREEN_WIDTH, SCREEN_HEIGHT, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, textureColorbuffer, 0);

	glGenTextures(1, &textureDepthbuffer);
	glBindTexture(GL_TEXTURE_2D, textureDepthbuffer);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32F, 1024, 768, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_NONE);

	// create a renderbuffer object for depth and stencil attachment (we won't be sampling these)

	glGenRenderbuffers(1, &rbo);
	glBindRenderbuffer(GL_RENDERBUFFER, rbo);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, SCREEN_WIDTH, SCREEN_HEIGHT); // use a single renderbuffer object for both a depth AND stencil buffer.
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo); // now actually attach it
	// now that we actually created the framebuffer and added all attachments we want to check if it is actually complete now

	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		std::cout << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!" << std::endl;

	glGenFramebuffers(1, &fboBlur);
	glBindFramebuffer(GL_FRAMEBUFFER, fboBlur);
	//textureBlur

	unsigned int textureColorBlurbuffer;
	glGenTextures(1, &textureColorBlurbuffer);
	glBindTexture(GL_TEXTURE_2D, textureColorBlurbuffer);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, SCREEN_WIDTH, SCREEN_HEIGHT, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, textureColorBlurbuffer, 0);
	// create a renderbuffer object for depth and stencil attachment (we won't be sampling these)

	glGenRenderbuffers(1, &rboBlur);
	glBindRenderbuffer(GL_RENDERBUFFER, rboBlur);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, SCREEN_WIDTH, SCREEN_HEIGHT); // use a single renderbuffer object for both a depth AND stencil buffer.
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo); // now actually attach it

	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		std::cout << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!" << std::endl;

	glBindFramebuffer(GL_FRAMEBUFFER, 0);

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

		//New Process. 
		//We need 3 textures. A normal texture. a blured texture, and a depth texture
		//Draw scene as normal, draw scene as blured, draw scene depth
		//Step 1 is render the scene in litshader, get as texture (DONE)
		//Step 2, render scene again using a blur shader, get as texture 
		//Step 3, render scene AGAIN, this time as a base depth shader, get as texture
		//With normal, blur, and depth textures, use dof shader, set uniforms etc, render screen-space quad

		//To simplify, get base screen as texture, get blured screen as texture, get depthscene as texture, use all 3 on the quad to get depth of field effect
		//Can i simplify it all? maybe normal acts as depth as well? Id rather not render everything 3 times

		glBindFramebuffer(GL_FRAMEBUFFER, fbo);
		glEnable(GL_DEPTH_TEST); // enable depth testing (is disabled for rendering screen-space quad)

		// make sure we clear the framebuffer's content
		// 
		glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		//Draw
		litShader.use();
		litShader.setMat4("_Projection", camera.getProjectionMatrix());
		litShader.setMat4("_View", camera.getViewMatrix());
		litShader.setVec3("_LightPos", lightTransform.position);
		
		litShader.setVec3("material.objectColor", objectColor);
		litShader.setFloat("material.ambient", matAmbient);
		litShader.setFloat("material.diffuse", matDiffuse);
		litShader.setFloat("material.specular", matSpecular);
		litShader.setFloat("material.shininess", matShiny);

		litShader.setVec3("dLight.direction", dLightPosition);
		litShader.setVec3("dLight.lightColor", dLightColor);
		litShader.setFloat("dLight.intensity", dLightIntensity);
		litShader.setVec3("dLight.ambient", glm::vec3(0.2f, 0.2f, 0.2f));
		litShader.setVec3("dLight.diffuse", glm::vec3(0.5f, 0.5f, 0.5f));
		litShader.setVec3("dLight.specular", glm::vec3(1.0f, 1.0f, 1.0f));

		//Draw cube
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, wallTexture);
		litShader.setMat4("_Model", cubeTransform.getModelMatrix());
		litShader.setInt("ourTexture", 1);
		cubeMesh.draw();

		//Draw sphere
		litShader.setMat4("_Model", sphereTransform.getModelMatrix());
		sphereMesh.draw();

		//Draw cylinder
		litShader.setMat4("_Model", cylinderTransform.getModelMatrix());
		cylinderMesh.draw();
		
		//Draw plane
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, conTexture);
		planeTransform.rotation = glm::vec3(-99,0,0);
		litShader.setMat4("_Model", planeTransform.getModelMatrix());
		litShader.setInt("ourTexture", 0);
		planeMesh.draw();

		//Draw light as a small sphere using unlit shader, ironically.
		unlitShader.use();
		unlitShader.setMat4("_Projection", camera.getProjectionMatrix());
		unlitShader.setMat4("_View", camera.getViewMatrix());
		unlitShader.setMat4("_Model", lightTransform.getModelMatrix());
		unlitShader.setVec3("_Color", lightColor);
		sphereMesh.draw();
		
		glBindFramebuffer(GL_FRAMEBUFFER, fboBlur);
		glEnable(GL_DEPTH_TEST); // enable depth testing (is disabled for rendering screen-space quad)
		// make sure we clear the framebuffer's content
		glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		
		blurShader.use();
		glBindVertexArray(quadVAO);
		glBindTexture(GL_TEXTURE_2D, textureColorbuffer);
		blurShader.setFloat("Intensity", blurIntensity);
		glDrawArrays(GL_TRIANGLES, 0, 6);

		//bind to default
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glDisable(GL_DEPTH_TEST); // disable depth test so screen-space quad isn't discarded due to depth test.
		// clear all relevant buffers
		glClearColor(1.0f, 1.0f, 1.0f, 1.0f); // set clear color to white (not really necessary actually, since we won't be able to see behind the quad anyways)
		glClear(GL_COLOR_BUFFER_BIT);

		//PPA
		screenShader.use();
		glBindVertexArray(quadVAO);

		//Having trouble getting 2 textures in 1 frag
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, textureColorbuffer);// use the color attachment texture as the texture of the quad plane
		screenShader.setInt("screenTexture", 0);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, textureColorBlurbuffer);
		screenShader.setInt("blurTexture", 1);
		glActiveTexture(GL_TEXTURE2);
		glBindTexture(GL_TEXTURE2, textureDepthbuffer);
		screenShader.setInt("depthTexture", 2);

		screenShader.setFloat("xs", SCREEN_HEIGHT);
		screenShader.setFloat("ys", SCREEN_WIDTH);
		screenShader.setFloat("test", test);
		screenShader.setFloat("focusDistance", focusDist);
		screenShader.setFloat("farDof", farDOF);
		screenShader.setFloat("nearDof", nearDOF);
		screenShader.setFloat("falloff", falloff);
		screenShader.setFloat("minStrength", minStrength);

		glDrawArrays(GL_TRIANGLES, 0, 6);


		//Draw UI
		ImGui::Begin("Settings");
		ImGui::ColorEdit3("Light Color", &lightColor.r);
		ImGui::DragFloat3("Light Position", (float*)&dLightPosition);
		ImGui::DragFloat("Intensity", &dLightIntensity, 0.1f, 0.0f, 5.0f);
		ImGui::DragFloat("DOF (1 on, 0 off)", &test, 1.0f, 0.0f, 1.0f);
		ImGui::End();

		ImGui::Begin("Depth Of Field");
		ImGui::DragFloat("Intensity", &blurIntensity, 0.1f, 0.1f, 10.0f);
		//ImGui::DragFloat("Focus Distance",&focusDist, 1.0f, 1.0f, 50.0f);
		ImGui::DragFloat("Far DOF", &farDOF, 0.1f, nearDOF+0.1f, 10.0f);
		ImGui::DragFloat("Near DOF", &nearDOF, 0.1f, 0.0f, farDOF - 0.1f);
		ImGui::DragFloat("Falloff", &falloff, 1.0f, 1.0f, 100.0f);
		ImGui::DragFloat("Min Strength", &minStrength, 0.01f, 0.0f, 1.0f);
		ImGui::End();

		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
		glfwPollEvents();

		glfwSwapBuffers(window);
	}

	//Delete buffers
	glDeleteVertexArrays(1, &quadVAO);
	glDeleteBuffers(1, &quadVBO);

	glDeleteFramebuffers(1, &fboBlur);
	glDeleteFramebuffers(1, &fbo);
	

	glfwTerminate();
	return 0;
}
//Author: Nicolas Delbue
GLuint createTexture(const char* filePath)
{
	stbi_set_flip_vertically_on_load(1);
	unsigned int texture;
	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);
	// set the texture wrapping/filtering options (on the currently bound texture object)
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	// load and generate the texture
	int width, height, nrChannels;
	unsigned char* data = stbi_load(filePath, &width, &height, &nrChannels, 0);
	if (data)
	{
		printf("Worked Loaded \n");
		switch (nrChannels)
		{
		case 0:
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
			break;
		case 1:
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
			break;
		case 2:
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
			break;
		case 3:
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
			break;
		default:
			printf("Unknown texture type \n");
			break;
		}
		glGenerateMipmap(GL_TEXTURE_2D);
	}
	else
	{
		std::cout << "Failed to load texture" << std::endl;
	}
	stbi_image_free(data);

	return texture;
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
