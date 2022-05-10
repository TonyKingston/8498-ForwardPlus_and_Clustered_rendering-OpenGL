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
	namespace Rendering {

        using std::string;
        using std::vector;
		class Model {
        public:
            Model(char* path) {
                loadModel(path);
            }
            void Draw(OGLShader& shader);
        private:
            // model data
            vector<OGLMesh> meshes;
            string directory;

            void loadModel(string path);
            void processNode(aiNode* node, const aiScene* scene);
            OGLMesh processMesh(aiMesh* mesh, const aiScene* scene);
            vector<OGLTexture> loadMaterialTextures(aiMaterial* mat, aiTextureType type,
                string typeName);
		};
	}
}
