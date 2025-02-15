#pragma once
#include "Common/Graphics/ResourceManager.h"

namespace NCL {
	namespace Rendering {

		using std::map;
		using std::string;

		class OGLResourceManager : public ResourceManager {
		public:
			OGLResourceManager() {}
			~OGLResourceManager();
			NCL::MeshGeometry* LoadMesh(string filename);
			NCL::MeshMaterial* LoadMaterial(const string filename, vector<TextureBase*>& textureBuffer);
			TextureBase* LoadTexture(string filename);
			ShaderBase* LoadShader(string shaderVert, string shaderFrag, string shaderGeom = "");
			ShaderBase* LoadShader(string shaderCompute);
			NCL::MeshAnimation* LoadAnimation(string filename);

		private:

		};
	}
}
