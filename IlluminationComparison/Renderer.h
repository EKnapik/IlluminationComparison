#pragma once
#include <map>
#include <random>
#include <DirectXMath.h>
#include <algorithm>
#include "SimpleShader.h"
#include "DXMathImpl.h"
#include "GameEntity.h"
#include "Camera.h"
#include "Lights.h"
#include "ParticleEmitter.h"

class ParticleEmitter;

class Renderer
{
public:
	Renderer(Camera *camera, ID3D11DeviceContext *context, ID3D11Device* device, ID3D11RenderTargetView* backBufferRTV, ID3D11DepthStencilView* depthStencilView, int width, int height);
	virtual ~Renderer();

	void RenderShadowMap();

	virtual void Render(FLOAT deltaTime, FLOAT totalTime) {};

	virtual void DrawOneMaterial(FLOAT deltaTime, FLOAT totalTime);

	void DrawMultipleMaterials(FLOAT deltaTime, FLOAT totalTime);

	void DrawSkyBox();

	void DrawParticleEmitters(FLOAT deltaTime, FLOAT totalTime);

	void PostProcess();

	void AddMesh(std::string name, Mesh* mesh);
	void AddMesh(std::string name, std::string path);

	Mesh* GetMesh(std::string name);
	Material* GetMaterial(std::string name);

	void AddMaterial(std::string name, Material* material);
	void AddMaterial(std::string name, std::wstring path, std::string sampler);
	///<summary>
	/// Adds a material with the specified name and path. Uses a Default sampler description
	///</summary>
	void AddMaterial(std::string name, std::wstring path);

	void AddCubeMaterial(std::string name, CubeMap* material);
	void AddCubeMaterial(std::string name, std::wstring path, std::string sampler, D3D11_RASTERIZER_DESC* rasterDesc, D3D11_DEPTH_STENCIL_DESC* depthStencilDesc);
	///<summary>
	/// Adds a material with the specified name and path. Uses a Default sampler description
	///</summary>
	void AddCubeMaterial(std::string name, std::wstring path);

	void AddVertexShader(std::string name, std::wstring path);
	void AddPixelShader(std::string name, std::wstring path);
	void AddGeometryShader(std::string name, std::wstring path);
	void AddGeometryShader(std::string name, std::wstring path, bool useStreamOut, bool allowStreamOutRasterization);

	void AddSampler(std::string name, D3D11_SAMPLER_DESC* sampleDesc);
	ID3D11SamplerState* GetSampler(std::string name);
	SimpleVertexShader* GetVertexShader(std::string name);
	SimplePixelShader* GetPixelShader(std::string name);
	SimpleGeometryShader* GetGeometryShader(std::string name);

	void SetSkyBox(std::string name);

	ID3D11ShaderResourceView* GetRandomTexture() { return randomSRV; }

	int width, height;

	Camera *camera;

	ID3D11Device*			device;
	ID3D11DeviceContext*	context;

	void SetGameEntities(std::vector<GameEntity*>* gameEntitys) { this->gameEntitys = gameEntitys; }
	void SetDirectionalLights(std::vector<SceneDirectionalLight>* directionalLights) { this->directionalLights = directionalLights; }
	void SetPointLights(std::vector<ScenePointLight*>* pointLights) { this->pointLights = pointLights; }
	void SetParticleEmitters(std::vector<ParticleEmitter*>* particleEmitters) { this->particleEmitters = particleEmitters; }

	// This section is the Post Process Boolean section.
	BOOL PostProcessing = false;
	BOOL Blur = false;
	BOOL EdgeDetect = false;
	BOOL Bloom = false;
	BOOL Emboss = false;
	BOOL BlurWithKernel = false;
	BOOL Sharpness = false;
	BOOL BottomSobel = false;
	BOOL ASCII = false;

	void DisableAllPostProcess();

protected:
	void SortObjects();
	void SetUpShadows();
	void SetUpRandomTexture();
	void SetUpPostProcessing();

	std::vector<GameEntity*>* gameEntitys;
	std::vector<GameEntity*> opaque;
	std::vector<GameEntity*> transparent;
	std::vector<SceneDirectionalLight>* directionalLights;
	std::vector<ScenePointLight*>* pointLights;
	std::vector<ParticleEmitter*>* particleEmitters;

	ID3D11RenderTargetView* backBufferRTV;
	ID3D11RenderTargetView* postProcessRTV;
	ID3D11RenderTargetView* ssaoRTV;
	ID3D11RenderTargetView* bloomExtractRTV; // will also be used for blurring
	ID3D11RenderTargetView* bloomHorizonatalRTV;
	ID3D11DepthStencilView* depthStencilView;
	ID3D11DepthStencilView* shadowDSV;
	ID3D11ShaderResourceView* shadowSRV;
	ID3D11ShaderResourceView* randomSRV;
	ID3D11ShaderResourceView* postProcessSRV;
	ID3D11ShaderResourceView* ssaoSRV;
	ID3D11ShaderResourceView* bloomExtractSRV; // will also be used for blurring
	ID3D11ShaderResourceView* bloomHorizonatalSRV;
	ID3D11RasterizerState* shadowRasterizer;
	ID3D11Texture1D* randomTexture;
	std::vector<VEC3> ssaoKernel;

	std::map<std::string, Mesh*>				MeshDictionary;
	std::map<std::string, Material*>			MaterialDictionary;
	std::map<std::string, SimpleVertexShader*>	VertexShaderDictionary;
	std::map<std::string, SimplePixelShader*>	PixelShaderDictionary;
	std::map<std::string, SimpleGeometryShader*>GeometryShaderDictionary;
	std::map<std::string, ID3D11SamplerState*>  SamplerDictionary;

	CubeMap* skyBox = nullptr;

	bool blendMode = false;

	int shadowMapSize = 1080;

	DirectX::XMFLOAT4X4 shadowDirectionalProjectionMatrix;
	DirectX::XMFLOAT4X4 shadowPointProjectionMatrix;
};

