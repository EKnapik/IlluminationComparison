#pragma once

#include "PostProcesser.h"
#include "Renderer.h"

class PostProcesser;

class DefferedRenderer : public Renderer
{
	friend class PostProcesser;
public:
	DefferedRenderer(Camera *camera, ID3D11DeviceContext *context, ID3D11Device* device,
		ID3D11RenderTargetView* backBufferRTV, ID3D11DepthStencilView* depthStencilView, int width, int height);
	virtual ~DefferedRenderer();
	void Render(FLOAT deltaTime, FLOAT totalTime);
	void AddPostProcessSystem(PostProcesser* newPostProcesser);

private:
	void gBufferRender(FLOAT deltaTime, FLOAT totalTime);
	void pointLightRender();
	void directionalLightRender();
	void DrawOpaqueMaterials();
	void DrawTransparentMaterials();

	// Post Processing System, IF RENDERING BLACK POST PROCESSOR MAY HAVE NOT BEEN INITALIZED;
	PostProcesser* postProcesser;
	bool postProcessingInit = false;

	// Albedo
	ID3D11RenderTargetView *	AlbedoRTV;
	ID3D11ShaderResourceView*	AlbedoSRV;

	// Normal
	ID3D11RenderTargetView *	NormalRTV;
	ID3D11ShaderResourceView*	NormalSRV;

	// Depth
	ID3D11RenderTargetView *	DepthRTV;
	ID3D11ShaderResourceView*	DepthSRV;

	// Light Buffer
	ID3D11RenderTargetView *	LightRTV;
	ID3D11ShaderResourceView*	LightSRV;

	// PBR (metallic, roughness)
	ID3D11RenderTargetView *	PBR_RTV;
	ID3D11ShaderResourceView*	PBR_SRV;

	// SSAO
	ID3D11RenderTargetView *	ssaoRTV;
	ID3D11ShaderResourceView*	ssaoSRV;

	// Unfinalized holding buffer
	ID3D11RenderTargetView *	unfinalizedRTV;
	ID3D11ShaderResourceView*	unfinalizedSRV;

	ID3D11SamplerState* simpleSampler;

	ID3D11BlendState* blendState;
	ID3D11RasterizerState* lightRastState;
	ID3D11BlendState* transBlendState;
};

