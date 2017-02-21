#pragma once
#include <DirectXMath.h>
#include <WICTextureLoader.h>
#include <random>
#include "DefferedRenderer.h"
#include "DXMathImpl.h"
#include "SimpleShader.h"

class DefferedRenderer;

class PostProcesser
{
public:
	PostProcesser(DefferedRenderer* renderingSystem);
	~PostProcesser();

	void renderKernel(FLOAT kernel[9], ID3D11ShaderResourceView* readFrom, ID3D11RenderTargetView* writeTo);
	void bloom(ID3D11ShaderResourceView* readFrom, ID3D11RenderTargetView* writeTo);
	void blur(ID3D11ShaderResourceView* readFrom, ID3D11RenderTargetView* writeTo);
	void ascii(ID3D11ShaderResourceView* readFrom, ID3D11RenderTargetView* writeTo);
	void ssao(ID3D11RenderTargetView* writeTo);

private:
	/// Moves readFrom into writeTo
	void passThrough(ID3D11ShaderResourceView* readFrom, ID3D11RenderTargetView* writeTo);
	void SetUpSSAO();

private:
	enum PostProcessFunction {DEFAULT, BLOOM, BLUR, ASCII, EDGE, EMBOSS, BLUR_K, SHARPNESS, SOBEL};

	DefferedRenderer* renderer;
	PostProcessFunction currentFunction = DEFAULT;

	ID3D11RenderTargetView* unfinalizedFrameRTV;
	// ID3D11RenderTargetView* bloomExtractRTV; // will also be used for blurring
	// ID3D11RenderTargetView* bloomHorizonatalRTV;

	ID3D11ShaderResourceView* unfinalizedFrameSRV;
	ID3D11ShaderResourceView* noiseSRV;
	// ID3D11ShaderResourceView* bloomExtractSRV; // will also be used for blurring
	// ID3D11ShaderResourceView* bloomHorizonatalSRV;
};