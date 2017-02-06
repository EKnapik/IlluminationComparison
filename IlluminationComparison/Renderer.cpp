#include "Renderer.h"
#include "WICTextureLoader.h"
#include "DDSTextureLoader.h"
#include <time.h>


using namespace DirectX;

FLOAT edgeDetectKernel[] = {
-1, -1, -1,
-1, 8, -1,
-1, -1, -1
};

FLOAT embossKernel[] = {
-2, -1, 0,
-1, 1, 1,
0, 1, 2
};

FLOAT blurKernel[] = {
-1, 2, 1,
2, 4, 2,
1, 2, 1
};

FLOAT sharpnessKernel[] = {
-1, -1, -1,
-1, 9, -1,
-1, -1, -1
};

FLOAT bottomSobelKernel[] = {
-1, -2, -1,
0, 0, 0,
1, 2, 1
};

FLOAT defaultKernel[] = {
0, 0, 0,
0, 1, 0,
0, 0, 0
};

Renderer::Renderer(Camera *camera, ID3D11DeviceContext *context, ID3D11Device* device, ID3D11RenderTargetView* backBufferRTV, ID3D11DepthStencilView* depthStencilView, int width, int height)
{
	this->camera = camera;
	this->device = device;
	this->context = context;
	this->backBufferRTV = backBufferRTV;
	this->depthStencilView = depthStencilView;
	this->width = width;
	this->height = height;

	D3D11_SAMPLER_DESC samplerDesc = {};
	samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;
	AddSampler("default", &samplerDesc);

	SetUpShadows();
	SetUpRandomTexture();
	SetUpPostProcessing();
}


Renderer::~Renderer()
{
	typedef std::map<std::string, Mesh*>::iterator mesh_type;
	for (mesh_type iterator = MeshDictionary.begin(); iterator != MeshDictionary.end(); iterator++) {
		iterator->second->Release();
	}

	typedef std::map<std::string, Material*>::iterator material_type;
	for (material_type iterator = MaterialDictionary.begin(); iterator != MaterialDictionary.end(); iterator++) {
		iterator->second->Release();
	}

	typedef std::map<std::string, SimplePixelShader*>::iterator pixel_type;
	for (pixel_type iterator = PixelShaderDictionary.begin(); iterator != PixelShaderDictionary.end(); iterator++) {
		delete iterator->second;
	}

	typedef std::map<std::string, SimpleVertexShader*>::iterator vertex_type;
	for (vertex_type iterator = VertexShaderDictionary.begin(); iterator != VertexShaderDictionary.end(); iterator++) {
		delete iterator->second;
	}

	typedef std::map<std::string, SimpleGeometryShader*>::iterator geometry_type;
	for (geometry_type iterator = GeometryShaderDictionary.begin(); iterator != GeometryShaderDictionary.end(); iterator++) {
		delete iterator->second;
	}

	typedef std::map<std::string, ID3D11SamplerState*>::iterator sampler_type;
	for (sampler_type iterator = SamplerDictionary.begin(); iterator != SamplerDictionary.end(); iterator++) {
		iterator->second->Release();
	}

	shadowDSV->Release();
	shadowSRV->Release();
	shadowRasterizer->Release();
	randomTexture->Release();
	randomSRV->Release();
	postProcessSRV->Release();
	postProcessRTV->Release();
	ssaoRTV->Release();
	ssaoSRV->Release();
	bloomExtractRTV->Release();
	bloomHorizonatalRTV->Release();
	bloomExtractSRV->Release();
	bloomHorizonatalSRV->Release();
}

void Renderer::RenderShadowMap()
{
	// Set up targets
	context->OMSetRenderTargets(0, 0, shadowDSV);
	context->ClearDepthStencilView(shadowDSV, D3D11_CLEAR_DEPTH, 1.0f, 0);
	context->RSSetState(shadowRasterizer);

	// Make a viewport to match the render target size
	D3D11_VIEWPORT viewport = {};
	viewport.TopLeftX = 0;
	viewport.TopLeftY = 0;
	viewport.Width = (float)shadowMapSize;
	viewport.Height = (float)shadowMapSize;
	viewport.MinDepth = 0.0f;
	viewport.MaxDepth = 1.0f;
	context->RSSetViewports(1, &viewport);

	// Set up our shadow VS shader
	SimpleVertexShader* shadowShader = GetVertexShader("shadow");
	shadowShader->SetShader();
	shadowShader->SetMatrix4x4("view", directionalLights->at(0).ViewMatrix );
	shadowShader->SetMatrix4x4("projection", shadowDirectionalProjectionMatrix);

	// Turn off pixel shader
	context->PSSetShader(0, 0, 0);

	// Loop through entities and draw them
	UINT stride = sizeof(Vertex);
	UINT offset = 0;

	for (unsigned int i = 0; i < gameEntitys->size(); i++)
	{
		// Grab the data from the first entity's mesh
		GameEntity* ge = gameEntitys->at(i);
		ID3D11Buffer* vb = GetMesh(gameEntitys->at(i)->GetMesh())->GetVertexBuffer();
		ID3D11Buffer* ib = GetMesh(gameEntitys->at(i)->GetMesh())->GetIndexBuffer();

		// Set buffers in the input assembler
		context->IASetVertexBuffers(0, 1, &vb, &stride, &offset);
		context->IASetIndexBuffer(ib, DXGI_FORMAT_R32_UINT, 0);

		shadowShader->SetMatrix4x4("world", *ge->GetWorld());
		shadowShader->CopyAllBufferData();

		// Finally do the actual drawing
		context->DrawIndexed(GetMesh(gameEntitys->at(i)->GetMesh())->GetIndexCount(), 0, 0);
	}

	// Change everything back
	context->OMSetRenderTargets(1, &backBufferRTV, depthStencilView);
	viewport.Width = (float)width;
	viewport.Height = (float)height;
	context->RSSetViewports(1, &viewport);
	context->RSSetState(0);
}

void Renderer::DrawOneMaterial(FLOAT deltaTime, FLOAT totalTime)
{
}

void Renderer::DrawMultipleMaterials(FLOAT deltaTime, FLOAT totalTime)
{
}

void Renderer::DrawSkyBox()
{
	CubeMap* sky = skyBox;
	SimpleVertexShader* vertexShader = GetVertexShader("skybox");
	SimplePixelShader* pixelShader = GetPixelShader("skybox");

	UINT stride = sizeof(Vertex);
	UINT offset = 0;
	ID3D11Buffer* vertTemp = GetMesh("cube")->GetVertexBuffer();
	context->IASetVertexBuffers(0, 1, &vertTemp, &stride, &offset);
	context->IASetIndexBuffer(GetMesh("cube")->GetIndexBuffer(), DXGI_FORMAT_R32_UINT, 0);

	vertexShader->SetMatrix4x4("view", *camera->GetView());
	vertexShader->SetMatrix4x4("projection", *camera->GetProjection());
	vertexShader->CopyAllBufferData();
	vertexShader->SetShader();

	pixelShader->SetShaderResourceView("Sky", sky->GetSRV());
	pixelShader->CopyAllBufferData();
	pixelShader->SetShader();
	context->RSSetState(sky->rastState);
	context->OMSetDepthStencilState(sky->skyDepthState, 0);

	context->DrawIndexed(GetMesh("cube")->GetIndexCount(), 0, 0);

	// Reset the states!
	context->RSSetState(0);
	context->OMSetDepthStencilState(0, 0);
}

void Renderer::DrawParticleEmitters(FLOAT deltaTime, FLOAT totalTime)
{
	for (int i = 0; i < particleEmitters->size(); i++)
	{
		if (particleEmitters->at(i)->initialized == false)
			particleEmitters->at(i)->Init(this);
		particleEmitters->at(i)->Draw(this, deltaTime, totalTime);
	}
}

void Renderer::PostProcess()
{
	UINT stride = sizeof(Vertex);
	UINT offset = 0;

	if (Blur)
	{
		context->OMSetRenderTargets(1, &backBufferRTV, 0);
		// Set up post process shader
		GetVertexShader("blur")->SetShader();

		GetPixelShader("blur")->SetShader();
		GetPixelShader("blur")->SetShaderResourceView("Pixels", postProcessSRV);
		GetPixelShader("blur")->SetSamplerState("Sampler", GetSampler("default"));
		GetPixelShader("blur")->SetInt("blurAmount", 1);
		GetPixelShader("blur")->SetFloat("pixelWidth", 1.0f / width);
		GetPixelShader("blur")->SetFloat("pixelHeight", 1.0f / height);
		GetPixelShader("blur")->CopyAllBufferData();

		// Now actually draw
		ID3D11Buffer* nothing = 0;
		context->IASetVertexBuffers(0, 1, &nothing, &stride, &offset);
		context->IASetIndexBuffer(0, DXGI_FORMAT_R32_UINT, 0);

		context->Draw(3, 0);

		GetPixelShader("blur")->SetShaderResourceView("Pixels", 0);
	}
	if (EdgeDetect)
	{
		context->OMSetRenderTargets(1, &backBufferRTV, 0);
		GetVertexShader("postprocess")->SetShader();

		GetPixelShader("kernel")->SetShader();
		GetPixelShader("kernel")->SetShaderResourceView("Pixels", postProcessSRV);
		GetPixelShader("kernel")->SetSamplerState("Sampler", GetSampler("default"));
		GetPixelShader("kernel")->SetFloat("pixelWidth", 1.0f / width);
		GetPixelShader("kernel")->SetFloat("pixelHeight", 1.0f / height);
		GetPixelShader("kernel")->SetFloat3("kernelA", VEC3(edgeDetectKernel[0], edgeDetectKernel[1], edgeDetectKernel[2]));
		GetPixelShader("kernel")->SetFloat3("kernelB", VEC3(edgeDetectKernel[3], edgeDetectKernel[4], edgeDetectKernel[5]));
		GetPixelShader("kernel")->SetFloat3("kernelC", VEC3(edgeDetectKernel[6], edgeDetectKernel[7], edgeDetectKernel[8]));
		GetPixelShader("kernel")->SetFloat("kernelWeight", 1);
		GetPixelShader("kernel")->CopyAllBufferData();

		// Now actually draw
		ID3D11Buffer* nothing = 0;
		context->IASetVertexBuffers(0, 1, &nothing, &stride, &offset);
		context->IASetIndexBuffer(0, DXGI_FORMAT_R32_UINT, 0);

		context->Draw(3, 0);

		GetPixelShader("kernel")->SetShaderResourceView("Pixels", 0);
	}
	if (Emboss)
	{
		context->OMSetRenderTargets(1, &backBufferRTV, 0);
		GetVertexShader("postprocess")->SetShader();

		GetPixelShader("kernel")->SetShader();
		GetPixelShader("kernel")->SetShaderResourceView("Pixels", postProcessSRV);
		GetPixelShader("kernel")->SetSamplerState("Sampler", GetSampler("default"));
		GetPixelShader("kernel")->SetFloat("pixelWidth", 1.0f / width);
		GetPixelShader("kernel")->SetFloat("pixelHeight", 1.0f / height);
		GetPixelShader("kernel")->SetFloat3("kernelA", VEC3(embossKernel[0], embossKernel[1], embossKernel[2]));
		GetPixelShader("kernel")->SetFloat3("kernelB", VEC3(embossKernel[3], embossKernel[4], embossKernel[5]));
		GetPixelShader("kernel")->SetFloat3("kernelC", VEC3(embossKernel[6], embossKernel[7], embossKernel[8]));
		GetPixelShader("kernel")->SetFloat("kernelWeight", 1);
		GetPixelShader("kernel")->CopyAllBufferData();

		// Now actually draw
		ID3D11Buffer* nothing = 0;
		context->IASetVertexBuffers(0, 1, &nothing, &stride, &offset);
		context->IASetIndexBuffer(0, DXGI_FORMAT_R32_UINT, 0);

		context->Draw(3, 0);

		GetPixelShader("kernel")->SetShaderResourceView("Pixels", 0);
	}
	if (BlurWithKernel)
	{
		context->OMSetRenderTargets(1, &backBufferRTV, 0);
		GetVertexShader("postprocess")->SetShader();

		GetPixelShader("kernel")->SetShader();
		GetPixelShader("kernel")->SetShaderResourceView("Pixels", postProcessSRV);
		GetPixelShader("kernel")->SetSamplerState("Sampler", GetSampler("default"));
		GetPixelShader("kernel")->SetFloat("pixelWidth", 1.0f / width);
		GetPixelShader("kernel")->SetFloat("pixelHeight", 1.0f / height);
		GetPixelShader("kernel")->SetFloat3("kernelA", VEC3(blurKernel[0], blurKernel[1], blurKernel[2]));
		GetPixelShader("kernel")->SetFloat3("kernelB", VEC3(blurKernel[3], blurKernel[4], blurKernel[5]));
		GetPixelShader("kernel")->SetFloat3("kernelC", VEC3(blurKernel[6], blurKernel[7], blurKernel[8]));
		GetPixelShader("kernel")->SetFloat("kernelWeight", 14);
		GetPixelShader("kernel")->CopyAllBufferData();

		// Now actually draw
		ID3D11Buffer* nothing = 0;
		context->IASetVertexBuffers(0, 1, &nothing, &stride, &offset);
		context->IASetIndexBuffer(0, DXGI_FORMAT_R32_UINT, 0);

		context->Draw(3, 0);

		GetPixelShader("kernel")->SetShaderResourceView("Pixels", 0);
	}
	if (Sharpness)
	{
		context->OMSetRenderTargets(1, &backBufferRTV, 0);
		GetVertexShader("postprocess")->SetShader();

		GetPixelShader("kernel")->SetShader();
		GetPixelShader("kernel")->SetShaderResourceView("Pixels", postProcessSRV);
		GetPixelShader("kernel")->SetSamplerState("Sampler", GetSampler("default"));
		GetPixelShader("kernel")->SetFloat("pixelWidth", 1.0f / width);
		GetPixelShader("kernel")->SetFloat("pixelHeight", 1.0f / height);
		GetPixelShader("kernel")->SetFloat3("kernelA", VEC3(sharpnessKernel[0], sharpnessKernel[1], sharpnessKernel[2]));
		GetPixelShader("kernel")->SetFloat3("kernelB", VEC3(sharpnessKernel[3], sharpnessKernel[4], sharpnessKernel[5]));
		GetPixelShader("kernel")->SetFloat3("kernelC", VEC3(sharpnessKernel[6], sharpnessKernel[7], sharpnessKernel[8]));
		GetPixelShader("kernel")->SetFloat("kernelWeight", 1);
		GetPixelShader("kernel")->CopyAllBufferData();

		// Now actually draw
		ID3D11Buffer* nothing = 0;
		context->IASetVertexBuffers(0, 1, &nothing, &stride, &offset);
		context->IASetIndexBuffer(0, DXGI_FORMAT_R32_UINT, 0);

		context->Draw(3, 0); 

		GetPixelShader("kernel")->SetShaderResourceView("Pixels", 0);
	}
	if (BottomSobel)
	{
		context->OMSetRenderTargets(1, &backBufferRTV, 0);
		GetVertexShader("postprocess")->SetShader();

		GetPixelShader("kernel")->SetShader();
		GetPixelShader("kernel")->SetShaderResourceView("Pixels", postProcessSRV);
		GetPixelShader("kernel")->SetSamplerState("Sampler", GetSampler("default"));
		GetPixelShader("kernel")->SetFloat("pixelWidth", 1.0f / width);
		GetPixelShader("kernel")->SetFloat("pixelHeight", 1.0f / height);
		GetPixelShader("kernel")->SetFloat3("kernelA", VEC3(bottomSobelKernel[0], bottomSobelKernel[1], bottomSobelKernel[2]));
		GetPixelShader("kernel")->SetFloat3("kernelB", VEC3(bottomSobelKernel[3], bottomSobelKernel[4], bottomSobelKernel[5]));
		GetPixelShader("kernel")->SetFloat3("kernelC", VEC3(bottomSobelKernel[6], bottomSobelKernel[7], bottomSobelKernel[8]));
		GetPixelShader("kernel")->SetFloat("kernelWeight", 1);
		GetPixelShader("kernel")->CopyAllBufferData();

		// Now actually draw
		ID3D11Buffer* nothing = 0;
		context->IASetVertexBuffers(0, 1, &nothing, &stride, &offset);
		context->IASetIndexBuffer(0, DXGI_FORMAT_R32_UINT, 0);
		context->Draw(3, 0);

		GetPixelShader("kernel")->SetShaderResourceView("Pixels", 0);
	}
	if (Bloom)
	{
		const float black[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
		context->ClearRenderTargetView(bloomExtractRTV, black);
		context->ClearRenderTargetView(bloomHorizonatalRTV, black);

		ID3D11Buffer* nothing = 0;
		SimplePixelShader* pixelShader;
		float intensityThreshold = 2.5f;
		float dir[2];
		GetVertexShader("postprocess")->SetShader();

		
		// extract
		context->OMSetRenderTargets(1, &bloomExtractRTV, 0);
		pixelShader = GetPixelShader("bloomExtract");
		pixelShader->SetShader();
		pixelShader->SetShaderResourceView("Pixels", 0);
		pixelShader->SetShaderResourceView("Pixels", postProcessSRV);
		pixelShader->SetSamplerState("Sampler", GetSampler("default"));
		pixelShader->SetFloat("threshold", intensityThreshold);
		pixelShader->CopyAllBufferData();
		// draw to the extract
		
		context->IASetVertexBuffers(0, 1, &nothing, &stride, &offset);
		context->IASetIndexBuffer(0, DXGI_FORMAT_R32_UINT, 0);
		context->Draw(3, 0);
		pixelShader->SetShaderResourceView("Pixels", 0);
		
		// blur horizontal
		dir[0] = 1.0f;
		dir[1] = 0.0f;
		context->OMSetRenderTargets(1, &bloomHorizonatalRTV, 0);
		pixelShader = GetPixelShader("linearBlur");
		pixelShader->SetShader();
		pixelShader->SetShaderResourceView("Pixels", 0);
		pixelShader->SetShaderResourceView("Pixels", bloomExtractSRV);
		pixelShader->SetSamplerState("Sampler", GetSampler("default"));
		pixelShader->SetFloat2("dir", dir);
		pixelShader->SetFloat("pixelWidth", 1.0f / width);
		pixelShader->SetFloat("pixelHeight", 1.0f / height);
		pixelShader->CopyAllBufferData();
		// draw to the extract
		context->IASetVertexBuffers(0, 1, &nothing, &stride, &offset);
		context->IASetIndexBuffer(0, DXGI_FORMAT_R32_UINT, 0);
		context->Draw(3, 0);
		pixelShader->SetShaderResourceView("Pixels", 0);
		context->ClearRenderTargetView(bloomExtractRTV, black);

		// blur vertical
		dir[0] = 0.0f;
		dir[1] = 1.0f;
		context->OMSetRenderTargets(1, &bloomExtractRTV, 0); // can resuse this texture
		pixelShader->SetShaderResourceView("Pixels", bloomHorizonatalSRV);
		pixelShader->SetSamplerState("Sampler", GetSampler("default"));
		pixelShader->SetFloat2("dir", dir);
		pixelShader->SetFloat("pixelWidth", 1.0f / width);
		pixelShader->SetFloat("pixelHeight", 1.0f / height);
		pixelShader->CopyAllBufferData();
		// draw to the extract
		context->Draw(3, 0);
		pixelShader->SetShaderResourceView("Pixels", 0);

		// additively blend to back buffer
		// bloomExtract now holds the blurred bright pixels
		context->OMSetRenderTargets(1, &backBufferRTV, 0);
		pixelShader = GetPixelShader("bloomCombine");
		pixelShader->SetShader();
		pixelShader->SetShaderResourceView("Source", postProcessSRV);
		pixelShader->SetShaderResourceView("Blurred", bloomExtractSRV);
		pixelShader->SetSamplerState("Sampler", GetSampler("default"));
		pixelShader->CopyAllBufferData();
		context->Draw(3, 0);
		pixelShader->SetShaderResourceView("Source", 0);
		pixelShader->SetShaderResourceView("Blurred", 0);
	}
	if (ASCII)
	{
		Mesh* meshTmp = GetMesh("quad");
		ID3D11Buffer* vertTemp = meshTmp->GetVertexBuffer();
		context->OMSetRenderTargets(1, &backBufferRTV, 0);
		GetVertexShader("quad")->SetShader();

		GetPixelShader("ascii")->SetShader();
		GetPixelShader("ascii")->SetFloat("width", float(width));
		GetPixelShader("ascii")->SetFloat("height", float(height));
		GetPixelShader("ascii")->SetFloat("pixelWidth", 1.0f / width);
		GetPixelShader("ascii")->SetFloat("pixelHeight", 1.0f / height);
		GetPixelShader("ascii")->SetShaderResourceView("Pixels", postProcessSRV);
		GetPixelShader("ascii")->SetShaderResourceView("ASCII", GetMaterial("ascii")->GetSRV());
		GetPixelShader("ascii")->SetSamplerState("Sampler", GetSampler("default"));
		GetPixelShader("ascii")->CopyAllBufferData();
		// Now actually draw
		context->IASetVertexBuffers(0, 1, &vertTemp, &stride, &offset);
		context->IASetIndexBuffer(meshTmp->GetIndexBuffer(), DXGI_FORMAT_R32_UINT, 0);
		context->DrawIndexed(meshTmp->GetIndexCount(), 0, 0);

		GetPixelShader("ascii")->SetShaderResourceView("Pixels", 0);
		GetPixelShader("ascii")->SetShaderResourceView("ASCII", 0);
	}
}

void Renderer::AddMesh(std::string name, Mesh * mesh)
{
	MeshDictionary.insert(std::pair<std::string, Mesh*>(name, mesh));
	mesh->AddReference();
}

void Renderer::AddMesh(std::string name, std::string path)
{
	Mesh* mesh = new Mesh(path, device);
	MeshDictionary.insert(std::pair<std::string, Mesh*>(name, mesh));
	mesh->AddReference();
}

void Renderer::AddMaterial(std::string name, Material * material)
{
	MaterialDictionary.insert(std::pair<std::string, Material*>(name, material));
	material->AddReference();
}

void Renderer::AddMaterial(std::string name, std::wstring path, std::string sampler)
{
	ID3D11ShaderResourceView* SRV;
	CreateWICTextureFromFile(device, context, path.c_str(), 0, &SRV);
	Material* mat = new Material(SRV, GetSampler(sampler));
	MaterialDictionary.insert(std::pair<std::string, Material*>(name, mat));
	mat->AddReference();
}

void Renderer::AddMaterial(std::string name, std::wstring path)
{
	AddMaterial(name, path, "default");
}

void Renderer::AddCubeMaterial(std::string name, CubeMap * material)
{
	MaterialDictionary.insert(std::pair<std::string, Material*>(name, material));
	material->AddReference();
}

void Renderer::AddCubeMaterial(std::string name, std::wstring path, std::string sampler, D3D11_RASTERIZER_DESC* rasterDesc, D3D11_DEPTH_STENCIL_DESC* depthStencilDesc)
{
	ID3D11ShaderResourceView* SRV;
	HRESULT error = CreateDDSTextureFromFile(device, path.c_str(), 0, &SRV);
	ID3D11RasterizerState* rastState;
	ID3D11DepthStencilState* depthState;
	device->CreateRasterizerState(rasterDesc, &rastState);
	device->CreateDepthStencilState(depthStencilDesc, &depthState);
	CubeMap* mat = new CubeMap(rastState, depthState, SRV, GetSampler(sampler));
	MaterialDictionary.insert(std::pair<std::string, Material*>(name, mat));
	mat->AddReference();
}

void Renderer::AddCubeMaterial(std::string name, std::wstring path)
{
	D3D11_DEPTH_STENCIL_DESC dsDesc = {};
	dsDesc.DepthEnable = true;
	dsDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
	dsDesc.DepthFunc = D3D11_COMPARISON_LESS_EQUAL; // Make sure we can see the sky (at max depth)

	D3D11_RASTERIZER_DESC rsDesc = {};
	rsDesc.FillMode = D3D11_FILL_SOLID;
	rsDesc.CullMode = D3D11_CULL_FRONT;
	rsDesc.DepthClipEnable = true;
	AddCubeMaterial(name, path, "default", &rsDesc, &dsDesc);
}

void Renderer::AddVertexShader(std::string name, std::wstring path)
{
	std::wstring debug = L"Debug/";
	debug += path;
	SimpleVertexShader* vertexShader = new SimpleVertexShader(device, context);
	
	if (!vertexShader->LoadShaderFile(debug.c_str()))
	{
		vertexShader->LoadShaderFile(path.c_str());
	}
	VertexShaderDictionary.insert(std::pair<std::string, SimpleVertexShader*>(name, vertexShader));
}

void Renderer::AddPixelShader(std::string name, std::wstring path)
{
	std::wstring debug = L"Debug/";
	debug += path;
	SimplePixelShader* pixelShader = new SimplePixelShader(device, context);
	if (!pixelShader->LoadShaderFile(debug.c_str()))
	{
		pixelShader->LoadShaderFile(path.c_str());
	}
	PixelShaderDictionary.insert(std::pair<std::string, SimplePixelShader*>(name, pixelShader));
}

void Renderer::AddGeometryShader(std::string name, std::wstring path)
{
	std::wstring debug = L"Debug/";
	debug += path;
	SimpleGeometryShader* geometryShader = new SimpleGeometryShader(device, context);
	if (!geometryShader->LoadShaderFile(debug.c_str()))
	{
		geometryShader->LoadShaderFile(path.c_str());
	}
	GeometryShaderDictionary.insert(std::pair<std::string, SimpleGeometryShader*>(name, geometryShader));
}

void Renderer::AddGeometryShader(std::string name, std::wstring path, bool useStreamOut, bool allowStreamOutRasterization)
{
	std::wstring debug = L"Debug/";
	debug += path;
	SimpleGeometryShader* geometryShader = new SimpleGeometryShader(device, context, useStreamOut, allowStreamOutRasterization);
	if (!geometryShader->LoadShaderFile(debug.c_str()))
	{
		geometryShader->LoadShaderFile(path.c_str());
	}
	GeometryShaderDictionary.insert(std::pair<std::string, SimpleGeometryShader*>(name, geometryShader));
}

void Renderer::AddSampler(std::string name, D3D11_SAMPLER_DESC * sampleDesc)
{
	ID3D11SamplerState* samplerState;
	device->CreateSamplerState(sampleDesc, &samplerState);
	SamplerDictionary.insert(std::pair<std::string, ID3D11SamplerState*>(name, samplerState));
}

ID3D11SamplerState * Renderer::GetSampler(std::string name)
{
	return SamplerDictionary.at(name);
}

void Renderer::SetSkyBox(std::string name)
{
	skyBox = (CubeMap*)GetMaterial(name);
}


/// Comparison for std::sort
bool frontToBack(GameEntity* s1, GameEntity* s2)
{
	return s1->GetPosition().z < s2->GetPosition().z;
}

bool backToFront(GameEntity* s1, GameEntity* s2)
{
	return s1->GetPosition().z > s2->GetPosition().z;
}


void Renderer::DisableAllPostProcess()
{
	PostProcessing = false;
	Blur = false;
	EdgeDetect = false;
	Bloom = false;
	Emboss = false;
	BlurWithKernel = false;
	Sharpness = false;
	BottomSobel = false;
	ASCII = false;
}

void Renderer::SortObjects()
{
	opaque.clear();
	transparent.clear();
	for (int i = 0; i < gameEntitys->size(); i++)
	{
		if (GetMaterial(gameEntitys->at(i)->GetMaterial())->transparency)
			transparent.push_back(gameEntitys->at(i));
		else
			opaque.push_back(gameEntitys->at(i));
	}
	std::sort(opaque.begin(), opaque.end(), frontToBack);
	std::sort(transparent.begin(), transparent.end(), backToFront);
}


void Renderer::SetUpShadows()
{
	// Get setup for shadows
	D3D11_TEXTURE2D_DESC shadowDesc = {};
	shadowDesc.Width = shadowMapSize;
	shadowDesc.Height = shadowMapSize;
	shadowDesc.ArraySize = 1;
	shadowDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE;
	shadowDesc.CPUAccessFlags = 0;
	shadowDesc.Format = DXGI_FORMAT_R32_TYPELESS;
	shadowDesc.MipLevels = 1;
	shadowDesc.MiscFlags = 0;
	shadowDesc.SampleDesc.Count = 1;
	shadowDesc.SampleDesc.Quality = 0;
	shadowDesc.Usage = D3D11_USAGE_DEFAULT;
	ID3D11Texture2D* shadowTexture;
	device->CreateTexture2D(&shadowDesc, 0, &shadowTexture);

	// Create the depth/stencil
	D3D11_DEPTH_STENCIL_VIEW_DESC shadowDSDesc = {};
	shadowDSDesc.Format = DXGI_FORMAT_D32_FLOAT;
	shadowDSDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	shadowDSDesc.Texture2D.MipSlice = 0;
	device->CreateDepthStencilView(shadowTexture, &shadowDSDesc, &shadowDSV);

	// Create the SRV for the shadow map
	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Format = DXGI_FORMAT_R32_FLOAT;
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MipLevels = 1;
	srvDesc.Texture2D.MostDetailedMip = 0;
	device->CreateShaderResourceView(shadowTexture, &srvDesc, &shadowSRV);

	// Release the texture reference since we don't need it
	shadowTexture->Release();

	// Create the special "comparison" sampler state for shadows
	D3D11_SAMPLER_DESC shadowSampDesc = {};
	shadowSampDesc.Filter = D3D11_FILTER_COMPARISON_MIN_MAG_MIP_LINEAR; // Could be anisotropic
	shadowSampDesc.ComparisonFunc = D3D11_COMPARISON_LESS;
	shadowSampDesc.AddressU = D3D11_TEXTURE_ADDRESS_BORDER;
	shadowSampDesc.AddressV = D3D11_TEXTURE_ADDRESS_BORDER;
	shadowSampDesc.AddressW = D3D11_TEXTURE_ADDRESS_BORDER;
	shadowSampDesc.BorderColor[0] = 1.0f;
	shadowSampDesc.BorderColor[1] = 1.0f;
	shadowSampDesc.BorderColor[2] = 1.0f;
	shadowSampDesc.BorderColor[3] = 1.0f;
	AddSampler("shadow", &shadowSampDesc);

	// Create a rasterizer state
	D3D11_RASTERIZER_DESC shadowRastDesc = {};
	shadowRastDesc.FillMode = D3D11_FILL_SOLID;
	shadowRastDesc.CullMode = D3D11_CULL_BACK;
	shadowRastDesc.DepthClipEnable = true;
	shadowRastDesc.DepthBias = 1000; // Multiplied by (smallest possible value > 0 in depth buffer)
	shadowRastDesc.DepthBiasClamp = 0.0f;
	shadowRastDesc.SlopeScaledDepthBias = 1.0f;
	device->CreateRasterizerState(&shadowRastDesc, &shadowRasterizer);

	// Orthographic to match the directional light
	//TODO: This shouldn't call direct X math stuff directly. Fix that
	XMMATRIX shProj = XMMatrixOrthographicLH(30, 30, 0.1f, 100.0f);
	XMStoreFloat4x4(&shadowDirectionalProjectionMatrix, XMMatrixTranspose(shProj));

	XMMATRIX shProjPersp = XMMatrixPerspectiveLH(10, 10, 0.1f, 100.0f);
	XMStoreFloat4x4(&shadowPointProjectionMatrix, XMMatrixTranspose(shProj));
}

void Renderer::SetUpRandomTexture()
{
	// Set up "random" stuff -------------------------------------
	unsigned int randomTextureWidth = 1024;

	// Random data for the 1D texture
	srand((unsigned int)time(0));
	std::vector<float> data(randomTextureWidth * 4);
	for (unsigned int i = 0; i < randomTextureWidth * 4; i++)
		data[i] = rand() / (float)RAND_MAX * 2.0f - 1.0f;

	// Set up texture
	D3D11_TEXTURE1D_DESC textureDesc;
	textureDesc.ArraySize = 1;
	textureDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	textureDesc.CPUAccessFlags = 0;
	textureDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	textureDesc.MipLevels = 1;
	textureDesc.MiscFlags = 0;
	textureDesc.Usage = D3D11_USAGE_DEFAULT;
	textureDesc.Width = 100;

	D3D11_SUBRESOURCE_DATA initData;
	initData.pSysMem = (void*)&data[0];
	initData.SysMemPitch = randomTextureWidth * sizeof(float) * 4;
	initData.SysMemSlicePitch = 0;
	device->CreateTexture1D(&textureDesc, &initData, &randomTexture);

	// Set up SRV for texture
	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
	srvDesc.Format = textureDesc.Format;
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE1D;
	srvDesc.Texture1D.MipLevels = 1;
	srvDesc.Texture1D.MostDetailedMip = 0;
	device->CreateShaderResourceView(randomTexture, &srvDesc, &randomSRV);
}

FLOAT lerp(FLOAT a, FLOAT b, FLOAT f)
{
	return a + f * (b - a);
}

void Renderer::SetUpPostProcessing()
{
	// Post Processing needs a Texture 
	D3D11_TEXTURE2D_DESC textureDesc = {};
	// TODO: We still don't pass in width and height
	textureDesc.Width = width;
	textureDesc.Height = height;
	textureDesc.ArraySize = 1;
	textureDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
	textureDesc.CPUAccessFlags = 0;
	textureDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	textureDesc.MipLevels = 1;
	textureDesc.MiscFlags = 0;
	textureDesc.SampleDesc.Count = 1;
	textureDesc.SampleDesc.Quality = 0;
	textureDesc.Usage = D3D11_USAGE_DEFAULT;

	ID3D11Texture2D* ppTexture;
	ID3D11Texture2D* ssaoTexture;
	ID3D11Texture2D* bloomExtractTexture;
	ID3D11Texture2D* bloomHorizontalTexture;
	HRESULT result = device->CreateTexture2D(&textureDesc, 0, &ppTexture);
	result = device->CreateTexture2D(&textureDesc, 0, &ssaoTexture);
	result = device->CreateTexture2D(&textureDesc, 0, &bloomExtractTexture);
	result = device->CreateTexture2D(&textureDesc, 0, &bloomHorizontalTexture);

	// It is also going to need its own render Target View so we can go 
	// ahead and set that up too.
	D3D11_RENDER_TARGET_VIEW_DESC rtvDesc = {};
	rtvDesc.Format = textureDesc.Format;
	rtvDesc.Texture2D.MipSlice = 0;
	rtvDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;

	device->CreateRenderTargetView(ppTexture, &rtvDesc, &postProcessRTV);
	device->CreateRenderTargetView(ssaoTexture, &rtvDesc, &ssaoRTV);
	device->CreateRenderTargetView(bloomExtractTexture, &rtvDesc, &bloomExtractRTV);
	device->CreateRenderTargetView(bloomHorizontalTexture, &rtvDesc, &bloomHorizonatalRTV);

	//Lastly create a Shader Resource View For it.
	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Format = textureDesc.Format;
	srvDesc.Texture2D.MipLevels = 1;
	srvDesc.Texture2D.MostDetailedMip = 0;
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;

	device->CreateShaderResourceView(ppTexture, &srvDesc, &postProcessSRV);
	device->CreateShaderResourceView(ssaoTexture, &srvDesc, &ssaoSRV);
	device->CreateShaderResourceView(bloomExtractTexture, &srvDesc, &bloomExtractSRV);
	device->CreateShaderResourceView(bloomHorizontalTexture, &srvDesc, &bloomHorizonatalSRV);

	// Release the texture because it is now stored on the GPU.
	ppTexture->Release();
	ssaoTexture->Release();
	bloomExtractTexture->Release();
	bloomHorizontalTexture->Release();


	// Sample kernel for ssao
	std::uniform_real_distribution<FLOAT> randomFloats(0.0, 1.0); // generates random floats between 0.0 and 1.0
	std::default_random_engine generator;
	for (INT i = 0; i < 64; ++i)
	{
		VEC3 sample(randomFloats(generator) * 2.0 - 1.0, randomFloats(generator) * 2.0 - 1.0, randomFloats(generator));

		VECTOR vec = GMath::GetVector(&sample);
		vec = GMath::Vec3Normalize(&vec);
		DirectX::XMStoreFloat3(&sample, vec);

		sample.x *= randomFloats(generator);
		sample.y *= randomFloats(generator);
		sample.z *= randomFloats(generator);
		FLOAT scale = FLOAT(i) / 64.0;

		// Scale samples s.t. they're more aligned to center of kernel
		scale = lerp(0.1f, 1.0f, scale * scale);
		sample.x *= scale;
		sample.y *= scale;
		sample.z *= scale;
		ssaoKernel.push_back(sample);
	}
}

Mesh * Renderer::GetMesh(std::string name)
{
	return MeshDictionary.at(name);
}

Material * Renderer::GetMaterial(std::string name)
{
	return MaterialDictionary.at(name);
}

SimpleVertexShader * Renderer::GetVertexShader(std::string name)
{
	return VertexShaderDictionary.at(name);
}

SimplePixelShader * Renderer::GetPixelShader(std::string name)
{
	return PixelShaderDictionary.at(name);
}

SimpleGeometryShader * Renderer::GetGeometryShader(std::string name)
{
	return GeometryShaderDictionary.at(name);
}


/*
I need to clean this up a bit and make this an actual primary class. That is pulling some
of the code out of the deffered rendering.

Sort objects front to back opaque.
sort objects back to front transluscent.

The deferred rendering has a specific way to render normal materials, and PBR materials
*/
