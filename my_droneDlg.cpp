
// my_droneDlg.cpp : implementation file
//

#include "stdafx.h"
#include "my_drone.h"
#include "my_droneDlg.h"
#include "afxdialogex.h"
#include "scope\OScopeCtrl.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CAboutDlg dialog used for App About

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_ABOUTBOX };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

// Implementation
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogEx(IDD_ABOUTBOX)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()


// CmydroneDlg dialog



CmydroneDlg::CmydroneDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_MY_DRONE_DIALOG, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CmydroneDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CmydroneDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
END_MESSAGE_MAP()


// CmydroneDlg message handlers

void CmydroneDlg::InitializeGraph()
{
   CRect rcGyro, rcAccel, rcMagnet;
   GetDlgItem(IDC_GRAPH_GYRO)->GetWindowRect(rcGyro);
   GetDlgItem(IDC_GRAPH_ACCEL)->GetWindowRect(rcAccel);
   GetDlgItem(IDC_GRAPH_MAGNET)->GetWindowRect(rcMagnet);
   GetDlgItem(IDC_GRAPH_GYRO)->DestroyWindow();
   GetDlgItem(IDC_GRAPH_ACCEL)->DestroyWindow();
   GetDlgItem(IDC_GRAPH_MAGNET)->DestroyWindow();

   ScreenToClient(rcGyro);
   ScreenToClient(rcAccel);
   ScreenToClient(rcMagnet);

   _graphCtrlGyro                                        = new COScopeCtrl(3);		//X,Y,Z
   _graphCtrlGyro->Create(WS_VISIBLE | WS_CHILD, rcGyro, this, IDC_GRAPH_GYRO);
   _graphCtrlGyro->SetRanges(-270., 270.);	//gyro rate range : +-300 deg/sec
   _graphCtrlGyro->autofitYscale                         = true;
   _graphCtrlGyro->SetYUnits(_T("Value"));
   _graphCtrlGyro->SetXUnits(_T("Time"));
   _graphCtrlGyro->SetLegendLabel(_T("Gyro(X) deg/sec"), 0);
   _graphCtrlGyro->SetLegendLabel(_T("Gyro(Y) deg/sec"), 1);
   _graphCtrlGyro->SetLegendLabel(_T("Gyro(Z) deg/sec"), 2);
   _graphCtrlGyro->SetPlotColor(RGB(255, 0, 0), 0);
   _graphCtrlGyro->SetPlotColor(RGB(0, 255, 0), 1);
   _graphCtrlGyro->SetPlotColor(RGB(0, 0, 255), 2);
   _graphCtrlGyro->InvalidateCtrl();

   _graphCtrlAccel                                       = new COScopeCtrl(3);		//X,Y,Z
   _graphCtrlAccel->Create(WS_VISIBLE | WS_CHILD, rcAccel, this, IDC_GRAPH_ACCEL);
   _graphCtrlAccel->SetRanges(-3., 3.);	//acceleration range : +- 18g
   _graphCtrlAccel->autofitYscale = true;
   _graphCtrlAccel->SetYUnits(_T("Value"));
   _graphCtrlAccel->SetXUnits(_T("Time"));
   _graphCtrlAccel->SetLegendLabel(_T("Accel(X)"), 0);
   _graphCtrlAccel->SetLegendLabel(_T("Accel(Y)"), 1);
   _graphCtrlAccel->SetLegendLabel(_T("Accel(Z)"), 2);
   _graphCtrlAccel->SetPlotColor(RGB(255, 0, 0), 0);
   _graphCtrlAccel->SetPlotColor(RGB(0, 255, 0), 1);
   _graphCtrlAccel->SetPlotColor(RGB(0, 0, 255), 2);
   _graphCtrlAccel->InvalidateCtrl();

   _graphCtrlMagnet                                      = new COScopeCtrl(3);		//X,Y,Z
   _graphCtrlMagnet->Create(WS_VISIBLE | WS_CHILD, rcMagnet, this, IDC_GRAPH_MAGNET);
   _graphCtrlMagnet->SetRanges(-0.6, 0.6);	//acceleration range : +- 18g
   _graphCtrlMagnet->autofitYscale = true;
   _graphCtrlMagnet->SetYUnits(_T("Value"));
   _graphCtrlMagnet->SetXUnits(_T("Time"));
   _graphCtrlMagnet->SetLegendLabel(_T("Magnet(X)"), 0);
   _graphCtrlMagnet->SetLegendLabel(_T("Magnet(Y)"), 1);
   _graphCtrlMagnet->SetLegendLabel(_T("Magnet(Z)"), 2);
   _graphCtrlMagnet->SetPlotColor(RGB(255, 0, 0), 0);
   _graphCtrlMagnet->SetPlotColor(RGB(0, 255, 0), 1);
   _graphCtrlMagnet->SetPlotColor(RGB(0, 0, 255), 2);
   _graphCtrlMagnet->InvalidateCtrl();
}


BOOL CmydroneDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// Add "About..." menu item to system menu.

	// IDM_ABOUTBOX must be in the system command range.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != nullptr)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	// TODO: Add extra initialization here
   InitializeGraph();
	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CmydroneDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialogEx::OnSysCommand(nID, lParam);
	}
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CmydroneDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CmydroneDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

