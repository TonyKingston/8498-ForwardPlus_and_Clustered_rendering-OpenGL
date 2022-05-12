#include "Model.h"
#include "OGLResourceManager.h"
#include <fstream>
#include "../Common/Assets.h"
#include "../CSC8503/CSC8503Common/Transform.h"
#include "../CSC8503/CSC8503Common/GameObject.h"
#include "../CSC8503/CSC8503Common/RenderObject.h"

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
	const aiScene* scene = importer.ReadFile(Assets::DATADIR + path, aiProcess_Triangulate |
		aiProcess_CalcTangentSpace | aiProcess_PreTransformVertices | aiProcess_OptimizeMeshes | aiProcess_RemoveRedundantMaterials);

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
		obj->GetTransform().SetPosition(Vector3(0, 0, 0))
			.SetScale(Vector3(10, 10, 10));
		//obj->GetTransform().SetPosition();
		//node->mTransformation.
		this->objects.push_back(obj);
	}

	// After we've processed all of the meshes (if any) we then recursively process each of the children nodes
	for (GLuint i = 0; i < node->mNumChildren; i++) {
		this->ProcessNode(node->mChildren[i], scene);
	}
}

GameObject* Model::ProcessMesh(aiMesh* mesh, const aiScene* scene) {
	// Data to fill
	vector<Vector3> positions;
	vector<Vector3> normals;
	vector<Vector2> texCoords;
	vector<Vector4> tangents;
	vector<GLuint> indices;
	vector<TextureBase*> textures;

	// Walk through each of the mesh's vertices
	for (GLuint i = 0; i < mesh->mNumVertices; i++) {
		Vertex vertex;
		Vector3 vector;

		// Positions
		vector.x = mesh->mVertices[i].x;
		vector.y = mesh->mVertices[i].y;
		vector.z = mesh->mVertices[i].z;
		vertex.position = vector;
		positions.push_back(vector);

		// Normals
		vector.x = mesh->mNormals[i].x;
		vector.y = mesh->mNormals[i].y;
		vector.z = mesh->mNormals[i].z;
		normals.push_back(vector);

		// Texture Coordinates
		if (mesh->mTextureCoords[0]) {
			// Check if the mesh contains texture coordinates
			Vector2 vec;
			vec.x = mesh->mTextureCoords[0][i].x;
			vec.y = mesh->mTextureCoords[0][i].y;
			texCoords.push_back(vec);
		}
		else {
			texCoords.push_back(Vector2());
		}

		// Tangent
		Vector4 tangent;
		tangent.x = mesh->mTangents[i].x;
		tangent.y = mesh->mTangents[i].y;
		tangent.z = mesh->mTangents[i].z;
		tangent.w = -1;
		tangents.push_back(vector);
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

		if (textures.size() == 0) {
			textures.push_back(resourceManager->LoadTexture("checkerboad.png"));
		}
	}

	GameObject* obj = new GameObject();
	MeshGeometry* oglMesh = new OGLMesh();
	oglMesh->SetVertexPositions(positions);
	oglMesh->SetVertexNormals(normals);
	oglMesh->SetVertexTangents(tangents);
	oglMesh->SetVertexTextureCoords(texCoords);
	oglMesh->SetVertexIndices(indices);
	SubMesh m;
	m.start = 0;
	m.count = indices.size();
	oglMesh->subMeshes.push_back(m);
	oglMesh->SetPrimitiveType(GeometryPrimitive::Triangles);
	oglMesh->UploadToGPU();

	//obj->SetRenderObject(new RenderObject(&obj->GetTransform(), oglMesh, textures, resourceManager->LoadShader("GameTechVert.glsl", "bufferFragment.glsl")));
	obj->SetRenderObject(new RenderObject(&obj->GetTransform(), oglMesh, textures, resourceManager->LoadShader("GameTechVert.glsl", "GameTechFrag.glsl")));
	meshes.push_back(oglMesh);
	return obj;
	//return OGLMesh(vertices, indices, textures);
	//return new RenderObject(Transform(),);
}

vector<OGLTexture*> Model::LoadMaterialTextures(aiMaterial* mat, aiTextureType type, string typeName) {
	vector<OGLTexture*> textures;

	for (GLuint i = 0; i < mat->GetTextureCount(type); i++) {
		aiString str;
		mat->GetTexture(type, i, &str);

		textures.push_back((OGLTexture*)resourceManager->LoadTexture(str.C_Str()));
		
	}
	return textures;
}