#pragma once
#include "GameObject.h"
#include <array>

LPDIRECT3D9			g_pD3D = NULL;
LPDIRECT3DDEVICE9	g_pd3dDevice = NULL;
LPD3DXFONT g_pFont = nullptr;

bool g_IsAABBMode = true;

std::array<GameObject, 2> objectList;

HRESULT InitD3D( HWND hWnd )
{
	if ( NULL == ( g_pD3D = Direct3DCreate9( D3D_SDK_VERSION ) ) )
		return false;

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

	g_pd3dDevice->SetRenderState( D3DRS_ZENABLE, TRUE );

	g_pd3dDevice->SetRenderState( D3DRS_AMBIENT, 0xffffffff );

	D3DXCreateFont( g_pd3dDevice, 15, 0, FW_NORMAL, 1, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS,
		DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, L"돋음체", &g_pFont );

	return S_OK;
}

VOID SetupMatrices()
{
	D3DXMATRIXA16 matWorld;

	D3DXVECTOR3 vEyePt( 0.0f, 3.0f, -10.0f );
	D3DXVECTOR3 vLookatPt( 0.0f, 0.0f, 0.0f );
	D3DXVECTOR3 vUpVec( 0.0f, 1.0f, 0.0f );
	D3DXMATRIXA16 matView;
	D3DXMatrixLookAtLH( &matView, &vEyePt, &vLookatPt, &vUpVec );
	g_pd3dDevice->SetTransform( D3DTS_VIEW, &matView );

	D3DXMATRIXA16 matProj;
	D3DXMatrixPerspectiveFovLH( &matProj, D3DX_PI / 4, 1.0f, 1.0f, 100.0f );
	g_pd3dDevice->SetTransform( D3DTS_PROJECTION, &matProj );
}

VOID InitGeometry()
{
	for ( int i = 0; i < objectList.size(); ++i )
	{
		objectList[i].Init( g_pd3dDevice );
	}
}

VOID Cleanup()
{
	if ( g_pd3dDevice != NULL )
		g_pd3dDevice->Release();

	if ( g_pD3D != NULL )
		g_pD3D->Release();

	for ( int i = 0; i < objectList.size(); ++i )
	{
		objectList[i].Release();
	}
}

bool checkAABB( AABBCollisionBox& abbox1, AABBCollisionBox& abbox2 )
{
	return ( 
		abbox1.m_max.x >= abbox2.m_min.x && abbox1.m_min.x <= abbox2.m_max.x
		&& abbox1.m_max.y >= abbox2.m_min.y && abbox1.m_min.y <= abbox2.m_max.y
		&& abbox1.m_max.z >= abbox2.m_min.z && abbox1.m_min.z <= abbox2.m_max.z );
}

//http://honria.egloos.com/m/770651
// 흠 설명이.. 되게 자세하네요
bool checkOBB( OBBCollisionBox& obbox1, OBBCollisionBox& obbox2 )
{
	float c[3][3];
	float absC[3][3];
	float d[3];
	float r0, r1, r;

	const float cutoff = 0.999999;
	bool existsParallelPair = false;
	D3DXVECTOR3 diff = obbox1.m_CenterPos - obbox2.m_CenterPos;

	for ( int i = 0; i < 3; ++i )
	{
		c[0][i] = D3DXVec3Dot( &obbox1.m_AxisDirection[0], &obbox2.m_AxisDirection[i] );
		absC[0][i] = abs( c[0][i] );
		if ( absC[0][i] > cutoff )
			existsParallelPair = true;
	}

	d[0] = D3DXVec3Dot( &diff, &obbox1.m_AxisDirection[0] );
	r = abs( d[0] );
	r0 = obbox1.m_AxisLength[0];
	r1 = obbox2.m_AxisLength[0] * absC[0][0] + obbox2.m_AxisLength[1] * absC[0][1] + obbox2.m_AxisLength[2] * absC[0][2];
	if ( r > r0 + r1 )
		return FALSE;

	for ( int i = 0; i < 3; ++i )
	{
		c[1][i] = D3DXVec3Dot( &obbox1.m_AxisDirection[1], &obbox2.m_AxisDirection[i] );
		absC[1][i] = abs( c[1][i] );
		if ( absC[1][i] > cutoff )
			existsParallelPair = true;
	}

	d[1] = D3DXVec3Dot( &diff, &obbox1.m_AxisDirection[1] );
	r = abs( d[1] );
	r0 = obbox1.m_AxisLength[1];
	r1 = obbox2.m_AxisLength[0] * absC[1][0] + obbox2.m_AxisLength[1] * absC[1][1] + obbox2.m_AxisLength[2] * absC[1][2];
	if ( r > r0 + r1 )
		return FALSE;

	for ( int i = 0; i < 3; ++i )
	{
		c[2][i] = D3DXVec3Dot( &obbox1.m_AxisDirection[2], &obbox2.m_AxisDirection[i] );
		absC[2][i] = abs( c[2][i] );
		if ( absC[2][i] > cutoff )
			existsParallelPair = true;
	}

	d[2] = D3DXVec3Dot( &diff, &obbox1.m_AxisDirection[2] );
	r = abs( d[2] );
	r0 = obbox1.m_AxisLength[2];
	r1 = obbox2.m_AxisLength[0] * absC[2][0] + obbox2.m_AxisLength[1] * absC[2][1] + obbox2.m_AxisLength[2] * absC[2][2];
	if ( r > r0 + r1 )
		return FALSE;

	r = abs( D3DXVec3Dot( &diff, &obbox2.m_AxisDirection[0] ) );
	r0 = obbox1.m_AxisLength[0] * absC[0][0] + obbox1.m_AxisLength[1] * absC[1][0] + obbox1.m_AxisLength[2] * absC[2][0];
	r1 = obbox2.m_AxisLength[0];
	if ( r > r0 + r1 )
		return FALSE;

	r = abs( D3DXVec3Dot( &diff, &obbox2.m_AxisDirection[1] ) );
	r0 = obbox1.m_AxisLength[0] * absC[0][1] + obbox1.m_AxisLength[1] * absC[1][1] + obbox1.m_AxisLength[2] * absC[2][1];
	r1 = obbox2.m_AxisLength[1];
	if ( r > r0 + r1 )
		return FALSE;

	r = abs( D3DXVec3Dot( &diff, &obbox2.m_AxisDirection[2] ) );
	r0 = obbox1.m_AxisLength[0] * absC[0][2] + obbox1.m_AxisLength[1] * absC[1][2] + obbox1.m_AxisLength[2] * absC[2][2];
	r1 = obbox2.m_AxisLength[2];
	if ( r > r0 + r1 )
		return FALSE;

	if ( existsParallelPair == true )
		return TRUE;

	r = abs( d[2] * c[1][0] - d[1] * c[2][0] );
	r0 = obbox1.m_AxisLength[1] * absC[2][0] + obbox1.m_AxisLength[2] * absC[1][0];
	r1 = obbox2.m_AxisLength[1] * absC[0][2] + obbox2.m_AxisLength[2] * absC[0][1];
	if ( r > r0 + r1 )
		return FALSE;

	r = abs( d[2] * c[1][1] - d[1] * c[2][1] );
	r0 = obbox1.m_AxisLength[1] * absC[2][1] + obbox1.m_AxisLength[2] * absC[1][1];
	r1 = obbox2.m_AxisLength[0] * absC[0][2] + obbox2.m_AxisLength[2] * absC[0][0];
	if ( r > r0 + r1 )
		return FALSE;

	r = abs( d[2] * c[1][2] - d[1] * c[2][2] );
	r0 = obbox1.m_AxisLength[1] * absC[2][2] + obbox1.m_AxisLength[2] * absC[1][2];
	r1 = obbox2.m_AxisLength[0] * absC[0][1] + obbox2.m_AxisLength[1] * absC[0][0];
	if ( r > r0 + r1 )
		return FALSE;

	r = abs( d[0] * c[2][0] - d[2] * c[0][0] );
	r0 = obbox1.m_AxisLength[0] * absC[2][0] + obbox1.m_AxisLength[2] * absC[0][0];
	r1 = obbox2.m_AxisLength[1] * absC[1][2] + obbox2.m_AxisLength[2] * absC[1][1];
	if ( r > r0 + r1 )
		return FALSE;

	r = abs( d[0] * c[2][1] - d[2] * c[0][1] );
	r0 = obbox1.m_AxisLength[0] * absC[2][1] + obbox1.m_AxisLength[2] * absC[0][1];
	r1 = obbox2.m_AxisLength[0] * absC[1][2] + obbox2.m_AxisLength[2] * absC[1][0];
	if ( r > r0 + r1 )
		return FALSE;

	r = abs( d[0] * c[2][2] - d[2] * c[0][2] );
	r0 = obbox1.m_AxisLength[0] * absC[2][2] + obbox1.m_AxisLength[2] * absC[0][2];
	r1 = obbox2.m_AxisLength[0] * absC[1][1] + obbox2.m_AxisLength[1] * absC[1][0];
	if ( r > r0 + r1 )
		return FALSE;

	r = abs( d[1] * c[0][0] - d[0] * c[1][0] );
	r0 = obbox1.m_AxisLength[0] * absC[1][0] + obbox1.m_AxisLength[1] * absC[0][0];
	r1 = obbox2.m_AxisLength[1] * absC[2][2] + obbox2.m_AxisLength[2] * absC[2][1];
	if ( r > r0 + r1 )
		return FALSE;

	r = abs( d[1] * c[0][1] - d[0] * c[1][1] );
	r0 = obbox1.m_AxisLength[0] * absC[1][1] + obbox1.m_AxisLength[1] * absC[0][1];
	r1 = obbox2.m_AxisLength[0] * absC[2][2] + obbox2.m_AxisLength[2] * absC[2][0];
	if ( r > r0 + r1 )
		return FALSE;

	r = abs( d[1] * c[0][2] - d[0] * c[1][2] );
	r0 = obbox1.m_AxisLength[0] * absC[1][2] + obbox1.m_AxisLength[1] * absC[0][2];
	r1 = obbox2.m_AxisLength[0] * absC[2][1] + obbox2.m_AxisLength[1] * absC[2][0];
	if ( r > r0 + r1 )
		return FALSE;

	return TRUE;
}

VOID Render()
{
	RECT textBox;
	SetRect( &textBox, 15, 15, 0, 0 );

	if ( NULL == g_pd3dDevice )
		return;

	g_pd3dDevice->Clear( 0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER,
		D3DCOLOR_XRGB( 0, 0, 0 ), 1.0f, 0 );

	if ( SUCCEEDED( g_pd3dDevice->BeginScene() ) )
	{
		// render
		for ( int i = 0; i < objectList.size(); ++i )
		{
			objectList[i].Render( g_pd3dDevice );
		}

		std::wstring status;

		if ( g_IsAABBMode )
		{
			status = L"AABB모드 (Q키 누르면 바뀝니다 )";
			if ( checkAABB( objectList[0].getAABBBox(), objectList[1].getAABBBox() ) )
				status.append( L" 충돌이네요! " );
		}
		else
		{
			status = L"OBB모드 (Q키 누르면 바뀝니다 )";
			if ( checkOBB( objectList[0].getOBBBox(), objectList[1].getOBBBox() ) )
				status.append( L" 충돌이네요! " );
		}

		g_pFont->DrawTextW( NULL, status.c_str(), -1, &textBox, DT_NOCLIP, D3DXCOLOR( 1.f, 1.f, 1.f, 1.f ) );

		g_pd3dDevice->EndScene();
	}

	g_pd3dDevice->Present( NULL, NULL, NULL, NULL );
}


void KeyboardProc( char input )
{
	switch ( input )
	{
	case 'w':
		objectList[0].MoveZ(0.1f);
		break;
	case 's':
		objectList[0].MoveZ( -0.1f );
		break;
	case 'a':
		objectList[0].MoveX( -0.1f );
		break;
	case 'd':
		objectList[0].MoveX( 0.1f );
		break;
	case 'q':
		g_IsAABBMode = !g_IsAABBMode;
		break;
	default:
		break;
	}
}

void Update()
{
	for ( int i = 0; i < objectList.size(); ++i )
	{
		objectList[i].update();
	}
}

LRESULT WINAPI MsgProc( HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam )
{
	switch ( msg )
	{
	case WM_CHAR:
		KeyboardProc( (char)wParam );
		return 0;

	case WM_DESTROY:
		Cleanup();
		PostQuitMessage( 0 );
		return 0;

	case WM_PAINT:
		Update();
		Render();
		ValidateRect( hWnd, NULL );
		return 0;
	}

	return DefWindowProc( hWnd, msg, wParam, lParam );
}

INT WINAPI WinMain( HINSTANCE hInst, HINSTANCE, LPSTR, INT )
{
	WNDCLASSEX wc = { 
		sizeof( WNDCLASSEX ), CS_CLASSDC, MsgProc, 0L, 0L,
		GetModuleHandle( NULL ), NULL, NULL, NULL, NULL,
		L"Collision", NULL
	};

	RegisterClassEx( &wc );

	HWND hWnd = CreateWindow( L"Collision", L"AABB/OBB Collision",
		WS_OVERLAPPEDWINDOW, 100, 100, 600, 600,
		GetDesktopWindow(), NULL, wc.hInstance, NULL );
	

	if ( SUCCEEDED( InitD3D( hWnd ) ) )
	{
		InitGeometry();
		SetupMatrices();

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
			{
				Render();
			}
		}
	}

	UnregisterClass( L"Collision", wc.hInstance );
	return 0;
}