#pragma once
#include <DirectXMath.h>
#include "DefferedRenderer.h"
#include "DXMathImpl.h"
#include "SimpleShader.h"

class PostProcesser
{
	friend class DefferedRenderer;
	friend class Renderer;
public:
	PostProcesser(DefferedRenderer* renderingSystem);
	~PostProcesser();
	/*
	Assumes that there will be a dummy render target that the current frame has been placed in.
	Default is to finalize that frame and place it within the backBufferRTV
	*/
	void renderKernel(FLOAT kernel[9]);
	void bloom();
	void blur();
	void ascii();

private:
	/// Moves current backBufferData into unfinalizedRTV
	void unfinalizeCurrentFrame();

private:
	enum PostProcessFunction {DEFAULT, BLOOM, BLUR, ASCII, EDGE, EMBOSS, BLUR_K, SHARPNESS, SOBEL};

	DefferedRenderer* renderer;
	PostProcessFunction currentFunction = DEFAULT;

	ID3D11RenderTargetView* postProcessRTV;
	ID3D11RenderTargetView* ssaoRTV;
	ID3D11RenderTargetView* bloomExtractRTV; // will also be used for blurring
	ID3D11RenderTargetView* bloomHorizonatalRTV;

	ID3D11ShaderResourceView* postProcessSRV;
	ID3D11ShaderResourceView* ssaoSRV;
	ID3D11ShaderResourceView* bloomExtractSRV; // will also be used for blurring
	ID3D11ShaderResourceView* bloomHorizonatalSRV;
};