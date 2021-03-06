/* Copyright Steve Rabin, 2008. 
 * All rights reserved worldwide.
 *
 * This software is provided "as is" without express or implied
 * warranties. You may freely copy and compile this source into
 * applications you distribute provided that the copyright text
 * below is included in the resulting source code, for example:
 * "Portions Copyright Steve Rabin, 2008"
 */

#include <Stdafx.h>

GameObject::GameObject( objectID id, unsigned int type, char* name )
: m_markedForDeletion(false),
  m_stateMachineManager(0),
  m_body(0),
  m_movement(0),
  m_tiny(0),
  m_quoridor(0)
{
	m_id = id;
	m_type = type;
	
	if( strlen(name) < GAME_OBJECT_MAX_NAME_SIZE ) {
		strcpy( m_name, name );
	}
	else {
		strcpy( m_name, "invalid_name" );
		ASSERTMSG(0, "GameObject::GameObject - name is too long" );
	}
}

GameObject::~GameObject( void )
{
	if(m_stateMachineManager)
	{
		delete m_stateMachineManager;
	}
	if(m_body)
	{
		delete m_body;
	}
	if(m_movement)
	{
		delete m_movement;
	}
	if(m_tiny)
	{
		delete m_tiny;
	}
	if (m_quoridor)
	{
		delete m_quoridor;
	}
}

void GameObject::CreateStateMachineManager( void )
{
	m_stateMachineManager = new StateMachineManager( *this );
}

void GameObject::CreateMovement( void )
{
	m_movement = new Movement( *this ); 
}

void GameObject::CreateBody( int health, D3DXVECTOR3& pos )
{
	m_body = new Body( health, pos, *this );
}

void GameObject::CreateTiny( CMultiAnim *pMA, std::vector< CTiny* > *pv_pChars, CSoundManager *pSM, double dTimeCurrent )
{
	m_tiny = new CTiny( *this );
	if( m_tiny == NULL )
	{
		return;
	}

	if( SUCCEEDED( m_tiny->Setup( pMA, pv_pChars, pSM, dTimeCurrent ) ) )
	{
		m_tiny->SetSounds( true );
	}
	else
	{
		delete m_tiny;
	}
}


void GameObject::CreateQuoridor(bool type, TileQ pos)
{
	m_quoridor = new QuoridorPlayer(type, pos);
}

void GameObject::Initialize( void )
{

}

/*---------------------------------------------------------------------------*
  Name:         Update

  Description:  Calls the update function of the current state machine.

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void GameObject::Update( void )
{
	if(m_stateMachineManager)
	{
		m_stateMachineManager->Update();
	}
	if (m_quoridor)
	{
		m_quoridor->Update();
	}
}

void GameObject::Animate( double dTimeDelta )
{
	if( m_movement )
	{
		m_movement->Animate( dTimeDelta );
	}
}

void GameObject::AdvanceTime( double dTimeDelta, D3DXVECTOR3 *pvEye )
{
	if( m_tiny )
	{
		m_tiny->AdvanceTime( dTimeDelta, pvEye );
	}
}

void GameObject::Draw( IDirect3DDevice9* pd3dDevice, D3DXMATRIX* pViewProj )
{
	if( m_tiny )
	{
		m_tiny->Draw();
	}
	if( m_movement )
	{
		m_movement->DrawDebugVisualization( pd3dDevice, pViewProj );
	}
}

void GameObject::RestoreDeviceObjects( LPDIRECT3DDEVICE9 pd3dDevice )
{
	if( m_tiny )
	{
		m_tiny->RestoreDeviceObjects( pd3dDevice );
	}
}

void GameObject::InvalidateDeviceObjects( void )
{

}

