#include "RenderObject.h"
#include "Common/Math/Maths.h"
#include "Common/Graphics/MeshGeometry.h"
#include "../CSC8503Common/Transform.h"

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

void RenderObject::Update(float dt) {
	/*if (parent) {
		transform = Transform(parent->GetTransform()->GetMatrix() * localTransform->GetMatrix());
	}
	else {
		transform = transform;
	}*/

	if (animation) {
		frameTime -= dt;
		while (frameTime < 0.0f) {
			for (int i = 0; i < 1; i++) {
				currentFrame = (currentFrame + 1) % animation->GetFrameCount();
				frameTime += 1.0f / animation->GetFrameRate();
			}
		}
	}
}