
// my_droneDlg.h : header file
//

#pragma once

// CmydroneDlg dialog
class COScopeCtrl;
class COglWndExt;
class CmydroneDlg : public CDialogEx
{
// Construction
public:
	CmydroneDlg(CWnd* pParent = nullptr);	// standard constructor

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_MY_DRONE_DIALOG };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support

   private:

   COScopeCtrl* _graphCtrlGyro;
   COScopeCtrl* _graphCtrlAccel;
   COScopeCtrl* _graphCtrlMagnet;
   COglWndExt* _modelWnd;

   LARGE_INTEGER m_frequency;
   LARGE_INTEGER m_performance_count;

   void InitializeGraph();

// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
   afx_msg void OnDestroy();
   afx_msg void OnTimer(UINT_PTR nIDEvent);
};
