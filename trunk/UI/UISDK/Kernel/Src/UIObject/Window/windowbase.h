#ifndef WINDOWBASE_H_A8C2427E_87C7_45e9_B6CE_AF3B4DBF49EF
#define WINDOWBASE_H_A8C2427E_87C7_45e9_B6CE_AF3B4DBF49EF

#include "UISDK\Kernel\Src\UIObject\Panel\panel.h"
#include "UISDK\Kernel\Src\UIObject\Window\SyncWindowHelper.h"
#include "UISDK\Kernel\Inc\Interface\iwindow.h"
#include "UISDK\Kernel\Src\Helper\commitbuffer\CommitWindowBufferListenerMgr.h"
#include "UISDK\Kernel\Src\Helper\MouseKeyboard\mousemanager.h"
#include "UISDK\Kernel\Src\Helper\dragdrop\windowdragdropmgr.h"
#include "UISDK\Kernel\Src\RenderLayer2\layer\windowrender.h"

// libo 20110821 9:57 (hangzhou xingyao jiari jiudian 8332)

namespace UI
{
class WindowRender;
class SkinRes;
// 
//	用于在创建窗口的时候，拦截第一个窗口消息，并进行子类化
//
class CREATE_WND_DATA  // 等同于 ATL 中的_ATL_WIN_MODULE70
{
public:
	CREATE_WND_DATA();
	~CREATE_WND_DATA();

public:
	void  AddCreateWndData(_AtlCreateWndData* pData, void* pThis);
	void* ExtractCreateWndData();

private:
	CRITICAL_SECTION   m_cs;
	_AtlCreateWndData* m_pCreateWndList;
};

// 2013.4.22 将windowbase基类修改为WindowRenderLayer，而不是Panel，从而优化整个分层渲染机制
// 默认一个窗口只有一个render layer，即窗口对象本身。当需要增加一个render layer时，将单独
// 创建一个render layer对象，该对象与窗口对象并没有什么关系，只是都被RenderChain管理起来了。
// 2014.3.19 将windowbase的基类重新修改为Panel，而不是WindowRenderLayer。重写了整个绘制流程
// 直接支持控件级分层。（以前只支持窗口级分层，在最后将每个层合并起来）
class WindowBase : public Panel
                 , public SyncWindowHelper<WindowBase>
{
public:
    WindowBase();
    ~WindowBase();

	UI_DECLARE_OBJECT(WindowBase, OBJ_WINDOW)

    // WndProc的原始消息处理   // 经过virtual扩展了
    VIRTUAL_BEGIN_MSG_MAP(WindowBase) 
        MESSAGE_HANDLER( WM_ERASEBKGND,           _OnEraseBkgnd )
        MESSAGE_HANDLER( WM_PAINT,                _OnPaint )
        MESSAGE_HANDLER( WM_SETCURSOR,            _OnSetCursor )
        MESSAGE_HANDLER( WM_NCHITTEST,            _OnNcHitTest )
        
        MESSAGE_HANDLER( WM_MOUSEMOVE,            _OnHandleMouseMessage )
        MESSAGE_HANDLER( WM_MOUSELEAVE,           _OnHandleMouseMessage )
        MESSAGE_HANDLER( WM_LBUTTONDOWN,          _OnHandleMouseMessage )
        MESSAGE_HANDLER( WM_LBUTTONUP,            _OnHandleMouseMessage )
        MESSAGE_HANDLER( WM_RBUTTONDOWN,          _OnHandleMouseMessage )
        MESSAGE_HANDLER( WM_RBUTTONUP,            _OnHandleMouseMessage )
        MESSAGE_HANDLER( WM_LBUTTONDBLCLK,        _OnHandleMouseMessage )
        MESSAGE_HANDLER( WM_MOUSEWHEEL,           _OnHandleMouseMessage )
        MESSAGE_HANDLER( WM_CANCELMODE,           _OnHandleMouseMessage )

        MESSAGE_HANDLER( WM_CHAR,                 _OnHandleKeyBoardMessage )
        MESSAGE_HANDLER( WM_KEYDOWN,              _OnHandleKeyBoardMessage )
        MESSAGE_HANDLER( WM_KEYUP,                _OnHandleKeyBoardMessage )
        MESSAGE_HANDLER( WM_SYSKEYDOWN,           _OnHandleKeyBoardMessage )
        MESSAGE_HANDLER( WM_SYSKEYUP,             _OnHandleKeyBoardMessage )

#if(WINVER >= 0x0601)
        MESSAGE_HANDLER( /*WM_TOUCH*/0x0240,      _OnHandleTouchMessage )
        MESSAGE_HANDLER( /*WM_GESTURE*/0x0119,    _OnHandleTouchMessage )
        MESSAGE_HANDLER( /*WM_GESTURENOTIFY*/0x011A, _OnHandleTouchMessage )
#endif

        MESSAGE_HANDLER( WM_IME_STARTCOMPOSITION, _OnHandleKeyBoardMessage )
        MESSAGE_HANDLER( WM_IME_ENDCOMPOSITION,   _OnHandleKeyBoardMessage )
        MESSAGE_HANDLER( WM_IME_COMPOSITION,      _OnHandleKeyBoardMessage )
        MESSAGE_HANDLER( WM_IME_SETCONTEXT,       _OnHandleKeyBoardMessage )
        MESSAGE_HANDLER( WM_IME_NOTIFY,           _OnHandleKeyBoardMessage )
        MESSAGE_HANDLER( WM_IME_CONTROL,          _OnHandleKeyBoardMessage )
        MESSAGE_HANDLER( WM_IME_COMPOSITIONFULL,  _OnHandleKeyBoardMessage )
        MESSAGE_HANDLER( WM_IME_SELECT,           _OnHandleKeyBoardMessage )
        MESSAGE_HANDLER( WM_IME_CHAR,             _OnHandleKeyBoardMessage )
        MESSAGE_HANDLER( WM_IME_REQUEST,          _OnHandleKeyBoardMessage )
        MESSAGE_HANDLER( WM_IME_KEYDOWN,          _OnHandleKeyBoardMessage )
        MESSAGE_HANDLER( WM_IME_KEYUP,            _OnHandleKeyBoardMessage )
        
        MESSAGE_HANDLER( WM_GETMINMAXINFO,        _OnGetMinMaxInfo )
        MESSAGE_HANDLER( WM_WINDOWPOSCHANGING,    _OnWindowPosChanging )
        MESSAGE_HANDLER( UI_WM_SYNC_WINDOW,       _OnSyncWindow )
        MESSAGE_HANDLER( WM_ENTERSIZEMOVE,        _OnEnterSizeMove )
        MESSAGE_HANDLER( WM_EXITSIZEMOVE,         _OnExitSizeMove )
        MESSAGE_HANDLER( WM_SIZE,                 _OnSize )
        MESSAGE_HANDLER( WM_SETFOCUS,             _OnSetFocus )
        MESSAGE_HANDLER( WM_KILLFOCUS,            _OnKillFocus )
        MESSAGE_HANDLER( WM_MOUSEACTIVATE,        _OnMouseActive )
		MESSAGE_HANDLER( WM_GETOBJECT,            _OnGetObject )
        MESSAGE_HANDLER( WM_CREATE,               _OnCreate )
        MESSAGE_HANDLER( WM_INITDIALOG,           _OnCreate )
        MESSAGE_HANDLER( WM_NCDESTROY,            _OnNcDestroy )
#if(_WIN32_WINNT >= 0x0501)
        MESSAGE_HANDLER( WM_THEMECHANGED,         _OnThemeChange )
#endif
    END_MSG_MAP()

    // UI事件的消息处理
    UI_BEGIN_MSG_MAP
        UIMSG_WM_ERASEBKGND( OnEraseBkgnd )
        UIMESSAGE_HANDLER_EX( UI_WM_GET_MOUSEKEYBOARD_MANAGER, OnGetMouseKeyboardMgr )
        
        UIMSG_DM_GETDEFID( OnGetDefId )
        UIMSG_DM_SETDEFID( OnSetDefId )
        UIMSG_WM_GETDESIREDSIZE( OnGetDesiredSize )
        UIMSG_WM_QUERYINTERFACE( QueryInterface )
        UIMSG_WM_GETOBJECTINFO( OnGetObjectInfo )
        UIMSG_WM_SERIALIZE( OnSerialize )
        UIMSG_WM_FINALCONSTRUCT( FinalConstruct )
        UIMSG_WM_PRECREATEWINDOW( PreCreateWindow )
     UI_END_MSG_MAP_CHAIN_PARENT( Panel )

    IWindowBase*   GetIWindowBase() { return m_pIWindowBase; }
    void  SetIWindowBase(IWindowBase* p) { m_pIWindowBase = p; }
    WindowRender*  GetWindowRender() { return &m_oWindowRender; }

public:
	operator   HWND() const;
    HWND       GetHWND();

    Object*    GetHoverObject();
    Object*    GetPressObject();
	void  SetPressObject(Object*);

	bool  Create(LPCTSTR szID,  HWND hWndParent = NULL, RECT* prc = NULL);
	void  DestroyWindow();
	long  DoModal(LPCTSTR szID, HWND hWndParent, bool canResize);
	long  DoModal(HINSTANCE hResInst, UINT nResID, LPCTSTR szID, HWND hWndParent);
	HWND  DoModeless(LPCTSTR szID, HWND hWndOnwer, bool canResize);
	HWND  DoModeless(HINSTANCE hResInst, UINT nResID, LPCTSTR szID, HWND hWndOnwer);
	void  EndDialog(INT_PTR);
	bool  Attach(HWND hWnd, LPCTSTR szID);
	void  Detach();

	BOOL  IsChildWindow();
	BOOL  IsWindowVisible();
	void  ShowWindow();
	void  HideWindow();
    void  SetActive(bool b);
    bool  IsActive();
    bool  IsDoModal();
    HRESULT  SetCanDrop(bool b);
    void  CenterWindow(HWND hWndCenter = NULL);
    void  CenterWindow(HMONITOR hMonitor = NULL);
    void  UpdateDesktopLayout();

	void  CalcWindowSizeByClientSize( SIZE sizeClient, SIZE* pSizeWindow );
	void  CalcClientRectByWindowRect( RECT* rcWindow, RECT* rcClient );
	void  Control_NotifyMe(const String&  idPath, int nNotifyMapID);
	void  Control_NotifyMe(Object*  pObj, int nNotifyMapID);

    WindowMouseMgr*  GetMouseMgr() { return &m_oMouseManager; }
    Object*  GetObjectUnderCursor();
    Object*  GetObjectByPos(Object* pObjParent, POINT* pt, bool bSkinBuilderInvoke=false);
    void  SetFocusObject(Object* pObj);

    // 获取字体流程：先获取自己的m_pTextRender，如果没有则调用自己的m_pWindow的GetRenderFont
    IRenderFont*  GetWindowDefaultRenderFont();
	void  SetDefaultRenderFont(LPCTSTR);
	LPCTSTR  GetDefaultRenderFontId();

	void  SetHardComposite(bool b);
	bool  IsHardComposite();
	void  DirectComposite();

    void  UpdateWindow(HDC hDC, RECT* prcInvalid);
	void  SaveMemBitmap(TCHAR* szFile);
	void  DrawMemBitmap(HDC hDC, RECT* prc, bool bAlphaBlend);
    void  EnableDwmTransition(bool b);
    void  SetCursorByHitTest(UINT nHitTest);

protected:
	bool  CreateUI(LPCTSTR szID);
	long  ModalLoop(HWND hWndParent);
    HMONITOR  GetPrimaryMonitor();
    

public:
	// public 虚函数，主要是分层窗口的实现与普通窗口不一致
	virtual void  CommitDoubleBuffet2Window(HDC hDCWnd, RECT* prcCommit, int nRectCount=1);
    virtual void  OnInnerInitWindow(){}
	
	virtual void  SetObjectPos( int x, int y, int cx, int cy, int nFlag=0) override;
	virtual void  virtualSetVisibleEx(VISIBILITY_TYPE eType) override;
	virtual void  virtualSetEnable(bool b) override;
	virtual bool  IsVisible() override;
	virtual bool  IsEnable() override;

protected:
    HRESULT    FinalConstruct(IUIApplication* p);
    BOOL       PreCreateWindow(CREATESTRUCT* pcs) { return TRUE; }
	void       OnSerialize(SERIALIZEDATA* pData);
    void       OnSetDefId(IObject* pButton);
    IObject*   OnGetDefId();

	virtual BOOL  PreTranslateMessage(UINT uMsg, WPARAM wParam, LPARAM lParam, LRESULT* pRet);
	
#pragma region // message handle
public:
	static LRESULT CALLBACK  StartWindowProc( HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam );
	static LRESULT CALLBACK  StartDialogProc( HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam );\
protected:
	static CREATE_WND_DATA  s_create_wnd_data;    // 创建窗口时，拦截第一个窗口消息，将HWND->this
	static LRESULT CALLBACK ThunkWndProc( HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam );
	LRESULT  StartProc( HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam, bool bWindowOrDialog );
	LRESULT  WndProc( UINT uMsg, WPARAM wParam, LPARAM lParam );
	LRESULT  DefWindowProc( UINT uMsg, WPARAM wParam, LPARAM lParam );
    LRESULT  WndProc_GetRetValue(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL bHandled, LRESULT lRet);

protected:
	LRESULT  _OnSetCursor( UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled );
    LRESULT  _OnNcHitTest( UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled );
	LRESULT  _OnEraseBkgnd( UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled );
	LRESULT  _OnPaint( UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled );
	LRESULT  _OnSize( UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled );
	LRESULT  _OnCreate( UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled );
	LRESULT  _OnNcDestroy( UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled );
	LRESULT  _OnHandleMouseMessage(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    LRESULT  _OnHandleKeyBoardMessage(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    LRESULT  _OnHandleTouchMessage(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT  _OnSetFocus( UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled );
	LRESULT  _OnKillFocus( UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled );
    LRESULT  _OnMouseActive( UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled );
	LRESULT  _OnThemeChange( UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled );
	LRESULT  _OnWindowPosChanging( UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled );
	LRESULT  _OnSyncWindow( UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled );
	LRESULT  _OnGetMinMaxInfo( UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled );
	LRESULT  _OnEnterSizeMove( UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled );
	LRESULT  _OnExitSizeMove( UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled );
	LRESULT  _OnGetObject( UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled );

	void      OnEraseBkgnd(IRenderTarget*);
    void      OnGetDesiredSize(SIZE* pSize);
    LRESULT   OnGetMouseKeyboardMgr(UINT uMsg, WPARAM wParam, LPARAM lParam);

#pragma endregion


#pragma region // member var
public:
	HWND  m_hWnd;               // 窗口句柄

protected:
    IWindowBase*  m_pIWindowBase;
	CWndProcThunk  m_thunk;              // ATL中的THUNK，用于将一个窗口过程作成自己的成员函数
	WNDPROC  m_oldWndProc;         // 该窗口的原始窗口过程

    WindowDragDropMgr      m_oDragDropManager;   // 拖拽管理器
	WindowMouseMgr         m_oMouseManager;      // 鼠标消息处理器
    WindowRender           m_oWindowRender;      // 多层渲染V2

	bool  m_bDoModal;           // 模态窗口数据
	bool  m_bEndModal;
	INT_PTR  m_lDoModalReturn;

	WindowStyle  m_windowStyle;

	long  m_lMinWidth;          // 窗口大小限制
	long  m_lMinHeight;
	long  m_lMaxWidth;
	long  m_lMaxHeight;
    String  m_strConfigWindowText; // 皮肤中配置的窗口标题

	bool  m_bFirsetEraseBkgnd;  // TODO: 解决第一次显示窗口时，会闪烁一下黑色的问题. 考虑优化，将消息映射做成为动态映射的机制

    // 机制：如果控件没有配置字体时，默认取窗口的字体，即m_pDefaultFont
    //
    // 1. 当xml中配置了窗口字体时，m_hFont为xml中的字体。
    // 2. 当没有配置字体时，使用窗口字体。
    // 3. 当窗口字体也没时，使用UI Font Res 默认字体
    // 4. 最后则尝试使用system default font.
	IRenderFont*  m_pDefaultFont;    

#pragma endregion

};



}

#endif // WINDOWBASE_H_A8C2427E_87C7_45e9_B6CE_AF3B4DBF49EF