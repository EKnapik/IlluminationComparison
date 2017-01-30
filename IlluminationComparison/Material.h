#pragma once

#include "SimpleShader.h"


class Material {
public:
	Material(ID3D11ShaderResourceView* SRV, ID3D11SamplerState* SamplerState);

	void AddReference() { references++; };

	void Release();

	bool transparency = false;

	ID3D11ShaderResourceView* GetSRV() { return SRV; };
	ID3D11SamplerState* GetSamplerState() { return SamplerState; }
protected:
	int references = 0;

	ID3D11ShaderResourceView* SRV;
	ID3D11SamplerState* SamplerState;

	virtual ~Material();
	
};

class CubeMap : public Material {
public:
	CubeMap(ID3D11RasterizerState* rastState, ID3D11DepthStencilState* skyDepthState, ID3D11ShaderResourceView* SRV, ID3D11SamplerState* SamplerState);
	ID3D11RasterizerState* rastState;
	ID3D11DepthStencilState* skyDepthState;
	~CubeMap();
};