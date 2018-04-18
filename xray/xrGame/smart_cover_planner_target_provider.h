////////////////////////////////////////////////////////////////////////////
//	Module 		: smart_cover_planner_target_provider.h
//	Created 	: 18.09.2007
//	Author		: Alexander Dudin
//	Description : Target provider for target selector
////////////////////////////////////////////////////////////////////////////

#ifndef SMART_COVER_PLANNER_TARGET_PROVIDER_H_INCLUDED
#define SMART_COVER_PLANNER_TARGET_PROVIDER_H_INCLUDED

#include "smart_cover_detail.h"
#include "action_base.h"
#include "smart_cover_planner_target_selector.h"
#include "stalker_decision_space.h"

namespace smart_cover {

class target_provider : 
	public	CActionBase<animation_planner>
{
private:
	typedef CActionBase<animation_planner> inherited;

public:
						target_provider					(animation_planner *object, LPCSTR name, StalkerDecisionSpace::EWorldProperties const &world_property, u32 const &loophole_value);
						//non copyable
						target_provider(const target_provider&) = delete;
						target_provider& operator=(const target_provider&) = delete;

	virtual	void		setup							(animation_planner *object, CPropertyStorage *storage);
	virtual	void		initialize						();
	virtual void		finalize						();

private:
	StalkerDecisionSpace::EWorldProperties m_world_property;
	u32					m_loophole_value;
};

class target_fire_no_lookout final: public target_provider
{
private:
	typedef target_provider inherited;

public:
						target_fire_no_lookout			(animation_planner *object, LPCSTR name, StalkerDecisionSpace::EWorldProperties const &world_property, u32 const &loophole_value);
	virtual	void		initialize						();

private:
	StalkerDecisionSpace::EWorldProperties m_world_property;
	u32					m_loophole_value;
};

} // namespace smart_cover

#endif // SMART_COVER_PLANNER_TARGET_PROVIDER_H_INCLUDED