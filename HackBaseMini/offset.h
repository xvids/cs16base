#pragma once

class AutoOffset
{
private:
	DWORD FarProc( const DWORD Address , DWORD LB , DWORD HB );
public:
	DWORD HwBase, HwSize, HwEnd;
	DWORD ClBase, ClSize, ClEnd;
	DWORD UiBase, UiSize, UiEnd;

	BYTE HLType;

	bool GetRendererInfo();

	void Error( const PCHAR Msg );
	DWORD GetModuleSize( const DWORD Address );

	PVOID ClientFuncs();
	PVOID EngineFuncs();
	PVOID StudioFuncs();

	PUserMsg FindUserMsgBase();

	DWORD Absolute(DWORD Address);
	DWORD FindPattern(PCHAR pattern, PCHAR mask, DWORD start, DWORD end, DWORD offset);
	DWORD FindPattern(PCHAR pattern, DWORD start, DWORD end, DWORD offset);
	DWORD FindReference(DWORD start, DWORD end, DWORD Address);
};

extern AutoOffset c_Offset;