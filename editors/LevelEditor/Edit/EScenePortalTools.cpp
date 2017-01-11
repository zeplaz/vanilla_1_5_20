#include "stdafx.h"
#pragma hdrstop

#include "EScenePortalTools.h"
#include "ui_leveltools.h"
#include "FramePortal.h"
#include "EScenePortalControls.h"
#include "portal.h"

/* TODO 1 -oAlexMX -cTODO: Create tools as AI Map */

void EScenePortalTool::CreateControls()
{
//	inherited::CreateControls();
    AddControl		(xr_new<TUI_ControlPortalSelect>(estDefault,etaSelect,	this));
	// frame
    pFrame 			= xr_new<TfraPortal>((TComponent*)0);
}
//----------------------------------------------------

void EScenePortalTool::RemoveControls()
{
	inherited::RemoveControls();
}
//----------------------------------------------------

void EScenePortalTool::FillProp(LPCSTR pref, PropItemVec& items)
{
	PHelper().CreateFlag32(items, PrepareKey(pref,"Common\\Draw Simple Model"),&m_Flags,			flDrawSimpleModel);
	inherited::FillProp	(pref, items);
}
//----------------------------------------------------

CCustomObject* EScenePortalTool::CreateObject(LPVOID data, LPCSTR name)
{
	CCustomObject* O	= xr_new<CPortal>(data,name);
    O->ParentTool		= this;
    return O;
}
//----------------------------------------------------

