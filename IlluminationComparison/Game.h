#pragma once

#include <vector>
#include "GameMath.h"
#include <map>
#include <string>

#ifdef WITH_DX
#include "DXCore.h"
#include "SimpleShader.h"
#endif // WITH_DX
#include "GameEntity.h"
#include "Camera.h"

#include "Lights.h"

#include "GameManager.h"
#include "Renderer.h"

#include "ParticleEmitter.h"

enum LEVEL_STATE {
	NO_TRANSITION,
	START,
	MAIN,
	WIN,
	LOSE,
	QUIT
};

enum POST_PROCESS {
	BLUR,
	EDGEDETECT,
	BLOOM,
	EMBOSS,
	BLURK,
	SHARPNESS,
	BOTTOMSOBEL,
	ASCII,
	NONE,
	NO_CHANGE
};

class Game 
	: public DXCore
{

public:
	Game(HINSTANCE hInstance);
	~Game();

	// Overridden setup and game loop methods, which
	// will be called automatically
	void Init();
	void OnResize();
	void Update(float deltaTime, float totalTime);
	void Draw(float deltaTime, float totalTime);

	// Overridden mouse input helper methods
	void OnMouseDown (WPARAM buttonState, int x, int y);
	void OnMouseUp	 (WPARAM buttonState, int x, int y);
	void OnMouseMove (WPARAM buttonState, int x, int y);
	void OnMouseWheel(float wheelDelta,   int x, int y);

	static Camera* GetCamera();
	static int levelstate;
	static int postProcessState;
private:
	static Camera* cameraPointer;
	void SetCamera(Camera* camera) { Game::cameraPointer = camera; }
	Camera* camera;
	bool mouseDown = false;

	// Initialization helper methods - feel free to customize, combine, etc.
	void LoadShaders(); 
	void LoadMeshes();
	void LoadMaterials();

	std::vector<GameEntity*> entities;

	GameManager gameManager;
	Renderer* renderer;

	// Keeps track of the old mouse position.  Useful for 
	// determining how far the mouse moved in a single frame.
	POINT prevMousePos;

	DirectionalLight light;

	bool O_toggle = false;
	bool prevTab = false;
	int skyboxChoice = 0;
};

