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

	void renderKernel(FLOAT* kernel, float weight, ID3D11ShaderResourceView* readFrom, ID3D11RenderTargetView* writeTo);
	void bloom(ID3D11ShaderResourceView* readFrom, ID3D11RenderTargetView* writeTo);
	void blur(float blurAmount, ID3D11ShaderResourceView* readFrom, ID3D11RenderTargetView* writeTo);
	void ascii(ID3D11ShaderResourceView* readFrom, ID3D11RenderTargetView* writeTo);
	void ssao(ID3D11RenderTargetView* writeTo);
	/// Moves readFrom into writeTo
	void passThrough(ID3D11ShaderResourceView* readFrom, ID3D11RenderTargetView* writeTo);

private:
	enum PostProcessFunction {DEFAULT, BLOOM, BLUR, ASCII, EDGE, EMBOSS, BLUR_K, SHARPNESS, SOBEL};

	DefferedRenderer* renderer;
	PostProcessFunction currentFunction = DEFAULT;

	ID3D11RenderTargetView* bloomExtractRTV; // will also be used for blurring
	ID3D11RenderTargetView* bloomHorizonatalRTV;

	ID3D11ShaderResourceView* asciiSRV;
	ID3D11ShaderResourceView* bloomExtractSRV; // will also be used for blurring
	ID3D11ShaderResourceView* bloomHorizonatalSRV;

	ID3D11SamplerState* ssaoSampler;

public:
	float ssaoRadius = 3.0f;

	FLOAT edgeDetectKernel[9] = {
		-1, -1, -1,
		-1, 8, -1,
		-1, -1, -1
	};

	FLOAT embossKernel[9] = {
		-2, -1, 0,
		-1, 1, 1,
		0, 1, 2
	};

	FLOAT blurKernel[9] = {
		-1, 2, 1,
		2, 4, 2,
		1, 2, 1
	};

	FLOAT sharpnessKernel[9] = {
		-1, -1, -1,
		-1, 9, -1,
		-1, -1, -1
	};

	FLOAT bottomSobelKernel[9] = {
		-1, -2, -1,
		0, 0, 0,
		1, 2, 1
	};

	FLOAT defaultKernel[9] = {
		0, 0, 0,
		0, 1, 0,
		0, 0, 0
	};
};