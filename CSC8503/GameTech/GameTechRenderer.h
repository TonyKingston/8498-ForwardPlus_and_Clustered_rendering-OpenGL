#pragma once

#include "Plugins/OpenGLRendering/OGLRenderer.h"
#include "Plugins/OpenGLRendering/OGLShader.h"
#include "Plugins/OpenGLRendering/OGLTexture.h"
#include "Plugins/OpenGLRendering/OGLMesh.h"
#include "Common/Math/Frustum.h"
#include "Common/Math/MathsFwd.h"

#include "CSC8503Common/GameWorld.h"
#include "CSC8503Common/NavigationMesh.h"
#include "Plugins/OpenGLRendering/OGLResourceManager.h"
#include "Assets/Shaders/Shared/SharedFwd.h"
#include <fstream>
#include <random>
#include <span>

namespace NCL {
	namespace CSC8503 {
		class RenderObject;
		//typedef OGLShaderStorageBuffer SSBO;
#define TILE_SIZE 16 // 16x16 tiles

// Using the same values as Doom 2016
#define CLUSTER_GRID_X 16
#define CLUSTER_GRID_Y 8
#define CLUSTER_GRID_Z 24

#define MAX_LIGHTS_PER_TILE 2048
#define LIGHT_RADIUS 40.0 / WORLD_SCALE

		constexpr unsigned int numClusters = CLUSTER_GRID_X * CLUSTER_GRID_Y * CLUSTER_GRID_Z;

		struct ClusterParams {
			float scaleFactor = 1.0f;
			float biasFactor = 1.0f;
			float ratio = 1.0f;
		};

		class GameTechRenderer : public OGLRenderer {
		public:
			GameTechRenderer(GameWorld& w, ResourceManager* rm, int type = 0, bool prepass = false);
			~GameTechRenderer();

			bool inSplitScreen = false;
			bool inDebugMode = false;

			void RenderStartView();
			void ResizeSceneTextures(float width, float height);

			// Updates the positon of all lights in the SSBO on the CPU.
			void UpdateLights(float dt);

			// Dispatches a compute shader to update the position of all lights in the SSBO.
			void UpdateLightsGPU(float dt);

			/// <summary>
			/// Adds N lights to the scene with random positions and colours.
			/// </summary>
			/// <param name="n">True if any lights were added, false if none were</param>
			/// <returns></returns>
			bool AddLights(uint n);

			/// <summary>
			/// Use to add lights to the scene with manually specified attributes.
			/// </summary>
			/// <param name="newLights">Span of lights to be added to the scene. 
			///	Will be truncated to not exceed MAX_LIGHTS.</param>
			/// <returns>True if any lights were added, false if none were</returns>
			bool AddLights(std::span<const PointLight> newLights);

			uint GetNumLight() const {
				return numLights;
			}

			int GetRenderingMode() const {
				return renderMode;
			}

			void InitLights(bool addDebugLights = false);

			void ToggleDebugMode() {
				inDebugMode = !inDebugMode;
			}


		protected:
			virtual void RenderFrame()	override final;
			virtual void BeginFrame() override final;

			void InitForward(bool withPrepass = false);
			void InitDeferred();
			void InitForwardPlus();
			void InitClustered(bool withPrepass = false);

			void ComputeTileGrid();
			void ComputeClusterGrid();
			void ComputeActiveClusters();
			void CompactClusterList();

			void RenderForward(bool withPrepass = false);
			void RenderDeferred();
			void RenderForwardPlus();
			void RenderClustered(bool withPrepass = false);

			void GenPrePassFBO();

			void DepthPrePass();

			void ForwardPlusCullLights();
			void ClusteredCullLights();

			virtual Matrix4 SetupDebugLineMatrix()	const override final;
			virtual Matrix4 SetupDebugStringMatrix() const override final;

			OGLShader* defaultShader;

			GameWorld& gameWorld;

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

			// Updates the currently bound shader with projMat and viewMat;
			void UpdateShaderMatrices();

			// Appends the span of lights to an SSBO on the GPU and increments numLights
			void UploadLights(std::span<const PointLight> lights);

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
			OGLShader* lightUpdateShader;
			OGLShader* pointLightShader;
			OGLShader* combineShader;
			OGLShader* forwardPlusShader;
			OGLShader* forwardPlusGridShader;
			OGLShader* forwardPlusCullShader;
			OGLShader* depthPrepassShader;
			OGLShader* debugShader;

			GLuint bufferFBO;
			GLuint depthColourTex;
			GLuint bufferColourTex, bufferNormalTex, bufferSpecTex, bufferDepthTex;
			GLuint bufferShadowTex; 
			GLuint pointLightFBO;
			GLuint lightDiffuseTex, lightSpecularTex;

			GLuint bufferFinalTex;

			GLuint forwardPlusFBO;

			
			GLuint lightSSBO;
			GLuint lightGridSSBO;
			GLuint aabbGridSSBO;
			GLuint activeClusterSSBO;

			// Test/debug buffers.
			GLuint globalListSSBO;
			GLuint globalCountSSBO;
			GLuint activeCountSSBO;

			int tilesX;
			int tilesY;
			int clusterX;
			int clusterY;
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
			bool usingPrepass = false;
			uint numLights = 0;
			float lightDt = 0.2f;

			Matrix4 viewMat;
			Matrix4 projMat;
			float aspect;
			
			// clustered 
			ClusterParams clusterParams;

			std::mt19937 lightGen;
			std::uniform_real_distribution<> lightDist;
			static constexpr Vector3 LIGHT_MIN_BOUNDS = Vector3(-560.0f, 0.0f, -230.0f) / WORLD_SCALE;
			static constexpr Vector3 LIGHT_MAX_BOUNDS = Vector3(510.0f, 400.0f, 220.0f) / WORLD_SCALE;

			// Clump lights in center corridor
			/*const Vector3 LIGHT_MIN_BOUNDS = Vector3(-300.0f, 0.0f, -80.0f) / WORLD_SCALE;
			const Vector3 LIGHT_MAX_BOUNDS = Vector3(250.0f, 250.0f, 60.0f) / WORLD_SCALE;*/

			//Depth disparity test
			/*const Vector3 LIGHT_MIN_BOUNDS = Vector3(-350.0f, 0.0f, -100.0f);
			const Vector3 LIGHT_MAX_BOUNDS = Vector3(300.0f, 420.0f, 80.0f);*/
		};
	}
}

