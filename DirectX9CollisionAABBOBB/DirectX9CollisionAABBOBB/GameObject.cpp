#include "GameObject.h"


GameObject::GameObject()
{
}


GameObject::~GameObject()
{
}

void GameObject::Init( LPDIRECT3DDEVICE9& d3dDevice )
{
	D3DXMatrixIdentity( &m_Matrix );

	InitMesh( d3dDevice );
	setBox();
}

// AABB, OBB 박스 두 개 모두 설정한다
void GameObject::setBox()
{
	float minX, minY, minZ;
	float maxX, maxY, maxZ;

	minX = minY = minZ = HUGE;
	maxX = maxY = maxZ = -HUGE;

	D3DXVECTOR3* pVertex;

	if ( SUCCEEDED( m_pMesh->LockVertexBuffer( D3DLOCK_READONLY, (VOID**)&pVertex ) ) )
	{
		for ( DWORD i = 0; i < m_pMesh->GetNumVertices(); ++i )
		{
			maxX = ( maxX < pVertex[i].x ? pVertex[i].x : maxX );
			maxY = ( maxY < pVertex[i].y ? pVertex[i].y : maxY );
			maxZ = ( maxZ < pVertex[i].z ? pVertex[i].z : maxZ );

			minX = ( minX < pVertex[i].x ? minX : pVertex[i].x );
			minY = ( minY < pVertex[i].y ? minY : pVertex[i].y );
			minZ = ( minZ < pVertex[i].z ? minZ : pVertex[i].z );
		}
	}
	
	m_pMesh->UnlockVertexBuffer();

	m_AABBBox.m_min = D3DXVECTOR3( minX, minY, minZ );
	m_AABBBox.m_max = D3DXVECTOR3( maxX, maxY, maxZ );

	// obb
	m_OBBBox.m_CenterPos = D3DXVECTOR3( ( minX + maxX ) / 2, ( minY + maxY ) / 2, ( minZ + maxZ ) / 2 );
	m_OBBBox.m_AxisDirection->x = ( maxX - minX ) / abs( maxX - minX );
	m_OBBBox.m_AxisDirection->y = ( maxY - minY ) / abs( maxY - minY );
	m_OBBBox.m_AxisDirection->z = ( maxZ - minZ ) / abs( maxZ - minZ );
	
	m_OBBBox.m_AxisLength[0] = abs( maxX - minX ) / 2;
	m_OBBBox.m_AxisLength[1] = abs( maxY - minY ) / 2;
	m_OBBBox.m_AxisLength[2] = abs( maxZ - minZ ) / 2;
}

void GameObject::updateAABBBox()
{
	D3DXVECTOR3 modifiedVertex;
	D3DXVECTOR4 tempMat;

	float minX, minY, minZ;
	float maxX, maxY, maxZ;

	minX = minY = minZ = HUGE;
	maxX = maxY = maxZ = -HUGE;

	D3DXVECTOR3* pVertex;
	if ( SUCCEEDED( m_pMesh->LockVertexBuffer( D3DLOCK_READONLY, (VOID**)&pVertex ) ) )
	{
		for ( DWORD i = 0; i < m_pMesh->GetNumVertices(); ++i )
		{
			D3DXVec3Transform( &tempMat, &pVertex[i], &m_Matrix );
			modifiedVertex = D3DXVECTOR3( tempMat.x, tempMat.y, tempMat.z );

			maxX = ( maxX < modifiedVertex.x ? modifiedVertex.x : maxX );
			maxY = ( maxY < modifiedVertex.y ? modifiedVertex.y : maxY );
			maxZ = ( maxZ < modifiedVertex.z ? modifiedVertex.z : maxZ );

			minX = ( minX < modifiedVertex.x ? minX : modifiedVertex.x );
			minY = ( minY < modifiedVertex.y ? minY : modifiedVertex.y );
			minZ = ( minZ < modifiedVertex.z ? minZ : modifiedVertex.z );
		}
	}

	m_AABBBox.m_min = D3DXVECTOR3( minX, minY, minZ );
	m_AABBBox.m_max = D3DXVECTOR3( maxX, maxY, maxZ );
}

void GameObject::InitMesh(LPDIRECT3DDEVICE9& d3dDevice)
{
	LPD3DXBUFFER pD3DXMtrlBuffer = nullptr;
	if ( d3dDevice )

	D3DXLoadMeshFromX( L"spaceMan.x", D3DXMESH_SYSTEMMEM,
		d3dDevice, NULL,
		&pD3DXMtrlBuffer, NULL, &m_NumMaterial,
		&m_pMesh );

	D3DXMATERIAL* d3dxMaterials = (D3DXMATERIAL*)pD3DXMtrlBuffer->GetBufferPointer();

	// skip error handling -.-;;
	m_pMeshMaterials = new D3DMATERIAL9[m_NumMaterial];
	m_pMeshTextures = new LPDIRECT3DTEXTURE9[m_NumMaterial];

	for ( DWORD i = 0; i < m_NumMaterial; ++i )
	{
		m_pMeshMaterials[i] = d3dxMaterials[i].MatD3D;

		m_pMeshMaterials[i].Ambient = m_pMeshMaterials[i].Diffuse;

		m_pMeshTextures[i] = NULL;
		
		D3DXCreateTextureFromFileA( d3dDevice,
			d3dxMaterials[i].pTextureFilename,
			&m_pMeshTextures[i] );
	}

	pD3DXMtrlBuffer->Release();
}

void GameObject::Render( LPDIRECT3DDEVICE9& d3dDevice )
{
	if ( d3dDevice->SetTransform( D3DTS_WORLD, &m_Matrix ) )
	{
		return;
	}

	for ( DWORD i = 0; i < m_NumMaterial; ++i )
	{
		d3dDevice->SetMaterial( &m_pMeshMaterials[i] );
		d3dDevice->SetTexture( 0, m_pMeshTextures[i] );

		m_pMesh->DrawSubset( i );
	}
}

void GameObject::Release()
{
	if ( m_pMeshMaterials != NULL )
	{
		delete[] m_pMeshMaterials;
	}

	if ( m_pMeshTextures )
	{
		for ( DWORD i = 0; i < m_NumMaterial; ++i )
		{
			if ( m_pMeshTextures[i] )
			{
				m_pMeshTextures[i]->Release();
			}
		}
		delete[] m_pMeshTextures;
	}

	if ( m_pMesh != NULL )
		m_pMesh->Release();
}

void GameObject::MoveX( float x )
{
	Move( x, 0, 0 );
}

void GameObject::MoveY( float y )
{
	Move( 0, y, 0 );
}

void GameObject::MoveZ( float z )
{
	Move( 0, 0, z );
}

void GameObject::update()
{

}

void GameObject::Move( float x, float y, float z )
{
	m_Position.x += x;
	m_Position.y += y;
	m_Position.z += z;

	D3DXMatrixTransformation( &m_Matrix, NULL, NULL, NULL, NULL, NULL, &m_Position );

	updateAABBBox();

	// update obb :: 회전이 없을땐 편하다
	m_OBBBox.m_CenterPos.x += x;
	m_OBBBox.m_CenterPos.y += y;
	m_OBBBox.m_CenterPos.z += z;
}
