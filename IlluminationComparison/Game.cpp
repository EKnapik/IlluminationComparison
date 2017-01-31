#include "Game.h"
#include "Vertex.h"
#include "WICTextureLoader.h"
#include "BouncingBallScene.h"
#include "Start.h"
#include "DefferedRenderer.h"
#include "Win.h"

// For the DirectX Math library
using namespace DirectX;

Camera* Game::cameraPointer = nullptr;
int Game::levelstate = LEVEL_STATE::NO_TRANSITION;
int Game::postProcessState = POST_PROCESS::NONE;

// --------------------------------------------------------
// Constructor
//
// DXCore (base class) constructor will set up underlying fields.
// DirectX itself, and our window, are not ready yet!
//
// hInstance - the application's OS-level handle (unique ID)
// --------------------------------------------------------
Game::Game(HINSTANCE hInstance)
	: DXCore( 
		hInstance,						// The application's handle
		"Illumination Comparison",	   // Text for the window's title bar
		1280,					      // Width of the window's client area
		720,                         // Height of the window's client area
		true)			            // Show extra stats (fps) in title bar?
{
#if defined(DEBUG) || defined(_DEBUG)
	// Do we want a console window?  Probably only in debug mode
	CreateConsoleWindow(500, 120, 32, 120);
	printf("Console window created successfully.  Feel free to printf() here.");
#endif
}

// --------------------------------------------------------
// Destructor - Clean up anything our game has created:
//  - Release all DirectX objects created here
//  - Delete any objects to prevent memory leaks
// --------------------------------------------------------
Game::~Game()
{
	// Release any (and all!) DirectX objects
	// we've made in the Game class
	delete renderer;
	delete camera;
}

// --------------------------------------------------------
// Called once per program, after DirectX and the window
// are initialized but before the game loop.
// --------------------------------------------------------
void Game::Init()
{
	camera = new Camera(width, height);
	Game::SetCamera(camera);
	// TODO: Renderer should only take the context and then create the buffers it needs
	renderer = new DefferedRenderer(camera, context, device, backBufferRTV, depthStencilView, width, height);


	LoadShaders();
	LoadMeshes();
	LoadMaterials();
	// Tell the input assembler stage of the pipeline what kind of
	// geometric primitives (points, lines or triangles) we want to draw.  
	context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	light.AmbientColor = VEC4(0.1f, 0.1f, 0.1f, 1.0f);
	light.DiffuseColor = VEC4(0, 0, 1, 1);
	light.Direction = VEC3(1, -1, 0);

	gameManager.SetActiveScene(new BouncingBallScene());
	renderer->SetSkyBox("japan");
}

// --------------------------------------------------------
// Loads shaders from compiled shader object (.cso) files using
// my SimpleShader wrapper for DirectX shader manipulation.
void Game::LoadShaders()
{
	renderer->AddVertexShader("default", L"VertexShader.cso");
	renderer->AddPixelShader("default", L"PixelShader.cso");

	renderer->AddVertexShader("skybox", L"SkyVertex.cso");
	renderer->AddPixelShader("skybox", L"SkyPixel.cso");

	renderer->AddVertexShader("shadow", L"ShadowVertex.cso");

	// Create shaders for deffered
	renderer->AddVertexShader("gBuffer", L"gBufferVertexShader.cso");
	renderer->AddPixelShader("gBuffer", L"gBufferPixelShader.cso");
	renderer->AddVertexShader("quad", L"quadVertexShader.cso");
	renderer->AddPixelShader("quad", L"quadPixelShader.cso");
	renderer->AddVertexShader("sphereLight", L"sphereLightVertexShader.cso");
	renderer->AddPixelShader("sphereLight", L"sphereLightPixelShader.cso");

	// Create shaders for Particle Systems
	renderer->AddPixelShader("particle", L"ParticlePS.cso");
	renderer->AddVertexShader("particle", L"ParticleVS.cso");
	renderer->AddGeometryShader("particle", L"ParticleGS.cso");
	renderer->AddGeometryShader("spawn", L"SpawnGS.cso", true, false);
	renderer->AddVertexShader("spawn", L"SpawnVS.cso");

	// Add Shaders for post processing
	renderer->AddVertexShader("blur", L"BlurPostProcessVS.cso");
	renderer->AddVertexShader("postprocess", L"PostProcessDefaultVS.cso");
	renderer->AddPixelShader("blur", L"BlurPostProcessPS.cso");
	renderer->AddPixelShader("kernel", L"KernelPS.cso");
	renderer->AddPixelShader("bloomExtract", L"BloomExtractPS.cso");
	renderer->AddPixelShader("linearBlur", L"LinearBlurPS.cso");
	renderer->AddPixelShader("bloomCombine", L"BloomCombinePS.cso");
	renderer->AddPixelShader("ascii", L"AsciiPS.cso");
	renderer->AddPixelShader("ssao", L"SSAOPS.cso");
}

void Game::LoadMeshes()
{
	renderer->AddMesh("cone", "Assets/cone.obj");
	renderer->AddMesh("cube", "Assets/cube.obj");
	renderer->AddMesh("cylinder", "Assets/cylinder.obj");
	renderer->AddMesh("helix", "Assets/helix.obj");
	renderer->AddMesh("sphere", "Assets/sphere.obj");
	renderer->AddMesh("torus", "Assets/torus.obj");
	//renderer->AddMesh("court", "Assets/Court.obj");
	renderer->AddMesh("panel", "Assets/Panel.obj");
	//renderer->AddMesh("golf", "Assets/golfball.obj");
	renderer->AddMesh("soccer", "Assets/soccerball.obj");
	renderer->AddMesh("bbcourt", "Assets/bbcourt.obj");
	//renderer->AddMesh("bat", "Assets/baseballbat.obj");
	// full screen quad mesh
	renderer->AddMesh("quad", new Mesh(device));
	//renderer->AddMesh("paddle", "Assets/paddle.obj");
}

void Game::LoadMaterials()
{
	renderer->AddMaterial("default", L"Assets/Textures/MetalPlate.png");
	renderer->AddMaterial("electricity", L"Assets/Textures/Electricity.png");
	renderer->AddMaterial("greenopaque", L"Assets/Textures/GreenOpaque.png");
	renderer->GetMaterial("greenopaque")->transparency = true;
	renderer->AddMaterial("gridclip", L"Assets/Textures/GridClip.png");
	renderer->AddMaterial("white", L"Assets/Textures/White.png");
	renderer->AddMaterial("ascii", L"Assets/Textures/asciiTexture.png");
	//renderer->AddMaterial("soccer", L"Assets/Textures/soccer_ball_diffuse.bmp");
	renderer->AddMaterial("basketball", L"Assets/Textures/basketball texture.jpg");
	renderer->AddMaterial("wood", L"Assets/Textures/wooden floor texture.jpg");
	//renderer->AddMaterial("bat", L"Assets/Textures/bat.jpg");
	//renderer->AddMaterial("batnormal", L"Assets/Textures/batnormal.JPG");
	renderer->AddMaterial("win", L"Assets/Textures/Win.png");
	renderer->AddMaterial("quit", L"Assets/Textures/Quit.png");
	renderer->AddMaterial("play", L"Assets/Textures/Play.png");

	renderer->AddCubeMaterial("skybox", L"Assets/Textures/SunnyCubeMap.dds");
	renderer->AddCubeMaterial("japan", L"Assets/Textures/Yokohama.dds");
	renderer->AddCubeMaterial("bridge", L"Assets/Textures/GoldenGateBridge.dds");
}

// --------------------------------------------------------
// Handle resizing DirectX "stuff" to match the new window size.
// For instance, updating our projection matrix's aspect ratio.
// --------------------------------------------------------
void Game::OnResize()
{
	// Handle base-level DX resize stuff
	DXCore::OnResize();

	// Update our projection matrix since the window size changed
	XMMATRIX P = XMMatrixPerspectiveFovLH(
		0.25f * 3.1415926535f,	// Field of View Angle
		(float)width / height,	// Aspect ratio
		0.1f,				  	// Near clip plane distance
		100.0f);			  	// Far clip plane distance
	XMStoreFloat4x4(camera->GetProjection(), XMMatrixTranspose(P)); // Transpose for HLSL!
}

// --------------------------------------------------------
// Update your game here - user input, move objects, AI, etc.
// --------------------------------------------------------
void Game::Update(float deltaTime, float totalTime)
{
	// Quit if the escape key is pressed
	if (GetAsyncKeyState(VK_ESCAPE))
		Quit();

	camera->Update(deltaTime);
	gameManager.Update(deltaTime);

	if (GetAsyncKeyState('R') & 0x8000)
	{
		gameManager.SetActiveScene(new BouncingBallScene());
	}

	/*
	if (GetAsyncKeyState(VK_LSHIFT) & 0x8000)
	{
		if (GetAsyncKeyState('1') & 0x8000)
		{
			gameManager.SetActiveScene(new BouncingBallScene());
		}
	}
	*/

	bool currShift = (GetAsyncKeyState(VK_RSHIFT) & 0x8000) != 0;
	bool currTab = (GetAsyncKeyState('	') & 0x8000) != 0;
	bool currO = (GetAsyncKeyState('O') & 0x8000) != 0;
	if (currShift)
	{
		if (currO && !O_toggle)
		{
			if (renderer->PostProcessing == true)
			{
				renderer->PostProcessing = false;
			} else
			{
				renderer->PostProcessing = true;
				renderer->Blur = false;
				renderer->Bloom = false;
				renderer->EdgeDetect = false;
				renderer->Emboss = false;
				renderer->BlurWithKernel = false;
				renderer->Sharpness = false;
				renderer->BottomSobel = false;
				renderer->ASCII = false;
				postProcessChoice = 0;
			}
		}
		O_toggle = currO;

		if (currTab && !prevTab)
			postProcessChoice = (postProcessChoice + 1) % 8;
		prevTab = currTab;

		switch (postProcessChoice)
		{
		case 0: renderer->ASCII = false;
				renderer->Blur = true;
				break;
		case 1: renderer->Blur = false;
				renderer->Bloom = true;
				break;
		case 2: renderer->Bloom = false;
				renderer->EdgeDetect = true;
				break;
		case 3: renderer->EdgeDetect = false;
				renderer->Emboss = true;
				break;
		case 4: renderer->Emboss = false;
				renderer->BlurWithKernel = true;
				break;
		case 5: renderer->BlurWithKernel = false;
				renderer->Sharpness = true;
				break;
		case 6: renderer->Sharpness = false;
				renderer->BottomSobel = true;
				break;
		case 7: renderer->BottomSobel = false;
				renderer->ASCII = true;
				break;
		}
	}



	if (gameManager.EntitiesDirty)
	{
		gameManager.EntitiesDirty = false;
		renderer->SetGameEntities(&gameManager.GameEntities);
	}
	if (gameManager.DirectionalLightsDirty)
	{
		gameManager.DirectionalLightsDirty = false;
		renderer->SetDirectionalLights(gameManager.GetDirectionalLights());
	}
	if (gameManager.PointLightsDirty)
	{
		gameManager.PointLightsDirty = false;
		renderer->SetPointLights(gameManager.GetPointLights());
	}
	if (gameManager.ParticleEmittersDirty)
	{
		gameManager.ParticleEmittersDirty = false;
		renderer->SetParticleEmitters(gameManager.GetParticleEmitters());
	}
}

// --------------------------------------------------------
// Clear the screen, redraw everything, present to the user
// --------------------------------------------------------
void Game::Draw(float deltaTime, float totalTime)
{
	renderer->RenderShadowMap();
	renderer->Render(deltaTime, totalTime);
	
	swapChain->Present(0, 0);
}


#pragma region Mouse Input

// --------------------------------------------------------
// Helper method for mouse clicking. 
// --------------------------------------------------------
void Game::OnMouseDown(WPARAM buttonState, int x, int y)
{
	// Add any custom code here...
	mouseDown = true;
	// Save the previous mouse position, so we have it for the future
	prevMousePos.x = x;
	prevMousePos.y = y;

	SetCapture(hWnd);
}

// --------------------------------------------------------
// Helper method for mouse release
// --------------------------------------------------------
void Game::OnMouseUp(WPARAM buttonState, int x, int y)
{
	// Add any custom code here...
	mouseDown = false;
	// We don't care about the tracking the cursor outside
	// the window anymore (we're not dragging if the mouse is up)
	ReleaseCapture();
}

// --------------------------------------------------------
// Helper method for mouse movement.  We only get this message
// if the mouse is currently over the window, or if we're 
// currently capturing the mouse.
// --------------------------------------------------------
void Game::OnMouseMove(WPARAM buttonState, int x, int y)
{
	if (mouseDown)
	{
		FLOAT scalar = .005;
		camera->RotateXY((y - prevMousePos.y) * scalar, (x - prevMousePos.x) * scalar);
	}

	// Save the previous mouse position, so we have it for the future
	prevMousePos.x = x;
	prevMousePos.y = y;
}

// --------------------------------------------------------
// Helper method for mouse wheel scrolling.  
// WheelDelta may be positive or negative, depending 
// on the direction of the scroll
// --------------------------------------------------------
void Game::OnMouseWheel(float wheelDelta, int x, int y)
{
	// Add any code here...
}
Camera * Game::GetCamera()
{
	if (cameraPointer == nullptr)
		return nullptr;
	else
		return cameraPointer;
}
#pragma endregion