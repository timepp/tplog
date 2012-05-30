#include "stdafx.h"

#include "config.h"
#include "helper.h"
#include "filtercreator.h"
#include "concretefilter.h"
#include <ShlObj.h>

#define CHECKHR_RN(st) if (FAILED(st)) {ATLASSERT(FALSE); return NULL; };
#define CHECKHR_RF(st) if (FAILED(st)) {ATLASSERT(FALSE); return false; };

CConfig* CConfig::Instance()
{
#pragma warning(disable:4640)
	static CConfig cfg;
#pragma warning(default:4640)
	return &cfg;
}

CConfig::CConfig()
{
	m_cfg.ui.theme_color = RGB(255, 128, 192);
	m_cfg.ui.theme_color_offline = RGB(128, 255, 128);
	m_cfg.ui.list.show_gridline = false;
	m_cfg.ui.list.calldepth_sign = L"    ";
	m_cfg.ui.placement.show_menubar = true;
	m_cfg.ui.placement.show_toolbar = true;
	m_cfg.ui.placement.show_statusbar = true;
	m_cfg.ui.placement.show_filterbar = true;
	m_cfg.syscfg.confirm_on_exit = true;
	m_cfg.syscfg.last_check_update_time = 0;
	m_cfg.ui.default_filter_cfg_page = 0;
	m_cfg.log_filter = NULL;
	m_cfg.log_quickfilter.level = 0;
	m_cfg.first_time_run = false;
}
CConfig::~CConfig()
{
}

CStringW CConfig::GetConfigFilePath() const
{
	return m_strConfigFilePath;
}

config& CConfig::GetConfig()
{
	return m_cfg;
}

bool CConfig::Load(LPCWSTR pszFileName)
{
	CStringW strPath = pszFileName;
	if (strPath.IsEmpty())
	{
		WCHAR szPath[MAX_PATH];
		SHGetFolderPathW(NULL, CSIDL_APPDATA, NULL, SHGFP_TYPE_CURRENT, szPath);
		strPath = szPath;

		strPath += L"\\Baidu";
		::CreateDirectoryW(strPath, NULL);

		strPath += L"\\bdlogview.xml";
	}

	if (m_strConfigFilePath != strPath && !m_strConfigFilePath.IsEmpty())
	{
		Save();
	}

	m_strConfigFilePath = strPath;

	// 不是绝对路径的话，变成绝对路径
	if (m_strConfigFilePath.GetLength() > 1 && m_strConfigFilePath[1] != L':')
	{
		WCHAR szPath[MAX_PATH];
		::GetFullPathNameW(m_strConfigFilePath, _countof(szPath), szPath, NULL);
		m_strConfigFilePath = szPath;
	}

	if (_waccess_s(m_strConfigFilePath, 0) != 0)
	{
		// 配置文件不存在，赋缺省值
		SetDefaultHilighter();
		Save();
		m_cfg.first_time_run = true;
		return true;
	}

	CComPtr<IXMLDOMDocument> pDoc;
	CHECKHR_RF(CoCreateInstance(CLSID_DOMDocument, NULL, CLSCTX_INPROC_SERVER, IID_IXMLDOMDocument, (void**)&pDoc));

	VARIANT_BOOL ret;
	CHECKHR_RF(pDoc->load(CComVariant(m_strConfigFilePath), &ret));

	CComPtr<IXMLDOMNode> pNode;
	pDoc->selectSingleNode(CComBSTR(L"/config/filter"), &pNode);
	if (pNode)
	{
		CComPtr<IXMLDOMNode> pSubNode;
		pNode->get_firstChild(&pSubNode);
		if (pSubNode)
		{
			m_cfg.log_filter = filter_creator::instance()->load(pSubNode);
		}
	}

	pNode = NULL;
	pDoc->selectSingleNode(CComBSTR(L"/config/quickfilter"), &pNode);
	if (pNode)
	{
		m_cfg.log_quickfilter.level = helper::XML_GetAttributeAsUint(pNode, L"level", 0);
		m_cfg.log_quickfilter.tags = helper::XML_GetAttributeAsString(pNode, L"tags", L"");
		m_cfg.log_quickfilter.text = helper::XML_GetAttributeAsString(pNode, L"text", L"");
	}
	
	CComPtr<IXMLDOMNodeList> pList;
	pDoc->selectNodes(CComBSTR(L"/config/hilighters/hilight"), &pList);
	if (pList)
	{
		m_cfg.hls.clear();
		long count = 0;
		pList->get_length(&count);
		for (long i = 0; i < count; i++)
		{
			CComPtr<IXMLDOMNode> phlNode;
			pList->get_item(i, &phlNode);

			CComPtr<IXMLDOMNode> pFilterNode;
			phlNode->get_firstChild(&pFilterNode);

			hilighter h;
			h.f = filter_creator::instance()->load(pFilterNode);
			h.name = helper::XML_GetAttributeAsString(phlNode, L"name", L"unnamed");
			h.d.color = helper::XML_GetAttributeAsColor(phlNode, L"color", 0);
			h.d.bkcolor = helper::XML_GetAttributeAsColor(phlNode, L"bkcolor", RGB(255, 255, 255));
			m_cfg.hls.push_back(h);
		}
	}

	pNode = NULL;
	pDoc->selectSingleNode(CComBSTR(L"/config/ui"), &pNode);
	if (pNode)
	{
		m_cfg.ui.theme_color = helper::XML_GetAttributeAsColor(pNode, L"theme_color",RGB(255, 128, 192));
		m_cfg.ui.theme_color_offline = helper::XML_GetAttributeAsColor(pNode, L"theme_color_offline", RGB(128, 255, 128));
		m_cfg.ui.default_filter_cfg_page = helper::XML_GetAttributeAsInt(pNode, L"default_filter_cfg_page", 0);
	}

	pNode = NULL;
	pDoc->selectSingleNode(CComBSTR(L"/config/ui/list"), &pNode);
	if (pNode)
	{
		m_cfg.ui.list.show_gridline = helper::XML_GetAttributeAsUint(pNode, L"show_grid", 0) > 0;
	}

	pNode = NULL;
	pDoc->selectSingleNode(CComBSTR(L"/config/ui/placement"), &pNode);
	if (pNode)
	{
		m_cfg.ui.placement.show_menubar = helper::XML_GetAttributeAsUint(pNode, L"show_menubar", 0) > 0;
		m_cfg.ui.placement.show_toolbar = helper::XML_GetAttributeAsUint(pNode, L"show_toolbar", 0) > 0;
		m_cfg.ui.placement.show_statusbar = helper::XML_GetAttributeAsUint(pNode, L"show_statusbar", 0) > 0;
		m_cfg.ui.placement.show_filterbar = helper::XML_GetAttributeAsUint(pNode, L"show_filterbar", 0) > 0;
	}

	pNode = NULL;
	pDoc->selectSingleNode(CComBSTR(L"/config/ui/savedpath"), &pNode);
	if (pNode)
	{
		m_cfg.ui.savedpath.logfile = helper::XML_GetAttributeAsString(pNode, L"logfile", L"");
		m_cfg.ui.savedpath.filter= helper::XML_GetAttributeAsString(pNode, L"filter", L"");
	}

	pNode = NULL;
	pDoc->selectSingleNode(CComBSTR(L"/config/search"), &pNode);
	if (pNode)
	{
		m_cfg.search.history = helper::XML_GetAttributeAsString(pNode, L"history", L"");
	}

	pNode = NULL;
	pDoc->selectSingleNode(CComBSTR(L"/config/sys"), &pNode);
	if (pNode)
	{
		m_cfg.syscfg.confirm_on_exit = helper::XML_GetAttributeAsUint(pNode, L"confirm_on_exit", 0) > 0;
		m_cfg.syscfg.last_check_update_time = helper::XML_GetAttributeAsUint(pNode, L"last_check_update_time", 0);
	}

	pNode = NULL;
	pDoc->selectSingleNode(CComBSTR(L"/config"), &pNode);
	if (pNode)
	{
		m_cfg.product_name = helper::XML_GetAttributeAsString(pNode, L"product_name", L"");
	}

	SetDefaultHilighter();
	return true;
}

bool CConfig::Save()
{
	CComPtr<IXMLDOMDocument> pDoc;
	CHECKHR_RF(CoCreateInstance(CLSID_DOMDocument, NULL, CLSCTX_INPROC_SERVER, IID_IXMLDOMDocument, (void**)&pDoc));

	CComPtr<IXMLDOMNode> pRootNode;
	pDoc->appendChild(helper::XML_CreateNode(pDoc, NODE_ELEMENT, L"config"), &pRootNode);

	CComPtr<IXMLDOMNode> pFilterNode;
	pRootNode->appendChild(helper::XML_CreateNode(pDoc, NODE_ELEMENT, L"filter"), &pFilterNode);
	filter_creator::instance()->save(m_cfg.log_filter, pDoc, pFilterNode);

	CComPtr<IXMLDOMNode> pQuickFilterNode;
	pRootNode->appendChild(helper::XML_CreateNode(pDoc, NODE_ELEMENT, L"quickfilter"), &pQuickFilterNode);
	helper::XML_AddAttributeUint(pDoc, pQuickFilterNode, L"level", m_cfg.log_quickfilter.level);
	helper::XML_AddAttribute(pDoc, pQuickFilterNode, L"tags", m_cfg.log_quickfilter.tags.c_str());
	helper::XML_AddAttribute(pDoc, pQuickFilterNode, L"text", m_cfg.log_quickfilter.text.c_str());

	CComPtr<IXMLDOMNode> pHilighterRoot;
	pRootNode->appendChild(helper::XML_CreateNode(pDoc, NODE_ELEMENT, L"hilighters"), &pHilighterRoot);
	for (size_t i = 0; i < m_cfg.hls.size(); i++)
	{
		CComPtr<IXMLDOMNode> pHilighterNode;
		pHilighterRoot->appendChild(helper::XML_CreateNode(pDoc, NODE_ELEMENT, L"hilight"), &pHilighterNode);
		helper::XML_AddAttribute(pDoc, pHilighterNode, L"name", m_cfg.hls[i].name.c_str());
		helper::XML_AddAttributeColor(pDoc, pHilighterNode, L"color", m_cfg.hls[i].d.color);
		helper::XML_AddAttributeColor(pDoc, pHilighterNode, L"bkcolor", m_cfg.hls[i].d.bkcolor);
		filter_creator::instance()->save(m_cfg.hls[i].f, pDoc, pHilighterNode);
	}

	CComPtr<IXMLDOMNode> pUINode;
	CComPtr<IXMLDOMNode> pUIListNode;
	CComPtr<IXMLDOMNode> pUIPlacementNode;
	CComPtr<IXMLDOMNode> pSavedPathNode;
	pRootNode->appendChild(helper::XML_CreateNode(pDoc, NODE_ELEMENT, L"ui"), &pUINode);
	helper::XML_AddAttributeColor(pDoc, pUINode, L"theme_color", m_cfg.ui.theme_color);
	helper::XML_AddAttributeColor(pDoc, pUINode, L"theme_color_offline", m_cfg.ui.theme_color_offline);
	helper::XML_AddAttributeInt(pDoc, pUINode, L"default_filter_cfg_page", m_cfg.ui.default_filter_cfg_page);
	pUINode->appendChild(helper::XML_CreateNode(pDoc, NODE_ELEMENT, L"list"), &pUIListNode);
	helper::XML_AddAttributeUint(pDoc, pUIListNode, L"show_grid", m_cfg.ui.list.show_gridline?1U:0U);
	pUINode->appendChild(helper::XML_CreateNode(pDoc, NODE_ELEMENT, L"placement"), &pUIPlacementNode);
	helper::XML_AddAttributeUint(pDoc, pUIPlacementNode, L"show_menubar", m_cfg.ui.placement.show_menubar?1U:0U);
	helper::XML_AddAttributeUint(pDoc, pUIPlacementNode, L"show_toolbar", m_cfg.ui.placement.show_toolbar?1U:0U);
	helper::XML_AddAttributeUint(pDoc, pUIPlacementNode, L"show_statusbar", m_cfg.ui.placement.show_statusbar?1U:0U);
	helper::XML_AddAttributeUint(pDoc, pUIPlacementNode, L"show_filterbar", m_cfg.ui.placement.show_filterbar?1U:0U);
	pUINode->appendChild(helper::XML_CreateNode(pDoc, NODE_ELEMENT, L"savedpath"), &pSavedPathNode);
	helper::XML_AddAttribute(pDoc, pSavedPathNode, L"logfile", m_cfg.ui.savedpath.logfile.c_str());
	helper::XML_AddAttribute(pDoc, pSavedPathNode, L"filter", m_cfg.ui.savedpath.filter.c_str());

	CComPtr<IXMLDOMNode> pSysNode;
	pRootNode->appendChild(helper::XML_CreateNode(pDoc, NODE_ELEMENT, L"sys"), &pSysNode);
	helper::XML_AddAttributeUint(pDoc, pSysNode, L"confirm_on_exit", m_cfg.syscfg.confirm_on_exit?1U:0U);
	helper::XML_AddAttributeUint(pDoc, pSysNode, L"last_check_update_time", static_cast<UINT>(m_cfg.syscfg.last_check_update_time));

	CComPtr<IXMLDOMNode> pSearchNode;
	pRootNode->appendChild(helper::XML_CreateNode(pDoc, NODE_ELEMENT, L"search"), &pSearchNode);
	helper::XML_AddAttribute(pDoc, pSearchNode, L"history", m_cfg.search.history.c_str());

	helper::XML_AddAttribute(pDoc, pRootNode, L"product_name", m_cfg.product_name.c_str());

	pDoc->save(CComVariant(m_strConfigFilePath));

	return true;
}

void CConfig::SetDefaultHilighter()
{
	if (m_cfg.hls.size() > 0)
	{
		return;
	}

	hilighter h;

	h.name = L"调试级别";
	h.f = new logclass_filter(16, 16);
	h.d.color = RGB(192, 192, 192);
	m_cfg.hls.push_back(h);

	h.name = L"警告级别";
	h.f = new logclass_filter(48, 48);
	h.d.color = RGB(0, 64, 255);
	m_cfg.hls.push_back(h);

	h.name = L"错误级别";
	h.f = new logclass_filter(64, 0);
	h.d.color = RGB(255, 0, 0);
	m_cfg.hls.push_back(h);
}
