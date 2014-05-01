#pragma once
#include <d3dx9.h>
#pragma warning( disable : 4996 ) // disable deprecated warning 
#include <strsafe.h>
#pragma warning( default : 4996 )
#include <windows.h>

struct AABBCollisionBox
{
	D3DXVECTOR3  m_min;
	D3DXVECTOR3  m_max;
};

struct OBBCollisionBox
{
	D3DXVECTOR3	m_AxisDirection[3];
	float		m_AxisLength[3];
	D3DXVECTOR3 m_CenterPos;
};

class GameObject
{
public:
	GameObject();
	virtual ~GameObject();
	void Init( LPDIRECT3DDEVICE9& d3dDevice );
	void Render( LPDIRECT3DDEVICE9& d3dDevice );
	void Release();
	void SetPosition( D3DXVECTOR3 );

	void update();

	void MoveX( float x );
	void MoveY( float y );
	void MoveZ( float z );
	void Move( float x, float y, float z );

	AABBCollisionBox getAABBBox() { return m_AABBBox; }
	OBBCollisionBox getOBBBox() { return m_OBBBox; }

private:
	void setBox();
	void updateAABBBox();
	void InitMesh( LPDIRECT3DDEVICE9& d3dDevice );

	// mesh
	LPD3DXMESH		m_pMesh;
	
	DWORD			m_NumMaterial = 0;
	D3DMATERIAL9*	m_pMeshMaterials;
	LPDIRECT3DTEXTURE9* m_pMeshTextures;

	// AABB
	AABBCollisionBox m_AABBBox;

	// OBB
	OBBCollisionBox m_OBBBox;

	// Matrix
	D3DXMATRIX	m_Matrix;
	D3DXVECTOR3	m_Position{ 0.f, 0.f, 0.f };
	D3DXVECTOR3	m_Rotation{ 0.f, 0.f, 0.f };
};

