#pragma once
#include <string>
#include <vector>
#include "OGLMesh.h"
#include "OGLShader.h"
#include "OGLTexture.h"

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

namespace NCL {
    class RenderObject;

    namespace CSC8503 {
        class GameObject;
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

            vector<CSC8503::GameObject*> objects;

        private:
            // model data
            //vector<OGLMesh> meshes;
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
