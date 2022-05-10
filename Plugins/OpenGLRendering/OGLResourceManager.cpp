#include "../../CSC8503/CSC8503Common/SystemDefines.h"
#include "OGLResourceManager.h"
#include "OGLMesh.h"
#include "OGLTexture.h"
#include "OGLShader.h";
#include "../../Common/MeshMaterial.h"
#include "../../Common/MeshGeometry.h"
#include "../../Common/MeshAnimation.h"
#include "../../Common/TextureLoader.h"
#include "../../Common/Assets.h"
#include <filesystem>


using namespace NCL::Rendering;

OGLResourceManager::~OGLResourceManager() {
}

NCL::MeshGeometry* OGLResourceManager::LoadMesh(string filename) {
	std::filesystem::path path = Assets::MESHDIR + filename;
	if (!std::filesystem::exists(path)) {
		return nullptr;
	}

	if (meshes.find(filename) != meshes.end()) {
		return meshes[filename];
	}

	auto loadFunc = [](const string& name, OGLMesh** into) {
		*into = new OGLMesh(name);
		(*into)->SetPrimitiveType(GeometryPrimitive::Triangles);
		(*into)->UploadToGPU();
	};

	OGLMesh* newMesh;
	loadFunc(filename, &newMesh);

	meshes.emplace(filename, newMesh);

	return newMesh;
}

NCL::MeshMaterial* OGLResourceManager::LoadMaterial(const string filename, vector<TextureBase*>& textureBuffer) {
	MeshMaterial* material = nullptr;
	std::filesystem::path path = Assets::MESHDIR + filename;
	if (!std::filesystem::exists(path)) {
		return nullptr;
	}

	if (materials.find(filename) != materials.end()) {
		material = materials[filename];
	}

	if (!material) material = new MeshMaterial(filename);

	if (material) {
		for (int i = 0; i < material->GetNumberOfLayers(); i++) {
			const MeshMaterialEntry* entry = material->GetMaterialForLayer(i);

			const string* filename = nullptr;
			entry->GetEntry("Diffuse", &filename);
			TextureBase* diffuse = LoadTexture(*filename);
			if (diffuse) textureBuffer.push_back(diffuse);
		}
		for (int i = 0; i < material->GetNumberOfLayers(); i++) {
			const string* filename = nullptr;
			const MeshMaterialEntry* entry = material->GetMaterialForLayer(i);
			entry->GetEntry("Bump", &filename);
			TextureBase* bump = LoadTexture(*filename);
			if (bump) textureBuffer.push_back(bump);
		}
		if (!textureBuffer.empty()) materials.emplace(filename, material);
	}

	return material;
}

TextureBase* OGLResourceManager::LoadTexture(string filename) {
	std::filesystem::path path = Assets::TEXTUREDIR + filename;

	if (!std::filesystem::exists(path)) {
		return nullptr;
	}

	if (textures.find(filename) != textures.end()) {
		return textures[filename];
	}

	TextureBase* newTex = TextureLoader::LoadAPITexture(filename);
	textures.emplace(filename, newTex);

	return newTex;

}

ShaderBase* OGLResourceManager::LoadShader(string shaderVert, string shaderFrag, string shaderGeom) {
	std::filesystem::path vertPath = Assets::SHADERDIR + shaderVert;
	std::filesystem::path fragPath = Assets::SHADERDIR + shaderFrag;
	if (!std::filesystem::exists(vertPath) || !std::filesystem::exists(fragPath)) {
		return nullptr;
	}
	string name = shaderVert + shaderFrag + shaderGeom;
	if (shaders.find(name) != shaders.end()) {
		return shaders[name];
	}

	ShaderBase* newShader = new OGLShader(shaderVert, shaderFrag, shaderGeom);
	shaders.emplace(name, newShader);

	return newShader;
}

NCL::MeshAnimation* OGLResourceManager::LoadAnimation(string filename) {
	std::filesystem::path path = Assets::ANIMDIR + filename;
	if (!std::filesystem::exists(path)) {
		return nullptr;
	}

	if (animations.find(filename) != animations.end()) {
		return animations[filename];
	}

	MeshAnimation* newAnim = new MeshAnimation(filename);

	animations.emplace(filename, newAnim);

	return newAnim;
}
