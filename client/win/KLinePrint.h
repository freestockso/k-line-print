
// KLinePrint.h : KLinePrint 应用程序的主头文件
//
#pragma once

#ifndef __AFXWIN_H__
	#error "在包含此文件之前包含“stdafx.h”以生成 PCH 文件"
#endif

#include "resource.h"       // 主符号

#include "PlaybackConfig.h"
#include "CalendarGenerator.h"
#include "TradeFacility.h"

// CKLinePrintApp:
// 有关此类的实现，请参阅 KLinePrint.cpp
//

class CKLinePrintApp : public CWinAppEx
{
public:
	CKLinePrintApp();

// 重写
public:
	virtual BOOL InitInstance();

	int GetPlaybackDate();
	void LoadPlaybackCalendar(PlaybackConfig pbConfig);
	BOOL ValidatePlaybackConfig(int nDate, PlaybackConfig pbConfig);

// 实现
	UINT			m_nAppLook;
	BOOL			m_bHiColorIcons;

	Calendar		m_cal;
	Calendar		m_PlaybackCalendar;

	PlaybackConfig	m_PlaybackConfig;		//	回放选项
	TradeFacility	m_ex;					//	交易所

	afx_msg void OnAppAbout();
	DECLARE_MESSAGE_MAP()

};

extern CKLinePrintApp theApp;

#define CALENDAR ((CKLinePrintApp*)AfxGetApp())->m_cal
#define PBCONFIG ((CKLinePrintApp*)AfxGetApp())->m_PlaybackConfig
#define EXCHANGE ((CKLinePrintApp*)AfxGetApp())->m_ex
#define PBCAL ((CKLinePrintApp*)AfxGetApp())->m_PlaybackCalendar

