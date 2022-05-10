#include "RenderObject.h"
#include "../../Common/MeshGeometry.h"

using namespace NCL::CSC8503;
using namespace NCL;

ResourceManager* RenderObject::manager = nullptr;

RenderObject::RenderObject(Transform* parentTransform, MeshGeometry* mesh, TextureBase* tex, ShaderBase* shader, Vector4 colour) {
	this->transform = parentTransform;
	this->mesh = mesh;
	this->texture = tex;
	textures.emplace_back(tex);
	this->shader = shader;
	this->colour = colour;
}

RenderObject::RenderObject(Transform* parentTransform, MeshGeometry* mesh, vector<TextureBase*> textures, ShaderBase* shader, Vector4 colour) {
	this->transform = parentTransform;
	this->mesh = mesh;
	this->textures = textures;
	this->texture = textures[0];
	this->shader = shader;
	this->colour = colour;
}

RenderObject::RenderObject(Transform* parentTransform, MeshGeometry* mesh, string material, ShaderBase* shader, Vector4 colour) {
	this->transform = parentTransform;
	this->mesh = mesh;
	vector<TextureBase*> textures;
#ifdef _WIN64
	manager->LoadMaterial(material, textures);
#endif
	this->textures = textures;
	this->texture = textures[0];
	this->shader = shader;
	this->colour = colour;
}

RenderObject::~RenderObject() {

}