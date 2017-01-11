/////////////////////////////////////////////////////////////////////////////////////////////////
#include "StdAfx.h"
#include "phcharacter.h"
#include "Physics.h"
#include "ExtendedGeom.h"
#include "PHCapture.h"
#include "Entity.h"
#include "inventory_item.h"
#include "../Include/xrRender/Kinematics.h"
#include "Actor.h"
#include "Inventory.h"
#include "CaptureBoneCallback.h"
extern	class CPHWorld	*ph_world;
///////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////
bool can_capture( CPHCharacter   *a_character, CPhysicsShellHolder	*a_taget_object  )
{
	if(!a_taget_object									||
	   !a_taget_object->m_pPhysicsShell					||
	   !a_taget_object->m_pPhysicsShell->isActive()		||
	   smart_cast<CInventoryItem*>(a_taget_object)		||
	   !a_character										||
	   !a_character->b_exist							||
	   !a_character->PhysicsRefObject()					||
	   !a_character->PhysicsRefObject()->Visual( )
	   ) return false;

	IKinematics* p_kinematics = smart_cast<IKinematics*>( a_character->PhysicsRefObject()->Visual( ) );
	VERIFY( p_kinematics );
	CInifile* ini = p_kinematics->LL_UserData();
	if(!ini								||
	   !ini->section_exist( "capture" )
	   ) return false;
	return true;
}

bool can_capture( CPHCharacter   *a_character, CPhysicsShellHolder	*a_taget_object, u16 a_taget_element  ) 
{

	if( !can_capture( a_character, a_taget_object)	||
		a_taget_element == BI_NONE					|| 
		!a_taget_object->Visual() 
		) return false;

	IKinematics* K=	smart_cast<IKinematics*>( a_taget_object->Visual( ) );

	if(!K || !K->LL_GetBoneInstance(a_taget_element).callback_param() )
		return false;
	return true;
}

static CBoneInstance * get_capture_bone( CPHCharacter   *a_character )
{
	VERIFY( a_character );
	VERIFY( a_character->PhysicsRefObject() );
	IKinematics* p_kinematics = smart_cast<IKinematics*>( a_character->PhysicsRefObject()->Visual( ) );
	VERIFY( p_kinematics );
	CInifile* ini = p_kinematics->LL_UserData();
	VERIFY( ini );
	VERIFY( ini->section_exist( "capture" ) );
	u16 capture_bone_id		=p_kinematics->LL_BoneID( ini->r_string( "capture", "bone" ) );

	R_ASSERT2( capture_bone_id != BI_NONE, "wrong capture bone" );

	return &p_kinematics->LL_GetBoneInstance( capture_bone_id );

}



CPHCapture::CPHCapture	( CPHCharacter   *a_character, CPhysicsShellHolder	*a_taget_object, CPHCaptureBoneCallback* cb /*=0*/ ):
	m_joint					( NULL )			,	
	m_ajoint				( NULL )			,
	m_body					( NULL )			,
	m_taget_object			( a_taget_object )	,
	m_character				( a_character )		,
	b_disabled				( false )			,	
	b_character_feedback	( false )			,
	e_state					( cstFree )
{
	if( !can_capture( a_character, a_taget_object) )
		return;

	VERIFY( m_taget_object );
	VERIFY( m_taget_object->m_pPhysicsShell );
	VERIFY( a_character );
	m_capture_bone			=get_capture_bone( a_character );
	VERIFY( m_capture_bone );
	m_taget_element			=m_taget_object->m_pPhysicsShell->NearestToPoint( m_capture_bone->mTransform.c, cb );
	if( ! m_taget_element )
		return;
	Init( );
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////
	CPHCapture::CPHCapture(CPHCharacter   *a_character,CPhysicsShellHolder	*a_taget_object,u16 a_taget_element):
	m_joint					( NULL )			,	
	m_ajoint				( NULL )			,
	m_body					( NULL )			,
	b_disabled				( false )			,
	b_character_feedback	( false )			,
	m_taget_object			( a_taget_object )	,
	m_character				( a_character )		,
	e_state					( cstFree )
{
	
	if( !can_capture( a_character, a_taget_object, a_taget_element ) )
		return;
	VERIFY( m_taget_object );
	VERIFY( a_character );
	m_capture_bone			=get_capture_bone( a_character ) ;
		
	IRenderVisual* V=m_taget_object->Visual( );
	VERIFY( V );

	IKinematics* K=	smart_cast<IKinematics*>( m_taget_object->Visual( ) );
	VERIFY( K );

	CBoneInstance& tag_bone=K->LL_GetBoneInstance( a_taget_element );
	VERIFY( tag_bone.callback_param( ) );

	m_taget_element					=(CPhysicsElement*)tag_bone.callback_param();
	VERIFY( m_taget_element );

	Init( );
}

void CPHCapture::Init( )
{
	VERIFY( m_taget_element );
	VERIFY( m_character );
	VERIFY( m_character->PhysicsRefObject() );
	VERIFY( m_character->PhysicsRefObject()->Visual( ) );
	VERIFY( m_capture_bone );

	IKinematics* p_kinematics = smart_cast<IKinematics*>( m_character->PhysicsRefObject()->Visual( ) );
	VERIFY( p_kinematics );
	CInifile	* ini = p_kinematics->LL_UserData();
	VERIFY( ini );

	Fvector dir;
	Fvector capture_bone_position;
	capture_bone_position.set(m_capture_bone->mTransform.c);
	b_character_feedback=true;
	(m_character->PhysicsRefObject())->XFORM().transform_tiny(capture_bone_position);

	m_taget_element->GetGlobalPositionDynamic(&dir);
	dir.sub(capture_bone_position,dir);

	m_pull_distance=ini->r_float("capture","pull_distance");
	if(dir.magnitude()>m_pull_distance)
		return;

	float 					pool_force_factor=4.f;
	m_capture_distance		=ini->r_float("capture","distance");					//distance
	m_capture_force			=ini->r_float("capture","capture_force");				//capture force
	m_capture_time			=ini->r_u32("capture","time_limit")*1000;				//time;		
	m_time_start			=Device.dwTimeGlobal;
	float max_pull_force    =ini->r_float("capture","pull_force");					//pull force
	m_pull_force			=pool_force_factor*ph_world->Gravity()*m_taget_element->PhysicsShell()->getMass();
	if(m_pull_force>max_pull_force) 
		m_pull_force=max_pull_force;
	float pulling_vel_scale =ini->r_float("capture","velocity_scale");				//

	m_taget_element->set_DynamicLimits(default_l_limit*pulling_vel_scale,default_w_limit*pulling_vel_scale);
	//m_taget_element->PhysicsShell()->set_ObjectContactCallback(object_contactCallbackFun);
	m_character->SetObjectContactCallback(object_contactCallbackFun);
	m_island.Init();
	CActor* A=smart_cast<CActor*>(m_character->PhysicsRefObject());
	if(A)
	{
		A->SetWeaponHideState(INV_STATE_BLOCK_ALL,true);
	}
	CPHUpdateObject::Activate();
	e_state = cstPulling;
}

void CPHCapture::Release()
{
	if( e_state==cstReleased || e_state== cstFree ) 
		return;
	if( m_joint ) 
	{
		m_island.RemoveJoint(m_joint);
		dJointDestroy(m_joint);
	}
	m_joint=NULL;
	if( m_ajoint )
	{
		m_island.RemoveJoint( m_ajoint );
		dJointDestroy(m_ajoint);
	}
	m_ajoint=NULL;

	if(m_body) 
	{
		m_island.RemoveBody( m_body );
		dBodyDestroy(m_body);
	}
	m_body=NULL;

	if(e_state==cstPulling&&m_taget_element&&!m_taget_object->getDestroy()&&m_taget_object->PPhysicsShell()&&m_taget_object->PPhysicsShell()->isActive())
	{
		m_taget_element->set_DynamicLimits();
	}

	b_collide=true;
	CActor* A=smart_cast<CActor*>(m_character->PhysicsRefObject());
	if(A)
	{
		A->SetWeaponHideState(INV_STATE_BLOCK_ALL,false);
//.		A->inventory().setSlotsBlocked(false);
	}

	e_state=cstReleased;
}

void CPHCapture::Deactivate()
{
	Release();
	//if(m_taget_element)
	//	m_taget_element->Enable();
	//if(m_taget_object&&m_taget_element&&!m_taget_object->getDestroy()&&m_taget_object->m_pPhysicsShell&&m_taget_object->m_pPhysicsShell->isActive())
	//{
	//	m_taget_element->set_ObjectContactCallback(0);

	//}
	if(m_character)
		m_character->SetObjectContactCallback(0);
	CPHUpdateObject::Deactivate();
	e_state			=cstFree;
	m_character		=NULL;
	m_taget_object	=NULL;
	m_taget_element	=NULL;
}