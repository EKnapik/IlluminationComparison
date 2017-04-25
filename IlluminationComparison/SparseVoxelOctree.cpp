#include "SparseVoxelOctree.h"
using namespace DirectX;

SparseVoxelOctree::SparseVoxelOctree(DefferedRenderer* const renderer)
{
	initVoxelCounter(renderer->device);
	initVoxelList(renderer->device, 16); // DirectX requires VoxelList to exist and be minimum size
	voxelizeGeometry(renderer, 0);
	// get count of voxels
	voxelListBuffer->Release();
	voxelListSRV->Release();
	voxelListUAV->Release();
	voxelCount = getCount(renderer->device, renderer->context);
	initVoxelList(renderer->device, voxelCount);
 	voxelizeGeometry(renderer, 1);
	// use breakpoint debug to check the voxel list
	Voxel* voxelList = cpuVoxelListCapture(renderer->device, renderer->context);
	octreeSize = voxelCount * 50;
	Node* cpuOctree = CPUCreateOctree(voxelList);
	delete voxelList;
	initOctree(renderer->device, cpuOctree);
	delete cpuOctree;


	// initOctree(renderer->device, NULL);
	// createOctree(renderer);
	// Use breakpoint debug to check the octree
	// cpuOctreeCapture(renderer->device, renderer->context);
	// mipMapUpOctree(renderer);
}


SparseVoxelOctree::~SparseVoxelOctree()
{
	octree->Release();
	octreeUAV->Release();
	octreeSRV->Release();
	deleteVoxelList();
}


void SparseVoxelOctree::DrawVoxelDebug(DefferedRenderer * const renderer)
{
	renderer->context->OMSetRenderTargets(1, &renderer->backBufferRTV, renderer->depthStencilView);

	SimpleVertexShader*   vertexShader = renderer->GetVertexShader("voxelDebug");
	SimplePixelShader*    pixelShader = renderer->GetPixelShader("voxelDebug");
	vertexShader->SetShader();
	vertexShader->SetMatrix4x4("view", *renderer->camera->GetView());
	vertexShader->SetMatrix4x4("projection", *renderer->camera->GetProjection());
	vertexShader->SetFloat("voxelScale", worldWidth/float(voxelDim));
	// can't use simple shader for voxelSRV
	renderer->context->VSSetShaderResources(0, 1, &voxelListSRV);
	vertexShader->CopyAllBufferData();

	pixelShader->SetShader(); 
	pixelShader->CopyAllBufferData();

	UINT stride = sizeof(Vertex);
	UINT offset = 0;
	Mesh* meshTmp = renderer->GetMesh("cube");
	ID3D11Buffer* vertTemp = meshTmp->GetVertexBuffer();
	renderer->context->IASetVertexBuffers(0, 1, &vertTemp, &stride, &offset);
	renderer->context->IASetIndexBuffer(meshTmp->GetIndexBuffer(), DXGI_FORMAT_R32_UINT, 0);
	renderer->context->DrawIndexedInstanced(meshTmp->GetIndexCount(), voxelCount, 0, 0, 0);
}


void SparseVoxelOctree::DrawOctreeDebug(DefferedRenderer * const renderer)
{
	renderer->context->OMSetRenderTargets(1, &renderer->backBufferRTV, renderer->depthStencilView);

	SimpleVertexShader*   vertexShader = renderer->GetVertexShader("octreeDebug");
	SimplePixelShader*    pixelShader = renderer->GetPixelShader("octreeDebug");

	vertexShader->SetShader();
	vertexShader->SetMatrix4x4("view", *renderer->camera->GetView());
	vertexShader->SetMatrix4x4("projection", *renderer->camera->GetProjection());
	vertexShader->SetFloat("worldSize", worldWidth);
	vertexShader->SetInt("maxOctreeIndex", octreeSize);
	renderer->context->VSSetShaderResources(0, 1, &octreeSRV);
	vertexShader->CopyAllBufferData();

	pixelShader->SetShader();
	pixelShader->CopyAllBufferData();

	// Set Topology to render the cube outline
	renderer->context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINELIST);
	UINT stride = sizeof(Vertex);
	UINT offset = 0;
	Mesh* meshTmp = renderer->GetMesh("cubeOutline");
	ID3D11Buffer* vertTemp = meshTmp->GetVertexBuffer();
	renderer->context->IASetVertexBuffers(0, 1, &vertTemp, &stride, &offset);
	renderer->context->IASetIndexBuffer(meshTmp->GetIndexBuffer(), DXGI_FORMAT_R32_UINT, 0);
	renderer->context->DrawIndexedInstanced(meshTmp->GetIndexCount(), octreeSize, 0, 0, 0);

	// reset topology
	renderer->context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
}

void SparseVoxelOctree::initVoxelCounter(ID3D11Device* device)
{
	// Create VoxelList Counter
	D3D11_BUFFER_DESC bufDesc;
	memset(&bufDesc, 0, sizeof(bufDesc));
	bufDesc.Usage = D3D11_USAGE_DEFAULT;
	bufDesc.ByteWidth = sizeof(INT32) * 4; // must be multiple of 16
	bufDesc.BindFlags = D3D11_BIND_UNORDERED_ACCESS;
	bufDesc.CPUAccessFlags = 0;
	bufDesc.StructureByteStride = sizeof(INT32);
	bufDesc.MiscFlags = 0;

	HRESULT result = device->CreateBuffer(&bufDesc, NULL, &counter);
	assert(result == S_OK);

	D3D11_UNORDERED_ACCESS_VIEW_DESC uavDesc;
	memset(&uavDesc, 0, sizeof(uavDesc));
	uavDesc.Format = DXGI_FORMAT_R32_SINT;
	uavDesc.Buffer.FirstElement = 0;
	uavDesc.Buffer.NumElements = 4;
	uavDesc.Buffer.Flags = 0;
	uavDesc.ViewDimension = D3D11_UAV_DIMENSION_BUFFER;

	// TODO: Check the results and error handle
	result = device->CreateUnorderedAccessView(counter, &uavDesc, &counterUAV);
	assert(result == S_OK);
}


void SparseVoxelOctree::initVoxelList(ID3D11Device* device, int numElements)
{
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
}


void SparseVoxelOctree::initOctree(ID3D11Device* device, Node* initData)
{
	//Calculate the maximum possilbe node number
	// TODO:
	// INITIALLY I WANT TO MAKE THE BUFFER JUST LARGE ENOUGH TO WORK THEN THE INTENTION IS TO DO AN AVERAGE
	// OF VALUES IF THEIR FINAL NODE RESULTS IN A COLLISION BECAUSE OF SIZE RESTRICTION
	D3D11_BUFFER_DESC bufDesc;
	memset(&bufDesc, 0, sizeof(bufDesc));
	bufDesc.Usage = D3D11_USAGE_DEFAULT;
	bufDesc.ByteWidth = sizeof(Node) * octreeSize;
	bufDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_UNORDERED_ACCESS;
	bufDesc.CPUAccessFlags = 0;
	bufDesc.StructureByteStride = sizeof(Node);
	bufDesc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;


	D3D11_SUBRESOURCE_DATA data;
	data.pSysMem = initData;
	data.SysMemPitch = 0;      // has no meaning for this buffer
	data.SysMemSlicePitch = 0; // has no meaning for this buffer

	HRESULT result = device->CreateBuffer(&bufDesc, &data, &octree);
	assert(result == S_OK);

	D3D11_UNORDERED_ACCESS_VIEW_DESC uavDesc;
	memset(&uavDesc, 0, sizeof(uavDesc));
	uavDesc.Format = DXGI_FORMAT_UNKNOWN;
	uavDesc.Buffer.FirstElement = 0;
	uavDesc.Buffer.NumElements = octreeSize;
	uavDesc.Buffer.Flags = 0;
	uavDesc.ViewDimension = D3D11_UAV_DIMENSION_BUFFER;

	// TODO: Check the results and error handle
	result = device->CreateUnorderedAccessView(octree, &uavDesc, &octreeUAV);
	assert(result == S_OK);

	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
	memset(&srvDesc, 0, sizeof(srvDesc));
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_BUFFER;
	srvDesc.Format = DXGI_FORMAT_UNKNOWN;
	srvDesc.Buffer.ElementWidth = octreeSize;

	result = device->CreateShaderResourceView(octree, &srvDesc, &octreeSRV);
	assert(result == S_OK);
}


// D3D11_CONSERVATIVE_RASTERIZATION_MODE
void SparseVoxelOctree::voxelizeGeometry(DefferedRenderer* renderer, int mode)
{
	D3D11_VIEWPORT viewport = {};
	viewport.TopLeftX = 0;
	viewport.TopLeftY = 0;
	viewport.Width = (float)voxelDim;
	viewport.Height = (float)voxelDim;
	viewport.MinDepth = 0.0f;
	viewport.MaxDepth = 1.0f;
	renderer->context->RSSetViewports(1, &viewport);

	ID3D11RasterizerState *voxelRastState;
	D3D11_RASTERIZER_DESC voxelRastDesc = {};
	voxelRastDesc.FillMode = D3D11_FILL_SOLID;
	voxelRastDesc.CullMode = D3D11_CULL_NONE;
	voxelRastDesc.DepthClipEnable = false;
	renderer->device->CreateRasterizerState(&voxelRastDesc, &voxelRastState);

	// Setup Matricies
	MAT4X4 viewProjX;
	MAT4X4 viewProjY;
	MAT4X4 viewProjZ;

	// Changes where the eyepoint and the projection point is at
	XMFLOAT3 eye = XMFLOAT3(5, 0, 0);
	XMFLOAT3 focus = XMFLOAT3(0, 0, 0);
	XMFLOAT3 up = XMFLOAT3(0, 1, 0);
	// near - far with the values 1.0f and 3.0f put the range -1.0f, 1.0f in z axis
	// width and height are tied to the z voxel depth.
	// Anything outside of the width and height as world space is clipped.
	//   This is an 8x8x2 region so when scaling z back it must be mul by (64 / (8/2))
	//   The other axis are fine
	MATRIX Ortho = XMMatrixOrthographicLH(worldWidth, worldWidth, 1.0f, 3.0f);
	XMVECTOR Eye = XMLoadFloat3(&eye);
	XMVECTOR Focus = XMLoadFloat3(&focus);
	XMVECTOR Up = XMLoadFloat3(&up);
	MATRIX ViewProj = XMMatrixLookAtLH(Eye, Focus, Up) * Ortho;
	// save X transpose
	DirectX::XMStoreFloat4x4(&viewProjX, DirectX::XMMatrixTranspose(ViewProj));

	eye = XMFLOAT3(0, 5, 0);
	focus = XMFLOAT3(0, 0, 0);
	up = XMFLOAT3(0, 0, -1);
	Eye = XMLoadFloat3(&eye);
	Focus = XMLoadFloat3(&focus);
	Up = XMLoadFloat3(&up);
	ViewProj = XMMatrixLookAtLH(Eye, Focus, Up) * Ortho; 
	// save Y transpose
	DirectX::XMStoreFloat4x4(&viewProjY, DirectX::XMMatrixTranspose(ViewProj));

	eye = XMFLOAT3(0, 0, 5);
	focus = XMFLOAT3(0, 0, 0);
	up = XMFLOAT3(0, 1, 0);
	Eye = XMLoadFloat3(&eye);
	Focus = XMLoadFloat3(&focus);
	Up = XMLoadFloat3(&up);
	ViewProj = XMMatrixLookAtLH(Eye, Focus, Up) * Ortho;
	// save Z transpose
	DirectX::XMStoreFloat4x4(&viewProjZ, DirectX::XMMatrixTranspose(ViewProj));

	// Render Geometry
	renderer->context->RSSetState(voxelRastState);
	ID3D11UnorderedAccessView* UAViews[2] = {counterUAV, voxelListUAV};
	const UINT * temp = (const UINT *)0;
	renderer->context->OMSetRenderTargetsAndUnorderedAccessViews(1, &renderer->backBufferRTV, 0, 1, 2, UAViews, temp);

	SimpleVertexShader*   vertexShader = renderer->GetVertexShader("voxelList");
	SimpleGeometryShader* geomShader = renderer->GetGeometryShader("voxelList");
	SimplePixelShader*    pixelShader = renderer->GetPixelShader("voxelList");
	vertexShader->SetShader();
	
	geomShader->SetShader();
	geomShader->SetMatrix4x4("ViewProjX", viewProjX);
	geomShader->SetMatrix4x4("ViewProjY", viewProjY);
	geomShader->SetMatrix4x4("ViewProjZ", viewProjZ);
	geomShader->SetInt("height", voxelDim);
	geomShader->SetInt("width", voxelDim);
	geomShader->CopyAllBufferData();
	
	pixelShader->SetShader();
	pixelShader->SetFloat("voxelWidth", voxelDim);
	pixelShader->SetFloat("worldWidth", worldWidth);
	pixelShader->SetInt("store", mode); // 0 to count 1 to store
	

	UINT stride = sizeof(Vertex);
	UINT offset = 0;
	Mesh* meshTmp;
	std::vector<GameEntity*> staticObjects = *renderer->GetStaticObjects();
	for (int i = 0; i < staticObjects.size(); i++)
	{
		PBRMaterial* material = renderer->GetMaterial(staticObjects.at(i)->GetMaterial());
		// Send texture Info
		pixelShader->SetSamplerState("basicSampler", material->GetSamplerState());
		pixelShader->SetShaderResourceView("albedoMap", material->GetAlbedoSRV());
		// pixelShader->SetShaderResourceView("normalMap", material->GetNormalSRV());
		// pixelShader->SetShaderResourceView("metalMap", material->GetMetallicSRV());
		// pixelShader->SetShaderResourceView("roughMap", material->GetRoughnessSRV());
		// pixelShader->SetFloat("metallic", material->GetMetallicParam());
		// pixelShader->SetFloat("roughness", material->GetRoughnessParam());
		pixelShader->CopyAllBufferData();

		// Send Geometry
		vertexShader->SetMatrix4x4("World", *staticObjects.at(i)->GetWorld());
		vertexShader->CopyAllBufferData();
		
		meshTmp = renderer->GetMesh(staticObjects.at(i)->GetMesh());
		ID3D11Buffer* vertTemp = meshTmp->GetVertexBuffer();
		renderer->context->IASetVertexBuffers(0, 1, &vertTemp, &stride, &offset);
		renderer->context->IASetIndexBuffer(meshTmp->GetIndexBuffer(), DXGI_FORMAT_R32_UINT, 0);
		renderer->context->DrawIndexed(meshTmp->GetIndexCount(), 0, 0);
	}

	// RESET STATES
	voxelRastState->Release();
	renderer->context->GSSetShader(0, 0, 0); // unset geometry shader
	renderer->context->RSSetState(0); // reset state
	renderer->context->OMSetRenderTargetsAndUnorderedAccessViews(0, 0, 0, 0, 0, 0, 0);
}


void SparseVoxelOctree::deleteVoxelList()
{
	counter->Release();
	counterUAV->Release();
	voxelListBuffer->Release();
	voxelListUAV->Release();
	voxelListSRV->Release();
}


void SparseVoxelOctree::createOctree(DefferedRenderer* renderer)
{
	// Allocate Octree
	SimpleComputeShader* computeShader = renderer->GetComputeShader("constructSVO");
	int squareDim = ceil(sqrt(voxelCount));
	computeShader->SetInt("numThreadRows", squareDim);
	computeShader->SetInt("MaxVoxelIndex", voxelCount);
	computeShader->SetInt("MaxOctreeDepth", maxOctreeDepth);
	// set SRV and UAV simple shader can not do these propperly
	renderer->context->CSSetShaderResources(0, 1, &voxelListSRV);
	renderer->context->CSGetUnorderedAccessViews(1, 1, &octreeUAV);
	computeShader->SetUnorderedAccessView("octree", octreeUAV);
	computeShader->CopyAllBufferData();
	computeShader->DispatchByThreads(squareDim, squareDim, 1);

	// Unbind the UAV and SRV
	computeShader->SetShaderResourceView("voxelList", 0);
	computeShader->SetUnorderedAccessView("octree", 0);

	// Save Values of the voxel list
	computeShader = renderer->GetComputeShader("storeSVO");
	computeShader->SetInt("numThreadRows", squareDim);
	computeShader->SetInt("MaxVoxelIndex", voxelCount);
	computeShader->SetInt("MaxOctreeDepth", maxOctreeDepth);
	// set SRV and UAV simple shader can not do these propperly
	renderer->context->CSSetShaderResources(0, 1, &voxelListSRV);
	renderer->context->CSGetUnorderedAccessViews(1, 1, &octreeUAV);
	computeShader->SetUnorderedAccessView("octree", octreeUAV);
	computeShader->CopyAllBufferData();
	computeShader->DispatchByThreads(squareDim, squareDim, 1);

	// Unbind the UAV and SRV
	computeShader->SetShaderResourceView("voxelList", 0);
	computeShader->SetUnorderedAccessView("octree", 0);
}


void SparseVoxelOctree::mipMapUpOctree(DefferedRenderer* renderer)
{
	// Set 8 compute shader values to traverse down the octree for their respective levels
	// averaging as they move back up.
	// IS IT POSSIBLE TO MAKE RECURSIVE COMPUTE SHADER CALLS?
	// SimpleComputeShader* computeShader = renderer->GetComputeShader("mipMapSVO");
}


/// http://web.eecs.utk.edu/~smarz1/projects/dc5.0/
int SparseVoxelOctree::getCount(ID3D11Device* device, ID3D11DeviceContext* context)
{
	// Make a staging buffer for copying
	D3D11_BUFFER_DESC stagingDesc;
	memset(&stagingDesc, 0, sizeof(stagingDesc));
	stagingDesc.Usage = D3D11_USAGE_STAGING;
	stagingDesc.ByteWidth = sizeof(INT32) * 4; // must be multiple of 16
	stagingDesc.BindFlags = 0;
	stagingDesc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
	stagingDesc.StructureByteStride = sizeof(INT32);
	stagingDesc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;

	ID3D11Buffer* stagingBuffer;
	device->CreateBuffer(&stagingDesc, 0, &stagingBuffer);

	// Copy the final data to the staging buffer
	context->CopyResource(stagingBuffer, counter);
	context->Flush();

	// Map for reading
	D3D11_MAPPED_SUBRESOURCE mapped;
	HRESULT hr = context->Map(stagingBuffer, 0, D3D11_MAP_READ, 0, &mapped);

	// Copy data and unmap
	INT32 finalCount[4];
	memcpy(finalCount, mapped.pData, sizeof(INT32)*4);
	context->Unmap(stagingBuffer, 0);
	stagingBuffer->Release();

	return finalCount[0];
}


Voxel* SparseVoxelOctree::cpuVoxelListCapture(ID3D11Device* device, ID3D11DeviceContext* context)
{
	D3D11_BUFFER_DESC stagingDesc;
	memset(&stagingDesc, 0, sizeof(stagingDesc));
	stagingDesc.Usage = D3D11_USAGE_STAGING;
	stagingDesc.ByteWidth = sizeof(Voxel) * voxelCount;
	stagingDesc.BindFlags = 0;
	stagingDesc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
	stagingDesc.StructureByteStride = sizeof(Voxel);
	stagingDesc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;

	ID3D11Buffer* stagingBuffer;
	HRESULT result = device->CreateBuffer(&stagingDesc, 0, &stagingBuffer);
	context->CopyResource(stagingBuffer, voxelListBuffer);
	context->Flush();

	// Map for reading
	D3D11_MAPPED_SUBRESOURCE mapped;
	HRESULT hr = context->Map(stagingBuffer, 0, D3D11_MAP_READ, 0, &mapped);

	Voxel* finalVoxelList = new Voxel[voxelCount];
	memcpy(finalVoxelList, mapped.pData, sizeof(Voxel) * voxelCount);
	context->Unmap(stagingBuffer, 0);
	stagingBuffer->Release();
	return finalVoxelList;
}


void SparseVoxelOctree::cpuOctreeCapture(ID3D11Device* device, ID3D11DeviceContext* context)
{
	D3D11_BUFFER_DESC stagingDesc;
	memset(&stagingDesc, 0, sizeof(stagingDesc));
	stagingDesc.Usage = D3D11_USAGE_STAGING;
	stagingDesc.ByteWidth = sizeof(Node) * octreeSize;
	stagingDesc.BindFlags = 0;
	stagingDesc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
	stagingDesc.StructureByteStride = sizeof(Node);
	stagingDesc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;

	ID3D11Buffer* stagingBuffer;
	HRESULT result = device->CreateBuffer(&stagingDesc, 0, &stagingBuffer);
	context->CopyResource(stagingBuffer, octree);
	context->Flush();

	// Map for reading
	D3D11_MAPPED_SUBRESOURCE mapped;
	HRESULT hr = context->Map(stagingBuffer, 0, D3D11_MAP_READ, 0, &mapped);

	Node finalOctree[1280];
	memcpy(finalOctree, mapped.pData, sizeof(Node) * 1000);
	context->Unmap(stagingBuffer, 0);
	stagingBuffer->Release();
}


VEC4 GetOctaveIndex(VEC3 pos)
{
	if (pos.x > 0)
	{
		if (pos.y > 0)
		{
			if (pos.z > 0)
			{
				return VEC4(0, 1, 1, 1);
			}
			else
			{
				return VEC4(3, 1, 1, -1);
			}
		}
		else
		{
			if (pos.z > 0)
			{
				return VEC4(4, 1, -1, 1);
			}
			else
			{
				return VEC4(7, 1, -1, -1);
			}
		}
	}
	else
	{
		if (pos.y > 0)
		{
			if (pos.z > 0)
			{
				return VEC4(1, -1, 1, 1);
			}
			else
			{
				return VEC4(2, -1, 1, -1);
			}
		}
		else
		{
			if (pos.z > 0)
			{
				return VEC4(5, -1, -1, 1);
			}
			else
			{
				return VEC4(6, -1, -1, -1);
			}
		}
	}
}


Node* SparseVoxelOctree::CPUCreateOctree(Voxel* voxelList)
{
	Node* cpuOctree = new Node[octreeSize];
	for (int i = 0; i < octreeSize; i++)
	{
		cpuOctree[i] = Node();
		cpuOctree[i].childPointer = -1;
		cpuOctree[i].flagBits = -1;
	}

	cpuOctree[0].padding = 0;
	for (int i = 0; i < voxelCount; i++)
	{
		Voxel voxel = voxelList[i];
		Voxel curVoxel = voxelList[i];
		// Go to position in octree node chunk
		int currLevel = 0;
		int currOctreeIndex;
		float curVoxelWidth = worldWidth / 2.0f; // start at the top level
		// the 0-7 index offset and the offset to move by if needing to traverse
		VEC4 octaveIndex = GetOctaveIndex(curVoxel.position);
		currOctreeIndex = octaveIndex.x;
		// the extra divide by 2.0f to center the octave
		VEC3 curOctavePos = VEC3(octaveIndex.y * curVoxelWidth / 2.0f, octaveIndex.z * curVoxelWidth / 2.0f, octaveIndex.w * curVoxelWidth / 2.0f);
		for (currLevel = 0; currLevel < maxOctreeDepth; currLevel++)
		{
			// ALLOCATE AND follow pointer to next octree level
			if (cpuOctree[currOctreeIndex].flagBits == -1)
			{
				cpuOctree[currOctreeIndex].position = curOctavePos;
				cpuOctree[currOctreeIndex].flagBits = currLevel+1;
				// move the pointer to allocate a child since this is no longer a leaf
				cpuOctree[0].padding += 8;
				cpuOctree[currOctreeIndex].childPointer = cpuOctree[0].padding;
			}
			currOctreeIndex = cpuOctree[currOctreeIndex].childPointer;
			// get to new position by moving then check again
			curVoxelWidth /= 2.0f;
			curVoxel.position.x -= octaveIndex.y * curVoxelWidth;
			curVoxel.position.y -= octaveIndex.z * curVoxelWidth;
			curVoxel.position.z -= octaveIndex.w * curVoxelWidth;
			curOctavePos.x -= octaveIndex.y * curVoxelWidth;
			curOctavePos.y -= octaveIndex.z * curVoxelWidth;
			curOctavePos.z -= octaveIndex.w * curVoxelWidth;

			octaveIndex = GetOctaveIndex(curVoxel.position);
			currOctreeIndex += octaveIndex.x;
		}
		// store
		cpuOctree[currOctreeIndex].position = voxelList[i].position; // might not need position;
		cpuOctree[currOctreeIndex].normal = voxelList[i].normal;
		cpuOctree[currOctreeIndex].color = voxelList[i].color;
		cpuOctree[currOctreeIndex].childPointer = 0;
		cpuOctree[currOctreeIndex].flagBits = maxOctreeDepth-1; //////////////////////////////////////////////////////////////////////////////////////////////////////
	}

	printf("Number of allocations %d\n", cpuOctree[0].padding);
	return cpuOctree;
}
