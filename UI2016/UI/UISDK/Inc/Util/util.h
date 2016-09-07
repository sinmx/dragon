#ifndef _UI_UTIL_H_
#define _UI_UTIL_H_

namespace UI { 
	class  C9Region;

	namespace Util 
{
	COLORREF  TranslateHexColor(LPCTSTR szColor);
	COLORREF  TranslateColor(LPCTSTR szColor);
	COLORREF  TranslateRGB(LPCTSTR szCol, TCHAR szSep = _T(','));
	bool  TranslateSIZE(LPCTSTR szText, SIZE* pSize, TCHAR szSep = _T(','));
	bool  TranslateRECT(LPCTSTR szRect, RECT* pRect, TCHAR szSep = _T(','));
	bool  TranslateImage9Region(LPCTSTR str, C9Region* p9Region, TCHAR szSep = _T(','));
	bool  IsKeyDown(UINT vk);

	interface ISplitStringEnum
	{
		virtual void  Release() = 0;
		virtual LPCTSTR GetText(UINT nIndex) = 0;
		virtual int   GetCount() = 0;
	};
	UINT SplitString(
		LPCTSTR szText,
		TCHAR szSep,
		ISplitStringEnum** pEnum
		);

	BOOL IsFullPath(LPCTSTR szPath);
	BOOL GetPathFileName(LPCTSTR szPath, TCHAR* szOutDir);
	BOOL GetPathFileExt(LPCTSTR szPath, TCHAR* szOutExt);

	enum FixAlphaMode
	{
		// 直接将alpha设置为255
		SET_ALPHA_255,

		// 如果当前alpha==0，将alpha设置为255
		SET_ALPHA_255_IF_ALPHA_IS_0,

		// 如果当前像素不是0，则将alpha设置为255
		SET_ALPHA_255_IF_RGBA_ISNOT_0,

		// alpha 反转 0->255, 255->0
		SET_ALPHA_INVERSE_0_255,

		// 将alpha设置为指定值wParam
		SET_ALPHA_VALUE,

		// 如果当前alpha值是255, 将alpha设置为指定值wParam
		// 但是没有值预乘处理
		SET_ALPHA_VALUE_IF_ALPHA_IS_255

		// ...
		// 如果当前alpha==wParam，将alpha设置为lParam
		//	SET_ALPHA_IF_ALPHA_EQUAL,      

		// 如果当前alpha<wParam，将alpha设置为lParam
		//	SET_ALPHA_IF_ALPHA_LESS,       
	};
	struct FixAlphaData
	{
		// 当仅指定了hDC，没有指定hBitmap时，从hDC中获取CurrentBitap
		HDC      hDC;
		HBITMAP  hBitmap;
		BOOL     bTopDownDib;
		RECT*    lprc;
		FixAlphaMode  eMode;
		WPARAM   wParam;
		LPARAM   lParam;
	};
	BOOL UISDKAPI FixBitmapAlpha(FixAlphaData* pData);
	BOOL UISDKAPI FixRenderTargetClipRegionAlpha(IRenderTarget*);

	void UISDKAPI PathInBin(LPCTSTR szRelative, TCHAR szAbsolution[MAX_PATH]);
}
}

#endif // _UI_UTIL_H_