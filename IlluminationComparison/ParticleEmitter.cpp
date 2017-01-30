#include "ParticleEmitter.h"

using namespace GMath;

void ParticleEmitter::Init(Renderer * renderer)
{
	particleTexture = renderer->GetMaterial(Texture)->GetSRV();
	particleSampler = renderer->GetSampler("default");

	this->particleVS = renderer->GetVertexShader(ParticleVS);
	this->particleGS = renderer->GetGeometryShader("particle");
	this->particlePS = renderer->GetPixelShader("particle");;

	this->spawnVS = renderer->GetVertexShader("spawn");
	this->spawnGS = renderer->GetGeometryShader("spawn");

	ZeroVec3(&Position);

	D3D11_BLEND_DESC blendDesc = {};
	blendDesc.AlphaToCoverageEnable = false;
	blendDesc.IndependentBlendEnable = false;
	blendDesc.RenderTarget[0].BlendEnable = true;
	blendDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
	blendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
	blendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_ONE;
	blendDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
	blendDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
	blendDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ONE;
	blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
	renderer->device->CreateBlendState(&blendDesc, &particleBlendState);

	D3D11_DEPTH_STENCIL_DESC depthDesc = {};
	depthDesc.DepthEnable = true;
	depthDesc.DepthFunc = D3D11_COMPARISON_LESS;
	depthDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
	renderer->device->CreateDepthStencilState(&depthDesc, &particleDepthState);

	// Particle geometry
	// Set up the vertices we want to put into the Vertex Buffer
	ParticleVertex vertices[1];
	vertices[0].Type = 0;
	vertices[0].Age = 0.0f;
	vertices[0].StartPosition = particleStartPosition;
	vertices[0].StartVelocity = particleStartVelocity;
	vertices[0].StartColor = particleStartColor;
	vertices[0].MidColor = particleMidColor;
	vertices[0].EndColor = particleEndColor;
	vertices[0].StartMidEndSize = VEC3(
		particleStartSize,
		particleMidSize,
		particleEndSize);

	// Create the vertex buffer
	D3D11_BUFFER_DESC vbd;
	vbd.Usage = D3D11_USAGE_IMMUTABLE;
	vbd.ByteWidth = sizeof(ParticleVertex) * 1;
	vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vbd.CPUAccessFlags = 0;
	vbd.MiscFlags = 0;
	vbd.StructureByteStride = 0;
	D3D11_SUBRESOURCE_DATA initialVertexData;
	initialVertexData.pSysMem = vertices;
	renderer->device->CreateBuffer(&vbd, &initialVertexData, &particleVB);

	// Create SO buffers
	spawnGS->CreateCompatibleStreamOutBuffer(&soBufferRead, 1000000);
	spawnGS->CreateCompatibleStreamOutBuffer(&soBufferWrite, 1000000);
	spawnFlip = false;
	frameCount = 0;

	initialized = true;
}

/// If a negative emitter lifetime is given the emitter will last forever
ParticleEmitter::ParticleEmitter(std::string particleVS, std::string texture,
			VEC3 startPos, VEC3 startVelocity, VEC3 acceleration,
			VEC4 startColor, VEC4 midColor, VEC4 endColor,
			float startSize, float midSize, float endSize,
			float emitterLifetime, float particleLifetime)
{
	this->ParticleVS = particleVS;
	this->Texture = texture;
	particleStartPosition = startPos;
	particleStartVelocity = startVelocity;
	particleConstantAccel = acceleration;
	particleStartColor = startColor;
	particleMidColor = midColor;
	particleEndColor = endColor;
	particleStartSize = startSize;
	particleMidSize = midSize;
	particleEndSize = endSize;

	particleAgeToSpawn = -emitterLifetime;
	particleMaxLifetime = particleLifetime;
	lastParticleLifetime = particleMaxLifetime * emitterLifetime;
}

/// If a negative emitter lifetime is given the emitter will last forever
ParticleEmitter::ParticleEmitter(VEC3 startPos, VEC3 startVelocity, VEC3 acceleration,
			VEC4 startColor, VEC4 midColor, VEC4 endColor,
			float startSize, float midSize, float endSize,
			float emitterLifetime, float particleLifetime)
{
	this->ParticleVS = "particle";
	this->Texture = "default";
	particleStartPosition = startPos;
	particleStartVelocity = startVelocity;
	particleConstantAccel = acceleration;
	particleStartColor = startColor;
	particleMidColor = midColor;
	particleEndColor = endColor;
	particleStartSize = startSize;
	particleMidSize = midSize;
	particleEndSize = endSize;

	particleAgeToSpawn = -emitterLifetime;
	particleMaxLifetime = particleLifetime;
	lastParticleLifetime = particleMaxLifetime * emitterLifetime;
}

ParticleEmitter::~ParticleEmitter()
{
	if (initialized)
	{
		particleVB->Release();
		soBufferRead->Release();
		soBufferWrite->Release();
		particleBlendState->Release();
		particleDepthState->Release();
	}
}

void ParticleEmitter::Draw(Renderer* renderer, float deltaTime, float totalTime)
{
	if (lastParticleLifetime < 0)
		dead = true;

	// Spawn particles
	DrawSpawn(renderer, deltaTime, totalTime);
	// Draw particles ----------------------------------------------------
	DrawParticles(renderer, deltaTime, totalTime);

	particleAgeToSpawn += deltaTime;
	lastParticleLifetime -= deltaTime;
}

void ParticleEmitter::DrawParticles(Renderer* renderer, float deltaTime, float totalTime)
{
	MAT4X4 worldMatrix;
	SetIdentity4X4(&worldMatrix);
	SetTransposeMatrix(&worldMatrix, &(CreateScaleMatrix(&VEC3(1,1,1)) * CreateRotationMatrix(&VEC3(0,0,0)) * CreateTranslationMatrix(&Position)));
	particleGS->SetMatrix4x4("world", worldMatrix);
	particleGS->SetMatrix4x4("view", *renderer->camera->GetView());
	particleGS->SetMatrix4x4("projection", *renderer->camera->GetProjection());
	particleGS->CopyAllBufferData();

	particleVS->SetFloat3("acceleration", particleConstantAccel);
	particleVS->SetFloat("maxLifetime", particleMaxLifetime);
	particleVS->CopyAllBufferData();

	particlePS->SetSamplerState("trilinear", particleSampler);
	particlePS->SetShaderResourceView("particleTexture", particleTexture);
	particlePS->CopyAllBufferData();

	particleVS->SetShader();
	particlePS->SetShader();
	particleGS->SetShader();

	// Set up states
	float factor[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
	renderer->context->OMSetBlendState(particleBlendState, factor, 0xffffffff);
	renderer->context->OMSetDepthStencilState(particleDepthState, 0);

	// Set buffers
	UINT particleStride = sizeof(ParticleVertex);
	UINT particleOffset = 0;
	renderer->context->IASetVertexBuffers(0, 1, &soBufferRead, &particleStride, &particleOffset);

	// Draw auto - draws based on current stream out buffer
	renderer->context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_POINTLIST);
	renderer->context->DrawAuto();

	// Unset Geometry Shader for next frame and reset states
	renderer->context->GSSetShader(0, 0, 0);
	renderer->context->OMSetBlendState(0, factor, 0xffffffff);
	renderer->context->OMSetDepthStencilState(0, 0);

	// Reset topology
	renderer->context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
}

void ParticleEmitter::SwapSOBuffers()
{
	ID3D11Buffer* temp = soBufferRead;
	soBufferRead = soBufferWrite;
	soBufferWrite = temp;
}

void ParticleEmitter::DrawSpawn(Renderer* renderer, float deltaTime, float totalTime)
{
	renderer->context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_POINTLIST);
	UINT stride = sizeof(ParticleVertex);
	UINT offset = 0;

	// Set/unset correct shaders
	// Set the delta time for the spawning
	spawnGS->SetFloat("dt", deltaTime);
	spawnGS->SetFloat("ageToSpawn", particleAgeToSpawn);
	spawnGS->SetFloat("maxLifetime", particleMaxLifetime);
	spawnGS->SetFloat("totalTime", totalTime);
	spawnGS->SetSamplerState("randomSampler", particleSampler);
	spawnGS->SetShaderResourceView("randomTexture", renderer->GetRandomTexture());
	spawnGS->SetShader();
	spawnGS->CopyAllBufferData();

	spawnVS->SetShader();
	spawnVS->CopyAllBufferData();

	renderer->context->PSSetShader(0, 0, 0); // No pixel shader needed

								   // Unbind vertex buffers (incase)
	ID3D11Buffer* unset = 0;
	renderer->context->IASetVertexBuffers(0, 1, &unset, &stride, &offset);

	// First frame?
	if (frameCount == 0)
	{
		// Draw using the seed vertex
		renderer->context->IASetVertexBuffers(0, 1, &particleVB, &stride, &offset);
		renderer->context->SOSetTargets(1, &soBufferWrite, &offset);
		renderer->context->Draw(1, 0);
		frameCount++;
	}
	else
	{
		// Draw using the buffers
		renderer->context->IASetVertexBuffers(0, 1, &soBufferRead, &stride, &offset);
		renderer->context->SOSetTargets(1, &soBufferWrite, &offset);
		renderer->context->DrawAuto();
	}

	// Unbind SO targets and shader
	SimpleGeometryShader::UnbindStreamOutStage(renderer->context);
	renderer->context->GSSetShader(0, 0, 0);

	// Swap after draw
	SwapSOBuffers();
}
