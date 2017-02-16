#pragma once

#include "GameMath.h"

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

	void RotateXY(FLOAT x, FLOAT y);

private:	
	///<summary>
	/// This is a LOOK TO view matrix.
	///</summary>
	MAT4X4 view;
	MAT4X4 invView;

	VEC3 position;
	VEC3 direction;
	FLOAT zFar;
	FLOAT zNear = 0.0f;
	FLOAT xRot;
	FLOAT yRot;

	MAT4X4 projection;
	MAT4X4 invProjection;


	FLOAT speed = 5;

};
