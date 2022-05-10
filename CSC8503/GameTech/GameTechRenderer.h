#pragma once

#include "../../Plugins/OpenGLRendering/OGLRenderer.h"
#include "../../Plugins/OpenGLRendering/OGLShader.h"
#include "../../Plugins/OpenGLRendering/OGLTexture.h"
#include "../../Plugins/OpenGLRendering/OGLMesh.h"
#include "../../Common/Frustum.h"

#include "../CSC8503Common/GameWorld.h"
#include "../CSC8503Common/NavigationMesh.h"
#include "../../Plugins/OpenGLRendering/OGLResourceManager.h"
#include <fstream>

namespace NCL {
	class Maths::Vector3;
	class Maths::Vector4;
	namespace CSC8503 {
		class RenderObject;
		class D_GUI;

		class GameTechRenderer : public OGLRenderer {
		public:
			GameTechRenderer(GameWorld& w, D_GUI* UI, ResourceManager* rm);
			~GameTechRenderer();

			bool inSplitScreen = false;
			bool ShowNavMesh = false;

			void RenderStartView();
			void ResizeSceneTextures(float width, float height);
		protected:
			void RenderFrame()	override;
			void BeginFrame() override;

			Matrix4 SetupDebugLineMatrix()	const override;
			Matrix4 SetupDebugStringMatrix()const override;

			OGLShader* defaultShader;

			GameWorld& gameWorld;
			D_GUI* gameUI;

			void BuildObjectList(Camera* current_camera);
			void SortObjectList();
			void RenderShadowMap();
			void RenderCamera(Camera* current_camera);
			void RenderSkybox(Camera* current_camera);
			void LoadSkybox();

			void FillBuffers(Camera* current_camera, float depth);
			void DrawPointLights(Camera* current_camera);
			void DrawPaintDecals(Camera* current_camera);
			void CombineBuffers(Camera* current_camera);

			void GenerateScreenTexture(GLuint& into, bool depth = false);
			void GenerateShadowBuffer(GLuint& into);

			vector<RenderObject*> activeObjects;
			RenderObject* root;
			Frustum      frameFrustum;
			Frustum      viceFrustum;

			OGLShader* skyboxShader;
			OGLMesh* skyboxMesh;
			GLuint		skyboxTex;

			//shadow mapping things
			OGLShader* shadowShader;
			GLuint		shadowTex;
			GLuint		shadowFBO;
			Matrix4     shadowMatrix;

			OGLShader* sceneShader;
			OGLShader* pointLightShader;
			OGLShader* combineShader;

			GLuint bufferFBO;
			GLuint bufferColourTex;
			GLuint bufferNormalTex;
			GLuint bufferShadowTex;
			GLuint bufferDepthTex;
			GLuint pointLightFBO;
			GLuint lightDiffuseTex;
			GLuint lightSpecularTex;

			GLuint bufferFinalTex;

			GLuint decalFBO;
			GLuint decalScoreTex;

			Vector4		lightColour;
			float		lightRadius;
			Vector3		lightPosition;

			//vice camera things
			OGLShader* print_shader;
			OGLShader* split_shader;
			OGLMesh* printer;
			GLuint printFBO;
			GLuint print_Tex;
			GLuint print_depth_Tex;

			GLuint skyboxFBO;
			GLuint skybox_tex;
			GLuint buffer_colour_depth_tex;
			GLuint depthFBO;

			OGLMesh* sphere;
			OGLMesh* quad;

			void LoadPrinter();
			void PresentScene(bool split, GLfloat offset);
			void SplitRender();

			//NavMesh Related
			OGLMesh* navMesh;
			OGLMesh* centroidMesh;
			OGLShader* NavMesh_shader;
			void LoadNavMesh();
			void RenderNavMesh(Camera* current_camera);

			//NavMesh02 Related
			OGLMesh* navMesh02;
			OGLMesh* centroidMesh02;
			void LoadNavMesh02();
			void RenderNavMesh02(Camera* current_camera);

			// Paint tech
			std::vector<GameObject*> decalsToBeGenerated; // paint decals to be generated from cubes this frame
			OGLTexture* paintTex;
			OGLShader* decalShader;
			OGLResourceManager* resourceManager;
			//start image
			void LoadStartImage();
			GLuint background_tex;
			GLuint loading_tex;
			OGLShader* loading_shader;
			bool loading;

			vector<GLuint> sceneTextures;
			vector<GLuint> screenTextures;
			vector<GLuint> sceneBuffers;
		};
	}
}

