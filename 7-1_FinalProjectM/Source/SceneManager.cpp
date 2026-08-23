///////////////////////////////////////////////////////////////////////////////
// shadermanager.cpp
// ============
// manage the loading and rendering of 3D scenes
//
//  AUTHOR: Brian Battersby - SNHU Instructor / Computer Science
//	Created for CS-330-Computational Graphics and Visualization, Nov. 1st, 2023
///////////////////////////////////////////////////////////////////////////////

#include "SceneManager.h"

#ifndef STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#endif

#include <glm/gtx/transform.hpp>

// declaration of global variables
namespace
{
	const char* g_ModelName = "model";
	const char* g_ColorValueName = "objectColor";
	const char* g_TextureValueName = "objectTexture";
	const char* g_UseTextureName = "bUseTexture";
	const char* g_UseLightingName = "bUseLighting";
}

/***********************************************************
 *  SceneManager()
 *
 *  The constructor for the class
 ***********************************************************/
SceneManager::SceneManager(ShaderManager *pShaderManager)
{
	m_pShaderManager = pShaderManager;
	m_basicMeshes = new ShapeMeshes();
}

/***********************************************************
 *  ~SceneManager()
 *
 *  The destructor for the class
 ***********************************************************/
SceneManager::~SceneManager()
{
	m_pShaderManager = NULL;
	delete m_basicMeshes;
	m_basicMeshes = NULL;
}

/***********************************************************
 *  CreateGLTexture()
 *
 *  This method is used for loading textures from image files,
 *  configuring the texture mapping parameters in OpenGL,
 *  generating the mipmaps, and loading the read texture into
 *  the next available texture slot in memory.
 ***********************************************************/
bool SceneManager::CreateGLTexture(const char* filename, std::string tag)
{
	int width = 0;
	int height = 0;
	int colorChannels = 0;
	GLuint textureID = 0;

	// indicate to always flip images vertically when loaded
	stbi_set_flip_vertically_on_load(true);

	// try to parse the image data from the specified image file
	unsigned char* image = stbi_load(
		filename,
		&width,
		&height,
		&colorChannels,
		0);

	// if the image was successfully read from the image file
	if (image)
	{
		std::cout << "Successfully loaded image:" << filename << ", width:" << width << ", height:" << height << ", channels:" << colorChannels << std::endl;

		glGenTextures(1, &textureID);
		glBindTexture(GL_TEXTURE_2D, textureID);

		// set the texture wrapping parameters
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		// set texture filtering parameters
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		// if the loaded image is in RGB format
		if (colorChannels == 3)
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, image);
		// if the loaded image is in RGBA format - it supports transparency
		else if (colorChannels == 4)
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);
		else
		{
			std::cout << "Not implemented to handle image with " << colorChannels << " channels" << std::endl;
			return false;
		}

		// generate the texture mipmaps for mapping textures to lower resolutions
		glGenerateMipmap(GL_TEXTURE_2D);

		// free the image data from local memory
		stbi_image_free(image);
		glBindTexture(GL_TEXTURE_2D, 0); // Unbind the texture

		// register the loaded texture and associate it with the special tag string
		m_textureIDs[m_loadedTextures].ID = textureID;
		m_textureIDs[m_loadedTextures].tag = tag;
		m_loadedTextures++;

		return true;
	}

	std::cout << "Could not load image:" << filename << std::endl;

	// Error loading the image
	return false;
}

/***********************************************************
 *  BindGLTextures()
 *
 *  This method is used for binding the loaded textures to
 *  OpenGL texture memory slots.  There are up to 16 slots.
 ***********************************************************/
void SceneManager::BindGLTextures()
{
	for (int i = 0; i < m_loadedTextures; i++)
	{
		// bind textures on corresponding texture units
		glActiveTexture(GL_TEXTURE0 + i);
		glBindTexture(GL_TEXTURE_2D, m_textureIDs[i].ID);
	}
}

/***********************************************************
 *  DestroyGLTextures()
 *
 *  This method is used for freeing the memory in all the
 *  used texture memory slots.
 ***********************************************************/
void SceneManager::DestroyGLTextures()
{
	for (int i = 0; i < m_loadedTextures; i++)
	{
		glGenTextures(1, &m_textureIDs[i].ID);
	}
}

/***********************************************************
 *  FindTextureID()
 *
 *  This method is used for getting an ID for the previously
 *  loaded texture bitmap associated with the passed in tag.
 ***********************************************************/
int SceneManager::FindTextureID(std::string tag)
{
	int textureID = -1;
	int index = 0;
	bool bFound = false;

	while ((index < m_loadedTextures) && (bFound == false))
	{
		if (m_textureIDs[index].tag.compare(tag) == 0)
		{
			textureID = m_textureIDs[index].ID;
			bFound = true;
		}
		else
			index++;
	}

	return(textureID);
}

/***********************************************************
 *  FindTextureSlot()
 *
 *  This method is used for getting a slot index for the previously
 *  loaded texture bitmap associated with the passed in tag.
 ***********************************************************/
int SceneManager::FindTextureSlot(std::string tag)
{
	int textureSlot = -1;
	int index = 0;
	bool bFound = false;

	while ((index < m_loadedTextures) && (bFound == false))
	{
		if (m_textureIDs[index].tag.compare(tag) == 0)
		{
			textureSlot = index;
			bFound = true;
		}
		else
			index++;
	}

	return(textureSlot);
}

/***********************************************************
 *  FindMaterial()
 *
 *  This method is used for getting a material from the previously
 *  defined materials list that is associated with the passed in tag.
 ***********************************************************/
bool SceneManager::FindMaterial(std::string tag, OBJECT_MATERIAL& material)
{
	if (m_objectMaterials.size() == 0)
	{
		return(false);
	}

	int index = 0;
	bool bFound = false;
	while ((index < m_objectMaterials.size()) && (bFound == false))
	{
		if (m_objectMaterials[index].tag.compare(tag) == 0)
		{
			bFound = true;
			material.ambientColor = m_objectMaterials[index].ambientColor;
			material.ambientStrength = m_objectMaterials[index].ambientStrength;
			material.diffuseColor = m_objectMaterials[index].diffuseColor;
			material.specularColor = m_objectMaterials[index].specularColor;
			material.shininess = m_objectMaterials[index].shininess;
		}
		else
		{
			index++;
		}
	}

	return(true);
}

/***********************************************************
 *  SetTransformations()
 *
 *  This method is used for setting the transform buffer
 *  using the passed in transformation values.
 ***********************************************************/
void SceneManager::SetTransformations(
	glm::vec3 scaleXYZ,
	float XrotationDegrees,
	float YrotationDegrees,
	float ZrotationDegrees,
	glm::vec3 positionXYZ)
{
	// variables for this method
	glm::mat4 modelView;
	glm::mat4 scale;
	glm::mat4 rotationX;
	glm::mat4 rotationY;
	glm::mat4 rotationZ;
	glm::mat4 translation;

	// set the scale value in the transform buffer
	scale = glm::scale(scaleXYZ);
	// set the rotation values in the transform buffer
	rotationX = glm::rotate(glm::radians(XrotationDegrees), glm::vec3(1.0f, 0.0f, 0.0f));
	rotationY = glm::rotate(glm::radians(YrotationDegrees), glm::vec3(0.0f, 1.0f, 0.0f));
	rotationZ = glm::rotate(glm::radians(ZrotationDegrees), glm::vec3(0.0f, 0.0f, 1.0f));
	// set the translation value in the transform buffer
	translation = glm::translate(positionXYZ);

	modelView = translation * rotationX * rotationY * rotationZ * scale;

	if (NULL != m_pShaderManager)
	{
		m_pShaderManager->setMat4Value(g_ModelName, modelView);
	}
}

/***********************************************************
 *  SetShaderColor()
 *
 *  This method is used for setting the passed in color
 *  into the shader for the next draw command
 ***********************************************************/
void SceneManager::SetShaderColor(
	float redColorValue,
	float greenColorValue,
	float blueColorValue,
	float alphaValue)
{
	// variables for this method
	glm::vec4 currentColor;

	currentColor.r = redColorValue;
	currentColor.g = greenColorValue;
	currentColor.b = blueColorValue;
	currentColor.a = alphaValue;

	if (NULL != m_pShaderManager)
	{
		m_pShaderManager->setIntValue(g_UseTextureName, false);
		m_pShaderManager->setVec4Value(g_ColorValueName, currentColor);
	}
}

/***********************************************************
 *  SetShaderTexture()
 *
 *  This method is used for setting the texture data
 *  associated with the passed in ID into the shader.
 ***********************************************************/
void SceneManager::SetShaderTexture(
	std::string textureTag)
{
	if (NULL != m_pShaderManager)
	{
		m_pShaderManager->setIntValue(g_UseTextureName, true);

		int textureID = -1;
		textureID = FindTextureSlot(textureTag);
		m_pShaderManager->setSampler2DValue(g_TextureValueName, textureID);
	}
}

/***********************************************************
 *  SetTextureUVScale()
 *
 *  This method is used for setting the texture UV scale
 *  values into the shader.
 ***********************************************************/
void SceneManager::SetTextureUVScale(float u, float v)
{
	if (NULL != m_pShaderManager)
	{
		m_pShaderManager->setVec2Value("UVscale", glm::vec2(u, v));
	}
}

/***********************************************************
 *  SetShaderMaterial()
 *
 *  This method is used for passing the material values
 *  into the shader.
 ***********************************************************/
void SceneManager::SetShaderMaterial(
	std::string materialTag)
{
	if (m_objectMaterials.size() > 0)
	{
		OBJECT_MATERIAL material;
		bool bReturn = false;

		bReturn = FindMaterial(materialTag, material);
		if (bReturn == true)
		{
			m_pShaderManager->setVec3Value("material.ambientColor", material.ambientColor);
			m_pShaderManager->setFloatValue("material.ambientStrength", material.ambientStrength);
			m_pShaderManager->setVec3Value("material.diffuseColor", material.diffuseColor);
			m_pShaderManager->setVec3Value("material.specularColor", material.specularColor);
			m_pShaderManager->setFloatValue("material.shininess", material.shininess);
		}
	}
}

/**************************************************************/
/*** STUDENTS CAN MODIFY the code in the methods BELOW for  ***/
/*** preparing and rendering their own 3D replicated scenes.***/
/*** Please refer to the code in the OpenGL sample project  ***/
/*** for assistance.                                        ***/
/**************************************************************/

/***********************************************************
*  DefineObjectMaterials()
 *
 *  This method is used for defining the materials used by
 *  the objects in the scene.  The materials are stored in a
 *  vector for later use when rendering the scene.
 ***********************************************************/
void SceneManager::DefineObjectMaterials()
{
	// Wood material for the cutting board and rolling pin
	// ambientStrength and shininess are both kept low on purpose
	// ambientStrength gets added once per light in the shader loop
	// (4 iteration loop), so a low value is needed to avoid
	// overexposure of the material. Shininess multiplies
	// specular brightniss in this shader. A high value will lead
	// to blown out highlight
	OBJECT_MATERIAL woodMaterial;
	woodMaterial.ambientColor = glm::vec3(0.25f, 0.25f, 0.15f);
	woodMaterial.ambientStrength = 0.05f;
	woodMaterial.diffuseColor = glm::vec3(0.5f, 0.4f, 0.3f);
	woodMaterial.specularColor = glm::vec3(0.3f, 0.3f, 0.25f);
	woodMaterial.shininess = 2.0f;
	woodMaterial.tag = "wood";
	m_objectMaterials.push_back(woodMaterial);

	// Accent material used for small, dark non-wood details
	// Specifically, the cutting board handle hole
	OBJECT_MATERIAL accentMaterial;
	accentMaterial.ambientColor = glm::vec3(0.05f, 0.05f, 0.05f);
	accentMaterial.ambientStrength = 0.05f;
	accentMaterial.diffuseColor = glm::vec3(0.2f, 0.2f, 0.2f);
	accentMaterial.specularColor = glm::vec3(0.2f, 0.2f, 0.2f);
	accentMaterial.shininess = 2.0f;
	accentMaterial.tag = "accent";
	m_objectMaterials.push_back(accentMaterial);

	// Tabletop material for the ground plane. 
	// Slightly more specular than the wood material and
	// visibly reflects light sources, as per the rubric.
	OBJECT_MATERIAL tableMaterial;
	tableMaterial.ambientColor = glm::vec3(0.12f, 0.12f, 0.12f);
	tableMaterial.ambientStrength = 0.04f;
	tableMaterial.diffuseColor = glm::vec3(0.32f, 0.32f, 0.32f);
	tableMaterial.specularColor = glm::vec3(0.15f, 0.15f, 0.15f);
	tableMaterial.shininess = 2.0f;
	tableMaterial.tag = "tabletop";
	m_objectMaterials.push_back(tableMaterial);
}

/***********************************************************
*  SetupSceneLights()
 *
 *  This method is used for adding and configuring the light
 *  sources for the scene.  The light sources are stored in a
 *  vector for later use when rendering the scene.
 ***********************************************************/
void SceneManager::SetupSceneLights()
{
	// Requirement that tells the shaders to use custom lighting 
	// in place of rendering the scene with the default light source
	m_pShaderManager->setBoolValue(g_UseLightingName, true);

	// Light 0: the main overhead key light source (neutral white),
	// positioned high and slightly off-center from the cutting
	// board and the rollin pin
	m_pShaderManager->setVec3Value("lightSources[0].position", 3.0f, 10.0f, 0.0f);
	m_pShaderManager->setVec3Value("lightSources[0].ambientColor", 0.5f, 0.5f, 0.5f);

	// Important: light.diffuseColor is declared but not read in CalcLightSource(), 
	// and is set for structural clarity
	m_pShaderManager->setVec3Value("lightSources[0].diffuseColor", 0.4f, 0.4f, 0.4f);
	m_pShaderManager->setVec3Value("lightSources[0].specularColor", 0.9f, 0.9f, 0.9f);
	m_pShaderManager->setFloatValue("lightSources[0].focalStrength", 64.0f);
	m_pShaderManager->setFloatValue("lightSources[0].specularIntensity", 0.4f);

	// Light 1: a secondary fill light source (slightly warm white),
	// positioned low and to the right of the cutting board
	m_pShaderManager->setVec3Value("lightSources[1].position", -5.0f, 6.0f, -4.0f);
	m_pShaderManager->setVec3Value("lightSources[1].ambientColor", 0.15f, 0.1f, 0.05f);
	m_pShaderManager->setVec3Value("lightSources[1].diffuseColor", 0.3f, 0.2f, 0.1f);
	m_pShaderManager->setVec3Value("lightSources[1].specularColor", 0.4f, 0.3f, 0.15f);
	m_pShaderManager->setFloatValue("lightSources[1].focalStrength", 16.0f);
	m_pShaderManager->setFloatValue("lightSources[1].specularIntensity", 0.15f);

	// Lights 2 and 3: disabled. The shader's lighting loop always runs all 4
	// slots, regardless of how many are being used. Zeroing these out ensures 
	// that they don't contribute to the scene lighting.
	m_pShaderManager->setVec3Value("lightSources[2].position", 0.0f, 0.0f, 0.0f);
	m_pShaderManager->setVec3Value("lightSources[2].ambientColor", 0.0f, 0.0f, 0.0f);
	m_pShaderManager->setVec3Value("lightSources[2].diffuseColor", 0.0f, 0.0f, 0.0f);
	m_pShaderManager->setVec3Value("lightSources[2].specularColor", 0.0f, 0.0f, 0.0f);
	m_pShaderManager->setFloatValue("lightSources[2].focalStrength", 1.0f);
	m_pShaderManager->setFloatValue("lightSources[2].specularIntensity", 0.0f);

	m_pShaderManager->setVec3Value("lightSources[3].position", 0.0f, 0.0f, 0.0f);
	m_pShaderManager->setVec3Value("lightSources[3].ambientColor", 0.0f, 0.0f, 0.0f);
	m_pShaderManager->setVec3Value("lightSources[3].diffuseColor", 0.0f, 0.0f, 0.0f);
	m_pShaderManager->setVec3Value("lightSources[3].specularColor", 0.0f, 0.0f, 0.0f);
	m_pShaderManager->setFloatValue("lightSources[3].focalStrength", 1.0f);
	m_pShaderManager->setFloatValue("lightSources[3].specularIntensity", 0.0f);
}

/***********************************************************
 *  LoadSceneTextures()
 *
 *  This method is used for preparing the 3D scene by loading
 *  the textures in memory to support the 3D scene
 *  rendering
 ***********************************************************/
void SceneManager::LoadSceneTextures()
{
	// Loading the wood texture for the cutting board
	CreateGLTexture("textures/wood.jpg", "wood");

	// Loading the second wood texture for the rolling pin
	CreateGLTexture("textures/wood2.jpg", "wood2");

	// Loading the third wood texture for the countertop
	CreateGLTexture("textures/wood3.png", "wood3");

	// Binding all loaded textures to their slots
	BindGLTextures();
}

/***********************************************************
 *  PrepareScene()
 *
 *  This method is used for preparing the 3D scene by loading
 *  the shapes in memory to support the 3D scene 
 *  rendering
 ***********************************************************/
void SceneManager::PrepareScene()
{
	// Loading texture images into memory
	LoadSceneTextures();

	// Defining the materials used by objects in the scene
	DefineObjectMaterials();
	// Adding and configuring the light sources for the scene
	SetupSceneLights();

	// Adding all the basic shapes needed for the cutting board and rolling pin
	m_basicMeshes->LoadPlaneMesh();
	m_basicMeshes->LoadBoxMesh();
	m_basicMeshes->LoadTorusMesh();
	m_basicMeshes->LoadCylinderMesh();
	m_basicMeshes->LoadSphereMesh();
}

/***********************************************************
 *  RenderScene()
 *
 *  This method is used for rendering the 3D scene by 
 *  transforming and drawing the basic 3D shapes
 ***********************************************************/
void SceneManager::RenderScene()
{
	// Declaring the transformation variables
	glm::vec3 scaleXYZ;
	float XrotationDegree = 0.0f;
	float YrotationDegree = 0.0f;
	float ZrotationDegree = 0.0f;
	glm::vec3 positionXYZ;

	/*** Set needed transformations before drawing the basic mesh.  ***/
	/*** This same ordering of code should be used for transforming ***/
	/*** and drawing all the basic 3D shapes.						***/
	/******************************************************************/

	//****************************************************************//
	//************** Table surface (ground plane) ********************//
	//****************************************************************//

	scaleXYZ = glm::vec3(20.0f, 1.0f, 10.0f);
	XrotationDegree = 0.0f;
	YrotationDegree = 0.0f;
	ZrotationDegree = 0.0f;
	positionXYZ = glm::vec3(0.0f, 0.0f, 0.0f);

	SetTransformations(
		scaleXYZ,
		XrotationDegree,
		YrotationDegree,
		ZrotationDegree,
		positionXYZ);

	// Choosing a light gray color for the table surface
	SetShaderTexture("wood3");
	SetTextureUVScale(1.0f, 1.0f);
	SetShaderMaterial("tabletop");
	m_basicMeshes->DrawPlaneMesh();

	//*************************//
	//Cutting board (main body)//
	//*************************//

	scaleXYZ = glm::vec3(4.0f, 0.2f, 3.5f);
	XrotationDegree = 0.0f;
	YrotationDegree = 0.0f;
	ZrotationDegree = 0.0f;
	positionXYZ = glm::vec3(6.0f, 0.1f, 3.0f); // Ensuring placement to the bottom-right of the scene to match the original image

	SetTransformations(
		scaleXYZ,
		XrotationDegree,
		YrotationDegree,
		ZrotationDegree,
		positionXYZ);

	// Applying wood texture to the cutting board body
	SetShaderTexture("wood");
	// Tiling the texture twice on the U axis to match the board's wide dimensions
	SetTextureUVScale(2.0f, 1.0f);
	SetShaderMaterial("wood");
	m_basicMeshes->DrawBoxMesh();

	//**********************//
	// Cutting board handle //
	//**********************//

	scaleXYZ = glm::vec3(1.2f, 0.2f, 1.5f);
	XrotationDegree = 0.0f;
	YrotationDegree = 0.0f;
	ZrotationDegree = 0.0f;
	positionXYZ = glm::vec3(3.4f, 0.1f, 3.0f); // Positioned to the left of the main body

	SetTransformations(
		scaleXYZ,
		XrotationDegree,
		YrotationDegree,
		ZrotationDegree,
		positionXYZ);

	// Setting the texture for the cutting board's handle
	SetShaderTexture("wood");
	// Tiling the texture to match the handle's dimensions
	SetTextureUVScale(1.0f, 1.0f);
	SetShaderMaterial("wood");
	m_basicMeshes->DrawBoxMesh();

	//***************************//
	// Cutting board handle hole //
	//***************************//

	scaleXYZ = glm::vec3(0.3f, 0.3f, 0.3f);
	XrotationDegree = 90.0f;  // Rotating to ensure the board lays flat
	YrotationDegree = 0.0f;
	ZrotationDegree = 0.0f;
	positionXYZ = glm::vec3(3.4f, 0.15f, 3.0f); // Centered on the handle

	SetTransformations(
		scaleXYZ,
		XrotationDegree,
		YrotationDegree,
		ZrotationDegree,
		positionXYZ);

	// Choosing a darker color for the hole
	SetShaderColor(0.2f, 0.2f, 0.2f, 1.0f);
	SetShaderMaterial("accent");
	m_basicMeshes->DrawTorusMesh();

	//**********************************//
	// Rolling pin main body (cylinder) //
	//**********************************//

	scaleXYZ = glm::vec3(0.6f, 3.0f, 0.6f);
	XrotationDegree = 0.0f;
	YrotationDegree = 0.0f;
	ZrotationDegree = 90.0f; // Rotated to lay horizontally
	positionXYZ = glm::vec3(3.5f, 0.6f, -3.5f); // Positioned at the top center of the scene

	SetTransformations(
		scaleXYZ,
		XrotationDegree,
		YrotationDegree,
		ZrotationDegree,
		positionXYZ);

	// Applying wood2 texture to the rolling pin's main body
	SetShaderTexture("wood2");
	// Tiling twice on the U axis to match the pin's length
	SetTextureUVScale(2.0f, 1.0f);
	SetShaderMaterial("wood");
	m_basicMeshes->DrawCylinderMesh();

	//**********************************//
	// Rolling pin left handle (cylinder) //
	//**********************************//

	scaleXYZ = glm::vec3(0.3f, 1.2f, 0.3f);
	XrotationDegree = 0.0f;
	YrotationDegree = 0.0f;
	ZrotationDegree = 90.0f;
	positionXYZ = glm::vec3(0.5f, 0.6f, -3.5f);

	SetTransformations(
		scaleXYZ,
		XrotationDegree,
		YrotationDegree,
		ZrotationDegree,
		positionXYZ);

	// Applying wood2 texture to the rolling pin's left handle
	SetShaderTexture("wood2");
	SetTextureUVScale(1.0f, 1.0f);
	SetShaderMaterial("wood");
	m_basicMeshes->DrawCylinderMesh();

	//*************************************//
	// Rolling pin right handle (cylinder) //
	//*************************************//

	scaleXYZ = glm::vec3(0.3f, 1.2f, 0.3f);
	XrotationDegree = 0.0f;
	YrotationDegree = 0.0f;
	ZrotationDegree = 90.0f;
	positionXYZ = glm::vec3(4.6f, 0.6f, -3.5f);

	SetTransformations(
		scaleXYZ,
		XrotationDegree,
		YrotationDegree,
		ZrotationDegree,
		positionXYZ);

	// Setting the texture for the rolling pin's right handle
	SetShaderTexture("wood2");
	SetTextureUVScale(1.0f, 1.0f);
	SetShaderMaterial("wood");
	m_basicMeshes->DrawCylinderMesh();

	//**************************************//
	// Rolling pin left handle end cap (sphere) //
	//**************************************//

	scaleXYZ = glm::vec3(0.35f, 0.35f, 0.35f);
	XrotationDegree = 0.0f;
	YrotationDegree = 0.0f;
	ZrotationDegree = 0.0f;
	positionXYZ = glm::vec3(-0.5f, 0.6f, -3.5f);

	SetTransformations(
		scaleXYZ,
		XrotationDegree,
		YrotationDegree,
		ZrotationDegree,
		positionXYZ);

	// Applying wood2 texture to the left handle's end cap
	SetShaderTexture("wood2");
	SetTextureUVScale(1.0f, 1.0f);
	SetShaderMaterial("wood");
	m_basicMeshes->DrawSphereMesh();

	//*******************************************//
	// Rolling pin right handle end cap (sphere) //
	//*******************************************//

	scaleXYZ = glm::vec3(0.35f, 0.35f, 0.35f);
	XrotationDegree = 0.0f;
	YrotationDegree = 0.0f;
	ZrotationDegree = 0.0f;
	positionXYZ = glm::vec3(4.5f, 0.6f, -3.5f);

	SetTransformations(
		scaleXYZ,
		XrotationDegree,
		YrotationDegree,
		ZrotationDegree,
		positionXYZ);

	// Setting wood2 texture for the right handle's end cap
	SetShaderTexture("wood2");
	SetTextureUVScale(1.0f, 1.0f);
	SetShaderMaterial("wood");
	m_basicMeshes->DrawSphereMesh();

	//*********************//
	// Cup (Open cylinder) //
	//*********************//

	// Scaling the cup to be taller and narrower
	scaleXYZ = glm::vec3(0.8f, 2.0f, 0.8f);
	XrotationDegree = 0.0f;
	YrotationDegree = 0.0f;
	ZrotationDegree = 0.0f;
	// Positioning to the upper left of the scene
	positionXYZ = glm::vec3(-4.0f, 0.0f, -3.0f);

	SetTransformations(
		scaleXYZ,
		XrotationDegree,
		YrotationDegree,
		ZrotationDegree,
		positionXYZ);

	// Light gray color for the cup
	SetShaderColor(0.85f, 0.85f, 0.85f, 1.0f);
	SetShaderMaterial("accent");
	m_basicMeshes->DrawCylinderMesh();

	//****************************//
	// Chef's Knife - Blade (Box) //
	//****************************//

	// Setting scale to make the blade long and thin
	scaleXYZ = glm::vec3(3.0f, 0.05f, 0.8f);
	XrotationDegree = 0.0f;
	YrotationDegree = 0.0f;
	ZrotationDegree = 0.0f;
	// Positioning the blade to the lower left of the scene
	positionXYZ = glm::vec3(-3.0f, 0.1f, 2.0f);

	SetTransformations(
		scaleXYZ,
		XrotationDegree,
		YrotationDegree,
		ZrotationDegree,
		positionXYZ);

	// Silver color for the blade
	SetShaderColor(0.75f, 0.75f, 0.80f, 1.0f);
	SetShaderMaterial("accent");
	m_basicMeshes->DrawBoxMesh();

	//******************************//
	// Chef's Knife - Handle (Box) //
	//******************************//

	// Setting scale to make the handle short and thick
	scaleXYZ = glm::vec3(1.2f, 0.04f, 0.55f);
	XrotationDegree = 0.0f;
	YrotationDegree = 0.0f;
	ZrotationDegree = 0.0f;
	// Positioning behind the blade to form the knife
	positionXYZ = glm::vec3(-4.8f, 0.1f, 2.0f);

	SetTransformations(
		scaleXYZ,
		XrotationDegree,
		YrotationDegree,
		ZrotationDegree,
		positionXYZ);

	// Dark brown color for the handle
	SetShaderColor(0.25f, 0.15f, 0.10f, 1.0f);
	SetShaderMaterial("wood");
	m_basicMeshes->DrawBoxMesh();
}