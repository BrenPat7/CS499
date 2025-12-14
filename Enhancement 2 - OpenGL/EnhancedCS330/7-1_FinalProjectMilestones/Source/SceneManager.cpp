///////////////////////////////////////////////////////////////////////////////
// scenemanager.cpp
// ================
// This file contains the implementation of the `SceneManager` class, which is 
// responsible for managing the preparation and rendering of 3D scenes. It 
// handles textures, materials, lighting configurations, and object rendering.
//
// AUTHOR: Brian Battersby
// INSTITUTION: Southern New Hampshire University (SNHU)
// COURSE: CS-330 Computational Graphics and Visualization
//
// INITIAL VERSION: November 1, 2023
// LAST REVISED: December 1, 2024
//
// RESPONSIBILITIES:
// - Load, bind, and manage textures in OpenGL.
// - Define materials and lighting properties for 3D objects.
// - Manage transformations and shader configurations.
// - Render complex 3D scenes using basic meshes.
//
// NOTE: This implementation leverages external libraries like `stb_image` for 
// texture loading and GLM for matrix and vector operations.
///////////////////////////////////////////////////////////////////////////////

#include "SceneManager.h"

#ifndef STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#endif

#define GLM_ENABLE_EXPERIMENTAL
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

void SceneManager::AddShape(ShapeType type, std::string material, std::string texture,
	glm::vec3 pos, glm::vec3 scale, glm::vec3 rot, glm::vec2 uv)
{
	Shape newShape;
	newShape.type = type;
	newShape.materialTag = material;
	newShape.textureTag = texture;
	newShape.position = pos;
	newShape.scale = scale;
	newShape.rotation = rot;
	newShape.uvScale = uv;

	m_sceneGraph.push_back(newShape);
}

void SceneManager::PrecomputeShapeTransforms()
{
	for (int i = 0; i < m_sceneGraph.size(); i++)
	{
		Shape& shape = m_sceneGraph[i];

		glm::mat4 scale = glm::scale(shape.scale);
		glm::mat4 rotationX = glm::rotate(glm::radians(shape.rotation.x), glm::vec3(1.0f, 0.0f, 0.0f));
		glm::mat4 rotationY = glm::rotate(glm::radians(shape.rotation.y), glm::vec3(0.0f, 1.0f, 0.0f));
		glm::mat4 rotationZ = glm::rotate(glm::radians(shape.rotation.z), glm::vec3(0.0f, 0.0f, 1.0f));
		glm::mat4 translation = glm::translate(shape.position);

		shape.cachedTransform = translation * rotationZ * rotationY * rotationX * scale;
	}
}

void SceneManager::OptimizeSceneGraph()
{
	std::sort(m_sceneGraph.begin(), m_sceneGraph.end(),
		[](const Shape& a, const Shape& b) {
			if (a.materialTag != b.materialTag)
				return a.materialTag < b.materialTag;
			return a.textureTag < b.textureTag;
		});
}

/***********************************************************
 * SceneManager()
 *
 * The constructor for the class
 ***********************************************************/
SceneManager::SceneManager(ShaderManager* pShaderManager)
{
	m_pShaderManager = pShaderManager;
	m_basicMeshes = new ShapeMeshes();
}

/***********************************************************
 * ~SceneManager()
 *
 * The destructor for the class
 ***********************************************************/
SceneManager::~SceneManager()
{
	m_pShaderManager = NULL;
	delete m_basicMeshes;
	m_basicMeshes = NULL;
}

/***********************************************************
 * CreateGLTexture()
 *
 * This method is used for loading textures from image files,
 * configuring the texture mapping parameters in OpenGL,
 * generating the mipmaps, and loading the read texture into
 * the next available texture slot in memory.
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
 * BindGLTextures()
 *
 * This method is used for binding the loaded textures to
 * OpenGL texture memory slots.  There are up to 16 slots.
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
 * DestroyGLTextures()
 *
 * This method is used for freeing the memory in all the
 * used texture memory slots.
 ***********************************************************/
void SceneManager::DestroyGLTextures()
{
	for (int i = 0; i < m_loadedTextures; i++)
	{
		glGenTextures(1, &m_textureIDs[i].ID);
	}
}

/***********************************************************
 * FindTextureID()
 *
 * This method is used for getting an ID for the previously
 * loaded texture bitmap associated with the passed in tag.
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
 * FindTextureSlot()
 *
 * This method is used for getting a slot index for the previously
 * loaded texture bitmap associated with the passed in tag.
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
 * FindMaterial()
 *
 * This method is used for getting a material from the previously
 * defined materials list that is associated with the passed in tag.
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
 * SetTransformations()
 *
 * This method is used for setting the transform buffer
 * using the passed in transformation values.
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

	modelView = translation * rotationZ * rotationY * rotationX * scale;

	if (NULL != m_pShaderManager)
	{
		m_pShaderManager->setMat4Value(g_ModelName, modelView);
	}
}

/***********************************************************
 * SetShaderColor()
 *
 * This method is used for setting the passed in color
 * into the shader for the next draw command
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
 * SetShaderTexture()
 *
 * This method is used for setting the texture data
 * associated with the passed in ID into the shader.
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
 * SetTextureUVScale()
 *
 * This method is used for setting the texture UV scale
 * values into the shader.
 ***********************************************************/
void SceneManager::SetTextureUVScale(float u, float v)
{
	if (NULL != m_pShaderManager)
	{
		m_pShaderManager->setVec2Value("UVscale", glm::vec2(u, v));
	}
}

/***********************************************************
 * SetShaderMaterial()
 *
 * This method is used for passing the material values
 * into the shader.
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

void SceneManager::DefineObjectMaterials()
{
	/*** STUDENTS - add the code BELOW for defining object materials. ***/
	/*** There is no limit to the number of object materials that can ***/
	/*** be defined. Refer to the code in the OpenGL Sample for help ***/
	OBJECT_MATERIAL plasticMaterial;
	plasticMaterial.diffuseColor = glm::vec3(1.0f, 1.0f, 1.0f);
	plasticMaterial.specularColor = glm::vec3(0.2f, 0.2f, 0.2f);
	plasticMaterial.shininess = 21.0;
	plasticMaterial.tag = "plastic";
	m_objectMaterials.push_back(plasticMaterial);

	OBJECT_MATERIAL woodMaterial;
	woodMaterial.diffuseColor = glm::vec3(0.6f, 0.5f, 0.2f);
	woodMaterial.specularColor = glm::vec3(0.1f, 0.2f, 0.2f);
	woodMaterial.shininess = 1.0;
	woodMaterial.tag = "wood";
	m_objectMaterials.push_back(woodMaterial);

	OBJECT_MATERIAL metalMaterial;
	metalMaterial.diffuseColor = glm::vec3(0.3f, 0.3f, 0.2f);
	metalMaterial.specularColor = glm::vec3(0.7f, 0.7f, 0.8f);
	metalMaterial.shininess = 8.0;
	metalMaterial.tag = "metal";
	m_objectMaterials.push_back(metalMaterial);

	OBJECT_MATERIAL glassMaterial;
	glassMaterial.diffuseColor = glm::vec3(0.3f, 0.3f, 0.2f);
	glassMaterial.specularColor = glm::vec3(0.9f, 0.9f, 0.8f);
	glassMaterial.shininess = 10.0;
	glassMaterial.tag = "glass";
	m_objectMaterials.push_back(glassMaterial);

	OBJECT_MATERIAL tileMaterial;
	tileMaterial.diffuseColor = glm::vec3(0.5f, 0.5f, 0.5f);
	tileMaterial.specularColor = glm::vec3(0.7f, 0.7f, 0.7f);
	tileMaterial.shininess = 6.0;
	tileMaterial.tag = "tile";
	m_objectMaterials.push_back(tileMaterial);

	OBJECT_MATERIAL stoneMaterial;
	stoneMaterial.diffuseColor = glm::vec3(0.5f, 0.5f, 0.5f);
	stoneMaterial.specularColor = glm::vec3(0.73f, 0.3f, 0.3f);
	stoneMaterial.shininess = 6.0;
	stoneMaterial.tag = "stone";
	m_objectMaterials.push_back(stoneMaterial);
}
void SceneManager::SetupSceneLights()
{
	// this line of code is NEEDED for telling the shaders to render
	// the 3D scene with custom lighting, if no light sources have
	// been added then the display window will be black - to use the
	// default OpenGL lighting then comment out the following line
	m_pShaderManager->setBoolValue(g_UseLightingName, true);
	/*** STUDENTS - add the code BELOW for setting up light sources ***/
	/*** Up to five point light sources can be defined. Refer to the code ***/
	/*** in the OpenGL Sample for help ***/
	//Directional Light
	m_pShaderManager->setVec3Value("directionalLight.direction", 0.2f, 5.2f, 0.5f);
	m_pShaderManager->setVec3Value("directionalLight.ambient", 0.15f, 0.15f, 0.15f);
	m_pShaderManager->setVec3Value("directionalLight.diffuse", 0.8f, 0.8f, 0.8f);
	m_pShaderManager->setVec3Value("directionalLight.specular", 1.0f, 0.9f, 0.40f);
	m_pShaderManager->setBoolValue("directionalLight.bActive", true);
	// point light 1
	m_pShaderManager->setVec3Value("pointLights[0].position", 0.0f, 12.0f, 0.0f);
	m_pShaderManager->setVec3Value("pointLights[0].ambient", 0.35f, 0.35f, 0.35f);
	m_pShaderManager->setVec3Value("pointLights[0].diffuse", 0.8f, 0.8f, 0.8f);
	m_pShaderManager->setVec3Value("pointLights[0].specular", 0.25f, 0.25f, 0.25f);
	m_pShaderManager->setBoolValue("pointLights[0].bActive", true);
	m_pShaderManager->setVec3Value("pointLights[1].position", 0.0f, 8.0f, 0.0f);
	m_pShaderManager->setVec3Value("pointLights[1].ambient", 0.0f, 0.0f, 0.0f);
	m_pShaderManager->setVec3Value("pointLights[1].diffuse", 0.6f, 0.6f, 0.65f);
	m_pShaderManager->setVec3Value("pointLights[1].specular", 0.2f, 0.2f, 0.2f);
	m_pShaderManager->setFloatValue("pointLights[1].linear", 0.10f);
	m_pShaderManager->setFloatValue("pointLights[1].quadratic", 0.05f);
	m_pShaderManager->setBoolValue("pointLights[1].bActive", true);
}
/***********************************************************
 * PrepareScene()
 *
 * This method is used for preparing the 3D scene by loading
 * the shapes, textures in memory to support the 3D scene
 * rendering
 ***********************************************************/
void SceneManager::PrepareScene()
{
	SetupSceneLights();
	DefineObjectMaterials();

	// only one instance of a particular mesh needs to be
	// loaded in memory no matter how many times it is drawn
	// in the rendered 3D scene

	//add the textures to the scene
	CreateGLTexture("textures/greyplastic.jpg", "plasticd_texture");
	CreateGLTexture("textures/greenplastic.jpg", "plasticc_texture");
	CreateGLTexture("textures/blueplastic.jpg", "plasticb_texture");
	CreateGLTexture("textures/Redplastic.jpg", "plastic_texture");
	CreateGLTexture("textures/sand.png", "sand_texture");
	CreateGLTexture("textures/brick.jpg", "brick_texture");
	CreateGLTexture("textures/whitecloth.jpg", "cloth_texture");

	// after the texture image data is loaded into memory, the
	// loaded textures need to be bound to texture slots - there
	// are a total of 16 available slots for scene textures
	BindGLTextures();

	m_basicMeshes->LoadPlaneMesh();
	m_basicMeshes->LoadBoxMesh();       // for the cube.
	m_basicMeshes->LoadSphereMesh();    // for the sphere.
	m_basicMeshes->LoadCylinderMesh();  // for the cylinder.
	m_basicMeshes->LoadPyramid4Mesh();   // for the pyramid.
	m_basicMeshes->LoadConeMesh();		// for the cone.
	m_basicMeshes->LoadTorusMesh();    // for the torus.

	// 1. Floor
	AddShape(PLANE, "stone", "sand_texture", glm::vec3(0, 0, 0), glm::vec3(20, 1, 10), glm::vec3(0, 0, 0), glm::vec2(1, 1));
	// 2. Wall
	AddShape(PLANE, "stone", "brick_texture", glm::vec3(0, 9, -10), glm::vec3(20, 1, 10), glm::vec3(90, 0, 0), glm::vec2(1, 1));

	// 3. Bassinet Legs
	AddShape(CYLINDER, "plastic", "plasticd_texture", glm::vec3(-1.5f, 0.55f, 1.1f), glm::vec3(0.1f, 1.72f, 0.1f), glm::vec3(0, 0, 0), glm::vec2(1, 1));
	AddShape(CYLINDER, "plastic", "plasticd_texture", glm::vec3(1.5f, 0.55f, 1.1f), glm::vec3(0.1f, 1.72f, 0.1f), glm::vec3(0, 0, 0), glm::vec2(1, 1));
	AddShape(CYLINDER, "plastic", "plasticd_texture", glm::vec3(-1.5f, 0.55f, -1.1f), glm::vec3(0.1f, 1.72f, 0.1f), glm::vec3(0, 0, 0), glm::vec2(1, 1));
	AddShape(CYLINDER, "plastic", "plasticd_texture", glm::vec3(1.4f, 0.65f, -1.0f), glm::vec3(0.1f, 1.7f, 0.1f), glm::vec3(90, 0, 0), glm::vec2(1, 1));

	// Bassinet Walls and Rails
	float boxLengthX = 2.0f;
	float boxLengthZ = 1.0f;
	float arcRadius = 1.0f;
	float railHeight = 2.95f;

	// Top Rails
	AddShape(BOX, "plastic", "plasticd_texture", glm::vec3(0.0f, railHeight, -boxLengthZ / 2 - arcRadius), glm::vec3(boxLengthX, 0.1f, 0.1f), glm::vec3(0, 0, 0), glm::vec2(1, 1));
	AddShape(BOX, "plastic", "plasticd_texture", glm::vec3(0.0f, railHeight, boxLengthZ / 2 + arcRadius), glm::vec3(boxLengthX, 0.1f, 0.1f), glm::vec3(0, 0, 0), glm::vec2(1, 1));
	AddShape(BOX, "plastic", "plasticd_texture", glm::vec3(-boxLengthX / 2 - arcRadius, railHeight, 0.0f), glm::vec3(0.1f, 0.1f, boxLengthZ), glm::vec3(0, 0, 0), glm::vec2(1, 1));
	AddShape(BOX, "plastic", "plasticd_texture", glm::vec3(boxLengthX / 2 + arcRadius, railHeight, 0.0f), glm::vec3(0.1f, 0.1f, boxLengthZ), glm::vec3(0, 0, 0), glm::vec2(1, 1));

	// Lower Rails
	float railHeightFloor = 2.15f;
	float boxLengthXFl = 1.65f;
	float boxLengthZFl = 0.9f;
	float arcRadiusFl = 0.5f;

	AddShape(BOX, "plastic", "plasticd_texture", glm::vec3(0.0f, railHeightFloor, -boxLengthZFl / 2 - arcRadiusFl), glm::vec3(boxLengthXFl, 0.1f, 0.1f), glm::vec3(0, 0, 0), glm::vec2(1, 1));
	AddShape(BOX, "plastic", "plasticd_texture", glm::vec3(0.0f, railHeightFloor, boxLengthZFl / 2 + arcRadiusFl), glm::vec3(boxLengthXFl, 0.1f, 0.1f), glm::vec3(0, 0, 0), glm::vec2(1, 1));
	AddShape(BOX, "plastic", "plasticd_texture", glm::vec3(-boxLengthXFl / 2 - arcRadiusFl, railHeightFloor, 0.0f), glm::vec3(0.1f, 0.1f, boxLengthZFl), glm::vec3(0, 0, 0), glm::vec2(1, 1));
	AddShape(BOX, "plastic", "plasticd_texture", glm::vec3(boxLengthXFl / 2 + arcRadiusFl, railHeightFloor, 0.0f), glm::vec3(0.1f, 0.1f, boxLengthZFl), glm::vec3(0, 0, 0), glm::vec2(1, 1));

	// Bassinet Cloth Panels
	float panelThickness = 0.18f;
	float panelRise = railHeight - 1.95f;
	float spanX = boxLengthX + 2 * arcRadius - 1;
	float spanZ = boxLengthZ + 2 * arcRadius - 1;
	float midY = (railHeight + 1.95f) * 0.5f;
	float angleX = glm::degrees(atan(panelRise / spanX));
	float angleZ = glm::degrees(atan(panelRise / spanZ));

	AddShape(BOX, "plastic", "cloth_texture", glm::vec3(0.0f, midY, -boxLengthZ / 2 - arcRadius + panelThickness * 0.5f), glm::vec3(spanX, panelRise, panelThickness), glm::vec3(-angleX, 0, 0), glm::vec2(1, 1));
	AddShape(BOX, "plastic", "cloth_texture", glm::vec3(0.0f, midY, boxLengthZ / 2 + arcRadius - panelThickness * 0.5f), glm::vec3(spanX, panelRise, panelThickness), glm::vec3(angleX, 0, 0), glm::vec2(1, 1));
	AddShape(BOX, "plastic", "cloth_texture", glm::vec3(-boxLengthX / 2 - arcRadius + panelThickness * 0.5f, midY, 0.0f), glm::vec3(panelThickness, panelRise, spanZ), glm::vec3(0, 0, angleZ), glm::vec2(1, 1));
	AddShape(BOX, "plastic", "cloth_texture", glm::vec3(boxLengthX / 2 + arcRadius - panelThickness * 0.5f, midY, 0.0f), glm::vec3(panelThickness, panelRise, spanZ), glm::vec3(0, 0, -angleZ), glm::vec2(1, 1));
	AddShape(PLANE, "stone", "cloth_texture", glm::vec3(0.0f, 2.05f, 0.0f), glm::vec3(1.3f, 1.2f, 1.0f), glm::vec3(0, 0, 0), glm::vec2(1, 1));

	// Bassinet Border Loops
	int numSpheres = 10;
	float thetaStep = glm::half_pi<float>() / numSpheres;

	for (int i = 0; i <= numSpheres; ++i) {
		float theta = glm::pi<float>() + i * thetaStep;
		glm::vec3 pos = glm::vec3(-boxLengthX / 2, railHeight, -boxLengthZ / 2) + glm::vec3(arcRadius * cos(theta), 0.0f, arcRadius * sin(theta));
		AddShape(SPHERE, "plastic", "plasticd_texture", pos, glm::vec3(0.15f), glm::vec3(0, 0, 0), glm::vec2(0.3f, 0.2f));
	}
	for (int i = 0; i <= numSpheres; ++i) {
		float theta = glm::pi<float>() * 1.5f + i * thetaStep;
		glm::vec3 pos = glm::vec3(boxLengthX / 2, railHeight, -boxLengthZ / 2) + glm::vec3(arcRadius * cos(theta), 0.0f, arcRadius * sin(theta));
		AddShape(SPHERE, "plastic", "plasticd_texture", pos, glm::vec3(0.15f), glm::vec3(0, 0, 0), glm::vec2(0.3f, 0.2f));
	}
	for (int i = 0; i <= numSpheres; ++i) {
		float theta = 0.0f + i * thetaStep;
		glm::vec3 pos = glm::vec3(boxLengthX / 2, railHeight, boxLengthZ / 2) + glm::vec3(arcRadius * cos(theta), 0.0f, arcRadius * sin(theta));
		AddShape(SPHERE, "plastic", "plasticd_texture", pos, glm::vec3(0.15f), glm::vec3(0, 0, 0), glm::vec2(0.3f, 0.2f));
	}
	for (int i = 0; i <= numSpheres; ++i) {
		float theta = glm::half_pi<float>() + i * thetaStep;
		glm::vec3 pos = glm::vec3(-boxLengthX / 2, railHeight, boxLengthZ / 2) + glm::vec3(arcRadius * cos(theta), 0.0f, arcRadius * sin(theta));
		AddShape(SPHERE, "plastic", "plasticd_texture", pos, glm::vec3(0.15f), glm::vec3(0, 0, 0), glm::vec2(0.3f, 0.2f));
	}

	// Mobile and Torus
	AddShape(TORUS, "plastic", "plasticd_texture", glm::vec3(0.0f, 6.0f, 0.0f), glm::vec3(0.5f, 0.5f, 0.25f), glm::vec3(90.0f, 0.0f, 0.0f), glm::vec2(1, 1));
	AddShape(CYLINDER, "plastic", "plasticd_texture", glm::vec3(0.0f, 6.25f, 0.0f), glm::vec3(0.10f, -2.05f, 0.10f), glm::vec3(0.0f, 0.0f, 90.0f), glm::vec2(1, 1));
	AddShape(CYLINDER, "plastic", "plasticd_texture", glm::vec3(0.0f, 6.25f, 0.0f), glm::vec3(0.10f, -0.35f, 0.10f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec2(1, 1));
	AddShape(CYLINDER, "plastic", "plasticd_texture", glm::vec3(2.05f, 6.25f, 0.0f), glm::vec3(0.10f, -3.35f, 0.10f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec2(1, 1));
	AddShape(SPHERE, "plastic", "plasticd_texture", glm::vec3(0.0f, 6.25f, 0.0f), glm::vec3(0.1f), glm::vec3(0, 0, 0), glm::vec2(1, 1));
	AddShape(SPHERE, "plastic", "plasticd_texture", glm::vec3(2.05f, 6.25f, 0.0f), glm::vec3(0.1f), glm::vec3(0, 0, 0), glm::vec2(1, 1));
	AddShape(CYLINDER, "default", "plasticd_texture", glm::vec3(0.525f, 5.30f, 0.0f), glm::vec3(0.02f, 0.65f, 0.02f), glm::vec3(0, 0, 0), glm::vec2(1, 1));
	AddShape(CYLINDER, "default", "plasticd_texture", glm::vec3(-0.525f, 5.30f, 0.0f), glm::vec3(0.02f, 0.65f, 0.02f), glm::vec3(0, 0, 0), glm::vec2(1, 1));
	AddShape(CYLINDER, "default", "plasticd_texture", glm::vec3(0.0f, 5.30f, 0.50f), glm::vec3(0.02f, 0.65f, 0.02f), glm::vec3(0, 0, 0), glm::vec2(1, 1));
	AddShape(CYLINDER, "default", "plasticd_texture", glm::vec3(0.0f, 5.30f, -0.50f), glm::vec3(0.02f, 0.65f, 0.02f), glm::vec3(0, 0, 0), glm::vec2(1, 1));

	// Toys
	AddShape(PYRAMID, "plastic", "plasticb_texture", glm::vec3(0.525f, 5.25f, 0.0f), glm::vec3(0.31f), glm::vec3(0, 0, 0), glm::vec2(0.1f, 0.1f));
	AddShape(SPHERE, "plastic", "plasticc_texture", glm::vec3(0.0f, 5.25f, -0.50f), glm::vec3(0.23f), glm::vec3(0, 0, 0), glm::vec2(0.2f, 0.2f));
	AddShape(BOX, "plastic", "plastic_texture", glm::vec3(0.0f, 5.35f, 0.50f), glm::vec3(0.28f), glm::vec3(0, 0, 0), glm::vec2(0.1f, 0.1f));
	glm::vec3 starScale(0.18f, 0.25f, 0.08f);
	AddShape(PYRAMID, "plastic", "plasticb_texture", glm::vec3(-0.525f, 5.35f, 0.0f), starScale, glm::vec3(0, 0, 0), glm::vec2(0.1f, 0.1f));
	AddShape(PYRAMID, "plastic", "plasticb_texture", glm::vec3(-0.525f, 5.26f, 0.10f), starScale, glm::vec3(65.0f, 0, 0), glm::vec2(0.1f, 0.1f));
	AddShape(PYRAMID, "plastic", "plasticb_texture", glm::vec3(-0.525f, 5.26f, -0.10f), starScale, glm::vec3(-65.0f, 0, 0), glm::vec2(0.1f, 0.1f));
	AddShape(PYRAMID, "plastic", "plasticb_texture", glm::vec3(-0.525f, 5.15f, 0.05f), starScale, glm::vec3(145.0f, 0, 0), glm::vec2(0.1f, 0.1f));
	AddShape(PYRAMID, "plastic", "plasticb_texture", glm::vec3(-0.525f, 5.15f, -0.05f), starScale, glm::vec3(-145.0f, 0, 0), glm::vec2(0.1f, 0.1f));

	// Couch
	AddShape(BOX, "wood", "plastic_texture", glm::vec3(5.0f, 0.3f, -8.0f), glm::vec3(16.0f, 2.3f, 6.2f), glm::vec3(0, 0, 0), glm::vec2(1, 1));
	AddShape(BOX, "wood", "plastic_texture", glm::vec3(5.0f, 0.8f, -9.4f), glm::vec3(13.0f, 7.8f, 1.6f), glm::vec3(0, 0, 0), glm::vec2(1, 1));
	AddShape(CYLINDER, "wood", "plastic_texture", glm::vec3(-1.4f, 0.65f, -8.0f), glm::vec3(3.2f, 1.5f, 3.2f), glm::vec3(0, 0, 90), glm::vec2(1, 1));
	AddShape(CYLINDER, "wood", "plastic_texture", glm::vec3(12.8f, 0.65f, -8.0f), glm::vec3(3.2f, 1.5f, 3.2f), glm::vec3(0, 0, 90), glm::vec2(1, 1));

	OptimizeSceneGraph();
	PrecomputeShapeTransforms();
}

/***********************************************************
 * RenderScene()
 *
 * This method is used for rendering the 3D scene by
 * transforming and drawing the basic 3D shapes
 ***********************************************************/
void SceneManager::RenderScene()
{
	std::string lastMaterial = "";
	std::string lastTexture = "";

	for (int i = 0; i < m_sceneGraph.size(); i++)
	{
		Shape& currentShape = m_sceneGraph[i];

		if (currentShape.materialTag != lastMaterial) {
			SetShaderMaterial(currentShape.materialTag);
			lastMaterial = currentShape.materialTag;
		}

		if (currentShape.textureTag != lastTexture) {
			SetShaderTexture(currentShape.textureTag);
			lastTexture = currentShape.textureTag;
		}

		SetTextureUVScale(currentShape.uvScale.x, currentShape.uvScale.y);

		if (m_pShaderManager != nullptr) {
			m_pShaderManager->setMat4Value("model", currentShape.cachedTransform);
		}

		switch (currentShape.type)
		{
		case PLANE: m_basicMeshes->DrawPlaneMesh(); break;
		case BOX: m_basicMeshes->DrawBoxMesh(); break;
		case SPHERE: m_basicMeshes->DrawSphereMesh(); break;
		case CYLINDER: m_basicMeshes->DrawCylinderMesh(); break;
		case PYRAMID: m_basicMeshes->DrawPyramid4Mesh(); break;
		case CONE: m_basicMeshes->DrawConeMesh(); break;
		case TORUS: m_basicMeshes->DrawTorusMesh(); break;
		}
	}
}