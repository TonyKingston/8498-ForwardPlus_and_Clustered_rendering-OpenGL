#include "Model.h"
#include "OGLResourceManager.h"
#include <fstream>
#include "../Common/Assets.h"
#include "../CSC8503/CSC8503Common/Transform.h"
#include "../CSC8503/CSC8503Common/GameObject.h"

using namespace NCL;
using namespace NCL::Rendering;
using namespace NCL::CSC8503;
using namespace NCL::Maths;

struct Vertex {
	Vector3 position;
	Vector3 normal;
	Vector2 texCoords;
	Vector3 tangent;
};

void Model::LoadModel(string path) {
	Assimp::Importer importer;
	const aiScene* scene = importer.ReadFile(Assets::DATADIR + path, aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_CalcTangentSpace);

	if (!scene || scene->mFlags == AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
		std::cout << "ERROR::ASSIMP:: " << importer.GetErrorString() << std::endl;
		return;
	}

	this->directory = path.substr(0, path.find_last_of('/'));
	this->ProcessNode(scene->mRootNode, scene);
}

void Model::ProcessNode(aiNode* node, const aiScene* scene) {
	// Process each mesh located at the current node
	for (GLuint i = 0; i < node->mNumMeshes; i++) {
		aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
		GameObject* obj = this->ProcessMesh(mesh, scene);
		this->objects.push_back(obj);
	}

	// After we've processed all of the meshes (if any) we then recursively process each of the children nodes
	for (GLuint i = 0; i < node->mNumChildren; i++) {
		this->ProcessNode(node->mChildren[i], scene);
	}
}

GameObject* Model::ProcessMesh(aiMesh* mesh, const aiScene* scene) {
	// Data to fill
	vector<Vertex> vertices;
	vector<GLuint> indices;
	vector<OGLTexture*> textures;

	// Walk through each of the mesh's vertices
	for (GLuint i = 0; i < mesh->mNumVertices; i++) {
		Vertex vertex;
		Vector3 vector;

		// Positions
		vector.x = mesh->mVertices[i].x;
		vector.y = mesh->mVertices[i].y;
		vector.z = mesh->mVertices[i].z;
		vertex.position = vector;

		// Normals
		vector.x = mesh->mNormals[i].x;
		vector.y = mesh->mNormals[i].y;
		vector.z = mesh->mNormals[i].z;
		vertex.normal = vector;

		// Texture Coordinates
		if (mesh->mTextureCoords[0]) {
			// Check if the mesh contains texture coordinates
			Vector2 vec;
			vec.x = mesh->mTextureCoords[0][i].x;
			vec.y = mesh->mTextureCoords[0][i].y;
			vertex.texCoords = vec;
		}
		else {
			vertex.texCoords = Vector2();
		}

		// Tangent
		vector.x = mesh->mTangents[i].x;
		vector.y = mesh->mTangents[i].y;
		vector.z = mesh->mTangents[i].z;
		vertex.tangent = vector;

		// Push onto the vector of vertices
		vertices.push_back(vertex);
	}

	// Loop through each of the mesh's faces and get its vertex indices
	for (GLuint i = 0; i < mesh->mNumFaces; i++) {
		aiFace face = mesh->mFaces[i];
		for (GLuint j = 0; j < face.mNumIndices; j++) {
			indices.push_back(face.mIndices[j]);
		}
	}

	// Process materials
	if (mesh->mMaterialIndex >= 0) {
		aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];

		// Diffuse maps
		std::vector<OGLTexture*> diffuseMaps = this->LoadMaterialTextures(material, aiTextureType_DIFFUSE, "texture_diffuse");
		textures.insert(textures.end(), diffuseMaps.begin(), diffuseMaps.end());

		// Normal maps
		std::vector<OGLTexture*> normalMaps = this->LoadMaterialTextures(material, aiTextureType_HEIGHT, "texture_normal");
		textures.insert(textures.end(), normalMaps.begin(), normalMaps.end());
	}

	GameObject* obj = new GameObject();
	return obj;
	//return OGLMesh(vertices, indices, textures);
	//return new RenderObject(Transform(),);
}

vector<OGLTexture*> Model::LoadMaterialTextures(aiMaterial* mat, aiTextureType type, string typeName) {
	vector<OGLTexture*> textures;

	for (GLuint i = 0; i < mat->GetTextureCount(type); i++) {
		aiString str;

		textures.push_back((OGLTexture*)resourceManager->LoadTexture(str.C_Str()));
		
	}
	return textures;
}