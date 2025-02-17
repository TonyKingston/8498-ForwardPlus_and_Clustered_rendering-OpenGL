#pragma once
#include "Common/Graphics/ResourceManager.h"

namespace NCL {
	namespace Rendering {

		using std::map;
		using std::string;

		class OGLResourceManager : public Singleton<OGLResourceManager>, public ResourceManager {
		public:

			NCL::MeshGeometry* LoadMesh(string filename) override;
			NCL::MeshMaterial* LoadMaterial(const string filename, vector<TextureBase*>& textureBuffer) override;
			TextureBase* LoadTexture(string filename) override;
			ShaderBase* LoadShader(string shaderVert, string shaderFrag, string shaderGeom = "") override;
			ShaderBase* LoadShader(string shaderCompute);
			NCL::MeshAnimation* LoadAnimation(string filename) override;

			friend class Singleton<OGLResourceManager>;
		protected:
			OGLResourceManager() = default;
		};
	}
}
