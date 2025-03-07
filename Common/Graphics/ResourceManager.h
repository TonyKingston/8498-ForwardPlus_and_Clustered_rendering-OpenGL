#pragma once
#include <string>
#include <unordered_map>
#include <vector>
#include "Misc.h"
#include "../CSC8503/CSC8503Common/SystemDefines.h"

namespace NCL {

	class MeshGeometry;
	class MeshMaterial;
	class MeshAnimation;

	using std::vector;
	using std::string;
	using std::unordered_map;

	namespace Rendering {

		class TextureBase;
		class ShaderBase;

		class ResourceManager {
		public:

			virtual ~ResourceManager();

			virtual MeshGeometry* LoadMesh(string fileName) = 0;
			virtual ShaderBase* LoadShader(string shaderVert, string shaderFrag, string shaderGeom = "") = 0;
			virtual TextureBase* LoadTexture(string filename) = 0;
			virtual MeshMaterial* LoadMaterial(string fileame, vector<TextureBase*>& textureBuffer) = 0;

#ifdef _WIN64
			virtual MeshAnimation* LoadAnimation(string filename) = 0;
#endif
		protected:
			ResourceManager() = default;

#ifdef _WIN64

			unordered_map<string, MeshAnimation*> animations;
#endif
			unordered_map<string, MeshMaterial*> materials;
			unordered_map<string, MeshGeometry*> meshes;
			unordered_map<string, TextureBase*> textures;
			unordered_map<string, ShaderBase*> shaders;
		};
			
		template <typename Derived>
		class SingletonResourceManager : public ResourceManager, public Singleton<Derived> {
		protected:
			SingletonResourceManager() = default;
			virtual ~SingletonResourceManager() = default;
		};

	};
};