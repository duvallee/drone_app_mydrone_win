
// my_drone.h : main header file for the PROJECT_NAME application
//

#pragma once

#ifndef __AFXWIN_H__
	#error "include 'stdafx.h' before including this file for PCH"
#endif

#include "resource.h"		// main symbols


// CmydroneApp:
// See my_drone.cpp for the implementation of this class
//

class CmydroneApp : public CWinApp
{
public:
	CmydroneApp();

// Overrides
public:
	virtual BOOL InitInstance();

// Implementation

	DECLARE_MESSAGE_MAP()
};

extern CmydroneApp theApp;
