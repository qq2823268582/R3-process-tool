
// PETOOLDlg.h: 头文件
//

#pragma once
#define WM_UPDATE_STATIC (WM_USER + 100)   //定义消息宏

// CPETOOLDlg 对话框
class CPETOOLDlg : public CDialogEx
{
// 构造
public:
	CPETOOLDlg(CWnd* pParent = nullptr);	// 标准构造函数

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_PETOOL_DIALOG };
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

	//遍历进程的函数
	afx_msg void EnumProcess();
	afx_msg void GetProcessBaseAndSize(IN DWORD pid, OUT DWORD* ImageBase, OUT DWORD* ImageSize);

	//点击进程列表触发
	afx_msg void OnNMClickList1(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void EnumModule(DWORD dwPId);

	//声明自定义的消息函数
	CWinThread * m_pThread;
	static UINT __cdecl ThreadFunction(LPVOID pParam);
	afx_msg LRESULT OnUpdateStatic(WPARAM wParam, LPARAM lParam);
};
