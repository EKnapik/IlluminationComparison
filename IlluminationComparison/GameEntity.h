#pragma once

#include "GameMath.h"
#include "Mesh.h"
#include "Material.h"
#include "Lights.h"

class GameEntity 
{
public:
	///<summary>
	/// Creates a game entity with the desired mesh.
	///</summary>
	GameEntity(std::string mesh, std::string material);

	///<summary>
	/// Sets the world Matrix to a new value
	///</summary>
	void SetWorld(MAT4X4* mat) { world = *mat; };

	///<summary>
	/// Gets the world Matrix to a new value
	///</summary>
	MAT4X4* GetWorld();

	///<summary>
	/// Modifies the position by the passed vector.
	///</summary>
	void Translate(VEC3 offset) { GMath::AddVec3(&position, &position, &offset); };

	///<summary>
	/// Sets the position to a new value.
	///</summary>
	void SetPosition(VEC3 vec) { position = vec; };

	///<summary>
	/// Modifies rotation by the passed vector.
	///</summary>
	void Rotate(VEC3 offset) { GMath::AddVec3(&rotation, &rotation, &offset); };

	///<summary>
	/// Sets the rotation to a new value.
	///</summary>
	void SetRotation(VEC3 vec) { rotation = vec; };

	///<summary>
	/// Sets the rotation to a new value.
	///</summary>
	void Scale(VEC3 offset) { GMath::AddVec3(&scale, &scale, &offset); };

	///<summary>
	/// Sets the scale to a new value.
	///</summary>
	void SetScale(VEC3 vec) { scale = vec; };

	void SetMesh(std::string mesh) { this->mesh = mesh; }
	std::string GetMesh() { return mesh; }

	void SetMaterial(std::string material) { this->material = material; }
	std::string GetMaterial() { return material; }

	void AddReference() { references++; }

	void Release();

	VEC3 GetPosition() { return position; };
	VEC3 GetRotation() { return rotation; };
	VEC3 GetScale() { return scale; };

	std::string pixelShader = "default";
	std::string vertexShader = "default";
private:
	///<summary>
	/// Deletes the game entity and all relevant assets.
	///</summary>
	~GameEntity();

	///<summary>
	/// The mesh for this object. Represented as a string. The renderer will 
	/// assign the proper mesh.
	///</summary>
	std::string mesh = "cube";

	///<summary>
	/// The material for this object. Represented as a string. The renderer will 
	/// assign the proper mesh.
	///</summary>
	std::string material = "default";

	///<summary>
	/// The world matrix of the object. Calculated at the end of the frame.
	///</summary>
	MAT4X4 world;

	///<summary>
	/// The position vector for this object.
	///</summary>
	VEC3 position;

	///<summary>
	/// The rotation vector for this object.
	///</summary>
	VEC3 rotation;

	///<summary>
	/// The scale vector for this object.
	///</summary>
	VEC3 scale;

	int references = 0;
};