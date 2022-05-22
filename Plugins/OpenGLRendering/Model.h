#pragma once
#include <string>
#include <vector>
#include "OGLMesh.h"
#include "OGLShader.h"
#include "OGLTexture.h"
#include "../CSC8503/CSC8503Common/SystemDefines.h"

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

namespace NCL {

// Adapted from https://learnopengl.com/Model-Loading/Model

    namespace CSC8503 {
        class GameObject;
        class RenderObject;
    }

	namespace Rendering {
        class ResourceManager;
        class OGLResourceManager;

        using std::string;
        using std::vector;
		class Model {
        public:
            Model(char* path, ResourceManager* manager) {
                resourceManager = (OGLResourceManager*)manager;
                LoadModel(path);
            }

            ~Model() {
                DeleteVector(meshes);
                DeleteVector(objects);
            }

            vector<CSC8503::GameObject*> objects;

        private:
            // model data
            vector<MeshGeometry*> meshes;
            string directory;

            void LoadModel(string path);
            void ProcessNode(aiNode* node, const aiScene* scene);
            CSC8503::GameObject* ProcessMesh(aiMesh* mesh, const aiScene* scene);
            vector<OGLTexture*> LoadMaterialTextures(aiMaterial* mat, aiTextureType type,
                string typeName);

            OGLResourceManager* resourceManager;
		};
	}
}
