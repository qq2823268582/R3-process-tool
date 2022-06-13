
// DLLINJECTDlg.h: 头文件
//

#pragma once
#include "afxwin.h" 
#define WM_UPDATE_STATIC (WM_USER + 100)   //定义消息宏

/*判断系统架构，并定义ZwCreateThreadEx函数指针*/


// CDLLINJECTDlg 对话框
class CDLLINJECTDlg : public CDialogEx
{
// 构造
public:
	CDLLINJECTDlg(CWnd* pParent = nullptr);	// 标准构造函数

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DLLINJECT_DIALOG };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 支持


// 实现
protected:
	HICON m_hIcon;

	// 生成的消息映射函数
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	CListCtrl m_1;
	CListCtrl m_2;
	CEdit m_3;
	CStatic m_4;
	afx_msg void OnBnClickedButton3();
	afx_msg void OnBnClickedButton1();
	afx_msg void OnBnClickedButton2();
	afx_msg void OnDropFiles(HDROP hDropInfo);
	virtual void OnOK();
	

	afx_msg void EnumProcess();
	afx_msg void EnumModule(DWORD dwPId);

	afx_msg void OnNMClickList1(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnNMRClickList1(NMHDR *pNMHDR, LRESULT *pResult);


	CMenu m_menu;
	afx_msg void OnReNew();
	afx_msg BOOL CheckDLL(IN DWORD dwPid, IN CString g_szDllPath, OUT HMODULE* pMODULE);
	afx_msg void OnNMClickList2(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnNMKillfocusList1(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnNMSetfocusList1(NMHDR *pNMHDR, LRESULT *pResult);
};
