#include "main.h"

AutoOffset c_Offset;

bool AutoOffset::GetRendererInfo()
{
	DWORD GameUI = (DWORD)GetModuleHandleA( "GameUI.dll" );
	DWORD vgui = (DWORD)GetModuleHandleA( "vgui.dll" );
	DWORD vgui2 = (DWORD)GetModuleHandleA( "vgui2.dll" );
	DWORD d3dim = (DWORD)GetModuleHandleA( "d3dim.dll" );

	HwBase = (DWORD)GetModuleHandleA( "hw.dll" ); // Hardware

	if ( HwBase == NULL )
	{
		HwBase = (DWORD)GetModuleHandleA( "sw.dll" ); // Software
		if ( HwBase == NULL )
		{
			HwBase = (DWORD)GetModuleHandleA( NULL ); // Non-Steam?
			if ( HwBase == NULL ) // Invalid module handle.
			{
				Error( "Invalid module handle." );
			}
			else
				HLType = RENDERTYPE_UNDEFINED;
		}
		else
			HLType = RENDERTYPE_SOFTWARE;
	}
	else
	{
		if ( d3dim == NULL )
			HLType = RENDERTYPE_HARDWARE;
		else
			HLType = RENDERTYPE_D3D;
	}

	HwSize = (DWORD)GetModuleSize( HwBase );

	if ( HwSize == NULL )
	{
		switch ( HwSize )
		{
		case RENDERTYPE_HARDWARE: HwSize = 0x122A000; break;
		case RENDERTYPE_UNDEFINED: HwSize = 0x2116000; break;
		case RENDERTYPE_SOFTWARE: HwSize = 0xB53000; break;
		default:Error( "Invalid renderer type." );
		}
	}

	HwEnd = HwBase + HwSize - 1;

	ClBase = (DWORD)GetModuleHandleA( "client.dll" );

	if ( ClBase != NULL ) {
		ClSize = (DWORD)GetModuleSize( ClBase );
		ClEnd = ClBase + ClSize - 1;
	}
	else {
		ClBase = HwBase;
		ClEnd = HwEnd;
		ClSize = HwSize;
	}

	if ( GameUI != NULL )
	{
		UiBase = GameUI;
		UiSize = (DWORD)GetModuleSize( UiBase );
		UiEnd = UiBase + UiSize - 1;
	}

	return ( HwBase && ClBase && GameUI && vgui && vgui2 );
}

DWORD AutoOffset::Absolute(DWORD Address)
{
	return Address + *(PDWORD)Address + 4;
}

void AutoOffset::Error( const PCHAR Msg )
{
	MessageBoxA( 0 , Msg , "Error" , MB_OK | MB_ICONERROR );
	ExitProcess( 0 );
}

DWORD AutoOffset::GetModuleSize( const DWORD Address )
{
	return PIMAGE_NT_HEADERS( Address + (DWORD)PIMAGE_DOS_HEADER( Address )->e_lfanew )->OptionalHeader.SizeOfImage;
}

DWORD AutoOffset::FarProc( const DWORD Address , DWORD LB , DWORD HB )
{
	return ( ( Address < LB ) || ( Address > HB ) );
}

DWORD AutoOffset::FindReference(DWORD start, DWORD end, DWORD Address)
{
	char szPattern[] = { 0x68 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00 };
	*(PDWORD)&szPattern[1] = Address;
	return FindPattern(szPattern, start, end, 0);
}

PVOID AutoOffset::ClientFuncs()
{
	PCHAR String = "ScreenFade";
	DWORD Address = FindPattern(String, HwBase, HwBase + HwSize, 0);
	PVOID ClientPtr = (PVOID)*(PDWORD)(FindReference(HwBase, HwBase + HwSize, Address) + 0x13); // all patch

	if (FarProc((DWORD)ClientPtr, HwBase, HwEnd))
		Error("Client address could not be found.");

	return ClientPtr;
}

PVOID AutoOffset::EngineFuncs()
{
	PVOID EnginePtr = (cl_enginefunc_t*)*(DWORD*)((DWORD)g_pClient->Initialize + 0x22); // old patch
	if (FarProc((DWORD)EnginePtr, HwBase, HwEnd))
	{
		EnginePtr = (cl_enginefunc_t*)*(DWORD*)((DWORD)g_pClient->Initialize + 0x1C); // new patch
		if (FarProc((DWORD)EnginePtr, ClBase, ClEnd))
		{
			EnginePtr = (cl_enginefunc_t*)*(DWORD*)((DWORD)g_pClient->Initialize + 0x1D); // steam
			if (FarProc((DWORD)EnginePtr, ClBase, ClEnd))
			{
				EnginePtr = (cl_enginefunc_t*)*(DWORD*)((DWORD)g_pClient->Initialize + 0x37); // hl-steam
				if (FarProc((DWORD)EnginePtr, ClBase, ClEnd))
				{
					Error("Engine address could not be found.");
				}
			}
		}
	}
	return EnginePtr;
}

PVOID AutoOffset::StudioFuncs()
{
	PVOID StudioPtr = (engine_studio_api_t*)*(DWORD*)((DWORD)g_pClient->HUD_GetStudioModelInterface + 0x30); // old patch
	if (FarProc((DWORD)StudioPtr, HwBase, HwEnd))
	{
		StudioPtr = (engine_studio_api_t*)*(DWORD*)((DWORD)g_pClient->HUD_GetStudioModelInterface + 0x1A); // new patch / steam
		if (FarProc((DWORD)StudioPtr, ClBase, ClEnd))
		{
			Error("Studio address could not be found.");
		}
	}
	return StudioPtr;
}

PUserMsg AutoOffset::FindUserMsgBase(void)
{
	BYTE Pattern_UserMsg[9] =
	{
		0x52 , 0x50 , 0xE8 , 0xFF , 0xFF , 0xFF , 0xFF , 0x83, 0x00
	};

	BYTE Pattern_UserMsg2[13] =
	{
		0xFF , 0xFF , 0xFF , 0x0C ,
		0x56 , 0x8B , 0x35 , 0xFF , 0xFF , 0xFF , 0xFF , 0x57, 0x00
	};

	DWORD Address = (DWORD)g_Engine.pfnHookUserMsg;

	DWORD UserMsgBase = Absolute(FindPattern((PCHAR)Pattern_UserMsg, "xxx????x", Address, Address + 0x32, 3));

	if (FarProc(UserMsgBase, HwBase, HwEnd))
	{
		Error("UserMsgBase #1 address could not be found.");
	}

	UserMsgBase = FindPattern((PCHAR)Pattern_UserMsg2, "???xxxx????x", UserMsgBase, UserMsgBase + 0x32, 7);

	if (FarProc(UserMsgBase, HwBase, HwEnd))
	{
		Error("UserMsgBase #2 address could not be found.");
	}

	return PUserMsg(**(PDWORD*)UserMsgBase);
}

DWORD AutoOffset::FindPattern(PCHAR pattern, PCHAR mask, DWORD start, DWORD end, DWORD offset)
{
	int patternLength = strlen(pattern);
	bool found = false;

	for (DWORD i = start; i < end - patternLength; i++)
	{
		found = true;
		for (int idx = 0; idx < patternLength; idx++)
		{
			if (mask[idx] == 'x' && pattern[idx] != *(PCHAR)(i + idx))
			{
				found = false;
				break;
			}
		}
		if (found)
		{
			return i + offset;
		}
	}

	return 0;
}

DWORD AutoOffset::FindPattern(PCHAR pattern, DWORD start, DWORD end, DWORD offset)
{
	int patternLength = strlen(pattern);
	bool found = false;

	for (DWORD i = start; i < end - patternLength; i++)
	{
		found = true;
		for (int idx = 0; idx < patternLength; idx++)
		{
			if (pattern[idx] != *(PCHAR)(i + idx))
			{
				found = false;
				break;
			}
		}
		if (found)
		{
			return i + offset;
		}
	}

	return 0;
}