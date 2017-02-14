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


PostProcesser::PostProcesser()
{
}


PostProcesser::~PostProcesser()
{
}


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

PostProcess()
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