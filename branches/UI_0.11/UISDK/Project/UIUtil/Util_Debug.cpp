#include "stdafx.h"
#include "h\util.h"

#ifdef UTIL_DEBUG
#include <time.h>
//#include <DbgHelp.h>
#include <imagehlp.h>
#include <tlhelp32.h>
#pragma comment(lib, "Dbghelp.lib")


namespace UI { namespace Util
{
/////////////////////////////////////////////////////////////////////
//	                       �͵�����صĲ���                        //
/////////////////////////////////////////////////////////////////////

// �����Լ��Ļص������ǣ�����֮ǰ�Ļص�����
LPTOP_LEVEL_EXCEPTION_FILTER   g_prevExceptionFilter = NULL;    
String                         g_strDumpPath;
bool                           g_bAddTime;

LONG WINAPI MyUnhandledExceptionFilter(	__in struct _EXCEPTION_POINTERS* ExceptionInfo	);

// Ϊ�˲��þ�̬���ӵ� dbghelp.lib�����ö�̬���ӵķ���
typedef BOOL (WINAPI *funcMiniDumpWriteDump)(\
		__in  HANDLE hProcess,\
		__in  DWORD ProcessId,\
		__in  HANDLE hFile,\
		__in  MINIDUMP_TYPE DumpType,\
		__in  PMINIDUMP_EXCEPTION_INFORMATION ExceptionParam,\
		__in  PMINIDUMP_USER_STREAM_INFORMATION UserStreamParam,\
		__in  PMINIDUMP_CALLBACK_INFORMATION CallbackParam\
		);

//
//	�����������ʱ������һ��dump�ļ�����¼����ʱ����Ϣ
//
//	Parameter
//		szFilePath
//			[in]  ���ɵ�dump�ļ���·������bAddTimeΪtrueʱ�������ļ�
//                ·�����������ϵ�ǰʱ����Ϊ����ÿ�α����ı�ʶ
//		bAddTime
//			[in]  �Ƿ�Ҫ�ڱ�����·�������ӱ�����ʱ�򡣲��������ӣ�
//                �����ɵ�dump�ļ��Ὣ��һ�ε��ļ����ǵ�
//	Remark
//		���������ʱ������ûص��������ڻص������У����ǾͿ�����
//      �κβ�����
//
void  /*UIUTILAPI*/ CreateDumpFileWhenCrash( LPCTSTR szFilePath,
 bool bAddTime )
{
	g_prevExceptionFilter = 
        SetUnhandledExceptionFilter(MyUnhandledExceptionFilter);
	g_strDumpPath         = szFilePath;
	g_bAddTime            = bAddTime;
}


//
//	����ʱ�Ļص�����
//
//	Return
//		EXCEPTION_EXECUTE_HANDLER = 1     ��ʾ���̽���
//		EXCEPTION_CONTINUE_EXECUTION = -1 ��ʾ�����쳣֮�����ִ��
//		EXCEPTION_CONTINUE_SEARCH = 0    ����ϵͳͨ�����쳣������������Ϣ�Ի��� 
//
LONG WINAPI MyUnhandledExceptionFilter(
        __in struct _EXCEPTION_POINTERS* pExceptionInfo)
{
	do 
	{
		// 1. ��dbghelp.dll�ж�̬���س�dump��������

		HMODULE hModule = ::LoadLibrary( _T("dbghelp.dll") );
		if( NULL == hModule ) break;

		funcMiniDumpWriteDump  pMiniDumpWriteDump;
		pMiniDumpWriteDump = (funcMiniDumpWriteDump)
            ::GetProcAddress( hModule, "MiniDumpWriteDump" );
		if( NULL == pMiniDumpWriteDump )	
            break;

		// 2. ����dump�ļ�

		if ( g_bAddTime )
		{
			String::size_type nPos = g_strDumpPath.rfind( _T('.') );
			if ( String::npos == nPos )	break;

			// ��ȡ��ǰʱ�䣬���뵽Ҫ���ɵ�dump�ļ�������
			time_t	 t; tm s_tm;
			TCHAR    szTime[128] = _T("");
			time(&t);localtime_s( &s_tm, &t );
			_stprintf( szTime, _T(" %d-%02d-%02d_%02d%02d%02d"),
                (s_tm.tm_year + 1900),
                s_tm.tm_mon+1, 
                s_tm.tm_mday, 
                s_tm.tm_hour, 
                s_tm.tm_min,  
                s_tm.tm_sec );

			g_strDumpPath.insert(nPos,szTime);
		}

		HANDLE  hFile = ::CreateFile( g_strDumpPath.c_str(), 
            FILE_WRITE_DATA, 
            FILE_SHARE_READ, 
            NULL, CREATE_ALWAYS, 
            FILE_ATTRIBUTE_NORMAL,
            NULL);
		if (NULL == hFile)	
            break;

		// 3. д��dump��Ϣ

		MINIDUMP_EXCEPTION_INFORMATION  mei;
		mei.ThreadId = GetCurrentThreadId();
		mei.ExceptionPointers = pExceptionInfo;
		mei.ClientPointers = FALSE;

		// TODO: ���Կ�����dump��Ϣ�м���汾��Ϣ�Ĳ���(PMINIDUMP_USER_STREAM_INFORMATION)
		BOOL bRet = pMiniDumpWriteDump( 
            GetCurrentProcess(), 
            GetCurrentProcessId(),
            hFile, 
            MiniDumpNormal, 
            &mei, NULL, NULL );
		if (FALSE == bRet)
		{
			DWORD dwErr = GetLastError();
		}
		
		// 4. ����
		CloseHandle( hFile );
		if (hModule)
		{
			::FreeLibrary( hModule );
			hModule = NULL;
		}
	} 
	while (false);

	if (g_prevExceptionFilter)
		SetUnhandledExceptionFilter(g_prevExceptionFilter);
	
	return EXCEPTION_CONTINUE_SEARCH;
}


#define TRACE_ID(iD) case iD: return L#iD;

//--------------------------------------------------------------------------------------
WCHAR* WINAPI DXUTTraceWindowsMessage( UINT uMsg )
{
    switch( uMsg )
    {
        TRACE_ID(WM_NULL);
        TRACE_ID(WM_CREATE);
        TRACE_ID(WM_DESTROY);
        TRACE_ID(WM_MOVE);
        TRACE_ID(WM_SIZE);
        TRACE_ID(WM_ACTIVATE);
        TRACE_ID(WM_SETFOCUS);
        TRACE_ID(WM_KILLFOCUS);
        TRACE_ID(WM_ENABLE);
        TRACE_ID(WM_SETREDRAW);
        TRACE_ID(WM_SETTEXT);
        TRACE_ID(WM_GETTEXT);
        TRACE_ID(WM_GETTEXTLENGTH);
        TRACE_ID(WM_PAINT);
        TRACE_ID(WM_CLOSE);
        TRACE_ID(WM_QUERYENDSESSION);
        TRACE_ID(WM_QUERYOPEN);
        TRACE_ID(WM_ENDSESSION);
        TRACE_ID(WM_QUIT);
        TRACE_ID(WM_ERASEBKGND);
        TRACE_ID(WM_SYSCOLORCHANGE);
        TRACE_ID(WM_SHOWWINDOW);
        TRACE_ID(WM_WININICHANGE);
        TRACE_ID(WM_DEVMODECHANGE);
        TRACE_ID(WM_ACTIVATEAPP);
        TRACE_ID(WM_FONTCHANGE);
        TRACE_ID(WM_TIMECHANGE);
        TRACE_ID(WM_CANCELMODE);
        TRACE_ID(WM_SETCURSOR);
        TRACE_ID(WM_MOUSEACTIVATE);
        TRACE_ID(WM_CHILDACTIVATE);
        TRACE_ID(WM_QUEUESYNC);
        TRACE_ID(WM_GETMINMAXINFO);
        TRACE_ID(WM_PAINTICON);
        TRACE_ID(WM_ICONERASEBKGND);
        TRACE_ID(WM_NEXTDLGCTL);
        TRACE_ID(WM_SPOOLERSTATUS);
        TRACE_ID(WM_DRAWITEM);
        TRACE_ID(WM_MEASUREITEM);
        TRACE_ID(WM_DELETEITEM);
        TRACE_ID(WM_VKEYTOITEM);
        TRACE_ID(WM_CHARTOITEM);
        TRACE_ID(WM_SETFONT);
        TRACE_ID(WM_GETFONT);
        TRACE_ID(WM_SETHOTKEY);
        TRACE_ID(WM_GETHOTKEY);
        TRACE_ID(WM_QUERYDRAGICON);
        TRACE_ID(WM_COMPAREITEM);
        TRACE_ID(WM_GETOBJECT);
        TRACE_ID(WM_COMPACTING);
        TRACE_ID(WM_COMMNOTIFY);
        TRACE_ID(WM_WINDOWPOSCHANGING);
        TRACE_ID(WM_WINDOWPOSCHANGED);
        TRACE_ID(WM_POWER);
        TRACE_ID(WM_COPYDATA);
        TRACE_ID(WM_CANCELJOURNAL);
        TRACE_ID(WM_NOTIFY);
        TRACE_ID(WM_INPUTLANGCHANGEREQUEST);
        TRACE_ID(WM_INPUTLANGCHANGE);
        TRACE_ID(WM_TCARD);
        TRACE_ID(WM_HELP);
        TRACE_ID(WM_USERCHANGED);
        TRACE_ID(WM_NOTIFYFORMAT);
        TRACE_ID(WM_CONTEXTMENU);
        TRACE_ID(WM_STYLECHANGING);
        TRACE_ID(WM_STYLECHANGED);
        TRACE_ID(WM_DISPLAYCHANGE);
        TRACE_ID(WM_GETICON);
        TRACE_ID(WM_SETICON);
        TRACE_ID(WM_NCCREATE);
        TRACE_ID(WM_NCDESTROY);
        TRACE_ID(WM_NCCALCSIZE);
        TRACE_ID(WM_NCHITTEST);
        TRACE_ID(WM_NCPAINT);
        TRACE_ID(WM_NCACTIVATE);
        TRACE_ID(WM_GETDLGCODE);
        TRACE_ID(WM_SYNCPAINT);
        TRACE_ID(WM_NCMOUSEMOVE);
        TRACE_ID(WM_NCLBUTTONDOWN);
        TRACE_ID(WM_NCLBUTTONUP);
        TRACE_ID(WM_NCLBUTTONDBLCLK);
        TRACE_ID(WM_NCRBUTTONDOWN);
        TRACE_ID(WM_NCRBUTTONUP);
        TRACE_ID(WM_NCRBUTTONDBLCLK);
        TRACE_ID(WM_NCMBUTTONDOWN);
        TRACE_ID(WM_NCMBUTTONUP);
        TRACE_ID(WM_NCMBUTTONDBLCLK);
        TRACE_ID(WM_NCXBUTTONDOWN);
        TRACE_ID(WM_NCXBUTTONUP);
        TRACE_ID(WM_NCXBUTTONDBLCLK);
        TRACE_ID(WM_INPUT);
        TRACE_ID(WM_KEYDOWN);
        TRACE_ID(WM_KEYUP);
        TRACE_ID(WM_CHAR);
        TRACE_ID(WM_DEADCHAR);
        TRACE_ID(WM_SYSKEYDOWN);
        TRACE_ID(WM_SYSKEYUP);
        TRACE_ID(WM_SYSCHAR);
        TRACE_ID(WM_SYSDEADCHAR);
        TRACE_ID(WM_UNICHAR);
        TRACE_ID(WM_IME_STARTCOMPOSITION);
        TRACE_ID(WM_IME_ENDCOMPOSITION);
        TRACE_ID(WM_IME_COMPOSITION);
        TRACE_ID(WM_INITDIALOG);
        TRACE_ID(WM_COMMAND);
        TRACE_ID(WM_SYSCOMMAND);
        TRACE_ID(WM_TIMER);
        TRACE_ID(WM_HSCROLL);
        TRACE_ID(WM_VSCROLL);
        TRACE_ID(WM_INITMENU);
        TRACE_ID(WM_INITMENUPOPUP);
        TRACE_ID(WM_MENUSELECT);
        TRACE_ID(WM_MENUCHAR);
        TRACE_ID(WM_ENTERIDLE);
        TRACE_ID(WM_MENURBUTTONUP);
        TRACE_ID(WM_MENUDRAG);
        TRACE_ID(WM_MENUGETOBJECT);
        TRACE_ID(WM_UNINITMENUPOPUP);
        TRACE_ID(WM_MENUCOMMAND);
        TRACE_ID(WM_CHANGEUISTATE);
        TRACE_ID(WM_UPDATEUISTATE);
        TRACE_ID(WM_QUERYUISTATE);
        TRACE_ID(WM_CTLCOLORMSGBOX);
        TRACE_ID(WM_CTLCOLOREDIT);
        TRACE_ID(WM_CTLCOLORLISTBOX);
        TRACE_ID(WM_CTLCOLORBTN);
        TRACE_ID(WM_CTLCOLORDLG);
        TRACE_ID(WM_CTLCOLORSCROLLBAR);
        TRACE_ID(WM_CTLCOLORSTATIC);
        TRACE_ID(MN_GETHMENU);
        TRACE_ID(WM_MOUSEMOVE);
        TRACE_ID(WM_LBUTTONDOWN);
        TRACE_ID(WM_LBUTTONUP);
        TRACE_ID(WM_LBUTTONDBLCLK);
        TRACE_ID(WM_RBUTTONDOWN);
        TRACE_ID(WM_RBUTTONUP);
        TRACE_ID(WM_RBUTTONDBLCLK);
        TRACE_ID(WM_MBUTTONDOWN);
        TRACE_ID(WM_MBUTTONUP);
        TRACE_ID(WM_MBUTTONDBLCLK);
        TRACE_ID(WM_MOUSEWHEEL);
        TRACE_ID(WM_XBUTTONDOWN);
        TRACE_ID(WM_XBUTTONUP);
        TRACE_ID(WM_XBUTTONDBLCLK);
        TRACE_ID(WM_PARENTNOTIFY);
        TRACE_ID(WM_ENTERMENULOOP);
        TRACE_ID(WM_EXITMENULOOP);
        TRACE_ID(WM_NEXTMENU);
        TRACE_ID(WM_SIZING);
        TRACE_ID(WM_CAPTURECHANGED);
        TRACE_ID(WM_MOVING);
        TRACE_ID(WM_POWERBROADCAST);
        TRACE_ID(WM_DEVICECHANGE);
        TRACE_ID(WM_MDICREATE);
        TRACE_ID(WM_MDIDESTROY);
        TRACE_ID(WM_MDIACTIVATE);
        TRACE_ID(WM_MDIRESTORE);
        TRACE_ID(WM_MDINEXT);
        TRACE_ID(WM_MDIMAXIMIZE);
        TRACE_ID(WM_MDITILE);
        TRACE_ID(WM_MDICASCADE);
        TRACE_ID(WM_MDIICONARRANGE);
        TRACE_ID(WM_MDIGETACTIVE);
        TRACE_ID(WM_MDISETMENU);
        TRACE_ID(WM_ENTERSIZEMOVE);
        TRACE_ID(WM_EXITSIZEMOVE);
        TRACE_ID(WM_DROPFILES);
        TRACE_ID(WM_MDIREFRESHMENU);
        TRACE_ID(WM_IME_SETCONTEXT);
        TRACE_ID(WM_IME_NOTIFY);
        TRACE_ID(WM_IME_CONTROL);
        TRACE_ID(WM_IME_COMPOSITIONFULL);
        TRACE_ID(WM_IME_SELECT);
        TRACE_ID(WM_IME_CHAR);
        TRACE_ID(WM_IME_REQUEST);
        TRACE_ID(WM_IME_KEYDOWN);
        TRACE_ID(WM_IME_KEYUP);
        TRACE_ID(WM_MOUSEHOVER);
        TRACE_ID(WM_MOUSELEAVE);
        TRACE_ID(WM_NCMOUSEHOVER);
        TRACE_ID(WM_NCMOUSELEAVE);
        TRACE_ID(WM_WTSSESSION_CHANGE);
        TRACE_ID(WM_TABLET_FIRST);
        TRACE_ID(WM_TABLET_LAST);
        TRACE_ID(WM_CUT);
        TRACE_ID(WM_COPY);
        TRACE_ID(WM_PASTE);
        TRACE_ID(WM_CLEAR);
        TRACE_ID(WM_UNDO);
        TRACE_ID(WM_RENDERFORMAT);
        TRACE_ID(WM_RENDERALLFORMATS);
        TRACE_ID(WM_DESTROYCLIPBOARD);
        TRACE_ID(WM_DRAWCLIPBOARD);
        TRACE_ID(WM_PAINTCLIPBOARD);
        TRACE_ID(WM_VSCROLLCLIPBOARD);
        TRACE_ID(WM_SIZECLIPBOARD);
        TRACE_ID(WM_ASKCBFORMATNAME);
        TRACE_ID(WM_CHANGECBCHAIN);
        TRACE_ID(WM_HSCROLLCLIPBOARD);
        TRACE_ID(WM_QUERYNEWPALETTE);
        TRACE_ID(WM_PALETTEISCHANGING);
        TRACE_ID(WM_PALETTECHANGED);
        TRACE_ID(WM_HOTKEY);
        TRACE_ID(WM_PRINT);
        TRACE_ID(WM_PRINTCLIENT);
        TRACE_ID(WM_APPCOMMAND);
        TRACE_ID(WM_THEMECHANGED);
        TRACE_ID(WM_HANDHELDFIRST);
        TRACE_ID(WM_HANDHELDLAST);
        TRACE_ID(WM_AFXFIRST);
        TRACE_ID(WM_AFXLAST);
        TRACE_ID(WM_PENWINFIRST);
        TRACE_ID(WM_PENWINLAST);
        TRACE_ID(WM_APP);
    default:
        return L"Unknown";
    }
}

//
// Hook Api
// ժ��Windows���ı�̣���5�棩P547
//
// Remarks
//   1. û�б��滻��ģ����Ȼ�ǵ���ԭʼ����
//   2. ������ReplaceAllModule֮���¼���������dll��Ȼ����ԭʼ������
//      ����������ҽ�LoadLibraryA,LoadLibraryW, LoadLibraryExA,
//      LoadLibraryExW��Ϊ�¼��ص�ģ�����Replace
//   3. GetProcAddress���ص���ԭʼ���������Ҳ��Ҫ�ҽ�GetProcAddress
//
//   ʹ��JUMPָ��ʵ�ֵ�HookApi�������ɷǳ����صĲ��㣬��˽��龡��
//   ����ʹ���������ȣ�����CPU�������Ժܴ���x86,Alpha������CPU�ϵ�
//   JUMPָ���ǲ�ͬ�ģ�����ʹ���ֹ�����Ļ���ָ�����ʹ���ַ�����Ч��
//   �ڶ������ַ�������ռʽ���̻߳����и����������á��߳���Ҫռ��һ
//   ����ʱ������д�������صĴ��롣�����뱻��дʱ����һ���߳̿�����ͼ
//   ���ø�ͬһ��������������������Եġ���ˣ�ֻ�е���֪���ڹ涨��ʱ
//   ��ֻ��һ���߳���ͼ����ĳ������ʱ������ʹ�����ַ�����
//
// ���÷�ʽ:
// PROC  pfnOrig = GetProcAddress(GetModuleHandle("Kernel32",
//      "ExitProcess");
// HMODULE hmodCaller = GetModuleHandle("DataBase.exe");
// 
// ReleaseIATEntryInOneModule(
//      "Kernel32.dll",  // Module containing the function(ANSI)
//      pfnOrig,         // Address of funcction in callee
//      MyExitProcess,   // Address of new function to be called
//      hmodCaller);     // Handle of module that should call the
//                       // new function
//
void ReplaceIATEntryInOneModule(
        PCSTR pszCalleeModuleName, 
        PROC pfnCurrent,
        PROC pfnNew,
        HMODULE hModCaller)
{
    ULONG   ulSize = 0;
    PIMAGE_IMPORT_DESCRIPTOR  pImportDesc =
        (PIMAGE_IMPORT_DESCRIPTOR)
        ::ImageDirectoryEntryToData(hModCaller, TRUE, 
        IMAGE_DIRECTORY_ENTRY_IMPORT, &ulSize);

    if (NULL == pImportDesc)
        return;  // This module has no import section.

    // Find the importdescriptor containing references
    // to callee's functions.
    for (; pImportDesc->Name; pImportDesc++)
    {
        PSTR pszModName = (PSTR)
            ((PBYTE)hModCaller + pImportDesc->Name);
        if (0 == lstrcmpiA(pszModName, pszCalleeModuleName))
            break;
    }

    if (0 == pImportDesc->Name)
        // This module doesn't import any function from this callee.
        return;

    // Get call's import address table(IAT)
    // for the callee's funcions.
    PIMAGE_THUNK_DATA pThunk = (PIMAGE_THUNK_DATA)
        ((PBYTE)hModCaller + pImportDesc->FirstThunk);

    // Replace current functoin address with new function address.
    for (; pThunk->u1.Function; pThunk++)
    {
        // Get the address of the function address.
        PROC*  ppfn = (PROC*)&pThunk->u1.Function;

        // Is this the function we're looking for?
        BOOL fFound = (*ppfn == pfnCurrent);

        // See the sample code for some stricky Window 98
        // stuff that goes here;
        if (fFound)
        {
            // The address match; change the import section address.
            ::WriteProcessMemory(GetCurrentProcess(), ppfn, &pfnNew,
                sizeof(pfnNew), NULL);
            return;  // We did it; get out.
        }
    }

    // If we get to here, the function is not in the callee's
    // import section.
}

HMODULE  ModuleFromAddress(PVOID pv)
{
    MEMORY_BASIC_INFORMATION  mbi;
    return ((VirtualQuery(pv, &mbi, sizeof(mbi))!=0) ?
        (HMODULE)mbi.AllocationBase : NULL);
}

// NOTE: This function must NOT be inlined
FARPROC  GetProcAddressRaw(HMODULE hmod, PCSTR pszProcName)
{
    return (::GetProcAddress(hmod, pszProcName));
}

//
//  fExcludeAPIHookMod
//      ��ע�᱾ģ�飬��Ϊ��ģ�����Ҫ������ԭʼ�ı�hook����api
//
void  ReplaceIATEntryInAllModules(
        PCSTR pszCalleeModName, 
        PROC pfnCurrent, 
        PROC pfnNew,
        BOOL fExcludeAPIHookMod
        )
{
    HMODULE  hmodThisMod = NULL;
    hmodThisMod = fExcludeAPIHookMod ? 
        ModuleFromAddress(ReplaceIATEntryInAllModules) : NULL;

    HANDLE snap = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, 
        GetCurrentProcessId());
    if(snap == INVALID_HANDLE_VALUE)
        return;

    MODULEENTRY32 me = { sizeof(me), 0 };
    BOOL bRet = Module32Next(snap, &me);
    while(bRet)
    {
        if (me.hModule != hmodThisMod)
        {
            ReplaceIATEntryInOneModule(pszCalleeModName, 
                pfnCurrent, pfnNew, me.hModule);
        }
        bRet = Module32Next(snap, &me);
    }
    CloseHandle(snap);
}


CApiHook::CApiHook()
{
	m_pszCalleeModName = NULL;
	m_pfnOrig = NULL;
	m_pfnHook = NULL;
	m_fExcludeAPIHookMod = FALSE;
}

CApiHook::CApiHook(
         PSTR pszCalleeModName, 
         PROC pfnCurrent, 
         PROC pfnNew,
         BOOL fExcludeAPIHookMod)
{
    m_pszCalleeModName = pszCalleeModName;
    m_pfnOrig = pfnCurrent;
    m_pfnHook = pfnNew;
    m_fExcludeAPIHookMod = fExcludeAPIHookMod;

    Hook(
        pszCalleeModName,
        pfnCurrent, 
        pfnNew, 
        fExcludeAPIHookMod);
}
CApiHook::~CApiHook()
{
   UnHook();
}
void  CApiHook::Hook(
		   PSTR pszCalleeModName, 
		   PROC pfnCurrent, 
		   PROC pfnNew,
		   BOOL fExcludeAPIHookMod
)
{
	ReplaceIATEntryInAllModules(
		pszCalleeModName,
		pfnCurrent, 
		pfnNew,  
		fExcludeAPIHookMod);
}
void  CApiHook::UnHook()
{
	if (!m_pszCalleeModName)
		return;

	ReplaceIATEntryInAllModules(
		m_pszCalleeModName,
		m_pfnOrig, 
		m_pfnOrig, 
		m_fExcludeAPIHookMod);

	m_pszCalleeModName = NULL;
	m_pfnOrig = NULL;
	m_pfnHook = NULL;
	m_fExcludeAPIHookMod = FALSE; 
}

}}
#endif