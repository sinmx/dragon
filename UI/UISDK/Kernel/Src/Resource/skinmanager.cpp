#include "StdAfx.h"
#include "skinmanager.h"

#include "UISDK\Kernel\Src\Resource\skinres.h"
#include "UISDK\Kernel\Inc\Interface\iuires.h"
#include "UISDK\Kernel\Src\Helper\timer\timermanager.h"
#include "UISDK\Kernel\Src\Helper\topwindow\topwindowmanager.h"
#include "UISDK\Kernel\Src\Resource\imagemanager.h"
#include "UISDK\Kernel\Src\Resource\colormanager.h"
#include "UISDK\Kernel\Src\Resource\stylemanager.h"
#include "UISDK\Kernel\Src\Resource\layoutmanager.h"
#include "UISDK\Kernel\Src\Resource\fontmanager.h"

using namespace UI;

SkinManager::SkinManager()
{
    m_pISkinManager = NULL;
	m_pCurActiveSkinRes = NULL;
	m_bDirty = false;

	m_pUIApplication = NULL;
}
SkinManager::~SkinManager()
{
    SAFE_DELETE(m_pISkinManager);
}

ISkinManager*  SkinManager::GetISkinManager()
{ 
    if (NULL == m_pISkinManager)
    {
        m_pISkinManager = new ISkinManager(this);
    }
    return m_pISkinManager; 
}

// ������UIApplication��������������ǰ�ͷ�
void  SkinManager::Destroy()
{
	//////////////////////////////////////////////////////////////////////////
	// �ͷŸ�Ƥ�������ڴ�

	vector<SkinRes*>::iterator  iter = m_vSkinRes.begin();
	vector<SkinRes*>::iterator  iterEnd = m_vSkinRes.end();

	for ( ; iter != iterEnd; iter++ )
	{
		SkinRes* p = (*iter);
		delete p; 
	}
	m_vSkinRes.clear();
}

// ������Ŀ¼�µ�Ƥ���б�
void  SkinManager::SetSkinDirection(LPCTSTR szDir)
{
    if (NULL == szDir || 0 == _tcslen(szDir))
        return;

    m_strSkinDir = szDir;

    WIN32_FIND_DATA  finddata;
    if (m_strSkinDir[m_strSkinDir.length()-1] != _T('\\') &&
        m_strSkinDir[m_strSkinDir.length()-1] != _T('/'))
    {
        m_strSkinDir.append(_T("\\"));
    }

    String strFind(m_strSkinDir);
    strFind.append(_T("*.*"));

    HANDLE hFind=::FindFirstFile(strFind.c_str(),&finddata);
    if(INVALID_HANDLE_VALUE == hFind)
        return;

    while (1)
    {
        if (finddata.dwFileAttributes&FILE_ATTRIBUTE_DIRECTORY)
        {
            if (finddata.cFileName[0] != '.')
            {
                // �ж�Ŀ¼�����Ƿ����skin.xml�ļ�
                String strPath = m_strSkinDir;
                strPath.append(finddata.cFileName);

                String strFile = strPath;
                strFile.append(_T("\\") XML_SKIN_XML);

                if (PathFileExists(strFile.c_str()))
                {
                    OnFindSkinInSkinDir(SKIN_PACKET_TYPE_DIR, finddata.cFileName, strPath.c_str());
                }
            } 
        }
        else
        {
            // �ж��ļ���׺�Ƿ���*.skn��ʽ
            TCHAR szExt[MAX_PATH] = _T("");
            Util::GetPathFileExt(finddata.cFileName, szExt);
            if (0 == _tcscmp(szExt, XML_SKIN_PACKET_EXT))
            {
				int nLength = _tcslen(finddata.cFileName);
				finddata.cFileName[nLength-4] = L'';
                OnFindSkinInSkinDir(SKIN_PACKET_TYPE_ZIP, finddata.cFileName, m_strSkinDir.c_str());
            }
        }
        if(!FindNextFile(hFind,&finddata))
            break;
    }
    FindClose(hFind);
}


// ��Ƥ��Ŀ¼������һ����Ƥ�� 
ISkinRes*  SkinManager::AddSkin(LPCTSTR  szName)
{
    if (NULL == szName)
        return NULL;

    String  strFilePath = m_strSkinDir;
    strFilePath.append(_T("\\"));
    strFilePath.append(szName);

    if (!PathFileExists(strFilePath.c_str()))
        return NULL;

    SkinRes* p = OnFindSkinInSkinDir(SKIN_PACKET_TYPE_DIR, szName, strFilePath.c_str());
    if (p)
        return p->GetISkinRes();

    return NULL;
}

// �ڵ���SetSkinDirection���������һ��Ƥ���ļ�������ø���Ӧ����
SkinRes*  SkinManager::OnFindSkinInSkinDir(SKIN_PACKET_TYPE eType, LPCTSTR szName, LPCTSTR szPath)
{
    SkinRes*  pSkin = new SkinRes(*this);
    pSkin->SetParam(eType, szName, szPath);

    m_vSkinRes.push_back(pSkin);
    return pSkin;
}

void  SkinManager::GetSkinDirection(TCHAR*  szOut)
{
    if (szOut)
    {
        _tcscpy(szOut, m_strSkinDir.c_str());
    }
}

IUIApplication*  SkinManager::GetUIApplication()
{
	return m_pUIApplication;
}

void SkinManager::SetUIApplication(IUIApplication* pUIApp)
{ 
	m_pUIApplication = pUIApp; 
	m_SkinBuilderRes.SetUIApplication(pUIApp);
}

void  ChangeSkinTimerProc(UINT_PTR, TimerItem* pItem)
{
    SkinManager* pThis = (SkinManager*)pItem->wParam;
    pThis->ChangeSkin((ISkinRes*)pItem->lParam, true);
}

// ����
// bSync���ڽ�������һ����ť�������������ť�ڻ����б�delete�ˣ����»����������ջ����
HRESULT SkinManager::ChangeSkin(ISkinRes* pISkinRes, bool bSync)
{
	if (NULL == pISkinRes)
		return E_INVALIDARG;

    SkinRes* pSkinRes = pISkinRes->GetImpl();
    if (pSkinRes == m_pCurActiveSkinRes)
        return S_FALSE;

    if (!bSync)
    {
        TimerItem  item;
        item.nRepeatCount = 1;
        item.pProc = ChangeSkinTimerProc;
        item.wParam = (WPARAM)this;
        item.lParam = (LPARAM)pISkinRes;
        TimerHelper::GetInstance()->SetTimer(1, &item);
        return E_PENDING;
    }

	SkinRes* pOldSkinRes = m_pCurActiveSkinRes;
	m_pCurActiveSkinRes = NULL;

    pSkinRes->Load();
    m_pCurActiveSkinRes = pSkinRes;
	
    ITopWindowManager* pTopWndMgr = m_pUIApplication->GetTopWindowMgr();
    if (pTopWndMgr)
        pTopWndMgr->GetImpl()->ChangeSkin(pSkinRes);

    pOldSkinRes->Unload();

	m_bDirty = true;
	return true;
}

HRESULT SkinManager::ChangeSkinHLS( short h, short l, short s, int nFlag )
{
	bool bRet = m_pCurActiveSkinRes->ChangeSkinHLS(h,l,s,nFlag);
	if (false == bRet)
		return E_FAIL;

	// ֪ͨ����ˢ��
    ITopWindowManager* pTopWndMgr = m_pUIApplication->GetTopWindowMgr();
    if (pTopWndMgr)
    {
        UIMSG  msg;
        msg.message = UI_WM_SKINCHANGED;
        pTopWndMgr->SendMessage2AllWnd(&msg);
        pTopWndMgr->UpdateAllWindow();
    }

	m_bDirty = true;
	return S_OK;
}

//
//	����Ƥ������
//
bool  SkinManager::LoadSkin(LPCTSTR szName)
{ 
	UI_LOG_INFO( _T("\n\n------------  LoadSkin: %s ----------------\n"), szName);

    SkinRes*  pSkinRes = this->GetSkinResByName(szName);
    if (NULL == pSkinRes)
    {
        UI_LOG_WARN(_T("GetSkinRes Failed. Name=%s"), szName);
        return false;
    }
    bool bRet = pSkinRes->Load();
    if (bRet)
        m_pCurActiveSkinRes = pSkinRes;

    return bRet;
}
//	����һ��Ƥ�������̵��У����ظ�Ƥ�����
//
//	Parameter
//		strSkinName
//			[in]	Ƥ������
//
//		strSkinXmlFullPath
//			[in]	Ƥ������Ӧ��ui.xml����·��
//
SkinRes* SkinManager::AddSkinRes(const String& strSkinName, const String& strSkinXmlFullPath)
{
#if 0
	//////////////////////////////////////////////////////////////////////////
	// 1. ����ui.xml

	ISkinConfigParse* pSkinInfoParse = IParseClassFactory::CreateSkinInfoParseInstance(m_pUIApplication, strSkinXmlFullPath);
	if (false == pSkinInfoParse->Create() )
	{
		UI_LOG_ERROR(_T("SkinManager::AddSkinRes create skininfo failed."));
		return NULL;
	}

	//////////////////////////////////////////////////////////////////////////
	// 2. ���ӵ��־��Զ���������

	SkinDescResItem* pSkinItem = new SkinDescResItem;
	pSkinItem->SetSkinName(strSkinName);
	pSkinItem->SetSkinXmlPath(strSkinXmlFullPath);

	m_UISkinDescRes.AddSkinItem(pSkinItem);


	//////////////////////////////////////////////////////////////////////////
	// 3. ��ʼ��Ƥ������

	SkinRes* pSkinRes = new SkinRes(pSkinItem);
	pSkinRes->SetProjectManager(this);
	pSkinRes->GetSkinConfigManager()->SetSkinConfigParse(pSkinInfoParse);


	//////////////////////////////////////////////////////////////////////////
	// 4. ���ӵ�����ʱ����������
	this->m_vSkinRes.push_back(pSkinRes);
	return pSkinRes;
#endif 
    return 0;
}


//
//	���浽�ļ���
//
//		NULL - NULL ȫ������
//		NULL -  xx  ��ǰƤ����ָ����Դ
//		 xx  - NULL ָ��Ƥ����������Դ
//
bool SkinManager::Save(SkinRes* pSkinRes)
{
	bool bRet = true;

	//////////////////////////////////////////////////////////////////////////
	// �����Ƥ����Ϣ

	int nSkinCount = (int)m_vSkinRes.size();
	for (int i = 0; i < nSkinCount; i++)
	{
		if (pSkinRes)
		{
			if (pSkinRes != m_vSkinRes[i])
				continue;

			pSkinRes->Save();
		}
		else
        {
			m_vSkinRes[i]->Save();
        }
	}

	return bRet;
}

//
//	��ȡһ��HSKIN��Ӧ����m_vSkinRes�е�����
//
//	ʧ�ܷ���-1
//
int SkinManager::GetSkinResIndex(SkinRes* pSkinRes)
{
	if (NULL == pSkinRes)
		return -1;

	int nSize = (int)m_vSkinRes.size();
	if (0 == nSize )
		return -1;

	for (int i = 0; i < nSize; i++)
	{
		if (m_vSkinRes[i] == pSkinRes)
			return i;
	}

	return -1;
}

//
//	ö�ٵ�ǰ�����е�Ƥ��
//
UINT SkinManager::GetSkinCount()
{
	return (UINT)m_vSkinRes.size(); 
}

SkinRes*  SkinManager::GetSkinResByIndex(long lIndex)
{
	int nSize = (int)m_vSkinRes.size();
	if (lIndex < 0 || lIndex >= nSize )
		return NULL;
	
	return m_vSkinRes[lIndex];
}

SkinRes*  SkinManager::GetSkinResByName(LPCTSTR szName)
{
    if (NULL == szName)
        return NULL;

    vector<SkinRes*>::iterator iter = m_vSkinRes.begin();
    for (; iter != m_vSkinRes.end(); iter++)
    {
        SkinRes* p = *iter;
        if (0 == _tcscmp(szName, p->GetName()))
            return p;
    }

    return NULL;
}

//
//	���õ�ǰ�ĻƤ��������Ƥ���༭�������ڱ༭��Ƥ����
//
//	һЩGET��������Ĭ������ڵ�ǰƤ�����Ե�
//
HRESULT SkinManager::SetActiveSkin(ISkinRes* pSkinRes)
{
	if (NULL == pSkinRes)
	{
		UI_LOG_WARN(_T("SkinManager::SetActiveSkin failed"));
		return E_INVALIDARG;
	}

	SkinRes* pSkinRes2 = pSkinRes->GetImpl(); // �ڲ���Ȼ����ΪSkinRes�����ڵ���
    m_pCurActiveSkinRes = pSkinRes2;
	return S_OK;
}

SkinRes* SkinManager::GetActiveSkin()
{
	if (NULL == m_pCurActiveSkinRes)
		return NULL;

	return m_pCurActiveSkinRes;
}
