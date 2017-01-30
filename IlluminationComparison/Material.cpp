#include "Material.h"

Material::Material(ID3D11ShaderResourceView* SRV, ID3D11SamplerState* SamplerState)
{
	this->SRV = SRV;
	this->SamplerState = SamplerState;
}

void Material::Release()
{
	references--;
	if (references <= 0)
	{
		delete this;
	}
}

Material::~Material()
{
	SRV->Release();
}

CubeMap::CubeMap(ID3D11RasterizerState * rastState, ID3D11DepthStencilState * skyDepthState, ID3D11ShaderResourceView* SRV, ID3D11SamplerState* SamplerState)
	: Material(SRV, SamplerState)
{
	this->rastState = rastState;
	this->skyDepthState = skyDepthState;
}

CubeMap::~CubeMap()
{
	rastState->Release();
	skyDepthState->Release();
}
