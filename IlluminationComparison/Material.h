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


class PBRMaterial {
public:
	PBRMaterial(ID3D11SamplerState* sampler, ID3D11ShaderResourceView* albedo, ID3D11ShaderResourceView* normal,
		ID3D11ShaderResourceView* metallic, ID3D11ShaderResourceView* roughness, ID3D11ShaderResourceView* ao);

	PBRMaterial(ID3D11SamplerState* sampler, ID3D11ShaderResourceView* albedo, ID3D11ShaderResourceView* normal,
		float metallic, float roughness, ID3D11ShaderResourceView* ao);

	void AddReference() { references++; };
	void Release();

	void SetMetallic(float newMetallic) { this->metallic = newMetallic; }
	void SetRoughness(float newRoughness) { this->metallic = newRoughness; }

	ID3D11SamplerState* GetSamplerState() const { return SamplerState; }
	ID3D11ShaderResourceView* GetAlbedoSRV() const { return albedoSRV; };
	ID3D11ShaderResourceView* GetNormalSRV() const { return normalSRV; };
	ID3D11ShaderResourceView* GetMetallicSRV() const { return metallicSRV; };
	ID3D11ShaderResourceView* GetRoughnessSRV() const { return roughnessSRV; };
	ID3D11ShaderResourceView* GetAOSRV() const { return aoSRV; };
	float GetMetallicParam() const { return metallic; }
	float GetRoughnessParam() const { return roughness; }

private:
	~PBRMaterial();

	int references = 0;
	ID3D11SamplerState* SamplerState;
	ID3D11ShaderResourceView* albedoSRV;
	ID3D11ShaderResourceView* normalSRV;
	ID3D11ShaderResourceView* metallicSRV;
	ID3D11ShaderResourceView* roughnessSRV;
	ID3D11ShaderResourceView* aoSRV;
	float metallic;
	float roughness;
};