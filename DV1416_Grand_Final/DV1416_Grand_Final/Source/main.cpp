// include the basic windows header files and the Direct3D header files
#include <windows.h>
#include <windowsx.h>
#include <d3d10.h>
#include <d3dx10.h>
#include "wtypes.h" //this is used to obtain screen resolution

// include the Direct3D Library file
#pragma comment (lib, "d3d10.lib")
#pragma comment (lib, "d3dx10.lib")

// define the screen resolution
int SCREEN_WIDTH;
int SCREEN_HEIGHT;

// global declarations
ID3D10Device* device;    // the pointer to our Direct3D device interface
ID3D10RenderTargetView* rtv;    // the pointer to the render target view
IDXGISwapChain* swapchain;    // the pointer to the swap chain class
ID3D10Buffer* pBuffer;		// global definition
ID3D10Effect* pEffect;    // global definition
ID3D10EffectTechnique* pTechnique;    // global definition
ID3D10EffectPass* pPass;
ID3D10InputLayout* pVertexLayout;
ID3D10EffectMatrixVariable* pTransform;    // global

D3D10_PASS_DESC PassDesc;

struct VERTEX
{
	D3DXVECTOR3 Position;
	D3DXCOLOR Color;
};

// function prototypes
void initD3D(HWND hWnd);    // sets up and initializes Direct3D
void render_frame(void);    // renders a single frame
void cleanD3D(void);    // closes Direct3D and releases memory
bool setScreenResolution(void); // gets the desktop resolution
void init_geometry(void); //sets the geometry
void init_pipeline(void); //Sets the pipeline 

// the WindowProc function prototype
LRESULT CALLBACK WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);


/*! Set the screen resolution which will be used throughout the game.
   * \return Returns a confirmed dynamic set resolution (true) or a pre-set static resolution (false) */


	
// the entry point for any Windows program
int WINAPI WinMain(HINSTANCE hInstance,
	HINSTANCE hPrevInstance,
	LPSTR lpCmdLine,
	int nCmdShow)
{
	if (!setScreenResolution())
	{
		SCREEN_HEIGHT = 800;
		SCREEN_WIDTH = 600;
	}

	HWND hWnd;
	WNDCLASSEX wc;

	ZeroMemory(&wc, sizeof(WNDCLASSEX));

	wc.cbSize = sizeof(WNDCLASSEX);
	wc.style = CS_HREDRAW | CS_VREDRAW;
	wc.lpfnWndProc = WindowProc;
	wc.hInstance = hInstance;
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	//wc.hbrBackground = (HBRUSH)COLOR_WINDOW;
	wc.lpszClassName = "WindowClass";

	RegisterClassEx(&wc);

	hWnd = CreateWindowEx(NULL,
		"WindowClass",
		"Our First Direct3D Program",
		WS_OVERLAPPEDWINDOW,
		300, 300,
		SCREEN_WIDTH, SCREEN_HEIGHT,
		NULL,
		NULL,
		hInstance,
		NULL);

	ShowWindow(hWnd, nCmdShow);

	// set up and initialize Direct3D
	initD3D(hWnd);
	init_geometry();
	init_pipeline();

	// enter the main loop:

	MSG msg;

	while(TRUE)
	{
		if(PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);

			if(msg.message == WM_QUIT)
				break;
		}

		render_frame();
	}

	// clean up DirectX and COM
	cleanD3D();

	return msg.wParam;
}


// this is the main message handler for the program
LRESULT CALLBACK WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch(message)
	{
	case WM_DESTROY:
		{
			PostQuitMessage(0);
			return 0;
		} break;
	}

	return DefWindowProc (hWnd, message, wParam, lParam);
}


// this function initializes and prepares Direct3D for use
void initD3D(HWND hWnd)
{
	DXGI_SWAP_CHAIN_DESC scd;    // create a struct to hold various swap chain information

	ZeroMemory(&scd, sizeof(DXGI_SWAP_CHAIN_DESC));    // clear out the struct for use

	scd.BufferCount = 1;    // create two buffers, one for the front, one for the back
	scd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;    // use 32-bit color
	scd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;    // tell how the chain is to be used
	scd.BufferDesc.Width = SCREEN_WIDTH;
	scd.BufferDesc.Height = SCREEN_HEIGHT;
	scd.OutputWindow = hWnd;    // set the window to be used by Direct3D
	scd.SampleDesc.Count = 1;    // set the level of multi-sampling
	scd.SampleDesc.Quality = 0;    // set the quality of multi-sampling
	scd.Windowed = FALSE;    // set to windowed or full-screen mode
	scd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

	// create a device class and swap chain class using the information in the scd struct
	D3D10CreateDeviceAndSwapChain(NULL,
		D3D10_DRIVER_TYPE_HARDWARE,
		NULL,
		0,
		D3D10_SDK_VERSION,
		&scd,
		&swapchain,
		&device);


	// get the address of the back buffer and use it to create the render target
	ID3D10Texture2D* pBackBuffer;
	swapchain->GetBuffer(0, __uuidof(ID3D10Texture2D), (LPVOID*)&pBackBuffer);
	device->CreateRenderTargetView(pBackBuffer, NULL, &rtv);
	pBackBuffer->Release();

	// set the back buffer as the render target
	device->OMSetRenderTargets(1, &rtv, NULL);

	D3D10_VIEWPORT viewport;    // create a struct to hold the viewport data

	ZeroMemory(&viewport, sizeof(D3D10_VIEWPORT));    // clear out the struct for use

	viewport.TopLeftX = 0;    // set the left to 0
	viewport.TopLeftY = 0;    // set the top to 0
	viewport.Width = SCREEN_WIDTH;    // set the width to the window's width
	viewport.Height = SCREEN_HEIGHT;    // set the height to the window's height

	device->RSSetViewports(1, &viewport);    // set the viewport
}
D3DXMATRIX& setupMatrices(void)
{
	D3DXMATRIX matTranslate;    // a matrix to store the translation information
	D3DXMATRIX matRotateX;
	D3DXMATRIX matScale;
	D3DXMATRIX matWorld;
	D3DXMATRIX matView;
	D3DXMATRIX matProjection;
	D3DXMATRIX matFinal;
	D3DXMATRIX matRotateY;

	static float dT = 0; dT += 0.001f;
	D3DXMatrixRotationY(&matRotateY,D3DXToRadian(180));

	// build a matrix to move the model 12 units along the x-axis and 4 units along the y-axis
	// store it to matTranslate
	//D3DXMatrixTranslation(&matTranslate, 12.0f, 4.0f, 0.0f);

	//Rotate around the x-angle by the amount of radians 3.14 (1 pi)
	D3DXMatrixRotationX(&matRotateX, D3DXToRadian(90));

	D3DXMatrixScaling(&matScale, 1.0f, 1.0f, 1.0f);


	matWorld = matScale * matRotateY;    // multiply matScale and matRotateX  to combine them
	
	static D3DXVECTOR3 pos = D3DXVECTOR3(0.0f,0.0f,0.0f); pos.z += 0.01f;

	D3DXMatrixLookAtLH(&matView,
		&pos,    // the camera position
		&D3DXVECTOR3(0.0f, 0.0f, 0.0f),    // the look-at position
		&D3DXVECTOR3(0.0f, 1.0f, 0.0f));    // the up direction

	D3DXMatrixPerspectiveFovLH(&matProjection,
		D3DXToRadian(45),    // the horizontal field of view
		(FLOAT)SCREEN_WIDTH / (FLOAT)SCREEN_HEIGHT,    // aspect ratio
		1.0f,    // the near view-plane
		200.0f);    // the far view-plane

	matFinal = matWorld * matView * matProjection;

	return matFinal;
}
// this is the function used to render a single frame
void render_frame(void)
{
	pTransform->SetMatrix(&setupMatrices()._11);

	// clear the window to a deep blue
	device->ClearRenderTargetView(rtv, D3DXCOLOR(0.0f, 0.2f, 0.4f, 1.0f));

	// select which input layout we are using
	device->IASetInputLayout(pVertexLayout);

	// select which primtive type we are using
	device->IASetPrimitiveTopology(D3D10_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	// select which vertex buffer to display
	UINT stride = sizeof(VERTEX);
	UINT offset = 0;
	device->IASetVertexBuffers(0, 1, &pBuffer, &stride, &offset);

	// apply the appropriate pass
	pPass->Apply(0);

	// draw the vertex buffer to the back buffer
	device->Draw(3, 0);

	// display the rendered frame
	swapchain->Present(0, 0);
}

bool setScreenResolution()
{
	RECT desktop; // Get a handle to the desktop window
	const HWND hDesktop = GetDesktopWindow(); //Get the size of screen to the variable desktop
	GetWindowRect(hDesktop, &desktop);
	SCREEN_WIDTH = desktop.right;
	SCREEN_HEIGHT = desktop.bottom;

	return true;
}
void init_geometry(void)
{
	// create three vertices using the VERTEX struct built earlier
	VERTEX OurVertices[] =
	{
		{D3DXVECTOR3(0.0f, 0.5f, 0.0f), D3DXCOLOR(1.0f, 0.0f, 0.0f, 1.0f)},
		{D3DXVECTOR3(0.45f, -0.5f, 0.0f), D3DXCOLOR(0.0f, 1.0f, 0.0f, 1.0f)},
		{D3DXVECTOR3(-0.45f, -0.5f, 0.0f), D3DXCOLOR(0.0f, 0.0f, 1.0f, 1.0f)}
	};

	// create the vertex buffer and store the pointer into pBuffer, which is created globally
	D3D10_BUFFER_DESC bd;
	bd.Usage = D3D10_USAGE_DYNAMIC;
	bd.ByteWidth = sizeof(VERTEX) * 3;
	bd.BindFlags = D3D10_BIND_VERTEX_BUFFER;
	bd.CPUAccessFlags = D3D10_CPU_ACCESS_WRITE;
	bd.MiscFlags = 0;

	device->CreateBuffer(&bd, NULL, &pBuffer);

	void* pVoid;    // the void pointer

	pBuffer->Map(D3D10_MAP_WRITE_DISCARD, 0, &pVoid);    // map the vertex buffer
	memcpy(pVoid, OurVertices, sizeof(OurVertices));    // copy the vertices to the buffer
	pBuffer->Unmap();
}
void init_pipeline(void)
{
	// load the effect file
	D3DX10CreateEffectFromFile("FX//effect.fx", 0, 0,
		"fx_4_0", 0, 0,
		device, 0, 0,
		&pEffect, 0, 0);

	// get the technique and the pass
	pTechnique = pEffect->GetTechniqueByIndex(0);
	pPass = pTechnique->GetPassByIndex(0);
	pPass->GetDesc(&PassDesc);

	// get the TimeElapsed effect variable
	pTransform = pEffect->GetVariableByName("Transform")->AsMatrix();

	// create the input element descriptions
	D3D10_INPUT_ELEMENT_DESC Layout[] =
	{
		// first input element: position
		{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D10_APPEND_ALIGNED_ELEMENT,
		D3D10_INPUT_PER_VERTEX_DATA, 0},

		// second input element: color
		{"COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D10_APPEND_ALIGNED_ELEMENT,
		D3D10_INPUT_PER_VERTEX_DATA, 0}
	};

	// use the input element descriptions to create the input layout
	device->CreateInputLayout(Layout,
								2,
								PassDesc.pIAInputSignature,
								PassDesc.IAInputSignatureSize,
								&pVertexLayout);
}
// this is the function that cleans up Direct3D and COM
void cleanD3D(void)
{
	swapchain->SetFullscreenState(FALSE, NULL);

	pBuffer->Release();
	pVertexLayout->Release();
	swapchain->Release();    // close and release the swap chain
	rtv->Release();    // close and release the render target view
	device->Release();    // close and release the 3D device
}
