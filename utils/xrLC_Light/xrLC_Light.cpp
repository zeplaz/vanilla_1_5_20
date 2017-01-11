// xrLC_Light.cpp : Defines the entry point for the DLL application.
//
#include "stdafx.h"
#include "xrLc_globaldata.h"
#pragma comment(lib,"xrCore.lib")
#pragma comment(lib,"xrCDB.lib")

#ifdef _MANAGED
#pragma managed(push, off)
#endif

i_lc_log *lc_log	= 0;


void __cdecl clMsg( const char *format, ...)
{
	va_list		mark;
	char buf	[4*256];
	va_start	( mark, format );
	vsprintf	( buf, format, mark );

	VERIFY			( lc_log );
	lc_log->clMsg	( buf );
}

void __cdecl Status	(const char *format, ...)
{
	va_list				mark;
	va_start			( mark, format );

	char				status	[1024	]	="";
	vsprintf			( status, format, mark );
	//strconcat			( sizeof(status), status, "    | %s", status ); 
	VERIFY				( lc_log );
	lc_log->Status		(status);
	
}
void Phase		( LPCSTR phase_name )
{
	VERIFY				( lc_log );
	lc_log->Phase		( phase_name );
}
void Progress	( const float F )
{
	VERIFY				( lc_log );
	lc_log->Progress		( F );
}

b_params	&g_params()
{
	VERIFY(lc_global_data());
	return lc_global_data()->g_params();
}

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
					 )
{
    return TRUE;
}

#ifdef _MANAGED
#pragma managed(pop)
#endif

