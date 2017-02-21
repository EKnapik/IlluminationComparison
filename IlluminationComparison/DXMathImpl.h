#pragma once

#include <DirectXMath.h>

#define PI 3.141593f

//Primitives
typedef float FLOAT;
typedef double DOUBLE;
typedef int INT;
typedef long LONG;

// Matrices
typedef DirectX::XMFLOAT3X3 MAT3X3;
typedef DirectX::XMFLOAT4X3 MAT4X3;
typedef DirectX::XMFLOAT4X4 MAT4X4;

//Vectors
typedef DirectX::XMFLOAT2 VEC2;
typedef DirectX::XMFLOAT3 VEC3;
typedef DirectX::XMFLOAT4 VEC4;

//SIMD types
typedef DirectX::XMVECTOR VECTOR;
typedef DirectX::XMMATRIX MATRIX;

namespace GMath
{
	inline MATRIX CreateTranslationMatrix(VEC3* vec)
	{
		return DirectX::XMMatrixTranslation(vec->x, vec->y, vec->z);
	}

	inline MATRIX CreateRotationMatrix(VEC3* vec)
	{
		return DirectX::XMMatrixRotationRollPitchYaw(vec->x, vec->y, vec->z);
	}

	inline MATRIX CreateScaleMatrix(VEC3* vec)
	{
		return DirectX::XMMatrixScaling(vec->x, vec->y, vec->z);
	}

	inline void SetIdentity4X4(MAT4X4* mat)
	{
		DirectX::XMStoreFloat4x4(mat, DirectX::XMMatrixIdentity());
	};

	inline void SetMatrix(MAT4X4* mat, MATRIX* matToSet)
	{
		DirectX::XMStoreFloat4x4(mat, *matToSet);
	};

	inline void SetTransposeMatrix(MAT4X4* mat, MATRIX* matToSet)
	{
		DirectX::XMStoreFloat4x4(mat, DirectX::XMMatrixTranspose(*matToSet));
	};

	inline MATRIX GetMatrix(MAT4X4* mat)
	{
		return DirectX::XMLoadFloat4x4(mat);
	};

	inline VECTOR GetVector(VEC2* vec)
	{
		return DirectX::XMLoadFloat2(vec);
	};

	inline VECTOR GetVector(VEC3* vec)
	{
		return DirectX::XMLoadFloat3(vec);
	};

	inline VECTOR GetVector(VEC4* vec)
	{
		return DirectX::XMLoadFloat4(vec);
	};


	inline MATRIX Mult(MATRIX* mat, VEC3* vec)
	{
		return DirectX::XMMatrixMultiply(*mat, DirectX::XMMatrixTranslation(vec->x, vec->y, vec->z));
	}

	inline VECTOR VectorScale(VECTOR* vec, FLOAT scale)
	{
		return DirectX::XMVectorScale(*vec, scale);
	}

	inline void VectorScale(VEC3* vec, FLOAT scale)
	{
		DirectX::XMStoreFloat3(vec, DirectX::XMVectorScale(GetVector(vec), scale));
	}

	inline void VectorScale(VEC3* scaled, VEC3* original, FLOAT scale)
	{
		DirectX::XMStoreFloat3(scaled, DirectX::XMVectorScale(GetVector(original), scale));
	}

	inline MATRIX Mult(MAT4X4* mat, VEC3* vec)
	{
		DirectX::XMStoreFloat4x4(mat, Mult(&GetMatrix(mat), vec));
	}

	inline void SetVector3(VEC3* vec, FLOAT x, FLOAT y, FLOAT z)
	{
		vec->x = x;
		vec->y = y;
		vec->z = z;
	};

	inline VECTOR Vector3Transform(VEC3* vec1, MATRIX* mat)
	{
		VECTOR vec = GetVector(vec1);
		return DirectX::XMVector3Transform(vec, *mat);
	}

	inline void ZeroVec3(VEC3* vec)
	{
		SetVector3(vec, 0, 0, 0);
	};

	inline void StoreVector(VEC3* location, VECTOR* vec)
	{
		DirectX::XMStoreFloat3(location, *vec);
	}

	inline VECTOR AddVector(VECTOR* vec1, VECTOR* vec2)
	{
		return DirectX::XMVectorAdd(*vec1, *vec2);
	};

	inline void AddVector(VECTOR* sum, VECTOR* vec1, VECTOR* vec2)
	{
		*sum = DirectX::XMVectorAdd(*vec1, *vec2);
	};

	inline void AddVec3(VEC3* sum, VEC3* vec1, VEC3* vec2)
	{
		DirectX::XMStoreFloat3(sum, AddVector(&GetVector(vec1), &GetVector(vec2)));
	};

	inline void AddVec3(VEC3* sum, VEC3* vec1, VECTOR vec)
	{
		DirectX::XMStoreFloat3(sum, AddVector(&GetVector(vec1), &vec));
	};

	inline VECTOR DotVector2(VECTOR* vec1, VECTOR* vec2)
	{
		return DirectX::XMVector2Dot(*vec1, *vec2);
	};

	inline VECTOR DotVector2(VEC2* vec1, VEC2* vec2)
	{
		return DirectX::XMVector2Dot(GetVector(vec1), GetVector(vec2));
	};

	inline void DotVector2(FLOAT* sum, VEC2* vec1, VEC2* vec2)
	{
		DirectX::XMStoreFloat(sum, DotVector2(vec1, vec2));
	};

	inline VECTOR DotVector3(VECTOR* vec1, VECTOR* vec2)
	{
		return DirectX::XMVector3Dot(*vec1, *vec2);
	};

	inline VECTOR DotVector3(VEC3* vec1, VEC3* vec2)
	{
		return DirectX::XMVector3Dot(GetVector(vec1), GetVector(vec2));
	};

	inline void DotVector3(FLOAT* sum, VEC3* vec1, VEC3* vec2)
	{
		DirectX::XMStoreFloat(sum, DotVector3(vec1, vec2));
	};

	inline VECTOR VectorCross(VECTOR* vec1, VECTOR* vec2)
	{
		return DirectX::XMVector3Cross(*vec1, *vec2);
	}

	inline VECTOR Vec3Normalize(VECTOR* vec)
	{
		return DirectX::XMVector3Normalize(*vec);
	}

	inline void SetMatrixLookTo(MAT4X4* toSet, VEC3* pos, VECTOR* look, VEC3* up)
	{
		SetMatrix(toSet, &(DirectX::XMMatrixLookToLH(GetVector(pos), *look, GetVector(up))));
	}

	inline void SetProjectionMatrix(MAT4X4* mat, FLOAT fov, FLOAT aspectRatio, FLOAT nearClip, FLOAT farClip)
	{
		// Create the Projection matrix
		// - This should match the window's aspect ratio, and also update anytime
		//   the window resizes (which is already happening in OnResize() below)
		
		XMStoreFloat4x4(mat, DirectX::XMMatrixPerspectiveFovLH(
			fov,		// Field of View Angle
			aspectRatio,		// Aspect ratio
			nearClip,						// Near clip plane distance
			farClip));					// Far clip plane distance); // Transpose for HLSL!
	}

	inline void SetInverseMatrix(MAT4X4* mat,  MAT4X4* matToInv)
	{
		MATRIX matrix = DirectX::XMLoadFloat4x4(matToInv);
		VECTOR vecNonUse;
		XMStoreFloat4x4(mat, XMMatrixInverse(&vecNonUse, matrix));
	}

	inline void SetInverseMatrix(MAT4X4* mat, MATRIX* matrix)
	{
		VECTOR vecNonUse;
		XMStoreFloat4x4(mat, XMMatrixInverse(&vecNonUse, *matrix));
	}

	inline void GetMagnitude(FLOAT* length, VEC3* vec)
	{
		DirectX::XMStoreFloat(length, DirectX::XMVector3Length(GetVector(vec)));
	}

	inline void SetMatrixLookAtLH(MAT4X4* toSet, VEC4* eye, VEC4* focus, VEC4* up)
	{
		MATRIX view = DirectX::XMMatrixLookAtLH(
			GetVector(eye),
			GetVector(focus),
			GetVector(up));
		DirectX::XMStoreFloat4x4(toSet, DirectX::XMMatrixTranspose(view));
	}

	/*inline void SetOrthographicLH()
	{

	}*/
}