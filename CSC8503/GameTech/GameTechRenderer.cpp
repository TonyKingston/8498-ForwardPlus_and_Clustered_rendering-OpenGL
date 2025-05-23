#include "GameTechRenderer.h"

#include "CSC8503Common/GameObject.h"
#include "Common/Math/Maths.h"
#include "Common/Graphics/Camera.h"
#include "Common/Graphics/TextureLoader.h"
#include "Common/Resources/Assets.h"
#include "Core/Misc/Image.h"
#include <cstddef>

#include "Common/stb/stb_image.h"
#include <random>

#include "Assets/Shaders/Shared/ComputeBindings.h"
#include "Assets/Shaders/Shared/TextureBindings.h"
#include "Assets/Shaders/Shared/LightDefinitions.h"
#include "Assets/Shaders/Shared/LightGridDefinitions.h"

using namespace NCL;
using namespace Rendering;
using namespace CSC8503;

#define SHADOWSIZE 4096

//const unsigned int MAX_LIGHTS = 49152;
constexpr unsigned int MAX_LIGHTS = 98304;

constexpr Vector3 GameTechRenderer::LIGHT_MIN_BOUNDS;
constexpr Vector3 GameTechRenderer::LIGHT_MAX_BOUNDS;

Matrix4 biasMatrix = Matrix4::Translation(Vector3(0.5, 0.5, 0.5)) * Matrix4::Scale(Vector3(0.5, 0.5, 0.5));

GameTechRenderer::GameTechRenderer(GameWorld& w, ResourceManager* rm, int type, bool prepass)
	: OGLRenderer(*Window::GetWindow()), gameWorld(w), renderMode(type), usingPrepass(prepass) {
	//	glEnable(GL_DEPTH_TEST);
	resourceManager = (OGLResourceManager*)rm;

	sphere = (OGLMesh*)resourceManager->LoadMesh("sphere.msh");

	quad = OGLMesh::GenerateQuad();

	shadowShader = new OGLShader("GameTechShadowVert.vert", "GameTechShadowFrag.frag");
	printShader = new OGLShader("PrinterVertex.vert", "PrinterFragment.frag");

	//GenerateShadowBuffer(shadowFBO);

	glClearColor(1, 1, 1, 1);

	//Skybox!
	skyboxShader = new OGLShader("skyboxVertex.vert", "skyboxFragment.frag");
	skyboxMesh = new OGLMesh();
	skyboxMesh->SetVertexPositions({ Vector3(-1, 1,-1), Vector3(-1,-1,-1) , Vector3(1,-1,-1) , Vector3(1,1,-1) });
	skyboxMesh->SetVertexIndices({ 0,1,2,2,3,0 });
	skyboxMesh->UploadToGPU();

	LoadSkybox();


	aspect = (float)currentWidth / (float)currentHeight;
	viewMat = gameWorld.GetMainCamera()->BuildViewMatrix();
	projMat = gameWorld.GetMainCamera()->BuildProjectionMatrix(aspect);

	InitLights(false);
	//glEnable(GL_MULTISAMPLE);
	glEnable(GL_DEPTH_TEST);

	switch (type) {
	case 0:
		InitForward(prepass);
		break;
	case 1:
		InitDeferred();
		break;
	case 2:
		InitForwardPlus();
		break;
	case 3:
		InitClustered(prepass);
		break;
	}

}

void NCL::CSC8503::GameTechRenderer::InitLights(const bool addDebugLights /*false*/) {

	lightUpdateShader = (OGLShader*) resourceManager->LoadShader("updateLights.comp");
	std::random_device rd;
	std::mt19937 gen(rd());
	std::uniform_real_distribution<> dis(0, 1);

	glGenBuffers(1, &lightSSBO);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, lightSSBO);
	glBufferData(GL_SHADER_STORAGE_BUFFER, MAX_LIGHTS * sizeof(PointLight), 0, GL_DYNAMIC_DRAW);

	if (addDebugLights) {
		Vector4 debugRadius = Vector4(40.0f, 0.0f, 0.0f, 0.0f);
		std::vector<PointLight> debugLights = {
			{.colour = Vector4(0.8f, 0.8f, 0.5f, 1.0f), .pos = Vector4(10.0f, 10.0f, 10.0f, 1.0f), .radius = debugRadius},
			{.colour = Vector4(0.0f, 1.0f, 0.0f, 1.0f), .pos = Vector4(80.0f, 10.0f, 10.0f, 1.0f), .radius = debugRadius },
			{.colour = Vector4(0.0f, 0.5f, 1.0f, 1.0f), .pos = Vector4(60.0f, 10.0f, 10.0f, 1.0f), .radius = debugRadius },
			{.colour = Vector4(0.2f, 0.8f, 1.0f, 1.0f), .pos = Vector4(-200.0f, 10.0f, 120.0f, 1.0f), .radius = debugRadius }
		};

		AddLights(debugLights);
	}

	glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
	sceneBuffers.push_back(lightSSBO);
}

void GameTechRenderer::InitForward(bool withPrepass) {
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, lightSSBO);

	if (withPrepass) {
		GenPrePassFBO();
		depthPrepassShader = (OGLShader*)resourceManager->LoadShader("DepthPassVert.vert", "DepthPassFrag.frag");
	}
}

void GameTechRenderer::InitDeferred() {
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, lightSSBO);

	sceneShader = (OGLShader*) resourceManager->LoadShader("GameTechVert.vert", "bufferFragment.frag");
	pointLightShader = new OGLShader("pointlightvertex.vert", "pointlightfrag.frag");
	combineShader = new OGLShader("combinevert.vert", "combinefrag.frag");

	glGenFramebuffers(1, &bufferFBO);
	glGenFramebuffers(1, &pointLightFBO);
	sceneBuffers.push_back(bufferFBO);
	sceneBuffers.push_back(pointLightFBO);

	GLenum buffers[2] = {
		GL_COLOR_ATTACHMENT0 ,
		GL_COLOR_ATTACHMENT1
	};

	GLenum buffers2[3] = {
		GL_COLOR_ATTACHMENT0 ,
		GL_COLOR_ATTACHMENT1,
		GL_COLOR_ATTACHMENT2
	};

	//LoadPrinter();
	//LoadStartImage();

	GenerateScreenTexture(bufferDepthTex, true);
	GenerateScreenTexture(bufferColourTex);
	GenerateScreenTexture(bufferNormalTex);
	//GenerateScreenTexture(bufferSpecTex);
	//GenerateScreenTexture(bufferShadowTex);
	GenerateScreenTexture(lightDiffuseTex);
	GenerateScreenTexture(lightSpecularTex);

	// And now attach them to our FBOs
	// Binding our attachment textures to their respective FBO's, firstly the first FBO 
	glBindFramebuffer(GL_FRAMEBUFFER, bufferFBO);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, bufferColourTex, 0);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, bufferNormalTex, 0);
	//glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, bufferShadowTex, 0);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, bufferDepthTex, 0);
	//glDrawBuffers(3, buffers2);
	glDrawBuffers(2, buffers2);

	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)  return;

	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)  return;

	// Setting up second FBO attachments
	glBindFramebuffer(GL_FRAMEBUFFER, pointLightFBO);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, lightDiffuseTex, 0);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, lightSpecularTex, 0);
	glDrawBuffers(2, buffers);

	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) return;

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	glEnable(GL_BLEND);

	loading = true;
}

void GameTechRenderer::InitForwardPlus() {

	GenPrePassFBO();

	forwardPlusShader = (OGLShader*)resourceManager->LoadShader("GameTechVert.vert", "forwardPlusFrag.frag");
	forwardPlusGridShader = (OGLShader*)resourceManager->LoadShader("forwardplusGrid.comp");
	forwardPlusCullShader = usingPrepass ? (OGLShader*)resourceManager->LoadShader("forwardplusCullAABB.comp") : (OGLShader*)resourceManager->LoadShader("forwardplusCull.comp");
	depthPrepassShader = (OGLShader*)resourceManager->LoadShader("DepthPassVert.vert", "DepthPassFrag.frag");
	debugShader = (OGLShader*)resourceManager->LoadShader("GameTechVert.vert", "forwardPlusDebugFrag.frag");

	tilesX = (currentWidth + (currentWidth % TILE_SIZE)) / TILE_SIZE;
	tilesY = (currentHeight + (currentHeight % TILE_SIZE)) / TILE_SIZE;
	//int sizeX = (unsigned int)std::ceilf(1920 / (float)TILE_SIZE);
	//int sizey = (unsigned int)std::ceilf(1080/ (float)TILE_SIZE);
	size_t numTiles = tilesX * tilesY;
	glGenBuffers(1, &lightGridSSBO);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, lightGridSSBO);
	//glBufferData(GL_SHADER_STORAGE_BUFFER, numTiles * 2 * sizeof(unsigned int), 0, GL_STATIC_COPY);
	//glBufferData(GL_SHADER_STORAGE_BUFFER, numTiles * 2 * sizeof(unsigned int), 0, GL_STATIC_COPY);
	//glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(LightGrid) * numTiles, NULL, GL_STATIC_COPY);
	glBufferData(GL_SHADER_STORAGE_BUFFER, numTiles * sizeof(int) * MAX_LIGHTS_PER_TILE, NULL, GL_STATIC_COPY);


	//glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, lightGridSSBO);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);


	glGenBuffers(1, &aabbGridSSBO);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, aabbGridSSBO);
	//glBufferData(GL_SHADER_STORAGE_BUFFER, numTiles * sizeof(TileAABB), NULL, GL_DYNAMIC_DRAW);
	glBufferData(GL_SHADER_STORAGE_BUFFER, numTiles * sizeof(TileFrustum), NULL, GL_DYNAMIC_DRAW);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, aabbGridSSBO);
//	glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

	glGenBuffers(1, &globalListSSBO);
	totalNumLights = numTiles * MAX_LIGHTS_PER_TILE;
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, globalListSSBO);

	glBufferData(GL_SHADER_STORAGE_BUFFER, totalNumLights * sizeof(unsigned int), NULL, GL_STATIC_COPY);
	//glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(LightGrid) * numTiles, NULL, GL_STATIC_COPY);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, globalListSSBO);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

	glGenBuffers(1, &globalCountSSBO);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, globalCountSSBO);

	glBufferData(GL_SHADER_STORAGE_BUFFER, numTiles * sizeof(float) , NULL, GL_STATIC_COPY);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 4, globalCountSSBO);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, COMPUTE_BINDING_LIGHT_BUFFER, lightSSBO);

	sceneBuffers.push_back(aabbGridSSBO);
	sceneBuffers.push_back(globalListSSBO);
	sceneBuffers.push_back(globalCountSSBO);

	ComputeTileGrid();
//	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, aabbGridSSBO);
}

void GameTechRenderer::InitClustered(bool withPrepass) {

	if (withPrepass) {
		GenPrePassFBO();
		depthPrepassShader = (OGLShader*)resourceManager->LoadShader("DepthPassVert.vert", "DepthPassFrag.frag");
		glGenBuffers(1, &activeClusterSSBO);
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, activeClusterSSBO);
		glBufferData(GL_SHADER_STORAGE_BUFFER, numClusters * sizeof(unsigned int), NULL, GL_STATIC_COPY);
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, COMPUTE_BINDING_TEST5, activeClusterSSBO);
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

		glGenBuffers(1, &activeCountSSBO);
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, activeCountSSBO);
		glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(unsigned int), NULL, GL_STATIC_COPY);
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, COMPUTE_BINDING_TEST6, activeCountSSBO);
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
		forwardPlusCullShader = (OGLShader*)resourceManager->LoadShader("clusterActiveCull.comp");
	}
	else {
		forwardPlusCullShader = (OGLShader*)resourceManager->LoadShader("clusterCull.comp");
	}

	forwardPlusShader = (OGLShader*)resourceManager->LoadShader("GameTechVert.vert", "clusterFrag.frag");
	forwardPlusGridShader = (OGLShader*)resourceManager->LoadShader("clusterGrid.comp");

	glGenBuffers(1, &lightGridSSBO);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, lightGridSSBO);
	glBufferData(GL_SHADER_STORAGE_BUFFER, numClusters * sizeof(int) * MAX_LIGHTS_PER_TILE, NULL, GL_STATIC_COPY);


	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, lightGridSSBO);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);


	glGenBuffers(1, &aabbGridSSBO);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, aabbGridSSBO);
	glBufferData(GL_SHADER_STORAGE_BUFFER, numClusters * sizeof(ClusterFrustum), NULL, GL_DYNAMIC_DRAW);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, COMPUTE_BINDING_GRID_BUFFER, aabbGridSSBO);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

	glGenBuffers(1, &globalListSSBO);
	totalNumLights = numClusters * MAX_LIGHTS_PER_TILE;
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, globalListSSBO);

	glBufferData(GL_SHADER_STORAGE_BUFFER, numClusters * sizeof(int), NULL, GL_STATIC_COPY);
	//glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(LightGrid) * numTiles, NULL, GL_STATIC_COPY);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, globalListSSBO);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

	glGenBuffers(1, &globalCountSSBO);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, globalCountSSBO);

	glBufferData(GL_SHADER_STORAGE_BUFFER, numClusters * sizeof(float), NULL, GL_STATIC_COPY);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 4, globalCountSSBO);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, COMPUTE_BINDING_LIGHT_BUFFER, lightSSBO);

	sceneBuffers.push_back(aabbGridSSBO);
	sceneBuffers.push_back(globalListSSBO);
	sceneBuffers.push_back(globalCountSSBO);

	float zFar = gameWorld.GetMainCamera()->GetFarPlane();
	float zNear = gameWorld.GetMainCamera()->GetNearPlane();
	// Doom 2016

	clusterParams = [&] 
	{
		return ClusterParams{(float)CLUSTER_GRID_Z / std::log2f(zFar / zNear),
			-((float)CLUSTER_GRID_Z * std::log2f(zNear) / std::log2f(zFar / zNear)),
			1};
	}();

	clusterX = (unsigned int)std::ceilf(currentWidth / (float)CLUSTER_GRID_X);
	clusterY = (unsigned int)std::ceilf(currentHeight / (float)CLUSTER_GRID_Y);

	ComputeClusterGrid();
	//	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, aabbGridSSBO);
}

void GameTechRenderer::UpdateLights(float dt) {
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, lightSSBO);
	PointLight* pointLights = (PointLight*) glMapBuffer(GL_SHADER_STORAGE_BUFFER, GL_READ_WRITE);

	for (uint i = 0; i < numLights; i++) {
		PointLight& light = pointLights[i];
		//light.position.x = fmod((light.position.y + (-4.5f * lightDt) - min + max), max) + min;
		//light.position.x = light.position.x;
		float min = LIGHT_MIN_BOUNDS[1];
		float max = LIGHT_MAX_BOUNDS[1];

		light.pos.y = fmod((light.pos.y + (-8.0f * dt) - min + max), max) + min;
	}

	glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
}

void GameTechRenderer::UpdateLightsGPU(float dt) {
	BindShader(lightUpdateShader);

	OGLShader* shader = lightUpdateShader;

	OGLShader::SetUniforms(shader,
		"noOfLights", numLights,
		"minBounds", LIGHT_MIN_BOUNDS,
		"maxBounds", LIGHT_MAX_BOUNDS,
		"dt", dt);

	glDispatchCompute(1, 1, 1);
    glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
}

bool GameTechRenderer::AddLights(uint n) {
	n = std::clamp(n, 0u, MAX_LIGHTS - numLights);

	if (n == 0) return false;

	std::vector<PointLight> newLights(n);

	Vector4 lightPos = Vector4(0.0f, 0.0f, 0.0f, 1.0f);
	for (uint i = 0; i < n; i++) {
		PointLight& light = newLights[i];
		for (int j = 0; j < 3; j++) {
			float min = LIGHT_MIN_BOUNDS[j];
			float max = LIGHT_MAX_BOUNDS[j];
			lightPos[j] = lightDist(lightGen) * (max - min) + min;
		}
		light.colour = Vector4(1.0 - lightDist(lightGen), 1.0 - lightDist(lightGen), 1.0 - lightDist(lightGen), 1.0f);
		light.pos = lightPos;
		light.radius = Vector4(LIGHT_RADIUS, 0.0, 0.0, 0.0);
	}

	UploadLights(newLights);
	return true;
}

bool GameTechRenderer::AddLights(std::span<const PointLight> newLights) {
	uint n = newLights.size();
	// Don't exceed MAX_LIGHTS when adding lights from the span
	n = std::clamp(n, 0u, MAX_LIGHTS - numLights);
	std::span<const PointLight> subSpan = newLights.subspan(0, n);
	if (subSpan.empty()) return false;
	UploadLights(subSpan);
	return true;
}

GameTechRenderer::~GameTechRenderer() {
	for (GLuint buffer : sceneBuffers) {
		glDeleteFramebuffers(1, &buffer);
	}
	for (GLuint tex : sceneTextures) {
		glDeleteTextures(1, &tex);
	}
}

void GameTechRenderer::ComputeTileGrid() {

	int sizeX = (unsigned int)std::ceilf(currentWidth / (float)TILE_SIZE);

	Camera* current = gameWorld.GetMainCamera();
	
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, COMPUTE_BINDING_GRID_BUFFER, aabbGridSSBO);
	BindShader(forwardPlusGridShader);

	Matrix4 invProj = (projMat).Inverse();

	glUniformMatrix4fv(glGetUniformLocation(forwardPlusGridShader->GetProgramID(), "inverseProj"), 1, false, invProj.array);
	glUniform2f(glGetUniformLocation(forwardPlusGridShader->GetProgramID(), "pixelSize"), 1.0f / currentWidth, 1.0f / currentHeight);
	glUniform1i(glGetUniformLocation(forwardPlusGridShader->GetProgramID(), "tilePxX"), sizeX);
//	glUniform1f(glGetUniformLocation(forwardPlusGridShader->GetProgramID(), "near"), current->GetNearPlane());
//	glUniform1f(glGetUniformLocation(forwardPlusGridShader->GetProgramID(), "far"), current->GetFarPlane());
	/*unsigned int query;
	GLuint64 timer;
	glGenQueries(1, &query);
	glBeginQuery(GL_TIME_ELAPSED, query);*/
	glDispatchCompute(tilesX, tilesY, 1);
	/*glEndQuery(GL_TIME_ELAPSED);
	glGetQueryObjectui64v(query, GL_QUERY_RESULT, &timer);
	float time = timer / 1000000.0;
	std::printf("Time spent on the GPU: %f ms\n", time);*/

	//glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
}

void GameTechRenderer::ComputeClusterGrid() {

	Camera* current = gameWorld.GetMainCamera();

	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, COMPUTE_BINDING_GRID_BUFFER, aabbGridSSBO);
	BindShader(forwardPlusGridShader);

	Matrix4 invProj = (projMat).Inverse();

	glUniformMatrix4fv(glGetUniformLocation(forwardPlusGridShader->GetProgramID(), "inverseProj"), 1, false, invProj.array);
	glUniform2f(glGetUniformLocation(forwardPlusGridShader->GetProgramID(), "pixelSize"), 1.0f / currentWidth, 1.0f / currentHeight);
	glUniform1i(glGetUniformLocation(forwardPlusGridShader->GetProgramID(), "tilePxX"), clusterX);
	glUniform1i(glGetUniformLocation(forwardPlusGridShader->GetProgramID(), "tilePxY"), clusterY);
	glUniform1f(glGetUniformLocation(forwardPlusGridShader->GetProgramID(), "near"), current->GetNearPlane());
	glUniform1f(glGetUniformLocation(forwardPlusGridShader->GetProgramID(), "far"), current->GetFarPlane());
	unsigned int query;
	GLuint64 timer;
	glGenQueries(1, &query);
	glBeginQuery(GL_TIME_ELAPSED, query);
	glDispatchCompute(CLUSTER_GRID_X, CLUSTER_GRID_Y, CLUSTER_GRID_Z);
	glEndQuery(GL_TIME_ELAPSED);
	glGetQueryObjectui64v(query, GL_QUERY_RESULT, &timer);
	std::printf("Time spent on the GPU: %f ms\n", timer / 1000000.0);
}

void GameTechRenderer::ComputeActiveClusters() {
	OGLShader* activeShader = (OGLShader*) resourceManager->LoadShader("activeClusters.comp");
	BindShader(activeShader);

	glUniform1i(glGetUniformLocation(activeShader->GetProgramID(), "depthTex"), 0);
	Cmds::BindTexture(0, bufferDepthTex);
	//glActiveTexture(GL_TEXTURE1);
	//glBindTexture(GL_TEXTURE_2D, 0);

	// TODO: Support 2f etc in SetUniforms...
	glUniform2f(glGetUniformLocation(activeShader->GetProgramID(), "pixelSize"), 1.0f / currentWidth, 1.0f / currentHeight);

	OGLShader::SetUniforms(activeShader,
		"projMatrix", projMat,
		"tilePxX", clusterX,
		"tilePxY", clusterY,
		"scale", clusterParams.scaleFactor,
		"bias", clusterParams.biasFactor);

	glDispatchCompute(currentWidth, currentHeight, 1);
	//glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
}

void GameTechRenderer::CompactClusterList() {
	OGLShader* activeShader = (OGLShader*)resourceManager->LoadShader("compactClusters.comp");	
	BindShader(activeShader);

	glDispatchCompute(CLUSTER_GRID_X, CLUSTER_GRID_Y, CLUSTER_GRID_Z);
	//glDispatchCompute(1, 1, 6);
	glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
}

void GameTechRenderer::DepthPrePass() {
	glBindFramebuffer(GL_FRAMEBUFFER, bufferFBO);
	glDepthMask(GL_TRUE);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glColorMask(0, 0, 0, 0);
	glDepthFunc(GL_LESS);
	glEnable(GL_DEPTH_TEST);
	BindShader(depthPrepassShader);

	int projLocation = 0;
	int viewLocation = 0;
	int modelLocation = 0;

	OGLShader* shader = depthPrepassShader;

	projLocation = glGetUniformLocation(shader->GetProgramID(), "projMatrix");
	viewLocation = glGetUniformLocation(shader->GetProgramID(), "viewMatrix");
	modelLocation = glGetUniformLocation(shader->GetProgramID(), "modelMatrix");

	glUniformMatrix4fv(projLocation, 1, false, (float*)&projMat);
	glUniformMatrix4fv(viewLocation, 1, false, (float*)&viewMat);

	for (const auto& i : activeObjects) {

		Matrix4 modelMatrix = (*i).GetTransform()->GetMatrix();
		glUniformMatrix4fv(modelLocation, 1, false, (float*)&modelMatrix);

		//glUniform1i(hasTexLocation, (OGLTexture*)(*i).GetDefaultTexture() ? 1 : 0);
		bool hasDiff = (OGLTexture*)(*i).GetDefaultTexture() ? true : false;
		bool hasMask = (*i).HasMask();
		int layerCount = (*i).GetMesh()->GetSubMeshCount();

		glUniform1i(glGetUniformLocation(shader->GetProgramID(), "hasMask"), hasMask);

		BindMesh((*i).GetMesh());

		if (hasDiff && hasMask) {
			BindTextureToShader((OGLTexture*)(*i).GetDefaultTexture(), "mainTex", 0);
		}

		for (int i = 0; i < layerCount; ++i) {
			DrawBoundMesh(i);
		}
	}
	
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void GameTechRenderer::ForwardPlusCullLights() {
	//glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, lightSSBO);

	BindShader(forwardPlusCullShader);

	glUniform1i(glGetUniformLocation(forwardPlusCullShader->GetProgramID(), "depthTex"), 0);
	Cmds::BindTexture(0, bufferDepthTex);

	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, 0);

	Matrix4 invProj = (projMat).Inverse();
	const Vector2 dimensions = GetDimensions();
	glUniformMatrix4fv(glGetUniformLocation(forwardPlusCullShader->GetProgramID(), "viewMatrix"), 1, false, (float*)&viewMat);
	glUniformMatrix4fv(glGetUniformLocation(forwardPlusCullShader->GetProgramID(), "projMatrix"), 1, false, (float*)&projMat);
	glUniformMatrix4fv(glGetUniformLocation(forwardPlusCullShader->GetProgramID(), "invProj"), 1, false, (float*)&invProj);

	glUniform1i(glGetUniformLocation(forwardPlusCullShader->GetProgramID(), "noOfLights"), numLights);
	glUniform1ui(glGetUniformLocation(forwardPlusCullShader->GetProgramID(), "totalNumLights"), totalNumLights);
	glUniform2iv(glGetUniformLocation(forwardPlusCullShader->GetProgramID(), "screenSize"), 1, (int*)&dimensions);
	glUniform2f(glGetUniformLocation(forwardPlusCullShader->GetProgramID(), "pixelSize"), 1.0f / currentWidth, 1.0f / currentHeight);

	glDispatchCompute(tilesX, tilesY, 1);
//	glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
}

void GameTechRenderer::ClusteredCullLights() {
	//glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, lightSSBO);

	BindShader(forwardPlusCullShader);

	Matrix4 invProj = (projMat).Inverse();
	const Vector2 dimensions = GetDimensions();
	glUniformMatrix4fv(glGetUniformLocation(forwardPlusCullShader->GetProgramID(), "viewMatrix"), 1, false, (float*)&viewMat);
	glUniformMatrix4fv(glGetUniformLocation(forwardPlusCullShader->GetProgramID(), "projMatrix"), 1, false, (float*)&projMat);
	glUniformMatrix4fv(glGetUniformLocation(forwardPlusCullShader->GetProgramID(), "invProj"), 1, false, (float*)&invProj);

	glUniform1i(glGetUniformLocation(forwardPlusCullShader->GetProgramID(), "noOfLights"), numLights);
	glUniform1ui(glGetUniformLocation(forwardPlusCullShader->GetProgramID(), "totalNumLights"), totalNumLights);
	glUniform2iv(glGetUniformLocation(forwardPlusCullShader->GetProgramID(), "screenSize"), 1, (int*)&dimensions);
	glUniform2f(glGetUniformLocation(forwardPlusCullShader->GetProgramID(), "pixelSize"), 1.0f / currentWidth, 1.0f / currentHeight);
	glUniform1f(glGetUniformLocation(forwardPlusCullShader->GetProgramID(), "near"), gameWorld.GetMainCamera()->GetNearPlane());
	glUniform1f(glGetUniformLocation(forwardPlusCullShader->GetProgramID(), "far"), gameWorld.GetMainCamera()->GetFarPlane());


	if (usingPrepass) {
		//glDispatchComputeIndirect();
		glDispatchCompute(CLUSTER_GRID_X, CLUSTER_GRID_Y, CLUSTER_GRID_Z);
	}
	else {
		glDispatchCompute(CLUSTER_GRID_X, CLUSTER_GRID_Y, CLUSTER_GRID_Z);
	    glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

	}
}

void GameTechRenderer::LoadPrinter() {
	printShader = new OGLShader("PrinterVertex.vert", "PrinterFragment.frag");
	split_shader = new OGLShader("SplitVertex.vert", "PrinterFragment.frag");
	printer = OGLMesh::GenerateQuad();

	glGenFramebuffers(1, &printFBO);

	glBindFramebuffer(GL_FRAMEBUFFER, printFBO);
	glGenTextures(1, &print_depth_Tex);
	glBindTexture(GL_TEXTURE_2D, print_depth_Tex);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH24_STENCIL8, currentWidth, currentHeight, 0, GL_DEPTH_STENCIL, GL_UNSIGNED_INT_24_8, NULL);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, print_depth_Tex, 0);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, GL_TEXTURE_2D, print_depth_Tex, 0);
	sceneTextures.push_back(print_depth_Tex);
	sceneBuffers.push_back(printFBO);

	//*************************************************//
	glGenTextures(1, &print_Tex);
	glBindTexture(GL_TEXTURE_2D, print_Tex);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, currentWidth, currentHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);

	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, bufferFinalTex, 0);
	glDrawBuffer(GL_COLOR_ATTACHMENT0);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	sceneTextures.push_back(print_Tex);
}

void GameTechRenderer::LoadSkybox() {
	string filenames[6] = {
		"/Cubemap/skyrender0004.png",
		"/Cubemap/skyrender0001.png",
		"/Cubemap/skyrender0003.png",
		"/Cubemap/skyrender0006.png",
		"/Cubemap/skyrender0002.png",
		"/Cubemap/skyrender0005.png"
	};

	std::array<Image, 6> images{};
	int flags[6] = { 0 };

	for (int i = 0; i < images.size(); ++i) {
		TextureLoader::LoadTexture(filenames[i], images[i], flags[i]);
		if (i > 0 && (images[i].Width() != images[i].Width() || images[i].Height() != images[i].Height())) {
			LOG_ERROR("{} cubemap input textures don't match in size?");
			return;
		}
	}
	glGenTextures(1, &skyboxTex);
	glBindTexture(GL_TEXTURE_CUBE_MAP, skyboxTex);

	GLenum type = images[0].Channels() == 4 ? GL_RGBA : GL_RGB;

	for (int i = 0; i < images.size(); ++i) {
		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, images[i].Width(), images[i].Height(), 0, type, GL_UNSIGNED_BYTE, images[i].Data());
	}

	glTexParameterf(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameterf(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

	glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
	sceneTextures.push_back(skyboxTex);
	//skybox fbo
	/*glGenFramebuffers(1, &depthFBO);
	glBindFramebuffer(GL_FRAMEBUFFER, depthFBO);
	GenerateScreenTexture(buffer_colour_depth_tex);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, buffer_colour_depth_tex, 0);
	glDrawBuffer(GL_COLOR_ATTACHMENT0);

	glGenFramebuffers(1, &skyboxFBO);
	GenerateScreenTexture(skybox_tex);
	glBindFramebuffer(GL_FRAMEBUFFER, skyboxFBO);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, skybox_tex, 0);
	glDrawBuffer(GL_COLOR_ATTACHMENT0);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	sceneBuffers.push_back(depthFBO);
	sceneBuffers.push_back(skyboxFBO);*/
}

void GameTechRenderer::GenerateScreenTexture(GLuint& into, bool depth) {
	glGenTextures(1, &into);
	glBindTexture(GL_TEXTURE_2D, into);

	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

	GLuint format = depth ? GL_DEPTH_COMPONENT24 : GL_RGBA8;
	GLuint type = depth ? GL_DEPTH_COMPONENT : GL_RGBA;


	glTexImage2D(GL_TEXTURE_2D, 0, format, currentWidth, currentHeight, 0, type, GL_UNSIGNED_BYTE, NULL);
	glTexImage2D(GL_TEXTURE_2D, 0, format, currentWidth, currentHeight, 0, type, GL_UNSIGNED_BYTE, NULL);

	glBindTexture(GL_TEXTURE_2D, 0);
	sceneTextures.push_back(into);
	screenTextures.push_back(into);
}

void GameTechRenderer::GenerateShadowBuffer(GLuint& into) {
	glGenTextures(1, &shadowTex);
	glBindTexture(GL_TEXTURE_2D, shadowTex);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT,
		SHADOWSIZE, SHADOWSIZE, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_R_TO_TEXTURE);
	glBindTexture(GL_TEXTURE_2D, 0);
	sceneTextures.push_back(shadowTex);

	glGenFramebuffers(1, &into);
	glBindFramebuffer(GL_FRAMEBUFFER, into);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, shadowTex, 0);
	glDrawBuffer(GL_NONE);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	sceneBuffers.push_back(into);
}

void GameTechRenderer::BindAndDraw(RenderObject* obj, bool hasDiff, bool hasBump) {
	vector<TextureBase*> textures = (*obj).GetTextures();
	vector<TextureBase*> specTex = (*obj).GetSpecTextures();
	int layerCount = (*obj).GetMesh()->GetSubMeshCount();

	BindMesh((*obj).GetMesh());
	int activeDiffuse = -1;
	int activeBump = -1;
	int activeSpec = -1;
	for (int i = 0; i < layerCount; ++i) {
		if (hasDiff && ((OGLTexture*)textures[i])->GetObjectID() != activeDiffuse) {
			BindTextureToShader((OGLTexture*)textures[i], "mainTex", TEXTURE_BINDING_DIFFUSE);
			activeDiffuse = ((OGLTexture*)textures[i])->GetObjectID();
		}
		if (hasBump && ((OGLTexture*)textures[i + layerCount])->GetObjectID() != activeDiffuse) {
			BindTextureToShader((OGLTexture*)textures[i + layerCount], "bumpTex", TEXTURE_BINDING_NORMAL);
			activeBump = ((OGLTexture*)textures[i + layerCount])->GetObjectID();
		}
		if (specTex.size() > 0 && ((OGLTexture*)specTex[i])->GetObjectID() != activeSpec) {
			BindTextureToShader((OGLTexture*)specTex[i], "specTex", TEXTURE_BINDING_SPECULAR);
			activeSpec = ((OGLTexture*)specTex[i])->GetObjectID();
		}
		DrawBoundMesh(i);
	}
}

void GameTechRenderer::UpdateShaderMatrices() {
	if (boundShader) {
		OGLShader::SetUniforms(boundShader,
			"projMatrix", projMat,
			"viewMatrix", viewMat);
		return;
		// Might need these at some point
		/*glUniformMatrix4fv(glGetUniformLocation(currentShader->GetProgram(), "modelMatrix"), 1, false, modelMatrix.values);
		glUniformMatrix4fv(glGetUniformLocation(currentShader->GetProgram(), "textureMatrix"), 1, false, textureMatrix.values);
		glUniformMatrix4fv(glGetUniformLocation(currentShader->GetProgram(), "shadowMatrix"), 1, false, shadowMatrix.values);*/
	}
	LOG_WARN("{} no shader was bound when calling", __FUNCTION__);
}

void GameTechRenderer::UploadLights(std::span<const PointLight> newLights) {
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, lightSSBO);
	glBufferSubData(GL_SHADER_STORAGE_BUFFER, numLights * sizeof(PointLight), newLights.size() * sizeof(PointLight), newLights.data());
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

	numLights += newLights.size();
}

void GameTechRenderer::ResizeSceneTextures(float width, float height) {
	glBindTexture(GL_TEXTURE_2D, print_depth_Tex);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH24_STENCIL8, currentWidth, currentHeight, 0, GL_DEPTH_STENCIL, GL_UNSIGNED_INT_24_8, NULL);
	glBindTexture(GL_TEXTURE_2D, 0);

	glBindTexture(GL_TEXTURE_2D, print_Tex);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, currentWidth, currentHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
	glBindTexture(GL_TEXTURE_2D, 0);

	for (auto tex : screenTextures) {
		if (tex != bufferDepthTex) {
			glBindTexture(GL_TEXTURE_2D, tex);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, currentWidth, currentHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
			glBindTexture(GL_TEXTURE_2D, 0);
		}
	}

	glBindTexture(GL_TEXTURE_2D, bufferDepthTex);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT24, currentWidth, currentHeight, GL_DEPTH_COMPONENT, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
	glBindTexture(GL_TEXTURE_2D, 0);
}

void GameTechRenderer::RenderFrame() {
	glEnable(GL_CULL_FACE);
	glClearColor(1, 1, 1, 1);
	BuildObjectList(gameWorld.GetMainCamera());
	SortObjectList();

	viewMat = gameWorld.GetMainCamera()->BuildViewMatrix();


	//RenderShadowMap();
	
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	switch (renderMode) {
	case 0:
		RenderForward(usingPrepass);
		break;
	case 1:
		RenderDeferred();
		break;
	case 2:
		RenderForwardPlus();
		break;
	case 3:
		RenderClustered(usingPrepass);
		break;
	}

	glDisable(GL_CULL_FACE);
}

void GameTechRenderer::RenderForward(bool withPrepass) {
	if (!withPrepass) {
		RenderSkybox(gameWorld.GetMainCamera());
		glDisable(GL_BLEND);
		RenderCamera(gameWorld.GetMainCamera());
		glEnable(GL_BLEND);
	}
	else {
		glDisable(GL_BLEND);
		DepthPrePass();

		glBindFramebuffer(GL_FRAMEBUFFER, forwardPlusFBO);

		//RenderSkybox(gameWorld.GetMainCamera());s
		glDepthMask(GL_FALSE);
		glColorMask(1, 1, 1, 1);
		glClear(GL_COLOR_BUFFER_BIT);

		glDepthFunc(GL_LEQUAL);
		glEnable(GL_DEPTH_TEST);
		RenderCamera(gameWorld.GetMainCamera());
		glEnable(GL_BLEND);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		BindShader(printShader);
		Cmds::BindTexture(0, bufferColourTex);
		quad->SetPrimitiveType(GeometryPrimitive::TriangleStrip);

		BindMesh(quad);
		DrawBoundMesh();
	}
}

void GameTechRenderer::RenderDeferred() {
	glBindFramebuffer(GL_FRAMEBUFFER, bufferFBO);
	glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
	glDisable(GL_BLEND);
	FillBuffers(gameWorld.GetMainCamera(), 0.0f);
	glEnable(GL_BLEND);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	/*glBindFramebuffer(GL_FRAMEBUFFER, depthFBO);
	glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
	glDisable(GL_BLEND);
	FillBuffers(gameWorld.GetMainCamera(), 1.0f);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);*/

	//RenderSkybox(gameWorld.GetMainCamera());

	DrawPointLights(gameWorld.GetMainCamera());
	CombineBuffers(gameWorld.GetMainCamera());

	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	//inSplitScreen ? SplitRender() : PresentScene(false, 0);

	//PresentScene(false, 0);
	glDisable(GL_CULL_FACE); //Todo - text indices are going the wrong way...

	//gameUI->UI_Render();
}

void GameTechRenderer::RenderForwardPlus() {
	glDisable(GL_BLEND);
	// should check for mask on transparent objects and then only sample them
	DepthPrePass();
	glEnable(GL_BLEND);

	ForwardPlusCullLights();
	glBindFramebuffer(GL_FRAMEBUFFER, forwardPlusFBO);

	//RenderSkybox(gameWorld.GetMainCamera());

	//glDisable(GL_CULL_FACE);
	glDisable(GL_BLEND);

	RenderCameraPlus(gameWorld.GetMainCamera());
	glEnable(GL_BLEND);
	//glEnable(GL_CULL_FACE);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	BindShader(printShader);
	const uint diffuseBinding = 0;
	Cmds::BindTexture(diffuseBinding, bufferColourTex);
	quad->SetPrimitiveType(GeometryPrimitive::TriangleStrip);

	BindMesh(quad);
	DrawBoundMesh();
}

void GameTechRenderer::RenderClustered(bool withPrepass) {

	if (withPrepass) {
		DepthPrePass();
		ComputeActiveClusters();
		CompactClusterList();
	}

	ClusteredCullLights();

	if (withPrepass) {
		glBindFramebuffer(GL_FRAMEBUFFER, forwardPlusFBO);
		glDepthMask(GL_FALSE);
		glColorMask(1, 1, 1, 1);
		glClear(GL_COLOR_BUFFER_BIT);

		glDepthFunc(GL_LEQUAL);
		glEnable(GL_DEPTH_TEST);

	}
	OGLShader* activeShader = nullptr;

	//glActiveTexture(GL_TEXTURE0 + 2);
	//glBindTexture(GL_TEXTURE_2D, shadowTex);

	BindShader(forwardPlusShader);

	for (const auto& i : activeObjects) {
		OGLShader* shader = forwardPlusShader;

		vector<TextureBase*> textures = (*i).GetTextures();

		if (activeShader != shader) {
			
			shader->SetUniform("cameraPos", gameWorld.GetMainCamera()->GetPosition());

			OGLShader::SetUniforms(shader,
				"projMatrix", projMat,
				"viewMatrix", viewMat,
				"noOfLights", numLights);

			/*int shadowTexLocation = glGetUniformLocation(shader->GetProgramID(), "shadowTex");
			glUniform1i(shadowTexLocation, 1);*/

			activeShader = shader;
		}

		Matrix4 modelMatrix = (*i).GetTransform()->GetMatrix();

		//Matrix4 fullShadowMat = shadowMatrix * modelMatrix;
		//glUniformMatrix4fv(shadowLocation, 1, false, (float*)&fullShadowMat);

		int layerCount = (*i).GetMesh()->GetSubMeshCount();
		const bool hasDiff = (OGLTexture*)(*i).GetDefaultTexture() ? true : false;
		const bool hasBump = textures.size() == layerCount * 2;
		const bool hasSpec = (*i).GetSpecTextures().size() > 0;

		OGLShader::SetUniforms(activeShader, 
			"modelMatrix", modelMatrix,
			"scale", clusterParams.scaleFactor,
			"bias", clusterParams.biasFactor,
			"tilePxX", clusterX,
			"tilePxY", clusterY,
			"inDebug", inDebugMode,
			"objectColour", i->GetColour(),
			"hasVertexColours", !(*i).GetMesh()->GetColourData().empty(),
			"hasTexture", (OGLTexture*)(*i).GetDefaultTexture() ? 1 : 0,
			"hasBump", hasBump,
			"hasSpec", hasSpec);

		if (i->GetAnimation()) {
			MeshGeometry* mesh = i->GetMesh();
			vector <Matrix4> frameMatrices;
			const vector<Matrix4> invBindPose = mesh->GetInverseBindPose();
			const Matrix4* frameData = i->GetAnimation()->GetJointData(i->GetCurrentFrame());

			for (unsigned int i = 0; i < mesh->GetJointCount(); ++i) {
				auto matrix = invBindPose[i];
				frameMatrices.emplace_back(frameData[i] * matrix);
			}

			int j = glGetUniformLocation(((OGLShader*)(*i).GetShader())->GetProgramID(), "joints");
			glUniformMatrix4fv(j, frameMatrices.size(), false,
				(float*)frameMatrices.data());
		}

		BindAndDraw(i, hasDiff, hasBump);
	}

	if (withPrepass) {
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		BindShader(printShader);
		const uint diffuseBinding = 0;
		Cmds::BindTexture(diffuseBinding, bufferColourTex);
		quad->SetPrimitiveType(GeometryPrimitive::TriangleStrip);

		BindMesh(quad);
		DrawBoundMesh();
	}
}

void GameTechRenderer::GenPrePassFBO() {
	glGenFramebuffers(1, &bufferFBO);
	glGenFramebuffers(1, &forwardPlusFBO);
	
	GLenum buffers[1] = {
		GL_COLOR_ATTACHMENT0
	};

	GenerateScreenTexture(bufferDepthTex, true);
	GenerateScreenTexture(depthColourTex);

	glBindFramebuffer(GL_FRAMEBUFFER, bufferFBO);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, depthColourTex, 0);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, bufferDepthTex, 0);
	glDrawBuffers(1, buffers);

	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)  return;

	sceneBuffers.push_back(bufferFBO);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	GenerateScreenTexture(bufferColourTex);

	glBindFramebuffer(GL_FRAMEBUFFER, forwardPlusFBO);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, bufferColourTex, 0);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, bufferDepthTex, 0);

	glDrawBuffers(1, buffers);
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)  return;

	sceneBuffers.push_back(forwardPlusFBO);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}


void GameTechRenderer::FillBuffers(Camera* current_camera, float depth) {
	BindShader(sceneShader);

	OGLShader* activeShader = nullptr;
	/*glActiveTexture(GL_TEXTURE0 + 2);
	glBindTexture(GL_TEXTURE_2D, shadowTex);*/

	int count = 0;
	for (const auto& i : activeObjects) {
		//OGLShader* shader = (OGLShader*)(*i).GetShader();
		OGLShader* shader = sceneShader;
		//BindShader(shader);

		vector<TextureBase*> textures = (*i).GetTextures();

		if (activeShader != shader) {
			shader->SetUniform("cameraPos", current_camera->GetPosition());

			OGLShader::SetUniforms(shader,
				"projMatrix", projMat,
				"viewMatrix", viewMat,
				"noOfLights", numLights);

			/*int shadowTexLocation = glGetUniformLocation(shader->GetProgramID(), "shadowTex");
			glUniform1i(shadowTexLocation, 2);*/

			activeShader = shader;
		}

		Matrix4 modelMatrix = (*i).GetTransform()->GetMatrix();

	/*	Matrix4 fullShadowMat = shadowMatrix * modelMatrix;
		glUniformMatrix4fv(shadowLocation, 1, false, (float*)&fullShadowMat);*/

		int layerCount = (*i).GetMesh()->GetSubMeshCount();
		bool hasDiff = (OGLTexture*)(*i).GetDefaultTexture() ? true : false;
		bool hasBump = textures.size() == layerCount * 2;
		bool hasSpec = (*i).GetSpecTextures().size() > 0;

		OGLShader::SetUniforms(activeShader,
			"modelMatrix", modelMatrix,
			"inDebug", inDebugMode,
			"objectColour", i->GetColour(),
			"hasVertexColours", !(*i).GetMesh()->GetColourData().empty(),
			"hasTexture", (OGLTexture*)(*i).GetDefaultTexture() ? 1 : 0,
			"hasBump", hasBump,
			"hasSpec", hasSpec,
			"isDepth", depth);

		if (i->GetAnimation()) {
			MeshGeometry* mesh = i->GetMesh();
			vector <Matrix4> frameMatrices;
			const vector<Matrix4> invBindPose = mesh->GetInverseBindPose();
			const Matrix4* frameData = i->GetAnimation()->GetJointData(i->GetCurrentFrame());

			for (unsigned int i = 0; i < mesh->GetJointCount(); ++i) {
				auto matrix = invBindPose[i];
				frameMatrices.emplace_back(frameData[i] * matrix);
			}

			int j = glGetUniformLocation(((OGLShader*)(*i).GetShader())->GetProgramID(), "joints");
			glUniformMatrix4fv(j, frameMatrices.size(), false,
				(float*)frameMatrices.data());

		}

		BindAndDraw(i, hasDiff, hasBump);
		count++;
	}

	glDisable(GL_CULL_FACE);
	DrawDebugData();
	glEnable(GL_CULL_FACE);
}

void GameTechRenderer::DrawPointLights(Camera* current_camera) {

//	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, lightSSBO);

	/** Draw Point Lights **/
	glBindFramebuffer(GL_FRAMEBUFFER, pointLightFBO);
	BindShader(pointLightShader);

	glClearColor(0, 0, 0, 1);
	glClear(GL_COLOR_BUFFER_BIT);
	glBlendFunc(GL_ONE, GL_ONE);
	Vector3 pos = current_camera->GetPosition();
	glCullFace(GL_FRONT);
	glDepthFunc(GL_ALWAYS);
	glDepthMask(GL_FALSE);

	const uint depthBinding = 0, normBinding = 1;
	// TODO: Shouldn't we have a specular texture here, what did we do with it?
	Cmds::BindTexture(depthBinding, bufferDepthTex);
	Cmds::BindTexture(normBinding, bufferNormalTex);

	//glUniform1i(glGetUniformLocation(pointLightShader->GetProgramID(), "shadowTex"), 2);
	//glActiveTexture(GL_TEXTURE2);
	//glBindTexture(GL_TEXTURE_2D, bufferShadowTex);
	const Vector2 cameraPos = current_camera->GetPosition();
	glUniform3fv(glGetUniformLocation(pointLightShader->GetProgramID(), "cameraPos"), 1, (float*)&cameraPos);
	//std::cout << current_camera->GetPosition() << std::endl;

	glUniform2f(glGetUniformLocation(pointLightShader->GetProgramID(), "pixelSize"), 1.0f / currentWidth, 1.0f / currentHeight);

	Matrix4 invViewProj = (projMat * viewMat).Inverse();

	int projLocation = glGetUniformLocation(pointLightShader->GetProgramID(), "projMatrix");
	int viewLocation = glGetUniformLocation(pointLightShader->GetProgramID(), "viewMatrix");
	int modelLocation = glGetUniformLocation(pointLightShader->GetProgramID(), "modelMatrix");
	glUniformMatrix4fv(projLocation, 1, false, (float*)&projMat);
	glUniformMatrix4fv(viewLocation, 1, false, (float*)&viewMat);

	glUniformMatrix4fv(glGetUniformLocation(pointLightShader->GetProgramID(), "inverseProjView"), 1, false, invViewProj.array);

	BindMesh(sphere);

	for (uint i = 0; i < numLights; i++) {
		glUniform1i(glGetUniformLocation(pointLightShader->GetProgramID(), "lightIndex"), i);
		DrawBoundMesh();
	}

	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glCullFace(GL_BACK);
	glDepthFunc(GL_LEQUAL);

	glDepthMask(GL_TRUE);

	glClearColor(0.2f, 0.2f, 0.2f, 1);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
//	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, 0);

}

void GameTechRenderer::CombineBuffers(Camera* current) {

	BindShader(combineShader);
	Matrix4 viewMatrix = viewMat;
	Matrix4 projMatrix = projMat;
	viewMatrix.ToIdentity();
	projMatrix.ToIdentity();
	Matrix4 identity = Matrix4();

	OGLShader::SetUniforms(combineShader,
		"projMatrix", projMatrix,
		"viewMatrix", viewMatrix,
		"modelMatrix", identity);

	Cmds::BindTexture(0, bufferColourTex);
	Cmds::BindTexture(1, lightDiffuseTex);
	Cmds::BindTexture(2, lightSpecularTex);

	//glUniform1i(glGetUniformLocation(combineShader->GetProgramID(), "skyboxTex"), 3);
	//glActiveTexture(GL_TEXTURE3);
	//glBindTexture(GL_TEXTURE_2D, skybox_tex);

	//glUniform1i(glGetUniformLocation(combineShader->GetProgramID(), "scene_depth"), 4);
	//glActiveTexture(GL_TEXTURE4);
	//glBindTexture(GL_TEXTURE_2D, buffer_colour_depth_tex);

	quad->SetPrimitiveType(GeometryPrimitive::TriangleStrip);
	//glBindFramebuffer(GL_FRAMEBUFFER, printFBO);
	//glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
	BindMesh(quad);
	DrawBoundMesh();
	//glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void GameTechRenderer::PresentScene(bool split, GLfloat offset) {
	OGLShader* activeShader = nullptr;
	if (split) {
		BindShader(split_shader);
		split_shader->SetUniform("offset", offset);
		activeShader = split_shader;
	}
	else {
		BindShader(printShader);
		activeShader = printShader;
	}

	Cmds::BindTexture(0, bufferFinalTex);

	BindMesh(quad);
	DrawBoundMesh();
}


void GameTechRenderer::BeginFrame() {
	//gameUI->UI_Frame();
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
	BindShader(nullptr);
	BindMesh(nullptr);
}

void GameTechRenderer::BuildObjectList(Camera* currentCamera) {
	activeObjects.clear();
	frameFrustum.FromMatrix(currentCamera->BuildProjectionMatrix((float)currentWidth / (float)currentHeight) * currentCamera->BuildViewMatrix());

	gameWorld.OperateOnContents(
		[&](GameObject* o) {
			if (o->IsActive()) {
				RenderObject* g = o->GetRenderObject();
			//	if (g && frameFrustum.InsideFrustum(*g)) {
					Vector3 dir = g->GetTransform()->GetPosition() - currentCamera->GetPosition();
					g->SetCameraDistance(dir.LengthSquared());
					activeObjects.emplace_back(g);
			//	}
			}
		}
	);
}

void GameTechRenderer::SortObjectList() {
	std::sort(activeObjects.begin(),
		activeObjects.end(),
		RenderObject::CompareByCameraDistance);
}

void GameTechRenderer::RenderShadowMap() {
	glBindFramebuffer(GL_FRAMEBUFFER, shadowFBO);
	glClear(GL_DEPTH_BUFFER_BIT);
	glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
	glViewport(0, 0, SHADOWSIZE, SHADOWSIZE);

	glCullFace(GL_FRONT);

	BindShader(shadowShader);
	int mvpLocation = glGetUniformLocation(shadowShader->GetProgramID(), "mvpMatrix");
	int hasJointsLocation = glGetUniformLocation(shadowShader->GetProgramID(), "hasJoints");

	Matrix4 shadowViewMatrix = Matrix4::BuildViewMatrix(lightPosition, Vector3(0, 0, 0), Vector3(0, 1, 0));
	Matrix4 shadowProjMatrix = Matrix4::Perspective(100.0f, 500.0f, 1, 45.0f);

	Matrix4 mvMatrix = shadowProjMatrix * shadowViewMatrix;

	shadowMatrix = biasMatrix * mvMatrix; //we'll use this one later on

	for (const auto& i : activeObjects) {
		if (i->GetAnimation()) {
			MeshGeometry* mesh = i->GetMesh();
			vector <Matrix4> frameMatrices;
			const vector<Matrix4> invBindPose = mesh->GetInverseBindPose();
			const Matrix4* frameData = i->GetAnimation()->GetJointData(i->GetCurrentFrame());

			for (unsigned int i = 0; i < mesh->GetJointCount(); ++i) {
				frameMatrices.emplace_back(frameData[i] * invBindPose[i]);
			}

			glUniformMatrix4fv(glGetUniformLocation(shadowShader->GetProgramID(), "joints"), frameMatrices.size(), false,
				(float*)frameMatrices.data());

			glUniform1i(hasJointsLocation, true);
		}
		else {
			glUniform1i(hasJointsLocation, false);
		}

		Matrix4 modelMatrix = (*i).GetTransform()->GetMatrix();
		Matrix4 mvpMatrix = mvMatrix * modelMatrix;
		glUniformMatrix4fv(mvpLocation, 1, false, (float*)&mvpMatrix);
		BindMesh((*i).GetMesh());
		int layerCount = (*i).GetMesh()->GetSubMeshCount();
		for (int i = 0; i < layerCount; ++i) {
			DrawBoundMesh(i);
		}
	}

	glViewport(0, 0, currentWidth, currentHeight);
	glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	glCullFace(GL_BACK);
}

void GameTechRenderer::RenderSkybox(Camera* current_camera) {

	glDisable(GL_CULL_FACE);
	glDisable(GL_BLEND);
	glDisable(GL_DEPTH_TEST);

	//glBindFramebuffer(GL_FRAMEBUFFER, skyboxFBO);
	//glClearColor(0, 0, 0, 1);
	//glClear(GL_COLOR_BUFFER_BIT);

	BindShader(skyboxShader);

	int projLocation = glGetUniformLocation(skyboxShader->GetProgramID(), "projMatrix");
	int viewLocation = glGetUniformLocation(skyboxShader->GetProgramID(), "viewMatrix");
	int texLocation = glGetUniformLocation(skyboxShader->GetProgramID(), "cubeTex");
	glUniformMatrix4fv(projLocation, 1, false, (float*)&projMat);
	glUniformMatrix4fv(viewLocation, 1, false, (float*)&viewMat);

	glUniform1i(texLocation, 0);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_CUBE_MAP, skyboxTex);

	BindMesh(skyboxMesh);
	DrawBoundMesh();

	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	glEnable(GL_CULL_FACE);
	glEnable(GL_BLEND);
	glEnable(GL_DEPTH_TEST);
}

void GameTechRenderer::RenderCamera(Camera* current_camera) {

	OGLShader* activeShader = nullptr;

	//glActiveTexture(GL_TEXTURE0 + 2);
	//glBindTexture(GL_TEXTURE_2D, shadowTex);

	//glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, lightSSBO);

	for (const auto& i : activeObjects) {
		OGLShader* shader = (OGLShader*)(*i).GetShader();
		BindShader(shader);

		vector<TextureBase*> textures = (*i).GetTextures();

		if (activeShader != shader) {

			shader->SetUniform("cameraPos", current_camera->GetPosition());

			OGLShader::SetUniforms(shader,
				"projMatrix", projMat,
				"viewMatrix", viewMat,
				"noOfLights", numLights);

			/*int shadowTexLocation = glGetUniformLocation(shader->GetProgramID(), "shadowTex");
			glUniform1i(shadowTexLocation, 1);*/

			activeShader = shader;
		}

		Matrix4 modelMatrix = (*i).GetTransform()->GetMatrix();

		//Matrix4 fullShadowMat = shadowMatrix * modelMatrix;
		//glUniformMatrix4fv(shadowLocation, 1, false, (float*)&fullShadowMat);

		int layerCount = (*i).GetMesh()->GetSubMeshCount();
		const bool hasDiff = (OGLTexture*)(*i).GetDefaultTexture() ? true : false;
		const bool hasBump = textures.size() == layerCount * 2;
		const bool hasSpec = (*i).GetSpecTextures().size() > 0;

		OGLShader::SetUniforms(activeShader,
			"modelMatrix", modelMatrix,
			"objectColour", i->GetColour(),
			"hasVertexColours", !(*i).GetMesh()->GetColourData().empty(),
			"hasTexture", (OGLTexture*)(*i).GetDefaultTexture() ? 1 : 0,
			"hasBump", hasBump,
			"hasSpec", hasSpec);

		if (i->GetAnimation()) {
			MeshGeometry* mesh = i->GetMesh();
			vector <Matrix4> frameMatrices;
			const vector<Matrix4> invBindPose = mesh->GetInverseBindPose();
			const Matrix4* frameData = i->GetAnimation()->GetJointData(i->GetCurrentFrame());

			for (unsigned int i = 0; i < mesh->GetJointCount(); ++i) {
				auto& matrix = invBindPose[i];
				frameMatrices.emplace_back(frameData[i] * matrix);
			}

			int j = glGetUniformLocation(((OGLShader*)(*i).GetShader())->GetProgramID(), "joints");
			glUniformMatrix4fv(j, frameMatrices.size(), false,
				(float*)frameMatrices.data());
		}

		BindAndDraw(i, hasDiff, hasBump);
	}
//	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, 0);

}

void GameTechRenderer::RenderCameraPlus(Camera* current_camera) {
	glDepthMask(GL_FALSE);
	glColorMask(1, 1, 1, 1);
	glClear(GL_COLOR_BUFFER_BIT);

	glDepthFunc(GL_LEQUAL);
	glEnable(GL_DEPTH_TEST);

	BindShader(forwardPlusShader);

	OGLShader* activeShader = nullptr;

	//glActiveTexture(GL_TEXTURE0 + 2);
	//glBindTexture(GL_TEXTURE_2D, shadowTex);

	//glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, lightSSBO);
	//glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, aabbGridSSBO);
	int sizeX = (unsigned int)std::ceilf(currentWidth / (float)TILE_SIZE);

	for (const auto& i : activeObjects) {
	//	OGLShader* shader = (OGLShader*)(*i).GetShader();
		OGLShader* shader = forwardPlusShader;

		vector<TextureBase*> textures = (*i).GetTextures();

		if (activeShader != shader) {
			shader->SetUniform("cameraPos", current_camera->GetPosition());

			OGLShader::SetUniforms(shader,
				"projMatrix", projMat,
				"viewMatrix", viewMat,
				"noOfLights", numLights,
				"numTilesX", tilesX,
				"tilePxX", sizeX);

			/*int shadowTexLocation = glGetUniformLocation(shader->GetProgramID(), "shadowTex");
			glUniform1i(shadowTexLocation, 1);*/

			activeShader = shader;
		}

		Matrix4 modelMatrix = (*i).GetTransform()->GetMatrix();

		int layerCount = (*i).GetMesh()->GetSubMeshCount();
		bool hasDiff = (OGLTexture*)(*i).GetDefaultTexture() ? true : false;
		bool hasBump = textures.size() == layerCount * 2;
		bool hasSpec = (*i).GetSpecTextures().size() > 0;

		OGLShader::SetUniforms(activeShader,
			"modelMatrix", modelMatrix,
			"inDebug", inDebugMode,
			"objectColour", i->GetColour(),
			"hasVertexColours", !(*i).GetMesh()->GetColourData().empty(),
			"hasTexture", (OGLTexture*)(*i).GetDefaultTexture() ? 1 : 0,
			"hasBump", hasBump,
			"hasSpec", hasSpec);

		//Matrix4 fullShadowMat = shadowMatrix * modelMatrix;
		//glUniformMatrix4fv(shadowLocation, 1, false, (float*)&fullShadowMat);

		BindAndDraw(i, hasDiff, hasBump);
	}
//	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, 0);
	//glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, 0);
}

Matrix4 GameTechRenderer::SetupDebugLineMatrix()	const {
	float screenAspect = (float)currentWidth / (float)currentHeight;
	Matrix4 viewMatrix = gameWorld.GetMainCamera()->BuildViewMatrix();
	Matrix4 projMatrix = gameWorld.GetMainCamera()->BuildProjectionMatrix(screenAspect);

	return projMatrix * viewMatrix;
}

Matrix4 GameTechRenderer::SetupDebugStringMatrix()	const {
	return Matrix4::Orthographic(-1, 1.0f, 100, 0, 0, 100);
}

void GameTechRenderer::LoadStartImage() {
	loading_shader = new OGLShader("LoadingVertex.vert", "LoadingFragment.frag");
	glGenTextures(1, &background_tex);
	glBindTexture(GL_TEXTURE_2D, background_tex);

	Image image;
	int flag = 0;

	stbi_set_flip_vertically_on_load(true);
	TextureLoader::LoadTexture("SpitoonBackground.png", image, flag);

	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, image.Width(), image.Height(), 0, GL_RGBA, GL_UNSIGNED_BYTE, image.Data());

	glBindTexture(GL_TEXTURE_2D, 0);
	sceneTextures.push_back(background_tex);

	/////////////////////
	glGenTextures(1, &loading_tex);
	glBindTexture(GL_TEXTURE_2D, loading_tex);

	stbi_set_flip_vertically_on_load(true);
	TextureLoader::LoadTexture("nyan.png", image, flag);
	stbi_set_flip_vertically_on_load(false);

	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, image.Width(), image.Height(), 0, GL_RGBA, GL_UNSIGNED_BYTE, image.Data());

	glBindTexture(GL_TEXTURE_2D, 0);
	sceneTextures.push_back(loading_tex);
}

void GameTechRenderer::RenderStartView() {
	static float movement = 0.0f;
	if (movement < 2.0f) {
		movement += Window::GetTimer()->GetTimeDeltaSeconds() / 5;
	}
	else if (movement > 2.0f) {

		loading = false;
	}

	BeginFrame();

	glDisable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	///////////
	BindShader(printShader);

	Cmds::BindTexture(0, background_tex);
	BindMesh(printer);
	DrawBoundMesh();
	glBindTexture(GL_TEXTURE_2D, 0);
	///////////
	if (loading) {
		BindShader(loading_shader);

		Cmds::BindTexture(0, loading_tex);
		glUniform1i(glGetUniformLocation(loading_shader->GetProgramID(), "nyan"), 0);
		glUniform1f(glGetUniformLocation(loading_shader->GetProgramID(), "movement"), movement);
		BindMesh(printer);
		DrawBoundMesh();
		glBindTexture(GL_TEXTURE_2D, 0);
	}

	BindShader(nullptr);
	///////////
	glEnable(GL_DEPTH_TEST);
	glBlendFunc(GL_ONE, GL_ZERO);
	glDisable(GL_BLEND);

	//gameUI->UI_Render();
	OGLRenderer::SwapBuffers();
}