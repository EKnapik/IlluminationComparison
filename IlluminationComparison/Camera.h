#pragma once
#include <DirectXMath.h>
#include "GameMath.h"

#define CAM_DIR XMFLOAT3(0, 0, 1)
#define CAM_UP XMFLOAT3(0, 1, 0)

class Camera {
public:
	Camera(int width, int height);
	~Camera();

	void Update(FLOAT dt);

	MAT4X4* GetView();
	MAT4X4* GetInvView();
	MAT4X4* GetProjection();
	MAT4X4* GetInvProjection();

	VEC3* GetPosition() { return &position; }
	VEC3* GetDirection() { return &direction; }
	FLOAT GetFarPlane() { return zFar; }
	VEC2  GetProjectionConsts();

	void Forward(float amount);
	void Backward(float amount);
	void StrafeRight(float amount);
	void StrafeLeft(float amount);
	void MoveUp(float amount) { position.y = position.y + amount; }
	void MoveDown(float amount) { MoveUp(-amount); }

	void AddXRot(float amount) { xRot = xRot + amount; }
	void SubXRot(float amount) { AddXRot(-amount); }
	void AddYRot(float amount) { yRot = yRot + amount; }
	void SubYRot(float amount) { AddYRot(-amount); }

	void ResetCamera();

private:	
	///<summary>
	/// This is a LOOK TO view matrix.
	///</summary>
	MAT4X4 view;
	MAT4X4 invView;

	VEC3 position;
	VEC3 direction;
	FLOAT zFar;
	FLOAT zNear = 0.1f;
	FLOAT xRot;
	FLOAT yRot;

	MAT4X4 projection;
	MAT4X4 invProjection;
	FLOAT speed = 5;

	void UpdateDirection();
	void UpdateViewMat();
};
