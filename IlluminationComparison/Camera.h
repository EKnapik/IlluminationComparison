#pragma once

#include "GameMath.h"

class Camera {
public:
	Camera(int width, int height);
	~Camera();

	void Update(FLOAT dt);

	MAT4X4* GetView();
	MAT4X4* GetProjection();
	VEC3* GetPosition() { return &position; }
	VEC3* GetDirection() { return &direction; }

	void RotateXY(FLOAT x, FLOAT y);

private:	
	///<summary>
	/// This is a LOOK TO view matrix.
	///</summary>
	MAT4X4 view;

	VEC3 position;
	VEC3 direction;
	FLOAT xRot;
	FLOAT yRot;

	MAT4X4 projection;


	FLOAT speed = 5;

};
