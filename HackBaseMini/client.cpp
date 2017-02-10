#include "main.h"

void HookUserMessages()
{
	pUserMsgBase = c_Offset.FindUserMsgBase();

	pResetHUD = HookUserMsg( "ResetHUD" , ResetHUD );
	pSetFOV = HookUserMsg( "SetFOV" , SetFOV );
	pTeamInfo = HookUserMsg( "TeamInfo" , TeamInfo );
	pCurWeapon = HookUserMsg( "CurWeapon" , CurWeapon );
	pDeathMsg = HookUserMsg( "DeathMsg" , DeathMsg );
}

void InitHack()
{
	g_Engine.Con_Printf( "Base loaded\n" );
}

void HUD_Frame( double time )
{
	static bool FirstFrame = true;

	if ( FirstFrame )
	{
		g_Screen.iSize = sizeof( SCREENINFO );
		g_Engine.pfnGetScreenInfo( &g_Screen );
		HookUserMessages();
		InitHack();
		FirstFrame = false;
	}

	g_Engine.pfnGetScreenInfo( &g_Screen );
	return g_Client.HUD_Frame( time );
}

void HUD_Redraw( float time , int intermission )
{
	g_Client.HUD_Redraw( time , intermission );
}

int HUD_Key_Event( int down , int keynum , const char *pszCurrentBinding )
{
	return g_Client.HUD_Key_Event( down , keynum , pszCurrentBinding );
}

void CL_CreateMove( float frametime , usercmd_s *cmd , int active )
{
	g_Client.CL_CreateMove( frametime , cmd , active );

}

void HookFunction()
{
	g_pClient->HUD_Frame = HUD_Frame;
	g_pClient->HUD_Redraw = HUD_Redraw;
	g_pClient->HUD_Key_Event = HUD_Key_Event;
	g_pClient->CL_CreateMove = CL_CreateMove;
}