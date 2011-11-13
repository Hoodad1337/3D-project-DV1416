// include the basic windows header files and the Direct3D header files
#include <windows.h>
#include <windowsx.h>
#include <d3d10.h>
#include <d3dx10.h>

// include the Direct3D Library file
#pragma comment (lib, "d3d10.lib")
#pragma comment (lib, "d3dx10.lib")

// define the screen resolution
#define SCREEN_WIDTH  800
#define SCREEN_HEIGHT 600

// global declarations
ID3D10Device* device;

ID3D10RenderTargetView* rtv;
ID3D10RenderTargetView* rtv2;

ID3D10RenderTargetView* rtvArray[2] = {rtv,rtv2};    // the pointer to the render target view

ID3D10DepthStencilView* dsv;    // the pointer to the depth stencil view
IDXGISwapChain* swapchain;    // the pointer to the swap chain class
ID3D10Buffer* pBuffer;
ID3D10Effect* pEffect;
ID3D10EffectTechnique* pTechnique;
ID3D10EffectPass* pPass;
ID3D10InputLayout* pVertexLayout;
ID3D10EffectMatrixVariable* pTransform;    // the pointer to the effect variable interface
D3D10_PASS_DESC PassDesc;

struct VERTEX {D3DXVECTOR3 Position; D3DXCOLOR Color;};

// function prototypes
void initD3D(HWND hWnd);
void render_frame(void);
void cleanD3D(void);
void init_geometry(void);
void init_pipeline(void);

// the WindowProc function prototype
LRESULT CALLBACK WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);


// the entry point for any Windows program
int WINAPI WinMain(HINSTANCE hInstance,
	HINSTANCE hPrevInstance,
	LPSTR lpCmdLine,
	int nCmdShow)
{
	HWND hWnd;
	WNDCLASSEX wc;

	ZeroMemory(&wc, sizeof(WNDCLASSEX));

	wc.cbSize = sizeof(WNDCLASSEX);
	wc.style = CS_HREDRAW | CS_VREDRAW;
	wc.lpfnWndProc = WindowProc;
	wc.hInstance = hInstance;
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.lpszClassName = "WindowClass";

	RegisterClassEx(&wc);

	hWnd = CreateWindowEx(NULL,
		"WindowClass",
		"Our Direct3D Program",
		WS_OVERLAPPEDWINDOW,
		0, 0,
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
	DXGI_SWAP_CHAIN_DESC scd;

	ZeroMemory(&scd, sizeof(DXGI_SWAP_CHAIN_DESC));

	scd.BufferCount = 1;
	scd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	scd.BufferDesc.Width = SCREEN_WIDTH;
	scd.BufferDesc.Height = SCREEN_HEIGHT;
	scd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	scd.OutputWindow = hWnd;
	scd.SampleDesc.Count = 1;
	scd.SampleDesc.Quality = 0;
	scd.Windowed = TRUE;
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


	// create a texture for the depth buffer
	D3D10_TEXTURE2D_DESC zbd;
	ZeroMemory(&zbd, sizeof(zbd));
	zbd.Width = SCREEN_WIDTH;    // set the width of the depth buffer
	zbd.Height = SCREEN_HEIGHT;    // set the height of the depth buffer
	zbd.ArraySize = 1;    // we are only creating one texture
	zbd.SampleDesc.Count = 1;    // no multi-sampling
	zbd.Format = DXGI_FORMAT_D32_FLOAT;    // one 32-bit float per pixel
	zbd.BindFlags = D3D10_BIND_DEPTH_STENCIL;    // texture is to be used as a depth buffer
	ID3D10Texture2D* pDepthBuffer;
	device->CreateTexture2D(&zbd, NULL, &pDepthBuffer);    // create the texture

	// create the depth buffer
	D3D10_DEPTH_STENCIL_VIEW_DESC dsvd;
	ZeroMemory(&dsvd, sizeof(dsvd));
	dsvd.Format = DXGI_FORMAT_D32_FLOAT;    // one 32-bit float per pixel
	dsvd.ViewDimension = D3D10_DSV_DIMENSION_TEXTURE2D;    // depth buffer is a 2D texture
	device->CreateDepthStencilView(pDepthBuffer, &dsvd, &dsv);    // create the depth buffer
	pDepthBuffer->Release();    // release the texture once the depth buffer is made

	// get the address of the back buffer and use it to create the render target
	ID3D10Texture2D* pBackBuffer;
	ID3D10Texture2D* pBlackBuffer;

	swapchain->GetBuffer(0, __uuidof(ID3D10Texture2D), (LPVOID*)&pBackBuffer);
	swapchain->GetBuffer(0, __uuidof(ID3D10Texture2D), (LPVOID*)&pBlackBuffer);
	device->CreateRenderTargetView(pBackBuffer, NULL, &rtv);
	device->CreateRenderTargetView(pBlackBuffer,NULL, &rtv2);
	pBackBuffer->Release();
	pBlackBuffer->Release();

	// set the back buffer as the render target
	device->OMSetRenderTargets(2, rtvArray, dsv);


	D3D10_VIEWPORT viewport;    // create a struct to hold the viewport data

	ZeroMemory(&viewport, sizeof(D3D10_VIEWPORT));    // clear out the struct for use

	viewport.TopLeftX = 0;    // set the left to 0
	viewport.TopLeftY = 0;    // set the top to 0
	viewport.Width = SCREEN_WIDTH;    // set the width to the window's width
	viewport.Height = SCREEN_HEIGHT;    // set the height to the window's height
	viewport.MinDepth = 0;    // the closest an object can be on the depth buffer is 0.0
	viewport.MaxDepth = 1;    // the farthest an object can be on the depth buffer is 1.0

	device->RSSetViewports(1, &viewport);    // set the viewport
}


// this is the function used to render a single frame
void render_frame(void)
{
	// clear the window to a deep blue and clear the depth buffer to 1.0f
	//device->ClearRenderTargetView(rtv, D3DXCOLOR(0.0f, 0.2f, 0.4f, 1.0f));
	device->ClearRenderTargetView(rtv2,D3DXCOLOR(0.0f, 0.0f, 0.0f, 1.0f));
	device->ClearDepthStencilView(dsv, D3D10_CLEAR_DEPTH, 1.0f, 0);

	device->OMSetRenderTargets(1,&rtv,NULL);
	
	// select which input layout we are using
	device->IASetInputLayout(pVertexLayout);

	// select which primtive type we are using
	device->IASetPrimitiveTopology(D3D10_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	// select which vertex buffer to display
	UINT stride = sizeof(VERTEX);
	UINT offset = 0;
	device->IASetVertexBuffers(0, 1, &pBuffer, &stride, &offset);

	// increase the time varaible and send to the effect
	static float Time = 0.0f; Time += 16.0f/1000.0f;

	D3DXMATRIX matRotate, matView, matProjection, matFinal;

	// create a rotation matrix
	D3DXMatrixRotationY(&matRotate, Time);

	// create a view matrix
	D3DXMatrixLookAtLH(&matView,
		&D3DXVECTOR3(0.0f, 0.0f, 2.0f),    // the camera position
		&D3DXVECTOR3(0.0f, 0.0f, 0.0f),    // the look-at position
		&D3DXVECTOR3(0.0f, 1.0f, 0.0f));    // the up direction

	// create a projection matrix
	D3DXMatrixPerspectiveFovLH(&matProjection,
		(float)D3DXToRadian(45),    // the horizontal field of view
		(FLOAT)SCREEN_WIDTH / (FLOAT)SCREEN_HEIGHT, // aspect ratio
		1.0f,    // the near view-plane
		100.0f);    // the far view-plane

	// combine the matrices and render
	matFinal = matRotate * matView * matProjection;
	pTransform->SetMatrix(&matFinal._11); 
	pPass->Apply(0);
	device->Draw(3, 0);

	//-----------------------------------------------------------------------------------

	device->OMSetRenderTargets(1,&rtv2,NULL);

	// select which input layout we are using
	device->IASetInputLayout(pVertexLayout);

	// select which primtive type we are using
	device->IASetPrimitiveTopology(D3D10_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	// select which vertex buffer to display
	stride = sizeof(VERTEX);
	offset = 0;
	device->IASetVertexBuffers(0, 1, &pBuffer, &stride, &offset);

	D3DXMATRIX matRotate2, matView2, matProjection2, matFinal2;

	// create a rotation matrix
	D3DXMatrixRotationY(&matRotate2, -Time);

	// create a view matrix
	D3DXMatrixLookAtLH(&matView2,
		&D3DXVECTOR3(0.0f, 0.0f, 2.0f),    // the camera position
		&D3DXVECTOR3(0.0f, 0.0f, 0.0f),    // the look-at position
		&D3DXVECTOR3(0.0f, 1.0f, 0.0f));    // the up direction

	// create a projection matrix
	D3DXMatrixPerspectiveFovLH(&matProjection2,
		(float)D3DXToRadian(45),    // the horizontal field of view
		(FLOAT)SCREEN_WIDTH / (FLOAT)SCREEN_HEIGHT, // aspect ratio
		1.0f,    // the near view-plane
		100.0f);    // the far view-plane

	// combine the matrices and render
	matFinal2 = matRotate2 * matView2 * matProjection2;
	pTransform->SetMatrix(&matFinal2._11); 
	pPass->Apply(0);
	device->Draw(3, 0);

	// display the rendered frame
	swapchain->Present(1, 0);
}


// this is the function that cleans up Direct3D and COM
void cleanD3D(void)
{
	swapchain->SetFullscreenState(FALSE, NULL);    // switch to windowed mode

	pBuffer->Release();    // close and release the vertex buffer
	pVertexLayout->Release();    // close and release the input layout object
	swapchain->Release();    // close and release the swap chain
	rtv->Release();    // close and release the render target view
	rtv2->Release();
	device->Release();    // close and release the 3D device
}


// this is the function that creates the geometry to render
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


// this function sets up the pipeline for rendering
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
