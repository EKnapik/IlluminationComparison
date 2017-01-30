#pragma once

#include "Renderer.h"

class DefferedRenderer : public Renderer
{
public:
	DefferedRenderer(Camera *camera, ID3D11DeviceContext *context, ID3D11Device* device,
		ID3D11RenderTargetView* backBufferRTV, ID3D11DepthStencilView* depthStencilView, int width, int height);
	virtual ~DefferedRenderer();
	void Render(FLOAT deltaTime, FLOAT totalTime);
	

private:

	void gBufferRender(FLOAT deltaTime, FLOAT totalTime);
	void pointLightRender();
	void directionalLightRender();
	void DrawOpaqueMaterials();
	void DrawTransparentMaterials();
	void DrawSSAO();

	// Albedo
	ID3D11RenderTargetView *	AlbedoRTV;
	ID3D11ShaderResourceView*	AlbedoSRV;

	// Normal
	ID3D11RenderTargetView *	NormalRTV;
	ID3D11ShaderResourceView*	NormalSRV;

	// Depth
	ID3D11RenderTargetView *	PositionRTV;
	ID3D11ShaderResourceView*	PositionSRV;

	ID3D11SamplerState* simpleSampler;
	ID3D11BlendState* blendState;
	ID3D11RasterizerState* lightRastState;
	ID3D11BlendState* transBlendState;

	float windowWidth;
	float windowHeight;
};

