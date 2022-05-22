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
#include <random>

namespace NCL {
	class Maths::Vector3;
	class Maths::Vector4;
	namespace CSC8503 {
		class RenderObject;
		//class D_GUI;
#define TILE_SIZE 16 // 16x16 tiles

// Using the same values as Doom 2016
#define CLUSTER_GRID_X 16
#define CLUSTER_GRID_Y 8
#define CLUSTER_GRID_Z 24

#define MAX_LIGHTS_PER_TILE 64

		const unsigned int numClusters = CLUSTER_GRID_X * CLUSTER_GRID_Y * CLUSTER_GRID_Z;

		struct Light {
			Vector4 colour;
			Vector4 position;
			Vector4 radius;
		};

		/*struct Light {
			Vector3 position;
			float radius;
			Vector4 colour;
		};*/


		struct TileAABB {
			Vector4 min;
			Vector4 max;
			Vector4 extent;
		};

		struct TilePlane {
			Vector4 normal;
			Vector4 distance;
		};
	
		struct TileFrustum {
			TilePlane plane[4];
		};

		struct LightGrid {
			unsigned int count;
			unsigned int lightIndices[MAX_LIGHTS_PER_TILE];
		};


		class GameTechRenderer : public OGLRenderer {
		public:
			GameTechRenderer(GameWorld& w, ResourceManager* rm, int type = 0);
			~GameTechRenderer();

			bool inSplitScreen = false;

			void RenderStartView();
			void ResizeSceneTextures(float width, float height);

			void UpdateLights(float dt);
			void AddLights(int n);

		protected:
			void RenderFrame()	override;
			void BeginFrame() override;

			void InitForward(bool withPrepass = false);
			void InitDeferred();
			void InitForwardPlus();
			void InitClustered();
			void InitLights();

			void ComputeTileGrid();
			void ComputeClusterGrid();

			void RenderForward(bool withPrepass = false);
			void RenderDeferred();
			void RenderForwardPlus();
			void RenderClustered();

			void DepthPrePass();

			void ForwardPlusCullLights();
			void ClusteredCullLights();

			Matrix4 SetupDebugLineMatrix()	const override;
			Matrix4 SetupDebugStringMatrix()const override;

			OGLShader* defaultShader;

			GameWorld& gameWorld;
			//D_GUI* gameUI;

			void BuildObjectList(Camera* current_camera);
			void SortObjectList();
			void RenderShadowMap();
			void RenderCamera(Camera* current_camera);
			void RenderCameraPlus(Camera* current_camera);
			void RenderSkybox(Camera* current_camera);
			void LoadSkybox();

			void FillBuffers(Camera* current_camera, float depth);
			void DrawPointLights(Camera* current_camera);
		//	void DrawPaintDecals(Camera* current_camera);
			void CombineBuffers(Camera* current_camera);

			void GenerateScreenTexture(GLuint& into, bool depth = false);
			void GenerateShadowBuffer(GLuint& into);

			void BindAndDraw(RenderObject* obj, bool hasDiff, bool hasBump);

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
			OGLShader* forwardPlusShader;
			OGLShader* forwardPlusGridShader;
			OGLShader* forwardPlusCullShader;
			OGLShader* depthPrepassShader;
			OGLShader* debugShader;

			GLuint bufferFBO;
			GLuint depthColourTex;
			GLuint bufferColourTex, bufferNormalTex, bufferDepthTex;
			GLuint bufferShadowTex; 
			GLuint pointLightFBO;
			GLuint lightDiffuseTex, lightSpecularTex;

			GLuint bufferFinalTex;

			GLuint forwardPlusFBO;

			
			GLuint lightSSBO;
			GLuint lightGridSSBO;
			GLuint aabbGridSSBO;
			GLuint globalListSSBO;
			GLuint globalCountSSBO;
			int tilesX;
			int tilesY;
			unsigned int totalNumLights;

			Vector4		lightColour;
			float		lightRadius;
			Vector3		lightPosition;

			//vice camera things
			OGLShader* printShader;
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

			int renderMode;
			int numLights = 0;
			float lightDt = 0.2f;

			Matrix4 viewMat;
			Matrix4 projMat;
			float aspect;

			// clustered 
			float scaleFactor;
			float biasFactor;


			std::mt19937 lightGen;
			std::uniform_real_distribution<> lightDist;
		};
	}
}

