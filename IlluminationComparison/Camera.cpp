#include "Camera.h"
#include <Windows.h>
#include <stdio.h>
using namespace GMath;
using namespace DirectX;

Camera::Camera(int width, int height)
{
	SetVector3(&position, 0, 1, -10);
	xRot = 0;
	yRot = 0;
	zFar = 100.0f;
	UpdateDirection();

	SetProjectionMatrix(&projection, 0.25f * 3.1415926535f, (float)width / height, zNear, zFar);
	SetInverseMatrix(&invProjection, &projection);
#ifdef WITH_DX
	SetTransposeMatrix(&projection, &GetMatrix(&projection));
	SetTransposeMatrix(&invProjection, &GetMatrix(&invProjection));
#endif // WITH_DX
}

Camera::~Camera()
{
}

void Camera::Update(FLOAT dt)
{
	UpdateDirection();
	if (GetAsyncKeyState('W') & 0x8000)
	{
		Forward(dt * speed);
	}
	else if (GetAsyncKeyState('S') & 0x8000)
	{
		Backward(dt * speed);
	}

	if (GetAsyncKeyState('D') & 0x8000)
	{
		StrafeRight(dt * speed);
	}
	else if (GetAsyncKeyState('A') & 0x8000)
	{
		StrafeLeft(dt * speed);
	}

	if (GetAsyncKeyState('E') & 0x8000)
	{
		MoveUp(dt * speed);
	}
	else if (GetAsyncKeyState('Q') & 0x8000)
	{
		MoveDown(dt * speed);
	}
}

MAT4X4 * Camera::GetView()
{
	return &view;
}

MAT4X4 * Camera::GetInvViewProj()
{
	return &invViewProj;
}

MAT4X4 * Camera::GetProjection()
{
	return &projection;
}

MAT4X4 * Camera::GetInvProjection()
{
	return &invProjection;
}

VEC2 Camera::GetProjectionConsts()
{
	float projA = zFar / (zFar - zNear);
	float projB = (-zFar * zNear) / (zFar - zNear);
	return VEC2(projA, projB);
}

// updates the current camera direction
// normaized direction of the camera
void Camera::UpdateDirection() {
	XMFLOAT3 dir = CAM_DIR;
	XMVECTOR curDir = XMLoadFloat3(&dir);
	XMMATRIX rotMat = XMMatrixRotationRollPitchYaw(xRot, yRot, 0);
	curDir = XMVector3Normalize(XMVector3Transform(curDir, rotMat));
	XMStoreFloat3(&direction, curDir);

	UpdateViewMat();
}


// could use the GetDir function but I would be creating
// the rotation matrix twice, once for up vector and once for direction
//
// Returns a column ordering view matrix of this camera
void Camera::UpdateViewMat() {
	XMFLOAT3 up = CAM_UP;
	XMVECTOR curPos = XMLoadFloat3(&position);
	XMVECTOR curDir = XMLoadFloat3(&direction);
	XMVECTOR curUp = XMLoadFloat3(&up);
	XMMATRIX rotMat = XMMatrixRotationRollPitchYaw(xRot, yRot, 0);
	curUp = XMVector3Transform(curUp, rotMat);

	// saves the transposed into column ordering view matrix
	XMMATRIX viewMat = XMMatrixLookToLH(curPos, curDir, curUp);
	XMMATRIX projMat = XMLoadFloat4x4(&projection);
	projMat = XMMatrixTranspose(projMat); // untranspose the matrix
	XMStoreFloat4x4(&view, viewMat);
	XMMATRIX viewProj = viewMat * projMat;
	SetInverseMatrix(&invViewProj, &viewProj); // Also update the inverse view matrix
	// Tranpose both
	SetTransposeMatrix(&view, &GetMatrix(&view));
	SetTransposeMatrix(&invViewProj, &GetMatrix(&invViewProj));
}

void Camera::Forward(float amount) {
	position.x = position.x + (direction.x*amount);
	position.y = position.y + (direction.y*amount);
	position.z = position.z + (direction.z*amount);
}

void Camera::Backward(float amount) {
	Forward(-amount);
}


void Camera::StrafeRight(float amount) {
	XMFLOAT3 right;
	XMFLOAT3 up = CAM_UP;
	XMVECTOR curRight;
	XMVECTOR curDir = XMLoadFloat3(&direction);
	XMVECTOR curUp = XMLoadFloat3(&up);
	XMMATRIX rotMat = XMMatrixRotationRollPitchYaw(xRot, yRot, 0);
	curUp = XMVector3Transform(curUp, rotMat);

	curRight = XMVector3Normalize(XMVector3Cross(curUp, curDir));
	curRight = curRight * amount;
	XMStoreFloat3(&right, curRight);

	position.x = position.x + right.x;
	position.y = position.y + right.y;
	position.z = position.z + right.z;
}


void Camera::StrafeLeft(float amount) {
	StrafeRight(-amount);
}

void Camera::ResetCamera() {
	position = XMFLOAT3(0, 0, -5);
	xRot = 0;
	yRot = 0;
}
