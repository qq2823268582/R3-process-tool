
// PETOOLDlg.cpp: 实现文件
//

#include "pch.h"
#include "framework.h"
#include "PETOOL.h"
#include "PETOOLDlg.h"
#include "afxdialogex.h"

#include <windows.h>
#include "Tlhelp32.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CPETOOLDlg 对话框
CPETOOLDlg::CPETOOLDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_PETOOL_DIALOG, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CPETOOLDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST1, m_1);
	DDX_Control(pDX, IDC_LIST2, m_2);
}

BEGIN_MESSAGE_MAP(CPETOOLDlg, CDialogEx)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_MESSAGE(WM_UPDATE_STATIC, &CPETOOLDlg::OnUpdateStatic)   //增加1个自定义的消息映射函数
	ON_NOTIFY(NM_CLICK, IDC_LIST1, &CPETOOLDlg::OnNMClickList1) //增加1个列表鼠标点击触发事件消息函数
END_MESSAGE_MAP()


// CPETOOLDlg 消息处理程序
BOOL CPETOOLDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// 设置此对话框的图标。  当应用程序主窗口不是对话框时，框架将自动
	//  执行此操作
	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);		// 设置小图标
	
	//固定窗口大小，不能拖动变大变小
	::SetWindowLong(m_hWnd, GWL_STYLE, WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX);

	//创建一个新线程，发送消息，更新控件
	m_pThread = AfxBeginThread(ThreadFunction, this);

	//1.设置列表控件的样式
	m_1.SetExtendedStyle(LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES);

	//2.给列表添加标题（4列）
	CRect rect1;
	m_1.GetClientRect(rect1);  //获取列表的长度，以便均匀划分
	m_1.InsertColumn(0, L"", LVCFMT_CENTER, 100);
	m_1.InsertColumn(1, L"进程", LVCFMT_LEFT, rect1.right / 4);
	m_1.InsertColumn(2, L"PID", LVCFMT_CENTER, rect1.right / 4);
	m_1.InsertColumn(3, L"镜像基址", LVCFMT_CENTER, rect1.right / 4);
	m_1.InsertColumn(4, L"镜像大小", LVCFMT_CENTER, rect1.right / 4);
	m_1.DeleteColumn(0);

	//3.设置列表控件的样式
	m_2.SetExtendedStyle(LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES);
	
	//4.给列表添加标题（2列）
	CRect rect2;
	m_2.GetClientRect(rect2);
	m_2.InsertColumn(0, L"", LVCFMT_CENTER, 100);
	m_2.InsertColumn(1, L"模块", LVCFMT_LEFT, rect2.right / 3);
	m_2.InsertColumn(2, L"模块位置", LVCFMT_LEFT, 2*rect2.right / 3);
	m_2.DeleteColumn(0);

	//5.遍历进程
	EnumProcess();

	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}

// 如果向对话框添加最小化按钮，则需要下面的代码
//  来绘制该图标。  对于使用文档/视图模型的 MFC 应用程序，
//  这将由框架自动完成。

void CPETOOLDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // 用于绘制的设备上下文

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// 使图标在工作区矩形中居中
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// 绘制图标
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

//当用户拖动最小化窗口时系统调用此函数取得光标
//显示。
HCURSOR CPETOOLDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}


//进程遍历
void CPETOOLDlg::EnumProcess()
{
	//1.定义字符串数组
	DWORD ImageBase = 0;
	DWORD ImageSize = 0;

	//2.清空列表的所有内容（不能清空标题，清空标题是DeleteColumn）
	m_1.DeleteAllItems();


	//3.定义进程信息结构体
	PROCESSENTRY32 pi;
	pi.dwSize = sizeof(PROCESSENTRY32);

	//4.创建进程快照
	HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	if (INVALID_HANDLE_VALUE == hSnapshot)
	{
		return;
	}

	//5.取出第一个进程的数据放入进程信息结构体
	BOOL bRet = Process32First(hSnapshot, &pi);
	
	//6.遍历快照链表
	while (bRet)
	{
		//6.1 插入进程名  //本行第1列
		//InsertItem：申请增加一行，并且插入这一行的第一列  //InsertItem之后再SetItemText，需要两者一起配合
	    //参数1：行号（0表示第1行）
		//参数2：文本字符串
		m_1.InsertItem(0, pi.szExeFile);

		//6.2 插入PID   //本行第2列
		//SetItemText ：在列表中设置文本
		//参数1：本来要填行号的（填0表示不按行号，一行完了，下一行再往上累加）
		//参数2：列号，本行的第几列（从0开始是本行的第一列，但是我们前面申请一行的时候，已经占用了本行的第一列了）
		//参数3：文本字符串
		CString temp1;
		temp1.Format(_T("%d"), pi.th32ProcessID);
		LPCTSTR str1 = temp1;
		m_1.SetItemText(0,1,str1);

		//6.3 获得进程的镜像基址、镜像大小
		GetProcessBaseAndSize(pi.th32ProcessID, &ImageBase, &ImageSize);
		
		//6.4 插入镜像基址   //本行第3列
		CString temp2;
		temp2.Format(_T("%x"), ImageBase);
		LPCTSTR str2 = temp2;
		m_1.SetItemText(0,2,str2);

		//6.5 插入镜像大小   //本行第4列
		CString temp3;
		temp3.Format(_T("%x"), ImageSize);
		LPCTSTR str3 = temp3;
		m_1.SetItemText(0,3,str3);

		//6.6.遍历下一个快照
		bRet = Process32Next(hSnapshot, &pi);
	}
	return;
}

//封装函数，获取进程的镜像基址、镜像大小
void CPETOOLDlg::GetProcessBaseAndSize(IN DWORD pid,OUT DWORD* ImageBase,OUT DWORD* ImageSize)
{	
	//1.创建传进来的pid的那个进程的模块快照
	MODULEENTRY32 me32 = { sizeof(MODULEENTRY32) };
	HANDLE hModuleSnap = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE,pid);
	if (hModuleSnap == INVALID_HANDLE_VALUE)
		return;
	//2.模块快照的第一个模块就是exe自身
	if (!Module32First(hModuleSnap, &me32)) 
	{
		CloseHandle(hModuleSnap);
		return;
	}
	//3.从模块信息结构体中取出进程的基址和大小
	*ImageBase = (DWORD)me32.modBaseAddr;	
	*ImageSize = me32.modBaseSize;
}


//当在进程列表鼠标左键点击选中某一个进程时触发   //可以在列表控件的事件那里创建
void CPETOOLDlg::OnNMClickList1(NMHDR *pNMHDR, LRESULT *pResult)
{	
	LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
	
	//1.获取选中行的行号
	int nIndex = m_1.GetSelectionMark();  

	//2.GetItemText：获取选中行的文本内容 （因为PID是在每一行的第2列，所以参数2为1）
	//参数1：行号
	//参数2：这一行的第几列   //0为第1列，1为第2列
	CString str = m_1.GetItemText(nIndex, 1);

	//3.将文本内容转化为整数
	int PID = _tstoi(LPCTSTR(str));

	//4.遍历模块
	EnumModule(PID);

	*pResult = 0;
}



//模块遍历
void CPETOOLDlg::EnumModule(DWORD dwPId)
{
	//1.清空列表的所有内容
	m_2.DeleteAllItems();
	
	//2.创建指定进程的所有模块快照
	MODULEENTRY32 me32 = { sizeof(MODULEENTRY32) };
	HANDLE hModuleSnap = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE,dwPId);            
	if (hModuleSnap == INVALID_HANDLE_VALUE)
		return;

	//3.通过模块快照句柄获取第一个模块信息
	if (!Module32First(hModuleSnap, &me32)) 
	{
		CloseHandle(hModuleSnap);
		return;
	}

	//4.循环获取模块信息
	do{
		//4.1 申请一行，并且插入本行的第1列
		m_2.InsertItem(0, me32.szModule);
		
		//4.2 插入本行的第2列
		m_2.SetItemText(0, 1, me32.szExePath);

	} while(Module32Next(hModuleSnap, &me32));
	
	//5.关闭句柄并退出函数
	CloseHandle(hModuleSnap);
	return;
}



//自定义更新控件的函数
LRESULT CPETOOLDlg::OnUpdateStatic(WPARAM wParam, LPARAM lParam)
{
	if (wParam == 0)
	{
		UpdateData(FALSE);
	}
	return 0;
}

//创建的线程，专门用来发送消息给窗口，以便更新控件
UINT CPETOOLDlg::ThreadFunction(LPVOID pParam)
{
	CPETOOLDlg *pDlg = (CPETOOLDlg *)pParam;

	//不停的循环更新
	while (TRUE)
	{
		::PostMessage(pDlg->m_hWnd, WM_UPDATE_STATIC, 0, 0);
		Sleep(1);
	}
	return 0;
}
