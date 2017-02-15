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



PostProcesser::PostProcesser(DefferedRenderer* renderingSystem)
{
	this->renderer = renderingSystem;
}

PostProcesser::~PostProcesser()
{
}



void PostProcesser::renderKernel(FLOAT kernel[9])
{
	UINT stride = sizeof(Vertex);
	UINT offset = 0;
	// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!#!#!#########!
	// renderer->context->OMSetRenderTargets(1, &backBufferRTV, 0);
	renderer->GetVertexShader("postprocess")->SetShader();

	renderer->GetPixelShader("kernel")->SetShader();
	renderer->GetPixelShader("kernel")->SetShaderResourceView("Pixels", postProcessSRV);
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

void PostProcesser::bloom()
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
	pixelShader->SetShaderResourceView("Pixels", postProcessSRV);
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
	// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!#!#!#########!
	// renderer->context->OMSetRenderTargets(1, &backBufferRTV, 0);
	pixelShader = renderer->GetPixelShader("bloomCombine");
	pixelShader->SetShader();
	pixelShader->SetShaderResourceView("Source", postProcessSRV);
	pixelShader->SetShaderResourceView("Blurred", bloomExtractSRV);
	pixelShader->SetSamplerState("Sampler", renderer->GetSampler("default"));
	pixelShader->CopyAllBufferData();
	renderer->context->Draw(3, 0);
	pixelShader->SetShaderResourceView("Source", 0);
	pixelShader->SetShaderResourceView("Blurred", 0);
}

void PostProcesser::blur()
{
	UINT stride = sizeof(Vertex);
	UINT offset = 0;
	// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!#!#!#########!
	// renderer->context->OMSetRenderTargets(1, &backBufferRTV, 0);
	// Set up post process shader
	renderer->GetVertexShader("blur")->SetShader();

	renderer->GetPixelShader("blur")->SetShader();
	renderer->GetPixelShader("blur")->SetShaderResourceView("Pixels", postProcessSRV);
	renderer->GetPixelShader("blur")->SetSamplerState("Sampler", renderer->GetSampler("default"));
	renderer->GetPixelShader("blur")->SetInt("blurAmount", 1);
	renderer->GetPixelShader("blur")->SetFloat("pixelWidth", 1.0f / renderer->width);
	renderer->GetPixelShader("blur")->SetFloat("pixelHeight", 1.0f / renderer->height);
	renderer->GetPixelShader("blur")->CopyAllBufferData();

	// Now actually draw
	ID3D11Buffer* nothing = 0;
	renderer->context->IASetVertexBuffers(0, 1, &nothing, &stride, &offset);
	renderer->context->IASetIndexBuffer(0, DXGI_FORMAT_R32_UINT, 0);

	renderer->context->Draw(3, 0);

	renderer->GetPixelShader("blur")->SetShaderResourceView("Pixels", 0);
}

void PostProcesser::ascii()
{
	UINT stride = sizeof(Vertex);
	UINT offset = 0;

	Mesh* meshTmp = renderer->GetMesh("quad");
	ID3D11Buffer* vertTemp = meshTmp->GetVertexBuffer();
	// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!#!#!#########!
	// renderer->context->OMSetRenderTargets(1, &backBufferRTV, 0);
	renderer->GetVertexShader("quad")->SetShader();

	renderer->GetPixelShader("ascii")->SetShader();
	renderer->GetPixelShader("ascii")->SetFloat("width", float(renderer->width));
	renderer->GetPixelShader("ascii")->SetFloat("height", float(renderer->height));
	renderer->GetPixelShader("ascii")->SetFloat("pixelWidth", 1.0f / renderer->width);
	renderer->GetPixelShader("ascii")->SetFloat("pixelHeight", 1.0f / renderer->height);
	renderer->GetPixelShader("ascii")->SetShaderResourceView("Pixels", postProcessSRV);
	// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!#!#!#########!
	// renderer->GetPixelShader("ascii")->SetShaderResourceView("ASCII", renderer->GetMaterial("ascii")->GetSRV());
	renderer->GetPixelShader("ascii")->SetSamplerState("Sampler", renderer->GetSampler("default"));
	renderer->GetPixelShader("ascii")->CopyAllBufferData();
	// Now actually draw
	renderer->context->IASetVertexBuffers(0, 1, &vertTemp, &stride, &offset);
	renderer->context->IASetIndexBuffer(meshTmp->GetIndexBuffer(), DXGI_FORMAT_R32_UINT, 0);
	renderer->context->DrawIndexed(meshTmp->GetIndexCount(), 0, 0);

	renderer->GetPixelShader("ascii")->SetShaderResourceView("Pixels", 0);
	renderer->GetPixelShader("ascii")->SetShaderResourceView("ASCII", 0);
}

void PostProcesser::unfinalizeCurrentFrame()
{
}

/*
SetUpPostProcessing()
{
	// Post Processing needs a Texture 
	D3D11_TEXTURE2D_DESC textureDesc = {};
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
*/