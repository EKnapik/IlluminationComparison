#include "PostProcesser.h"


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

using namespace DirectX;

PostProcesser::PostProcesser(DefferedRenderer* renderingSystem)
{
	this->renderer = renderingSystem;

	// Post Processing needs a Texture 
	D3D11_TEXTURE2D_DESC textureDesc = {};
	textureDesc.Width = renderer->width;
	textureDesc.Height = renderer->height;
	textureDesc.ArraySize = 1;
	textureDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
	textureDesc.CPUAccessFlags = 0;
	textureDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	textureDesc.MipLevels = 1;
	textureDesc.MiscFlags = 0;
	textureDesc.SampleDesc.Count = 1;
	textureDesc.SampleDesc.Quality = 0;
	textureDesc.Usage = D3D11_USAGE_DEFAULT;

	ID3D11Texture2D* bloomExtractTexture;
	ID3D11Texture2D* bloomHorizontalTexture;
	HRESULT result = renderer->device->CreateTexture2D(&textureDesc, 0, &bloomExtractTexture);
	result = renderer->device->CreateTexture2D(&textureDesc, 0, &bloomHorizontalTexture);

	// It is also going to need its own render Target View so we can go 
	// ahead and set that up too.
	D3D11_RENDER_TARGET_VIEW_DESC rtvDesc = {};
	rtvDesc.Format = textureDesc.Format;
	rtvDesc.Texture2D.MipSlice = 0;
	rtvDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;

	renderer->device->CreateRenderTargetView(bloomExtractTexture, &rtvDesc, &bloomExtractRTV);
	renderer->device->CreateRenderTargetView(bloomHorizontalTexture, &rtvDesc, &bloomHorizonatalRTV);

	//Lastly create a Shader Resource View For it.
	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Format = textureDesc.Format;
	srvDesc.Texture2D.MipLevels = 1;
	srvDesc.Texture2D.MostDetailedMip = 0;
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;

	renderer->device->CreateShaderResourceView(bloomExtractTexture, &srvDesc, &bloomExtractSRV);
	renderer->device->CreateShaderResourceView(bloomHorizontalTexture, &srvDesc, &bloomHorizonatalSRV);

	// Release the texture because it is now stored on the GPU.
	bloomExtractTexture->Release();
	bloomHorizontalTexture->Release();

	CreateWICTextureFromFile(renderer->device, renderer->context, L"Assets/Textures/asciiTexture.png", 0, &asciiSRV);
	SetUpSSAO();
}

PostProcesser::~PostProcesser()
{
	bloomExtractRTV->Release();
	bloomExtractSRV->Release();

	bloomHorizonatalRTV->Release();
	bloomHorizonatalSRV->Release();

	noiseSRV->Release();
	asciiSRV->Release();
}



void PostProcesser::renderKernel(FLOAT kernel[9], ID3D11ShaderResourceView* readFrom, ID3D11RenderTargetView* writeTo)
{
	UINT stride = sizeof(Vertex);
	UINT offset = 0;
	renderer->context->OMSetRenderTargets(1, &writeTo, 0);
	renderer->GetVertexShader("postprocess")->SetShader();

	renderer->GetPixelShader("kernel")->SetShader();
	renderer->GetPixelShader("kernel")->SetShaderResourceView("Pixels", readFrom);
	renderer->GetPixelShader("kernel")->SetSamplerState("Sampler", renderer->GetSampler("default"));
	renderer->GetPixelShader("kernel")->SetFloat("pixelWidth", 1.0f / renderer->width);
	renderer->GetPixelShader("kernel")->SetFloat("pixelHeight", 1.0f / renderer->height);
	renderer->GetPixelShader("kernel")->SetFloat3("kernelA", VEC3(kernel[0], kernel[1], kernel[2]));
	renderer->GetPixelShader("kernel")->SetFloat3("kernelB", VEC3(kernel[3], kernel[4], kernel[5]));
	renderer->GetPixelShader("kernel")->SetFloat3("kernelC", VEC3(kernel[6], kernel[7], kernel[8]));
	renderer->GetPixelShader("kernel")->SetFloat("kernelWeight", 1);
	renderer->GetPixelShader("kernel")->CopyAllBufferData();

	// Now actually draw
	ID3D11Buffer* nothing = 0;
	renderer->context->IASetVertexBuffers(0, 1, &nothing, &stride, &offset);
	renderer->context->IASetIndexBuffer(0, DXGI_FORMAT_R32_UINT, 0);

	renderer->context->Draw(3, 0);

	renderer->GetPixelShader("kernel")->SetShaderResourceView("Pixels", 0);
}


void PostProcesser::bloom(ID3D11ShaderResourceView* readFrom, ID3D11RenderTargetView* writeTo)
{
	const float black[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
	renderer->context->ClearRenderTargetView(bloomExtractRTV, black);
	renderer->context->ClearRenderTargetView(bloomHorizonatalRTV, black);

	UINT stride = sizeof(Vertex);
	UINT offset = 0;
	ID3D11Buffer* nothing = 0;
	SimplePixelShader* pixelShader;
	float intensityThreshold = 2.5f;
	float dir[2];
	renderer->GetVertexShader("postprocess")->SetShader();


	// extract
	renderer->context->OMSetRenderTargets(1, &bloomExtractRTV, 0);
	pixelShader = renderer->GetPixelShader("bloomExtract");
	pixelShader->SetShader();
	pixelShader->SetShaderResourceView("Pixels", 0);
	pixelShader->SetShaderResourceView("Pixels", readFrom);
	pixelShader->SetSamplerState("Sampler", renderer->GetSampler("default"));
	pixelShader->SetFloat("threshold", intensityThreshold);
	pixelShader->CopyAllBufferData();
	// draw to the extract

	renderer->context->IASetVertexBuffers(0, 1, &nothing, &stride, &offset);
	renderer->context->IASetIndexBuffer(0, DXGI_FORMAT_R32_UINT, 0);
	renderer->context->Draw(3, 0);
	pixelShader->SetShaderResourceView("Pixels", 0);

	// blur horizontal
	dir[0] = 1.0f;
	dir[1] = 0.0f;
	renderer->context->OMSetRenderTargets(1, &bloomHorizonatalRTV, 0);
	pixelShader = renderer->GetPixelShader("linearBlur");
	pixelShader->SetShader();
	pixelShader->SetShaderResourceView("Pixels", 0);
	pixelShader->SetShaderResourceView("Pixels", bloomExtractSRV);
	pixelShader->SetSamplerState("Sampler", renderer->GetSampler("default"));
	pixelShader->SetFloat2("dir", dir);
	pixelShader->SetFloat("pixelWidth", 1.0f / renderer->width);
	pixelShader->SetFloat("pixelHeight", 1.0f / renderer->height);
	pixelShader->CopyAllBufferData();
	// draw to the extract
	renderer->context->IASetVertexBuffers(0, 1, &nothing, &stride, &offset);
	renderer->context->IASetIndexBuffer(0, DXGI_FORMAT_R32_UINT, 0);
	renderer->context->Draw(3, 0);
	pixelShader->SetShaderResourceView("Pixels", 0);
	renderer->context->ClearRenderTargetView(bloomExtractRTV, black);

	// blur vertical
	dir[0] = 0.0f;
	dir[1] = 1.0f;
	renderer->context->OMSetRenderTargets(1, &bloomExtractRTV, 0); // can resuse this texture
	pixelShader->SetShaderResourceView("Pixels", bloomHorizonatalSRV);
	pixelShader->SetSamplerState("Sampler", renderer->GetSampler("default"));
	pixelShader->SetFloat2("dir", dir);
	pixelShader->SetFloat("pixelWidth", 1.0f / renderer->width);
	pixelShader->SetFloat("pixelHeight", 1.0f / renderer->height);
	pixelShader->CopyAllBufferData();
	// draw to the extract
	renderer->context->Draw(3, 0);
	pixelShader->SetShaderResourceView("Pixels", 0);

	// additively blend to back buffer
	// bloomExtract now holds the blurred bright pixels
	renderer->context->OMSetRenderTargets(1, &writeTo, 0);
	pixelShader = renderer->GetPixelShader("bloomCombine");
	pixelShader->SetShader();
	pixelShader->SetShaderResourceView("Source", readFrom);
	pixelShader->SetShaderResourceView("Blurred", bloomExtractSRV);
	pixelShader->SetSamplerState("Sampler", renderer->GetSampler("default"));
	pixelShader->CopyAllBufferData();
	renderer->context->Draw(3, 0);
	pixelShader->SetShaderResourceView("Source", 0);
	pixelShader->SetShaderResourceView("Blurred", 0);
	
}

void PostProcesser::blur(ID3D11ShaderResourceView* readFrom, ID3D11RenderTargetView* writeTo)
{
	UINT stride = sizeof(Vertex);
	UINT offset = 0;
	renderer->context->OMSetRenderTargets(1, &writeTo, 0);
	// Set up post process shader
	SimpleVertexShader* vertexShader = renderer->GetVertexShader("blur");
	SimplePixelShader* pixelShader = renderer->GetPixelShader("blur");
	vertexShader->SetShader();
	pixelShader->SetShader();

	pixelShader->SetShaderResourceView("Pixels", readFrom);
	pixelShader->SetSamplerState("Sampler", renderer->GetSampler("default"));
	pixelShader->SetInt("blurAmount", 1);
	pixelShader->SetFloat("pixelWidth", 1.0f / renderer->width);
	pixelShader->SetFloat("pixelHeight", 1.0f / renderer->height);
	pixelShader->CopyAllBufferData();

	// Now actually draw
	ID3D11Buffer* nothing = 0;
	renderer->context->IASetVertexBuffers(0, 1, &nothing, &stride, &offset);
	renderer->context->IASetIndexBuffer(0, DXGI_FORMAT_R32_UINT, 0);

	renderer->context->Draw(3, 0);

	pixelShader->SetShaderResourceView("Pixels", 0);
	
}

void PostProcesser::ascii(ID3D11ShaderResourceView* readFrom, ID3D11RenderTargetView* writeTo)
{
	UINT stride = sizeof(Vertex);
	UINT offset = 0;
	Mesh* meshTmp = renderer->GetMesh("quad");
	ID3D11Buffer* vertTemp = meshTmp->GetVertexBuffer();

	// setup writing to final buffer
	renderer->context->OMSetRenderTargets(1, &writeTo, 0);
	SimpleVertexShader* vertexShader = renderer->GetVertexShader("quad");
	SimplePixelShader* pixelShader = renderer->GetPixelShader("ascii");
	vertexShader->SetShader();

	pixelShader->SetShader();
	pixelShader->SetFloat("width", float(renderer->width));
	pixelShader->SetFloat("height", float(renderer->height));
	pixelShader->SetFloat("pixelWidth", 1.0f / renderer->width);
	pixelShader->SetFloat("pixelHeight", 1.0f / renderer->height);
	pixelShader->SetShaderResourceView("Pixels", readFrom);
	renderer->GetPixelShader("ascii")->SetShaderResourceView("ASCII", asciiSRV);
	pixelShader->SetSamplerState("Sampler", renderer->GetSampler("default"));
	pixelShader->CopyAllBufferData();
	// Now actually draw
	renderer->context->IASetVertexBuffers(0, 1, &vertTemp, &stride, &offset);
	renderer->context->IASetIndexBuffer(meshTmp->GetIndexBuffer(), DXGI_FORMAT_R32_UINT, 0);
	renderer->context->DrawIndexed(meshTmp->GetIndexCount(), 0, 0);

	pixelShader->SetShaderResourceView("Pixels", 0);
	pixelShader->SetShaderResourceView("ASCII", 0);
}


void PostProcesser::ssao(ID3D11RenderTargetView* writeTo)
{
	UINT stride = sizeof(Vertex);
	UINT offset = 0;
	Mesh* meshTmp = renderer->GetMesh("quad");
	ID3D11Buffer* vertTemp = meshTmp->GetVertexBuffer();
	SimpleVertexShader* vertexShader = renderer->GetVertexShader("quadPBR");
	SimplePixelShader* pixelShader = renderer->GetPixelShader("ssao");

	const float white[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
	renderer->context->ClearRenderTargetView(writeTo, white);
	renderer->context->OMSetRenderTargets(1, &writeTo, 0);

	vertexShader->SetShader();
	vertexShader->SetMatrix4x4("invProjection", *renderer->camera->GetInvProjection());
	vertexShader->SetMatrix4x4("invViewProj", *renderer->camera->GetInvViewProj());
	vertexShader->SetFloat3("cameraPosition", *renderer->camera->GetPosition());
	vertexShader->CopyAllBufferData();

	pixelShader->SetShader();
	pixelShader->SetMatrix4x4("view", *renderer->camera->GetView());
	pixelShader->SetMatrix4x4("projection", *renderer->camera->GetProjection());
	pixelShader->SetFloat3("cameraPosition", *renderer->camera->GetPosition());
	pixelShader->SetFloat("width", float(renderer->width));
	pixelShader->SetFloat("height", float(renderer->height));
	pixelShader->SetFloat("zFar", renderer->camera->GetFarPlane());

	pixelShader->SetShaderResourceView("texNoise", noiseSRV);
	pixelShader->SetShaderResourceView("gNormal", renderer->NormalSRV);
	pixelShader->SetShaderResourceView("gDepth", renderer->DepthSRV);
	pixelShader->SetSamplerState("Sampler", renderer->GetSampler("default"));
	pixelShader->CopyAllBufferData();
	// Now actually draw
	renderer->context->IASetVertexBuffers(0, 1, &vertTemp, &stride, &offset);
	renderer->context->IASetIndexBuffer(meshTmp->GetIndexBuffer(), DXGI_FORMAT_R32_UINT, 0);
	renderer->context->DrawIndexed(meshTmp->GetIndexCount(), 0, 0);

	pixelShader->SetShaderResourceView("texNoise", 0);
	pixelShader->SetShaderResourceView("gNormal", 0);
	pixelShader->SetShaderResourceView("gPosition", 0);
}


void PostProcesser::passThrough(ID3D11ShaderResourceView* readFrom, ID3D11RenderTargetView* writeTo)
{
	UINT stride = sizeof(Vertex);
	UINT offset = 0;
	SimpleVertexShader* vertexShader = renderer->GetVertexShader("postprocess");
	SimplePixelShader* pixelShader = renderer->GetPixelShader("passThrough");
	renderer->context->OMSetRenderTargets(1, &writeTo, 0);
	vertexShader->SetShader();
	vertexShader->CopyAllBufferData();
	pixelShader->SetShader();
	pixelShader->SetShaderResourceView("Pixels", readFrom);
	pixelShader->SetSamplerState("Sampler", renderer->GetSampler("default"));
	pixelShader->CopyAllBufferData();

	// Now actually draw
	ID3D11Buffer* nothing = 0;
	renderer->context->IASetVertexBuffers(0, 1, &nothing, &stride, &offset);
	renderer->context->IASetIndexBuffer(0, DXGI_FORMAT_R32_UINT, 0);

	renderer->context->Draw(3, 0);
	pixelShader->SetShaderResourceView("Pixels", 0);
}

/*
SetUpPostProcessing()
{
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
*/
void PostProcesser::SetUpSSAO()
{
	// Set up "random" stuff -------------------------------------
	unsigned int randomTextureSize = 4;

	// Random data for the noise texture
	std::uniform_real_distribution<FLOAT> randomFloats(0.0, 1.0); // generates floats
	std::default_random_engine generator;
	std::vector<VEC4> ssaoNoise(randomTextureSize * randomTextureSize);
	for (unsigned int i = 0; i < randomTextureSize * randomTextureSize; i++)
		ssaoNoise[i] = VEC4(randomFloats(generator), randomFloats(generator), 0.0f, 1.0f);

	ID3D11Texture2D* noiseTexture;
	// Set up texture
	D3D11_TEXTURE2D_DESC textureDesc;
	textureDesc.Width = randomTextureSize;
	textureDesc.Height = randomTextureSize;
	textureDesc.ArraySize = 1;
	textureDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	textureDesc.CPUAccessFlags = 0;
	textureDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	textureDesc.MipLevels = 1;
	textureDesc.MiscFlags = 0;
	textureDesc.SampleDesc.Count = 1;
	textureDesc.SampleDesc.Quality = 0;
	textureDesc.Usage = D3D11_USAGE_DEFAULT;
	
	D3D11_SUBRESOURCE_DATA initData;
	ZeroMemory(&initData, sizeof(D3D11_SUBRESOURCE_DATA));
	
	initData.pSysMem = (void*)&ssaoNoise[0];
	initData.SysMemPitch = randomTextureSize * sizeof(float) * 4;
	initData.SysMemSlicePitch = randomTextureSize * randomTextureSize * sizeof(float) * 4;
	renderer->device->CreateTexture2D(&textureDesc, &initData, &noiseTexture);

	// Set up SRV for texture
	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
	srvDesc.Format = textureDesc.Format;
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture1D.MipLevels = 1;
	srvDesc.Texture1D.MostDetailedMip = 0;
	renderer->device->CreateShaderResourceView(noiseTexture, &srvDesc, &noiseSRV);

    noiseTexture->Release();
}