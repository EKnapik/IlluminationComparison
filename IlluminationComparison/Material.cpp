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

PBRMaterial::PBRMaterial(ID3D11SamplerState * sampler, ID3D11ShaderResourceView * albedo, ID3D11ShaderResourceView * normal, ID3D11ShaderResourceView * metallic, ID3D11ShaderResourceView * roughness, ID3D11ShaderResourceView * ao)
{
	this->SamplerState = sampler;
	this->albedoSRV = albedo;
	this->normalSRV = normal;
	this->metallicSRV = metallic;
	this->roughnessSRV = roughness;
	this->aoSRV = ao;
}

PBRMaterial::PBRMaterial(ID3D11SamplerState * sampler, ID3D11ShaderResourceView * albedo, ID3D11ShaderResourceView * normal, float metallic, float roughness, ID3D11ShaderResourceView * ao)
{
	this->SamplerState = sampler;
	this->albedoSRV = albedo;
	this->normalSRV = normal;
	this->metallic = metallic;
	this->roughness = roughness;
	this->aoSRV = ao;
}

void PBRMaterial::Release()
{
	references--;
	if (references <= 0)
	{
		delete this;
	}
}

PBRMaterial::~PBRMaterial()
{
	albedoSRV->Release();
	normalSRV->Release();
	if (metallicSRV != nullptr)
		metallicSRV->Release();
	if (roughnessSRV != nullptr)
		roughnessSRV->Release();
	aoSRV->Release();
}
