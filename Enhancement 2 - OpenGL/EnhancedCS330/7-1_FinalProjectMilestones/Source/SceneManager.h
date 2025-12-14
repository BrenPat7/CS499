#pragma once

#include "ShaderManager.h"
#include "ShapeMeshes.h"

#include <string>
#include <vector>
#include <algorithm>

class SceneManager
{
public:
	SceneManager(ShaderManager* pShaderManager);
	~SceneManager();

	enum ShapeType {
		PLANE,
		BOX,
		SPHERE,
		CYLINDER,
		PYRAMID,
		CONE,
		TORUS
	};

	struct Shape {
		ShapeType type;
		std::string materialTag;
		std::string textureTag;
		glm::vec2 uvScale;
		glm::vec3 position;
		glm::vec3 scale;
		glm::vec3 rotation;
		glm::mat4 cachedTransform;
	};

	struct TEXTURE_INFO
	{
		std::string tag;
		uint32_t ID;
	};

	struct OBJECT_MATERIAL
	{
		glm::vec3 diffuseColor;
		glm::vec3 specularColor;
		float shininess;
		std::string tag;
	};

private:
	ShaderManager* m_pShaderManager;
	ShapeMeshes* m_basicMeshes;
	int m_loadedTextures;
	TEXTURE_INFO m_textureIDs[16];
	std::vector<OBJECT_MATERIAL> m_objectMaterials;
	std::vector<Shape> m_sceneGraph;

	bool CreateGLTexture(const char* filename, std::string tag);
	void BindGLTextures();
	void DestroyGLTextures();
	int FindTextureID(std::string tag);
	int FindTextureSlot(std::string tag);
	bool FindMaterial(std::string tag, OBJECT_MATERIAL& material);

	void SetTransformations(glm::vec3 scaleXYZ, float XrotationDegrees, float YrotationDegrees, float ZrotationDegrees, glm::vec3 positionXYZ);
	void SetShaderColor(float redColorValue, float greenColorValue, float blueColorValue, float alphaValue);
	void SetShaderTexture(std::string textureTag);
	void SetTextureUVScale(float u, float v);
	void SetShaderMaterial(std::string materialTag);

	void AddShape(ShapeType type, std::string material, std::string texture, glm::vec3 pos, glm::vec3 scale, glm::vec3 rot, glm::vec2 uv);
	void PrecomputeShapeTransforms();
	void OptimizeSceneGraph();

public:
	void PrepareScene();
	void RenderScene();
	void SetupSceneLights();
	void DefineObjectMaterials();
};