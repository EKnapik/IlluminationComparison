#include "SparseVoxelOctree.h"


SparseVoxelOctree::SparseVoxelOctree(ID3D11Device * device)
{
	this->device = device;
}

SparseVoxelOctree::~SparseVoxelOctree()
{
	if (initialized)
	{
		octreeUAV->Release();
		octreeSRV->Release();
	}
}


void SparseVoxelOctree::initSVO()
{
	initVoxelCounter();
	voxelizeGeometry(0);
	// GET VALUE FROM COUNTER TO PASS INTO INIT VOXEL LIST !!!!!!!!!!!!!!!!!!!!!!!!!!
	initVoxelList(0);
	voxelizeGeometry(1);

	initOctree();
	createOctree(0);
	createOctree(1);
	mipMapUpOctree();

	deleteVoxelList(); // also will delete the counter
	initialized = true;
}


void SparseVoxelOctree::initVoxelCounter()
{
	// Create VoxelList Counter
	D3D11_BUFFER_DESC bufDesc;
	memset(&bufDesc, 0, sizeof(bufDesc));
	bufDesc.Usage = D3D11_USAGE_DEFAULT;
	bufDesc.ByteWidth = sizeof(INT32);
	bufDesc.BindFlags = D3D11_BIND_UNORDERED_ACCESS;
	bufDesc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
	bufDesc.StructureByteStride = sizeof(INT32);
	bufDesc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;

	HRESULT result = device->CreateBuffer(&bufDesc, NULL, &counter);
	assert(result == S_OK);

	D3D11_UNORDERED_ACCESS_VIEW_DESC uavDesc;
	memset(&uavDesc, 0, sizeof(uavDesc));
	uavDesc.Format = DXGI_FORMAT_UNKNOWN;
	uavDesc.Buffer.FirstElement = 0;
	uavDesc.Buffer.NumElements = 1;
	uavDesc.Buffer.Flags = 0;
	uavDesc.ViewDimension = D3D11_UAV_DIMENSION_BUFFER;

	// TODO: Check the results and error handle
	result = device->CreateUnorderedAccessView(counter, &uavDesc, &counterUAV);
	assert(result == S_OK);
}


void SparseVoxelOctree::initVoxelList(int numElements)
{
	ID3D11Buffer *voxelListBuffer;

	D3D11_BUFFER_DESC bufDesc;
	memset(&bufDesc, 0, sizeof(bufDesc));
	bufDesc.Usage = D3D11_USAGE_DEFAULT;
	bufDesc.ByteWidth = sizeof(Voxel) * numElements;
	bufDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_UNORDERED_ACCESS;
	bufDesc.CPUAccessFlags = 0;
	bufDesc.StructureByteStride = sizeof(Voxel);
	bufDesc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;

	HRESULT result = device->CreateBuffer(&bufDesc, NULL, &voxelListBuffer);
	assert(result == S_OK);

	D3D11_UNORDERED_ACCESS_VIEW_DESC uavDesc;
	memset(&uavDesc, 0, sizeof(uavDesc));
	uavDesc.Format = DXGI_FORMAT_UNKNOWN;
	uavDesc.Buffer.FirstElement = 0;
	uavDesc.Buffer.NumElements = numElements;
	uavDesc.Buffer.Flags = 0;
	uavDesc.ViewDimension = D3D11_UAV_DIMENSION_BUFFER;

	// TODO: Check the results and error handle
	result = device->CreateUnorderedAccessView(voxelListBuffer, &uavDesc, &voxelListUAV);
	assert(result == S_OK);

	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
	memset(&srvDesc, 0, sizeof(srvDesc));
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_BUFFER;
	srvDesc.Format = DXGI_FORMAT_UNKNOWN;
	srvDesc.Buffer.ElementWidth = numElements;

	result = device->CreateShaderResourceView(voxelListBuffer, &srvDesc, &voxelListSRV);
	assert(result == S_OK);

	// release the cpu buffer
	voxelListBuffer->Release();
}

void SparseVoxelOctree::initOctree()
{
	/*
	//Calculate the maximum possilbe node number
	int totalNode = 1;
	int nTmp = 1;
	for (int i = 1; i <= octreeLevel; ++i)
	{
		nTmp *= 8;
		totalNode += nTmp;
	}
	cout << "Max possible node: " << totalNode << endl;
	*/
	// TODO:
	// INITIALLY I WANT TO MAKE THE BUFFER JUST LARGE ENOUGH TO WORK THEN THE INTENTION IS TO DO AN AVERAGE
	// OF VALUES IF THEIR FINAL NODE RESULTS IN A COLLISION BECAUSE OF SIZE RESTRICTION
	int numOctreeNodes = 0;

	ID3D11Buffer *octreeBuffer;

	D3D11_BUFFER_DESC bufDesc;
	memset(&bufDesc, 0, sizeof(bufDesc));
	bufDesc.Usage = D3D11_USAGE_DEFAULT;
	bufDesc.ByteWidth = sizeof(Node) * numOctreeNodes;
	bufDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_UNORDERED_ACCESS;
	bufDesc.CPUAccessFlags = 0;
	bufDesc.StructureByteStride = sizeof(Node);
	bufDesc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;

	HRESULT result = device->CreateBuffer(&bufDesc, NULL, &octreeBuffer);
	assert(result == S_OK);

	D3D11_UNORDERED_ACCESS_VIEW_DESC uavDesc;
	memset(&uavDesc, 0, sizeof(uavDesc));
	uavDesc.Format = DXGI_FORMAT_UNKNOWN;
	uavDesc.Buffer.FirstElement = 0;
	uavDesc.Buffer.NumElements = numOctreeNodes;
	uavDesc.Buffer.Flags = 0;
	uavDesc.ViewDimension = D3D11_UAV_DIMENSION_BUFFER;

	// TODO: Check the results and error handle
	result = device->CreateUnorderedAccessView(octreeBuffer, &uavDesc, &octreeUAV);
	assert(result == S_OK);

	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
	memset(&srvDesc, 0, sizeof(srvDesc));
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_BUFFER;
	srvDesc.Format = DXGI_FORMAT_UNKNOWN;
	srvDesc.Buffer.ElementWidth = numOctreeNodes;

	result = device->CreateShaderResourceView(octreeBuffer, &srvDesc, &octreeSRV);
	assert(result == S_OK);

	// release the cpu buffer
	octreeBuffer->Release();
}

void SparseVoxelOctree::voxelizeGeometry(int mode)
{
	ID3D11RasterizerState *voxelRastState;
	D3D11_RASTERIZER_DESC voxelRastDesc = {};
	voxelRastDesc.FillMode = D3D11_FILL_SOLID;
	voxelRastDesc.CullMode = D3D11_CULL_NONE;
	voxelRastDesc.DepthClipEnable = false;
	device->CreateRasterizerState(&voxelRastDesc, &voxelRastState);

	// context->OMSetRenderTargets(1, 0, 0);
	// context->OMSetDepthStencilState(0, 0);
	// context->RSSetState(voxelRastState);
	SimpleVertexShader*   vertexShader;
	SimpleGeometryShader* geomShader;
	SimplePixelShader*    pixelShader;
	vertexShader->SetShader();
	vertexShader->CopyAllBufferData();

	geomShader->SetShader();
	// TODO FIX THESE MATRICIES ******************************************
	geomShader->SetMatrix4x4("ViewProjX", 0);
	geomShader->SetMatrix4x4("ViewProjY", 0);
	geomShader->SetMatrix4x4("ViewProjZ", 0);
	geomShader->SetInt("height", voxelDim);
	geomShader->SetInt("width", voxelDim);

	pixelShader->SetShader();
	pixelShader->SetInt("store", mode); // 0 to count 1 to store
	pixelShader->SetUnorderedAccessView("voxelList", voxelListUAV);
	pixelShader->SetUnorderedAccessView("atomicCounter", counterUAV);

	UINT stride = sizeof(Vertex);
	UINT offset = 0;
	Mesh* meshTmp;
	for (int i = 0; i < opaque.size(); i++)
	{
		PBRMaterial* material = GetMaterial(opaque.at(i)->GetMaterial());
		// Send texture Info
		pixelShader->SetSamplerState("basicSampler", material->GetSamplerState());
		pixelShader->SetShaderResourceView("albedoMap", material->GetAlbedoSRV());
		pixelShader->SetShaderResourceView("normalMap", material->GetNormalSRV());
		pixelShader->SetShaderResourceView("metalMap", material->GetMetallicSRV());
		pixelShader->SetShaderResourceView("roughMap", material->GetRoughnessSRV());
		pixelShader->SetFloat("metallic", material->GetMetallicParam());
		pixelShader->SetFloat("roughness", material->GetRoughnessParam());
		pixelShader->CopyAllBufferData();

		// Send Geometry
		geomShader->SetMatrix4x4("World", *opaque.at(i)->GetWorld());
		geomShader->CopyAllBufferData();
		
		meshTmp = GetMesh(opaque.at(i)->GetMesh());
		ID3D11Buffer* vertTemp = meshTmp->GetVertexBuffer();
		context->IASetVertexBuffers(0, 1, &vertTemp, &stride, &offset);
		context->IASetIndexBuffer(meshTmp->GetIndexBuffer(), DXGI_FORMAT_R32_UINT, 0);
		context->DrawIndexed(meshTmp->GetIndexCount(), 0, 0);
	}

	// RESET STATES
	// renderer->context->GSSetShader(0, 0, 0); // unset geometry shader
	// context->RSSetState(0); // reset state
}

void SparseVoxelOctree::deleteVoxelList()
{
	counter->Release();
	counterUAV->Release();
	voxelListUAV->Release();
	voxelListSRV->Release();
}

void SparseVoxelOctree::createOctree(int mode)
{
	// For each node in the node list execute a compute shader to first 'allocate'
		// The allocation helps with avoiding moving data around and can tell when collisions happen for after
	// For each node in the node list execute a compute shader to set and store the data
}

void SparseVoxelOctree::mipMapUpOctree()
{
	// Set 8 compute shader values to traverse down the octree for their respective levels
	// averaging as they move back up.
	// IS IT POSSIBLE TO MAKE RECURSIVE COMPUTE SHADER CALLS?
}
