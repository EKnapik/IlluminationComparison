#pragma once
#include "GameMath.h"
#include "SimpleShader.h"
#include "Camera.h"
#include "Renderer.h"

class Renderer;

struct ParticleVertex
{
	int Type;
	float Age;
	VEC3 StartPosition;
	VEC3 StartVelocity;
	VEC4 StartColor;
	VEC4 MidColor;
	VEC4 EndColor;
	VEC3 StartMidEndSize;
};

class ParticleEmitter {
public:
	void Init(Renderer* renderer);

	ParticleEmitter(std::string particleVS, std::string texture,
		VEC3 startPos, VEC3 startVelocity, VEC3 acceleration,
		VEC4 startColor, VEC4 midColor, VEC4 endColor,
		float startSize, float midSize, float endSize,
		float emitterLifetime, float particleLifetime);

	ParticleEmitter(VEC3 startPos, VEC3 startVelocity, VEC3 acceleration,
		VEC4 startColor, VEC4 midColor, VEC4 endColor,
		float startSize, float midSize, float endSize,
		float emitterLifetime, float particleLifetime);

	virtual ~ParticleEmitter();

	void Draw(Renderer* renderer, float deltaTime, float totalTime);

	bool initialized = false;
	bool dead = false;

	VEC3 Position;

protected:
	virtual void DrawParticles(Renderer* renderer, float deltaTime, float totalTime);
	void SwapSOBuffers();
	void DrawSpawn(Renderer* renderer, float deltaTime, float totalTime);

	std::string ParticleVS;
	std::string Texture;

	// Particle-related
	ID3D11Buffer* particleVB;
	ID3D11Buffer* soBufferRead;
	ID3D11Buffer* soBufferWrite;
	bool spawnFlip;
	int frameCount;
	float lastParticleLifetime;
	
	SimpleVertexShader* particleVS;
	SimplePixelShader* particlePS;
	SimpleGeometryShader* particleGS;

	SimpleVertexShader* spawnVS;
	SimpleGeometryShader* spawnGS;

	ID3D11ShaderResourceView* particleTexture;
	ID3D11SamplerState* particleSampler;
	ID3D11BlendState* particleBlendState;
	ID3D11DepthStencilState* particleDepthState;

	// Particle params
	VEC3 particleStartPosition;
	VEC3 particleStartVelocity;
	VEC4 particleStartColor;
	VEC4 particleMidColor;
	VEC4 particleEndColor;
	float particleStartSize;
	float particleMidSize;
	float particleEndSize;

	float particleAgeToSpawn;
	float particleMaxLifetime;
	VEC3 particleConstantAccel;
};