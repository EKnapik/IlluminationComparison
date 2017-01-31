#include "Camera.h"
#include <Windows.h>

using namespace GMath;

Camera::Camera(int width, int height)
{
	SetVector3(&position, 0, 1, -10);
	SetVector3(&direction, 0, 0, 1);
	xRot = 0;
	yRot = 0;
	zFar = 100.0f;

	SetProjectionMatrix(&projection, 0.25f * 3.1415926535f, (float)width / height, 0.1f, zFar);
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
	VEC3 forward(0, 0, 1);
	VEC3 up(0, 1, 0);
	VEC3 rot(xRot, yRot, 0);
	MATRIX rotation = CreateRotationMatrix(&rot);
	VECTOR look = Vector3Transform(&forward, &rotation);
	StoreVector(&direction, &look);
	SetMatrixLookTo(&view, &position, &look, &up);
	SetInverseMatrix(&invView, &view);
#ifdef WITH_DX
	SetTransposeMatrix(&view, &GetMatrix(&view));
	SetTransposeMatrix(&invView, &GetMatrix(&invView));
#endif // WITH_DX
	
	if (GetAsyncKeyState(VK_LSHIFT) & 0x8000)
	{
		if (GetAsyncKeyState('W') & 0x8000)
		{
			AddVec3(&position, &position, VectorScale(&Vec3Normalize(&look), dt * speed));
		}
		else if (GetAsyncKeyState('S') & 0x8000)
		{
			AddVec3(&position, &position, VectorScale(&Vec3Normalize(&look), dt * -speed));
		}

		if (GetAsyncKeyState('A') & 0x8000)
		{
			AddVec3(&position, &position, VectorScale(&Vec3Normalize(&VectorCross(&look, &GetVector(&up))), dt * -speed));
		}
		else if (GetAsyncKeyState('D') & 0x8000)
		{
			AddVec3(&position, &position, VectorScale(&Vec3Normalize(&VectorCross(&GetVector(&up), &look)), dt * speed));
		}

		if (GetAsyncKeyState(' ') & 0x8000)
		{
			AddVec3(&position, &position, VectorScale(&GetVector(&up), dt* speed));
		}

		else if (GetAsyncKeyState('X') & 0x8000)
		{
			AddVec3(&position, &position, VectorScale(&GetVector(&up), -dt* speed));
		}
	}
	else
	{
		if (GetAsyncKeyState('W') & 0x8000)
		{
			
			AddVec3(&position, &position, VectorScale(&Vec3Normalize(&GetVector(&VEC3(0, 0, 1))), dt * speed));
		}
		else if (GetAsyncKeyState('S') & 0x8000)
		{
			AddVec3(&position, &position, VectorScale(&Vec3Normalize(&GetVector(&VEC3(0, 0, 1))), dt * -speed));
		}

		if (GetAsyncKeyState('D') & 0x8000)
		{
			AddVec3(&position, &position, VectorScale(&Vec3Normalize(&GetVector(&VEC3(1, 0, 0))), dt * speed));
		}
		else if (GetAsyncKeyState('A') & 0x8000)
		{
			AddVec3(&position, &position, VectorScale(&Vec3Normalize(&GetVector(&VEC3(1, 0, 0))), dt * -speed));
		}

		if (GetAsyncKeyState('Z') & 0x8000)
		{
			AddVec3(&position, &position, VectorScale(&Vec3Normalize(&GetVector(&VEC3(0, 1, 0))), dt * speed));
		}
		else if (GetAsyncKeyState('X') & 0x8000)
		{
			AddVec3(&position, &position, VectorScale(&Vec3Normalize(&GetVector(&VEC3(0, 1, 0))), dt * -speed));
		}
	}
}

MAT4X4 * Camera::GetView()
{
	return &view;
}

MAT4X4 * Camera::GetInvView()
{
	return &invView;
}

MAT4X4 * Camera::GetProjection()
{
	return &projection;
}

MAT4X4 * Camera::GetInvProjection()
{
	return &invProjection;
}

void Camera::RotateXY(FLOAT x, FLOAT y)
{
	xRot += x;
	yRot += y;
}
