#pragma once
#include "Common/Graphics/TextureBase.h"
#include "Common/Graphics/ShaderBase.h"
#include "Common/Graphics/MeshMaterial.h"
#include "Common/Graphics/MeshAnimation.h"
#include "Common/Graphics/ResourceManager.h"
#include "Common/Math/Maths.h"
#include <vector>

namespace NCL {
	using namespace NCL::Rendering;
	using std::vector;
	class MeshGeometry;

	namespace Maths {
		class Vector4;
	}
	namespace CSC8503 {
		class Transform;

		class RenderObject {
		public:
			RenderObject(Transform* parentTransform, MeshGeometry* mesh, TextureBase* tex, ShaderBase* shader, Maths::Vector4 colour = Maths::Vector4(1, 1, 1, 1));
			RenderObject(Transform* parentTransform, MeshGeometry* mesh, vector<TextureBase*> textures, ShaderBase* shader, Maths::Vector4 colour = Maths::Vector4(1, 1, 1, 1));
			RenderObject(Transform* parentTransform, MeshGeometry* mesh, string material, ShaderBase* shader, Maths::Vector4 colour = Maths::Vector4(1, 1, 1, 1));
			~RenderObject();

			static void InitManager(ResourceManager* resourceManager) {
				manager = resourceManager;
			}

			void Update(float dt);

			void SetDefaultTexture(TextureBase* t) {
				texture = t;
			}

			TextureBase* GetDefaultTexture() const {
				return texture;
			}

			TextureBase* GetBumpTexture() const {
				if (textures.size() > 1) {
					return textures[1];
				}
				return nullptr;
			}

			vector<TextureBase*> GetTextures() const {
				return textures;
			}

			void SetSpecularTextures(vector<TextureBase*> texes) {
				specularTextures = texes;
			}

			vector<TextureBase*> GetSpecTextures() const {
				return specularTextures;
			}

			MeshGeometry* GetMesh() const {
				return mesh;
			}

			MeshAnimation* GetAnimation() const {
				return animation;
			}

			void SetAnimation(MeshAnimation* anim) {
				animation = anim;
			}

			void SetAnimationList(MeshAnimation** anim) {
				animList = anim;
			}

			int GetCurrentFrame() const {
				return currentFrame;
			}

			void ResetCurrentFrame() {
				currentFrame = 0;
			}

			Transform* GetTransform() const {
				return transform;
			}

			ShaderBase* GetShader() const {
				return shader;
			}

			void SetColour(const Maths::Vector4& c) {
				colour = c;
			}

			Maths::Vector4 GetColour() const {
				return colour;
			}

			void AddChild(RenderObject* o) {
				o->parent = this;
				children.push_back(o);
			}

			vector<RenderObject*> GetChildren() {
				return children;
			}

			std::vector < RenderObject* >::const_iterator GetChildIteratorStart() {
				return children.begin();
			}

			std::vector < RenderObject* >::const_iterator GetChildIteratorEnd() {
				return children.end();
			}

			float GetBoundingRadius() const { return boundingRadius; }
			void SetBoundingRadius(float f) { boundingRadius = f; }

			float GetCameraDistance() const { return distanceFromCamera; }
			void SetCameraDistance(float f) { distanceFromCamera = f; }

			static bool CompareByCameraDistance(RenderObject* a, RenderObject* b) {
				return (a->distanceFromCamera < b->distanceFromCamera) ? true : false;
			}
			void setAnimationIndex(int ats) {
				currentAnim = ats;
			}

			MeshAnimation** animList;


			//Default colour
			Vector4 GetDefaultColour() {
				return dColour;
			}

			void SetDefaultColour(Vector4 defaultColour) {
				dColour = defaultColour;
			}

			void SetHasMask(bool val) {
				hasMask = val;
			}

			bool HasMask() {
				return hasMask;
			}

		protected:
			MeshGeometry* mesh;
			TextureBase* texture;
			vector<TextureBase*> textures;
			vector<TextureBase*> diffuseTextures;
			vector<TextureBase*> bumpTextures;
			vector<TextureBase*> specularTextures;
			ShaderBase* shader;
			Transform* transform;
			Transform* localTransform;
			Maths::Vector4			colour;
			Maths::Vector4          dColour = Vector4();

			bool hasMask = false;


			MeshAnimation* animation;
			int             currentFrame;
			int				currentAnim;

			RenderObject* parent;
			vector<RenderObject*> children;

			float distanceFromCamera;
			float boundingRadius = 200;

			static ResourceManager* manager;
			float frameTime;
		};
	}
}
