#pragma once
#include <DirectXMath.h>
#include "DXMathImpl.h"
#include "SimpleShader.h"

class PostProcesser
{
public:
	PostProcesser();
	~PostProcesser();

private:
	ID3D11RenderTargetView* postProcessRTV;
	ID3D11RenderTargetView* ssaoRTV;
	ID3D11RenderTargetView* bloomExtractRTV; // will also be used for blurring
	ID3D11RenderTargetView* bloomHorizonatalRTV;

	ID3D11ShaderResourceView* postProcessSRV;
	ID3D11ShaderResourceView* ssaoSRV;
	ID3D11ShaderResourceView* bloomExtractSRV; // will also be used for blurring
	ID3D11ShaderResourceView* bloomHorizonatalSRV;
};

/*
given the current back buffer and has a pointer to the gbuffer
places contents back to the back buffer when done
Not as efficient but easier to manage when thinking of infinite memory.
*/