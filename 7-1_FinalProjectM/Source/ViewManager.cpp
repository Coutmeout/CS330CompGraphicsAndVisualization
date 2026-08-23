///////////////////////////////////////////////////////////////////////////////
// viewmanager.h
// ============
// manage the viewing of 3D objects within the viewport
//
//  AUTHOR: Brian Battersby - SNHU Instructor / Computer Science
//	Created for CS-330-Computational Graphics and Visualization, Nov. 1st, 2023
///////////////////////////////////////////////////////////////////////////////

#include "ViewManager.h"
#include "camera.h"

// GLM Math Header inclusions
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/type_ptr.hpp>

// declaration of the global variables and defines
namespace
{
	// Variables for window width and height
	const int WINDOW_WIDTH = 1000;
	const int WINDOW_HEIGHT = 800;
	const char* g_ViewName = "view";
	const char* g_ProjectionName = "projection";

	// camera object used for viewing and interacting with
	// the 3D scene
	Camera* g_pCamera = nullptr;

	// these variables are used for mouse movement processing
	float gLastX = WINDOW_WIDTH / 2.0f;
	float gLastY = WINDOW_HEIGHT / 2.0f;
	bool gFirstMouse = true;

	// time between current frame and last frame
	float gDeltaTime = 0.0f; 
	float gLastFrame = 0.0f;

	// the following variable is false when orthographic projection
	// is off and true when it is on
	bool bOrthographicProjection = false;
}

/***********************************************************
 *  ViewManager()
 *
 *  The constructor for the class
 ***********************************************************/
ViewManager::ViewManager(
	ShaderManager *pShaderManager)
{
	// initialize the member variables
	m_pShaderManager = pShaderManager;
	m_pWindow = NULL;
	g_pCamera = new Camera();
	// default camera view parameters
	g_pCamera->Position = glm::vec3(0.0f, 5.0f, 12.0f);
	g_pCamera->Front = glm::vec3(0.0f, -0.5f, -2.0f);
	g_pCamera->Up = glm::vec3(0.0f, 1.0f, 0.0f);
	g_pCamera->Zoom = 80;
}

/***********************************************************
 *  ~ViewManager()
 *
 *  The destructor for the class
 ***********************************************************/
ViewManager::~ViewManager()
{
	// free up allocated memory
	m_pShaderManager = NULL;
	m_pWindow = NULL;
	if (NULL != g_pCamera)
	{
		delete g_pCamera;
		g_pCamera = NULL;
	}
}

/***********************************************************
 *  CreateDisplayWindow()
 *
 *  This method is used to create the main display window.
 ***********************************************************/
GLFWwindow* ViewManager::CreateDisplayWindow(const char* windowTitle)
{
	GLFWwindow* window = nullptr;

	// try to create the displayed OpenGL window
	window = glfwCreateWindow(
		WINDOW_WIDTH,
		WINDOW_HEIGHT,
		windowTitle,
		NULL, NULL);
	if (window == NULL)
	{
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return NULL;
	}
	glfwMakeContextCurrent(window);

	// tell GLFW to capture all mouse events
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	// this callback is used to receive mouse moving events
	glfwSetCursorPosCallback(window, &ViewManager::Mouse_Position_Callback);

	// this callback is used to receive mouse scrolling events
	glfwSetScrollCallback(window, Mouse_Scroll_Callback);

	// enable blending for supporting tranparent rendering
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	m_pWindow = window;

	return(window);
}

/***********************************************************
 *  Mouse_Position_Callback()
 *
 *  This method is automatically called from GLFW whenever
 *  the mouse is moved within the active GLFW display window.
 ***********************************************************/
void ViewManager::Mouse_Position_Callback(GLFWwindow* window, double xMousePos, double yMousePos)
{
	// if the camera is null, do nothing
	if (g_pCamera == nullptr)
		return;

	// when the first mouse move event occurs, the position is recorded so subsequent mouse movement can be calculated for offset values
	if (gFirstMouse)
	{
		gLastX = static_cast<float>(xMousePos);
		gLastY = static_cast<float>(yMousePos);
		gFirstMouse = false;
	}

	// calculating how far the mouse moved since the last frame
	float xOffset = static_cast<float>(xMousePos) - gLastX;
	// reverse the yOffset since the y-axis is inverted in OpenGL
	float yOffset = gLastY - static_cast<float>(yMousePos);

	// storing the current position for the next frame
	gLastX = static_cast<float>(xMousePos);
	gLastY = static_cast<float>(yMousePos);

	// passing the offset values to the camera for processing
	g_pCamera->ProcessMouseMovement(xOffset, yOffset);
}

void ViewManager::Mouse_Scroll_Callback(GLFWwindow* window, double xOffset, double yOffset)
{
	// if the camer is null, do nothing
	if (g_pCamera == nullptr)
		return;

	// adjusting the camera movement speed based on the mouse scroll wheel movement
	// scroll up to increase speed, scroll down to decrease speed
	g_pCamera->MovementSpeed += static_cast<float>(yOffset) * 0.5f;

	// enforcing a minimum speed limit to prevent the camera from moving too slowly
	if (g_pCamera->MovementSpeed < 0.5f)
	{
		g_pCamera->MovementSpeed = 0.5f;
	}

	// enforcing a maximum speed limit to prevent the camera from moving too quickly
	if (g_pCamera->MovementSpeed > 10.0f)
	{
		g_pCamera->MovementSpeed = 10.0f;
	}
}

/***********************************************************
 *  ProcessKeyboardEvents()
 *
 *  This method is called to process any keyboard events
 *  that may be waiting in the event queue.
 ***********************************************************/
void ViewManager::ProcessKeyboardEvents()
{
	// close the window if the escape key has been pressed
	if (glfwGetKey(m_pWindow, GLFW_KEY_ESCAPE) == GLFW_PRESS)
	{
		glfwSetWindowShouldClose(m_pWindow, true);
	}

	// if the camera is null, do nothing
	if (g_pCamera == nullptr)
		return;

	//***********************************//
	// WASD for camera movement controls // 
	//***********************************//

	// Move forward (W)
	if (glfwGetKey(m_pWindow, GLFW_KEY_W) == GLFW_PRESS)
	{
		g_pCamera->ProcessKeyboard(FORWARD, gDeltaTime);
	}

	// Move backward (S)
	if (glfwGetKey(m_pWindow, GLFW_KEY_S) == GLFW_PRESS)
	{
		g_pCamera->ProcessKeyboard(BACKWARD, gDeltaTime);
	}

	// Pan left (A)
	if (glfwGetKey(m_pWindow, GLFW_KEY_A) == GLFW_PRESS)
	{
		g_pCamera->ProcessKeyboard(LEFT, gDeltaTime);
	}

	// Pan right (D)
	if (glfwGetKey(m_pWindow, GLFW_KEY_D) == GLFW_PRESS)
	{
		g_pCamera->ProcessKeyboard(RIGHT, gDeltaTime);
	}

	//***********************************//
	// Vertical camera movement controls //
	//***********************************//

	// Move up (Q)
	if (glfwGetKey(m_pWindow, GLFW_KEY_Q) == GLFW_PRESS)
	{
		g_pCamera->ProcessKeyboard(UP, gDeltaTime);
	}

	// Move down (E)
	if (glfwGetKey(m_pWindow, GLFW_KEY_E) == GLFW_PRESS)
	{
		g_pCamera->ProcessKeyboard(DOWN, gDeltaTime);
	}

	//*************************//
	// Toggle projection modes //
	//*************************//

	// Switch to perspective projection (P)
	if (glfwGetKey(m_pWindow, GLFW_KEY_P) == GLFW_PRESS)
	{
		bOrthographicProjection = false;
		// small delay to prevent rapid toggling
		glfwWaitEventsTimeout(0.2);
	}

	// Switch to orthographic projection (O)
	if (glfwGetKey(m_pWindow, GLFW_KEY_O) == GLFW_PRESS)
	{
		bOrthographicProjection = true;
		// small delay to prevent rapid toggling
		glfwWaitEventsTimeout(0.2);
	}
}

/***********************************************************
 *  PrepareSceneView()
 *
 *  This method is used for preparing the 3D scene by loading
 *  the shapes, textures in memory to support the 3D scene 
 *  rendering
 ***********************************************************/
void ViewManager::PrepareSceneView()
{
	glm::mat4 view;
	glm::mat4 projection;

	// per-frame timing
	float currentFrame = glfwGetTime();
	gDeltaTime = currentFrame - gLastFrame;
	gLastFrame = currentFrame;

	// process any keyboard events that may be waiting in the 
	// event queue
	ProcessKeyboardEvents();

	// get the current view matrix from the camera
	view = g_pCamera->GetViewMatrix();

	//*********************************************************//
	// Switch between perspective and orthographic projections //
	//*********************************************************//

	if (bOrthographicProjection)
	{
		// 2D flat view looking straight down the Z-axis with no perspective
		// Bottom plane is not visible in this mode
		float orthoSize = 10.0f;
		projection = glm::ortho(
			-orthoSize,                                          // left
			orthoSize,                                           // right
			-orthoSize / ((float)WINDOW_WIDTH / WINDOW_HEIGHT),  // bottom
			orthoSize / ((float)WINDOW_WIDTH / WINDOW_HEIGHT),   // top
			0.1f,                                                // near clip
			100.0f                                               // far clip
		);

		// Overriding the view to look down at the scene for orthographic projection mode
		view = glm::lookAt(
			glm::vec3(0.0f, 20.0f, 0.0f), // Camera position (above the scene)
			glm::vec3(0.0f, 0.0f, 0.0f),  // Look at the origin (center of the scene)
			glm::vec3(0.0f, 0.0f, -1.0f)  // Up vector (pointing down the negative Z-axis)
		);
	}
	else
	{
		// Perspective projection with depth perception
		// Objects farther away appear smaller, providing a more realistic 3D view
		projection = glm::perspective(
			glm::radians(g_pCamera->Zoom),                  // field of view in radians
			(GLfloat)WINDOW_WIDTH / (GLfloat)WINDOW_HEIGHT, // aspect ratio
			0.1f,                                           // near clipping plane
			100.0f                                          // far clipping plane
		);
	}

	// if the shader manager object is valid
	if (NULL != m_pShaderManager)
	{
		// set the view matrix into the shader for proper rendering
		m_pShaderManager->setMat4Value(g_ViewName, view);
		// set the view matrix into the shader for proper rendering
		m_pShaderManager->setMat4Value(g_ProjectionName, projection);
		// set the view position of the camera into the shader for proper rendering
		m_pShaderManager->setVec3Value("viewPosition", g_pCamera->Position);
	}
}