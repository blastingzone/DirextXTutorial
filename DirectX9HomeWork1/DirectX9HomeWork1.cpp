#include <Windows.h>
#include <mmsystem.h>
#include <d3dx9.h>
#pragma warning( disable : 4996 )
#include <strsafe.h>
#pragma warning( default : 4996 )

LPDIRECT3D9             g_pD3D = NULL;
LPDIRECT3DDEVICE9       g_pd3dDevice = NULL;
LPDIRECT3DVERTEXBUFFER9 g_pVB = NULL;
LPDIRECT3DTEXTURE9		g_pTexture = NULL;

// for mesh
LPD3DXMESH				g_pMesh = NULL;
D3DMATERIAL9*			g_pMeshMaterials = NULL;
LPDIRECT3DTEXTURE9*		g_pMeshTextures = NULL;
DWORD					g_dwNumMaterials = 0L;

// for second Tiger
// 같은 전역변수를 재사용하면 제대로 나오지 않음.
// 그렇다고 전역변수를 늘려서 해결하는 것은 좋은 방법이 아닌데...
LPD3DXMESH				g_pMeshSecond = NULL;
D3DMATERIAL9*			g_pMeshMaterialsSecond = NULL;
LPDIRECT3DTEXTURE9*		g_pMeshTexturesSecond = NULL;
DWORD					g_dwNumMaterialsSecond = 0L;


struct CUSTOMVERTEX
{
	D3DXVECTOR3 position;
	D3DXVECTOR3 normal;
	FLOAT tu, tv;
};

#define D3DFVF_CUSTOMVERTEX (D3DFVF_XYZ|D3DFVF_NORMAL|D3DFVF_TEX1)

HRESULT InitD3D( HWND hWnd )
{
	if ( NULL == ( g_pD3D = Direct3DCreate9( D3D_SDK_VERSION ) ) )
		return E_FAIL;

	D3DPRESENT_PARAMETERS d3dpp;
	ZeroMemory( &d3dpp, sizeof( d3dpp ) );
	d3dpp.Windowed = TRUE;
	d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;
	d3dpp.BackBufferFormat = D3DFMT_UNKNOWN;
	d3dpp.EnableAutoDepthStencil = TRUE;
	d3dpp.AutoDepthStencilFormat = D3DFMT_D16;

	if ( FAILED( g_pD3D->CreateDevice( D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, hWnd,
		D3DCREATE_SOFTWARE_VERTEXPROCESSING,
		&d3dpp, &g_pd3dDevice ) ) )
	{
		return E_FAIL;
	}

	g_pd3dDevice->SetRenderState( D3DRS_CULLMODE, D3DCULL_NONE );

	g_pd3dDevice->SetRenderState( D3DRS_ZENABLE, TRUE );

	return S_OK;
}


HRESULT LoadMesh()
{
	LPD3DXBUFFER pD3DXMtrlBuffer;

	if ( FAILED( D3DXLoadMeshFromX( L"Tiger.x", D3DXMESH_SYSTEMMEM,
		g_pd3dDevice, NULL,
		&pD3DXMtrlBuffer, NULL, &g_dwNumMaterials,
		&g_pMesh ) ) )
	{
		if ( FAILED( D3DXLoadMeshFromX( L"..\Tiger.x", D3DXMESH_SYSTEMMEM,
			g_pd3dDevice, NULL,
			&pD3DXMtrlBuffer, NULL, &g_dwNumMaterials,
			&g_pMesh ) ) )
		{
			MessageBox( NULL, L"Could not find tiger.x", L"Meshes.exe", MB_OK );
			return E_FAIL;
		}
	}

	D3DXMATERIAL* d3dxMaterials = (D3DXMATERIAL*)pD3DXMtrlBuffer->GetBufferPointer();
	g_pMeshMaterials = new D3DMATERIAL9[g_dwNumMaterials];
	if ( NULL == g_pMeshMaterials )
		return E_OUTOFMEMORY;
	g_pMeshTextures = new LPDIRECT3DTEXTURE9[g_dwNumMaterials];
	if ( NULL == g_pMeshTextures )
		return E_OUTOFMEMORY;

	for ( DWORD i = 0; i < g_dwNumMaterials; ++i )
	{
		g_pMeshMaterials[i] = d3dxMaterials[i].MatD3D;

		g_pMeshMaterials[i].Ambient = g_pMeshMaterials[i].Diffuse;

		g_pMeshTextures[i] = NULL;

		if ( d3dxMaterials[i].pTextureFilename != NULL &&
			lstrlenA( d3dxMaterials[i].pTextureFilename ) > 0 )
		{
			if ( FAILED( D3DXCreateTextureFromFileA( g_pd3dDevice,
				d3dxMaterials[i].pTextureFilename,
				&g_pMeshTextures[i] ) ) )
			{
				const CHAR* strPrefix = "..\\";
				CHAR strTexture[MAX_PATH];
				strcpy_s( strTexture, MAX_PATH, strPrefix );
				strcat_s( strTexture, MAX_PATH, d3dxMaterials[i].pTextureFilename );
				if ( FAILED( D3DXCreateTextureFromFileA( g_pd3dDevice,
					strTexture,
					&g_pMeshTextures[i] ) ) )
				{
					MessageBox( NULL, L"Could not find texture map", L"Meshes.exe", MB_OK );
					return E_FAIL;
				}
			}
		}
	}
	pD3DXMtrlBuffer->Release();

	// 법선 벡터를 만들어준다
	if ( !( g_pMesh->GetFVF() & D3DFVF_NORMAL ) )
	{
		ID3DXMesh* pTempMesh = 0;
		g_pMesh->CloneMeshFVF(
			D3DXMESH_MANAGED,
			g_pMesh->GetFVF() | D3DFVF_NORMAL,
			g_pd3dDevice,
			&pTempMesh );

		D3DXComputeNormals( pTempMesh, 0 );

		g_pMesh->Release();
		g_pMesh = pTempMesh;
	}

	return S_OK;
}

// 위의 함수와 완전히 동일한 일을 수행한다.
HRESULT LoadSecondMesh()
{
	LPD3DXBUFFER pD3DXMtrlBuffer;

	if ( FAILED( D3DXLoadMeshFromX( L"Tiger.x", D3DXMESH_SYSTEMMEM,
		g_pd3dDevice, NULL,
		&pD3DXMtrlBuffer, NULL, &g_dwNumMaterialsSecond,
		&g_pMeshSecond ) ) )
	{
		if ( FAILED( D3DXLoadMeshFromX( L"..\Tiger.x", D3DXMESH_SYSTEMMEM,
			g_pd3dDevice, NULL,
			&pD3DXMtrlBuffer, NULL, &g_dwNumMaterialsSecond,
			&g_pMesh ) ) )
		{
			MessageBox( NULL, L"Could not find tiger.x", L"Meshes.exe", MB_OK );
			return E_FAIL;
		}
	}

	D3DXMATERIAL* d3dxMaterials = (D3DXMATERIAL*)pD3DXMtrlBuffer->GetBufferPointer();
	g_pMeshMaterialsSecond = new D3DMATERIAL9[g_dwNumMaterialsSecond];
	if ( NULL == g_pMeshMaterialsSecond )
		return E_OUTOFMEMORY;
	g_pMeshTexturesSecond = new LPDIRECT3DTEXTURE9[g_dwNumMaterialsSecond];
	if ( NULL == g_pMeshTexturesSecond )
		return E_OUTOFMEMORY;

	for ( DWORD i = 0; i < g_dwNumMaterialsSecond; ++i )
	{
		g_pMeshMaterialsSecond[i] = d3dxMaterials[i].MatD3D;

		g_pMeshMaterialsSecond[i].Ambient = g_pMeshMaterialsSecond[i].Diffuse;

		g_pMeshTexturesSecond[i] = NULL;

		if ( d3dxMaterials[i].pTextureFilename != NULL &&
			lstrlenA( d3dxMaterials[i].pTextureFilename ) > 0 )
		{
			if ( FAILED( D3DXCreateTextureFromFileA( g_pd3dDevice,
				d3dxMaterials[i].pTextureFilename,
				&g_pMeshTexturesSecond[i] ) ) )
			{
				const CHAR* strPrefix = "..\\";
				CHAR strTexture[MAX_PATH];
				strcpy_s( strTexture, MAX_PATH, strPrefix );
				strcat_s( strTexture, MAX_PATH, d3dxMaterials[i].pTextureFilename );
				if ( FAILED( D3DXCreateTextureFromFileA( g_pd3dDevice,
					strTexture,
					&g_pMeshTexturesSecond[i] ) ) )
				{
					MessageBox( NULL, L"Could not find texture map", L"Meshes.exe", MB_OK );
					return E_FAIL;
				}
			}
		}
	}
	pD3DXMtrlBuffer->Release();

	// 법선 벡터를 만들어준다
	if ( !( g_pMeshSecond->GetFVF() & D3DFVF_NORMAL ) )
	{
		ID3DXMesh* pTempMesh = 0;
		g_pMeshSecond->CloneMeshFVF(
			D3DXMESH_MANAGED,
			g_pMeshSecond->GetFVF() | D3DFVF_NORMAL,
			g_pd3dDevice,
			&pTempMesh );

		D3DXComputeNormals( pTempMesh, 0 );

		g_pMeshSecond->Release();
		g_pMeshSecond = pTempMesh;
	}

	return S_OK;
}

HRESULT InitGeometry()
{
	if ( FAILED( D3DXCreateTextureFromFile( g_pd3dDevice, L"banana.bmp", &g_pTexture ) ) )
	{
		// If texture is not in current folder, try parent folder
		if ( FAILED( D3DXCreateTextureFromFile( g_pd3dDevice, L"..\\banana.bmp", &g_pTexture ) ) )
		{
			MessageBox( NULL, L"Could not find banana.bmp", L"DirectX9HomeWork1.exe", MB_OK );
			return E_FAIL;
		}
	}

	if ( FAILED( g_pd3dDevice->CreateVertexBuffer( 50 * 2 * sizeof( CUSTOMVERTEX ),
		0, D3DFVF_CUSTOMVERTEX,
		D3DPOOL_DEFAULT, &g_pVB, NULL ) ) )
	{
		return E_FAIL;
	}

	CUSTOMVERTEX* pVertices;
	if ( FAILED( g_pVB->Lock( 0, 0, (void**)&pVertices, 0 ) ) )
		return E_FAIL;
	for ( DWORD i = 0; i < 50; i++ )
	{
		FLOAT theta = ( 2 * D3DX_PI * i ) / ( 50 - 1 );
		pVertices[2 * i + 0].position = D3DXVECTOR3( sinf( theta ), -1.0f, cosf( theta ) );
		pVertices[2 * i + 0].normal = D3DXVECTOR3( sinf( theta ), 0.0f, cosf( theta ) );
		pVertices[2 * i + 0].tu = ( (FLOAT)i ) / ( 50 - 1 );
		pVertices[2 * i + 0].tv = 1.0f;
		pVertices[2 * i + 1].position = D3DXVECTOR3( sinf( theta ), 1.0f, cosf( theta ) );
		pVertices[2 * i + 1].normal = D3DXVECTOR3( sinf( theta ), 0.0f, cosf( theta ) );
		pVertices[2 * i + 1].tu = ( (FLOAT)i ) / ( 50 - 1 );
		pVertices[2 * i + 1].tv = 0.0f;
	}
	g_pVB->Unlock();

	return S_OK;
}



VOID Cleanup()
{
	// meshes 
	if ( g_pMeshMaterials != NULL )
		delete[] g_pMeshMaterials;

	if ( g_pMeshTextures )
	{
		for ( DWORD i = 0; i < g_dwNumMaterials; ++i )
		{
			if ( g_pMeshTextures[i] )
				g_pMeshTextures[i]->Release();
		}
		delete[] g_pMeshTextures;
	}

	if ( g_pMesh != NULL )
		g_pMesh->Release();

	if ( g_pMeshMaterialsSecond != NULL )
		delete[] g_pMeshMaterialsSecond;

	if ( g_pMeshTexturesSecond )
	{
		for ( DWORD i = 0; i < g_dwNumMaterialsSecond; ++i )
		{
			if ( g_pMeshTexturesSecond[i] )
				g_pMeshTexturesSecond[i]->Release();
		}
		delete[] g_pMeshTexturesSecond;
	}

	if ( g_pMeshSecond != NULL )
		g_pMeshSecond->Release();

	// cylinder
	if ( g_pTexture != NULL )
		g_pTexture->Release();

	if ( g_pVB != NULL )
		g_pVB->Release();

	if ( g_pd3dDevice != NULL )
		g_pd3dDevice->Release();

	if ( g_pD3D != NULL )
		g_pD3D->Release();
}


VOID SetupCylinderMatrices()
{
	D3DXMATRIXA16 matWorld;
	D3DXMatrixIdentity( &matWorld );
	D3DXMatrixRotationX( &matWorld, timeGetTime() / 1000.0f );
	g_pd3dDevice->SetTransform( D3DTS_WORLD, &matWorld );

	D3DXVECTOR3 vEyePt( 0.0f, 3.0f, -5.0f );
	D3DXVECTOR3 vLookatPt( 0.0f, 0.0f, 0.0f );
	D3DXVECTOR3 vUpVec( 0.0f, 1.0f, 0.0f );
	D3DXMATRIXA16 matView;
	D3DXMatrixLookAtLH( &matView, &vEyePt, &vLookatPt, &vUpVec );
	g_pd3dDevice->SetTransform( D3DTS_VIEW, &matView );

	D3DXMATRIXA16 matProj;
	D3DXMatrixPerspectiveFovLH( &matProj, D3DX_PI / 4, 1.0f, 1.0f, 100.0f );
	g_pd3dDevice->SetTransform( D3DTS_PROJECTION, &matProj );
}

VOID SetupTigerMatrices()
{
	D3DXMATRIXA16 matWorld;
	D3DXMATRIXA16 matRotateWorld;
	D3DXMATRIXA16 resultMatrix;

	D3DXMatrixIdentity( &matWorld );
	D3DXMatrixIdentity( &matRotateWorld );

	D3DXMatrixRotationY( &matRotateWorld, timeGetTime() / 2000.0f );
	D3DXMatrixTranslation( &matWorld, 1.5f, 1.5f, 0.0f );
	// 원 그리면서 움직이기
	D3DXMatrixMultiply( &resultMatrix, &matWorld, &matRotateWorld );
	g_pd3dDevice->SetTransform( D3DTS_WORLD, &resultMatrix );
}


VOID SetupTigerSecondMatrices()
{
	D3DXMATRIXA16 matWorld;
	D3DXMATRIXA16 matRotateWorld;
	D3DXMATRIXA16 resultMatrix;

	D3DXMatrixIdentity( &matWorld );
	D3DXMatrixIdentity( &matRotateWorld );

	D3DXMatrixRotationY( &matRotateWorld, timeGetTime() / 1000.0f );
	D3DXMatrixTranslation( &matWorld, -1.5f, -1.0f, 0.3f );
	// 제자리 돌기
	D3DXMatrixMultiply( &resultMatrix, &matRotateWorld , &matWorld );
	g_pd3dDevice->SetTransform( D3DTS_WORLD, &resultMatrix );
}

VOID SetupLightEnvironment()
{
	D3DMATERIAL9 mtrl;
	ZeroMemory( &mtrl, sizeof( D3DMATERIAL9 ) );
	mtrl.Diffuse.r = mtrl.Ambient.r = 1.0f;
	mtrl.Diffuse.g = mtrl.Ambient.g = 1.0f;
	mtrl.Diffuse.b = mtrl.Ambient.b = 0.0f;
	mtrl.Diffuse.a = mtrl.Ambient.a = 1.0f;
	g_pd3dDevice->SetMaterial( &mtrl );

	g_pd3dDevice->LightEnable( 1, TRUE );
	g_pd3dDevice->SetRenderState( D3DRS_LIGHTING, TRUE );
	g_pd3dDevice->SetRenderState( D3DRS_AMBIENT, 0x00202020 );
}

VOID SetupLights1()
{
	D3DXVECTOR3 vecDir;
	D3DLIGHT9 light;
	ZeroMemory( &light, sizeof( D3DLIGHT9 ) );
	light.Type = D3DLIGHT_DIRECTIONAL;
	// green light
	light.Diffuse.r = 0.0f;
	light.Diffuse.g = 1.0f;
	light.Diffuse.b = 0.0f;
	vecDir = D3DXVECTOR3( cosf( timeGetTime() / 350.0f ),
		1.0f,
		sinf( timeGetTime() / 350.0f ) );
	D3DXVec3Normalize( (D3DXVECTOR3*)&light.Direction, &vecDir );
	light.Range = 1000.0f;

	g_pd3dDevice->SetLight( 1, &light );
}

VOID SetUpLights2()
{
	// create Second Light
	D3DXVECTOR3 vecDir;
	D3DLIGHT9 light;
	ZeroMemory( &light, sizeof( D3DLIGHT9 ) );
	light.Type = D3DLIGHT_DIRECTIONAL;
	// red light
	light.Diffuse.r = 1.0f;
	light.Diffuse.g = 0.0f;
	light.Diffuse.b = 0.0f;
	vecDir = D3DXVECTOR3( cosf( timeGetTime() / 350.0f ),
		1.0f,
		sinf( timeGetTime() / 350.0f ) );
	D3DXVec3Normalize( (D3DXVECTOR3*)&light.Direction, &vecDir );
	light.Range = 1000.0f;
	g_pd3dDevice->SetLight( 1, &light );
}

VOID LoadBananaTexture()
{
	if ( FAILED( D3DXCreateTextureFromFile( g_pd3dDevice, L"banana.bmp", &g_pTexture ) ) )
	{
		// If texture is not in current folder, try parent folder
		if ( FAILED( D3DXCreateTextureFromFile( g_pd3dDevice, L"..\\banana.bmp", &g_pTexture ) ) )
		{
			MessageBox( NULL, L"Could not find banana.bmp", L"DirectX9HomeWork1.exe", MB_OK );
		}
	}
}

VOID LoadStoneTexture()
{
	if ( FAILED( D3DXCreateTextureFromFile( g_pd3dDevice, L"stone.bmp", &g_pTexture ) ) )
	{
		// If texture is not in current folder, try parent folder
		if ( FAILED( D3DXCreateTextureFromFile( g_pd3dDevice, L"..\\stone.bmp", &g_pTexture ) ) )
		{
			MessageBox( NULL, L"Could not find stone.bmp", L"DirectX9HomeWork1.exe", MB_OK );
		}
	}
}

// return 0 for one second, and return 1 for next one second, repeat.
int HeartBeatOneSecond()
{
	int second = (int)( timeGetTime() / 1000.0f );

	return (( second % 2 ) ? 1 : 0 );
}


VOID Render()
{
	g_pd3dDevice->Clear( 0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER,
		D3DCOLOR_XRGB( 0, 0, 255 ), 1.0f, 0 );

	// Begin the scene
	if ( SUCCEEDED( g_pd3dDevice->BeginScene() ) )
	{
		// Setup the world, view, and projection matrices
		SetupCylinderMatrices();

		// Setup the Lights and materials
		SetupLightEnvironment();

		// swtich Light every one second
		if ( HeartBeatOneSecond() )
		{
			SetupLights1();
			LoadBananaTexture();
		}
		else
		{
			SetUpLights2();
			LoadStoneTexture();
		}

		g_pd3dDevice->SetTexture( 0, g_pTexture );
		g_pd3dDevice->SetTextureStageState( 0, D3DTSS_COLOROP, D3DTOP_MODULATE );
		g_pd3dDevice->SetTextureStageState( 0, D3DTSS_COLORARG1, D3DTA_TEXTURE );
		g_pd3dDevice->SetTextureStageState( 0, D3DTSS_COLORARG2, D3DTA_DIFFUSE );
		g_pd3dDevice->SetTextureStageState( 0, D3DTSS_ALPHAOP, D3DTOP_DISABLE );

		// Render the vertex buffer contents
		g_pd3dDevice->SetStreamSource( 0, g_pVB, 0, sizeof( CUSTOMVERTEX ) );
		g_pd3dDevice->SetFVF( D3DFVF_CUSTOMVERTEX );
		g_pd3dDevice->DrawPrimitive( D3DPT_TRIANGLESTRIP, 0, 2 * 50 - 2 );

		// setup Tiger matrix
		SetupTigerMatrices();

		// draw Tiger
		if ( SUCCEEDED( LoadMesh() ) )
		{
			for ( DWORD i = 0; i < g_dwNumMaterials; ++i )
			{
				g_pd3dDevice->SetMaterial( &g_pMeshMaterials[i] );
				g_pd3dDevice->SetTexture( 0, g_pMeshTextures[i] );

				g_pMesh->DrawSubset( i );
			}
		}

		// setup Tiger 2 matrix
		SetupTigerSecondMatrices();

		if ( SUCCEEDED( LoadSecondMesh() ) )
		{
			for ( DWORD i = 0; i < g_dwNumMaterialsSecond; ++i )
			{
				g_pd3dDevice->SetMaterial( &g_pMeshMaterialsSecond[i] );
				g_pd3dDevice->SetTexture( 0, g_pMeshTexturesSecond[i] );

				g_pMeshSecond->DrawSubset( i );
			}
		}

		// End the scene
		g_pd3dDevice->EndScene();
	}

	// Present the backbuffer contents to the display
	g_pd3dDevice->Present( NULL, NULL, NULL, NULL );
}


LRESULT WINAPI MsgProc( HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam )
{
	switch ( msg )
	{
	case WM_DESTROY:
		Cleanup();
		PostQuitMessage( 0 );
		return 0;
	}

	return DefWindowProc( hWnd, msg, wParam, lParam );
}


INT WINAPI wWinMain( HINSTANCE hInst, HINSTANCE, LPWSTR, INT )
{
	UNREFERENCED_PARAMETER( hInst );

	WNDCLASSEX wc =
	{
		sizeof( WNDCLASSEX ), CS_CLASSDC, MsgProc, 0L, 0L,
		GetModuleHandle( NULL ), NULL, NULL, NULL, NULL,
		L"D3D Tutorial", NULL
	};
	RegisterClassEx( &wc );

	HWND hWnd = CreateWindow( L"D3D Tutorial", L"D3D Tutorial 04: Lights",
		WS_OVERLAPPEDWINDOW, 100, 100, 300, 300,
		NULL, NULL, wc.hInstance, NULL );

	// Initialize Direct3D
	if ( SUCCEEDED( InitD3D( hWnd ) ) )
	{
		if ( SUCCEEDED( InitGeometry() ) )
		{
			ShowWindow( hWnd, SW_SHOWDEFAULT );
			UpdateWindow( hWnd );

			MSG msg;
			ZeroMemory( &msg, sizeof( msg ) );
			while ( msg.message != WM_QUIT )
			{
				if ( PeekMessage( &msg, NULL, 0U, 0U, PM_REMOVE ) )
				{
					TranslateMessage( &msg );
					DispatchMessage( &msg );
				}
				else
					Render();
			}
		}
	}

	UnregisterClass( L"D3D Tutorial", wc.hInstance );
	return 0;
}



