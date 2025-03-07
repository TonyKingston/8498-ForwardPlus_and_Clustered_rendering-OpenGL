#include "pch.h"
#include "ResourceManager.h"
#include "MeshGeometry.h"
#include "MeshMaterial.h"
#include "MeshAnimation.h"
#include "TextureBase.h"
#include "ShaderBase.h"

using namespace NCL::Rendering;
ResourceManager::~ResourceManager() {
	DeleteMap(meshes);
	DeleteMap(materials);
#ifdef _WIN64
	DeleteMap(animations);
#endif
	DeleteMap(textures);
	DeleteMap(shaders);
}
