
// DLLINJECTDlg.cpp: 实现文件
//

#include "pch.h"
#include "framework.h"
#include "DLLINJECT.h"
#include "DLLINJECTDlg.h"
#include "afxdialogex.h"
#include <windows.h>
#include "Tlhelp32.h"
#include <psapi.h>
#ifdef _DEBUG
#define new DEBUG_NEW
#endif

int PID = 0;
HMODULE g_hModule = NULL;//模块句柄
CString g_szDllPath;     //目标进程被注入的DLL的路径
CString szDLLPATH;       //目标进程被卸载的DLL的路径

int m_nSelectedItemIndex;
int nIndex_1;


#ifdef _WIN64
typedef	DWORD(WINAPI* pZwCreateThreadEx)(
	PHANDLE ThreadHandle,
	ACCESS_MASK DesiredAccess,
	LPVOID ObjectAttributes,
	HANDLE ProcessHandle,
	LPTHREAD_START_ROUTINE lpStartAddress,
	LPVOID lpParameter,
	ULONG CreateThreadFlags,
	SIZE_T ZeroBits,
	SIZE_T StackSize,
	SIZE_T MaximumStackSize,
	LPVOID pUnkown
	);
#else
typedef DWORD(WINAPI* pZwCreateThreadEx)(
	PHANDLE ThreadHandle,
	ACCESS_MASK DesiredAccess,
	LPVOID ObjectAttributes,
	HANDLE ProcessHandle,
	LPTHREAD_START_ROUTINE lpStartAddress,
	LPVOID lpParameter,
	BOOL CreateSuspended,
	DWORD dwStackSize,
	DWORD dw1,
	DWORD dw2,
	LPVOID pUnkown
	);
#endif

// CDLLINJECTDlg 对话框
CDLLINJECTDlg::CDLLINJECTDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_DLLINJECT_DIALOG, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CDLLINJECTDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST1, m_1);
	DDX_Control(pDX, IDC_LIST2, m_2);
	DDX_Control(pDX, IDC_EDIT1, m_3);
	DDX_Control(pDX, IDC_STATIC11, m_4);
}

BEGIN_MESSAGE_MAP(CDLLINJECTDlg, CDialogEx)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_BUTTON3, &CDLLINJECTDlg::OnBnClickedButton3)
	ON_BN_CLICKED(IDC_BUTTON1, &CDLLINJECTDlg::OnBnClickedButton1)
	ON_BN_CLICKED(IDC_BUTTON2, &CDLLINJECTDlg::OnBnClickedButton2)
	ON_WM_DROPFILES()
	ON_NOTIFY(NM_CLICK, IDC_LIST1, &CDLLINJECTDlg::OnNMClickList1)
	//ON_MESSAGE(WM_UPDATE_STATIC, &CDLLINJECTDlg::OnUpdateStatic)   //增加1个自定义的消息映射函数
	ON_NOTIFY(NM_RCLICK, IDC_LIST1, &CDLLINJECTDlg::OnNMRClickList1)
	ON_COMMAND(ID_32771, &CDLLINJECTDlg::OnReNew)
	
	ON_NOTIFY(NM_CLICK, IDC_LIST2, &CDLLINJECTDlg::OnNMClickList2)
	ON_NOTIFY(NM_KILLFOCUS, IDC_LIST1, &CDLLINJECTDlg::OnNMKillfocusList1)
	ON_NOTIFY(NM_SETFOCUS, IDC_LIST1, &CDLLINJECTDlg::OnNMSetfocusList1)
END_MESSAGE_MAP()

// CDLLINJECTDlg 消息处理程序
BOOL CDLLINJECTDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();	
	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);		// 设置小图标
//-----------------------------------------------------------------
	//加载菜单图标
	m_menu.LoadMenu(IDR_MENU1);

	//1.固定窗口大小，不能拖动变大变小
	::SetWindowLong(m_hWnd, GWL_STYLE, WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX);

	//2.设置列表控件的样式
	m_1.SetExtendedStyle(LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES);

	//3.给列表添加标题（3列）
	CRect rect1;
	m_1.GetClientRect(rect1);  //获取列表的长度，以便均匀划分
	m_1.InsertColumn(0, L"", LVCFMT_CENTER, 100);
	m_1.InsertColumn(1, L"进程", LVCFMT_LEFT, rect1.right / 4);
	m_1.InsertColumn(2, L"PID", LVCFMT_CENTER, rect1.right / 4);
	m_1.InsertColumn(3, L"进程路径", LVCFMT_LEFT, rect1.right / 2);
	m_1.DeleteColumn(0);

	//4.设置列表控件的样式
	m_2.SetExtendedStyle(LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES);

	//5.给列表添加标题（2列）
	CRect rect2;
	m_2.GetClientRect(rect2);
	m_2.InsertColumn(0, L"", LVCFMT_CENTER, 100);
	m_2.InsertColumn(1, L"DLL模块", LVCFMT_LEFT, rect2.right / 3);
	m_2.InsertColumn(2, L"DLL模块位置", LVCFMT_LEFT, 2 * rect2.right / 3);
	m_2.DeleteColumn(0);

	//5.遍历进程
	EnumProcess();

	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}

// 如果向对话框添加最小化按钮，则需要下面的代码
//  来绘制该图标。  对于使用文档/视图模型的 MFC 应用程序，
//  这将由框架自动完成。
void CDLLINJECTDlg::OnPaint()
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
HCURSOR CDLLINJECTDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

//浏览按钮
void CDLLINJECTDlg::OnBnClickedButton3()
{
	CFileDialog dlg(TRUE, NULL, NULL, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, _T("DLL Files(*.dll)|*.DLL||"), AfxGetMainWnd());

	//模态对话框显示出来
	CString strPath;
	if (dlg.DoModal() == IDOK)
	{
		strPath = dlg.GetPathName();
	}
	//将控件文本设置为文件路径
	m_3.SetWindowTextW(strPath);
}

enum COLOR
{
	COLOR_RED = -2,
	COLOR_GREEN = -3,
	COLOR_BLUE = -4,
};

//拖动文件
void CDLLINJECTDlg::OnDropFiles(HDROP hDropInfo)
{
	//1.获得拖拽的文件的文件名
	wchar_t FileName[MAX_PATH + 1] = { 0 };
	::DragQueryFileW(hDropInfo, 0, FileName, MAX_PATH);
	//2.打开文件，获得句柄
	HANDLE hFile = CreateFile(FileName, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	//3.读取文件的DOS头部分
	IMAGE_DOS_HEADER dosHeader;
	DWORD dwRead;
	ReadFile(hFile, &dosHeader, sizeof(dosHeader), &dwRead, NULL);
	if (dosHeader.e_magic != IMAGE_DOS_SIGNATURE)
	{
		m_4.SetWindowTextW(_T("不是有效的MZ标志"));
		return;
	}
	//4.读取文件的NT头
	IMAGE_NT_HEADERS ntHeaders;
	SetFilePointer(hFile, dosHeader.e_lfanew, NULL, FILE_BEGIN);
	ReadFile(hFile, &ntHeaders, sizeof(ntHeaders), &dwRead, NULL);
	if (ntHeaders.Signature != IMAGE_NT_SIGNATURE)
	{
		m_4.SetWindowTextW(_T("不是有效的PE标记"));
		return;
	}
	if (ntHeaders.OptionalHeader.Magic != 0x10b)
	{
		m_4.SetWindowTextW(_T("不是32位的PE文件"));
		return;
	}
	//5.检测是不是dll
	CString str1 = PathFindExtensionW(FileName);
	CString str2 = _T(".dll");
	CString str3 = _T(".DLL");
	if (str1 != str2 && str1 != str3)
	{
		m_4.SetWindowTextW(_T("不是有效的DLL文件"));
		return;
	}
	else
	{
		m_4.SetWindowTextW(_T("有效的DLL文件"));
	}
		
	//6.将文件名设置为编辑框的变量
	m_3.SetWindowTextW(FileName);
	//7.关闭句柄
	CloseHandle(hFile);

	CDialogEx::OnDropFiles(hDropInfo);
}

//注释掉防止在编辑框按ENTER键闪退
void CDLLINJECTDlg::OnOK()
{
	// TODO: 在此添加专用代码和/或调用基类
	//CDialogEx::OnOK();
}


//比较DLL路径与注入的DLL的路径是否相同，来查找DLL
//用途1：检查DLL是否在里面
//用途2：传出目标DLL的基址，以便卸载目标DLL时使用
BOOL CDLLINJECTDlg::CheckDLL(IN DWORD dwPid, IN CString g_szDllPath, OUT HMODULE* pMODULE)
{
	//1.定义模块信息结构体
	MODULEENTRY32 me32 = { sizeof(MODULEENTRY32) };  //定义结构体用于保存遍历的进程信息

	//2.创建模块快照
	HANDLE hModuleSnap = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, dwPid);
	int n = GetLastError();
	if (hModuleSnap == INVALID_HANDLE_VALUE)
	{
		//m_4.SetWindowTextW(_T("1创建模块快照失败"));
		return FALSE;
	}

	//3.通过模块快照句柄获取第一个模块
	BOOL bRet = Module32First(hModuleSnap, &me32);
	if (!bRet)
	{
		CloseHandle(hModuleSnap);
		//m_4.SetWindowTextW(_T("1获取第一个模块失败"));
		return FALSE;
	}

	//4.循环获取模块信息,直到获得注入DLL的模块基址
	while (bRet)
	{
		if (!StrCmpW(me32.szExePath, g_szDllPath))
		{
			*pMODULE = me32.hModule;
			CloseHandle(hModuleSnap);
			return TRUE;
		}

		bRet = Module32Next(hModuleSnap, &me32);
	}

	//5.如果所有都遍历过，没有找到
	CloseHandle(hModuleSnap);
	return FALSE;
}

//注入DLL按钮
void CDLLINJECTDlg::OnBnClickedButton1()
{
	//1.打开要被注入的进程
	HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, PID);

	if (!hProcess)
	{
		m_4.SetWindowTextW(_T("打开要被注入的进程失败"));
		return ;
	}

	//2.在要被注入的进程中创建内存，用于存放注入dll的路径
	m_3.GetWindowTextW(g_szDllPath);
	const wchar_t* wstr = (LPCTSTR)g_szDllPath;
	char str[100] = { 0 };
	wcstombs(str, wstr, wcslen(wstr));
	DWORD DllPathLen = strlen(str) + 1;
	LPVOID Buff = VirtualAllocEx(hProcess, NULL, DllPathLen, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
	if (Buff == NULL)
	{
		m_4.SetWindowTextW(_T("申请内存失败"));
		return ;
	}

	//3.将dll路径写入刚刚创建的内存中	
	DWORD write_len = 0;
	WriteProcessMemory(hProcess, Buff, str, DllPathLen, &write_len);
	if (DllPathLen != write_len)
	{
		m_4.SetWindowTextW(_T("WriteProcessMemory失败"));
		return ;
	}

	//4.从kernel32.dll中获取LoadLibrary函数
	LPVOID LoadLibraryBase = GetProcAddress(GetModuleHandle(_T("kernel32.dll")), "LoadLibraryA");
	if (LoadLibraryBase == NULL)
	{
		m_4.SetWindowTextW(_T("获取LoadLibrary函数指针失败"));
		return ;
	}

	//5.加载ntdll.dll并从中获取内核函数ZwCreateThread，并使用函数指针指向此函数
	HMODULE Ntdll = LoadLibrary(_T("ntdll.dll"));
	pZwCreateThreadEx ZwCreateThreadEx =(pZwCreateThreadEx)GetProcAddress(Ntdll, "ZwCreateThreadEx");
	if (ZwCreateThreadEx == NULL)
	{
		m_4.SetWindowTextW(_T("获取ZwCreateThreadEx函数指针失败"));
		return ;
	}

	//6.执行ZwCreateThread函数，在指定进程中创建线程加载要被注入的dll
	HANDLE hRemoteThread = NULL;
	DWORD dwStatus = ZwCreateThreadEx(&hRemoteThread,PROCESS_ALL_ACCESS,NULL,hProcess,(LPTHREAD_START_ROUTINE)LoadLibraryBase,Buff,0, 0, 0, 0,NULL);
	if (hRemoteThread == NULL)
	{
		m_4.SetWindowTextW(_T("执行ZwCreateThread函数失败"));
		return ;
	}

	
	//7.释放不需要的变量以及内存
	CloseHandle(hProcess);
	FreeModule(Ntdll);
	VirtualFreeEx(hProcess, Buff, 0, MEM_RELEASE);
	CloseHandle(hRemoteThread);	

	MessageBox(_T("注入DLL成功"));
}

//卸载DLL按钮
void CDLLINJECTDlg::OnBnClickedButton2()
{
	//1.判断DLL路径是否为空
	if (szDLLPATH.IsEmpty())
	{
		m_4.SetWindowTextW(_T("DLL路径为空，没有注入DLL"));
		return;
	}
	//2.检查目标进程内是否有指定要卸载的DLL   
	//参数1 PID：之前单击进程列表时触发记录
	//参数2 szDLLPATH：之前单击模块列表时触发记录
	//参数3 g_hModule:函数遍历模块寻找到目标DLL时传出
	if (!CheckDLL(IN PID, IN szDLLPATH, OUT &g_hModule))
		return;

	//3.打开目标进程，获得打开的句柄
	HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, PID);
	if (!hProcess)
	{
		m_4.SetWindowTextW(_T("打开要被注入的进程失败"));
		return;
	}

	//4.从kernel32.dll中获取FreeLibrary函数指针
	LPVOID FreeLibraryBase = GetProcAddress(GetModuleHandle(_T("kernel32.dll")), "FreeLibrary");
	if (FreeLibraryBase == NULL)
	{
		m_4.SetWindowTextW(_T("获取FreeLibrary函数指针失败"));
		return;
	}

	//5.加载ntdll.dll并从中获取内核函数ZwCreateThread，并使用函数指针指向此函数
	HMODULE Ntdll = LoadLibrary(_T("ntdll.dll"));
	pZwCreateThreadEx ZwCreateThreadEx = (pZwCreateThreadEx)GetProcAddress(Ntdll, "ZwCreateThreadEx");
	if (ZwCreateThreadEx == NULL)
	{
		m_4.SetWindowTextW(_T("获取ZwCreateThreadEx函数指针失败"));
		return;
	}

	//6.执行ZwCreateThread函数，在指定进程中创建线程卸载被注入的dll  //在此之前必须先遍历目标进程内的模块，找到模块的基址
	HANDLE hRemoteThread = NULL;
	DWORD dwStatus = ZwCreateThreadEx(&hRemoteThread, PROCESS_ALL_ACCESS, NULL, hProcess, (LPTHREAD_START_ROUTINE)FreeLibraryBase, g_hModule, 0, 0, 0, 0, NULL);
	if (hRemoteThread == NULL)
	{
		m_4.SetWindowTextW(_T("执行ZwCreateThread函数失败"));
		return;
	}
	
	//7.等待线程结束
	CloseHandle(hRemoteThread);
	CloseHandle(hProcess);

	MessageBox(_T("卸载DLL成功"));			
}


//------------------------------------进程列表相关---------------------------------
//进程遍历
void CDLLINJECTDlg::EnumProcess()
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
		m_1.SetItemText(0, 1, str1);

		//6.3 根据PID，获得打开进程的句柄
		HANDLE hProc = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, pi.th32ProcessID);

		//6.4 根据打开进程的句柄获得进程的路径		
		TCHAR processPath[MAX_PATH] = L"";

		GetModuleFileNameEx(hProc, 0, processPath, MAX_PATH);
		m_1.SetItemText(0, 2, processPath);
		
		//6.5.遍历下一个快照
		bRet = Process32Next(hSnapshot, &pi);
	}
	return;
}

//进程列表中鼠标左键单击某个进程时触发
void CDLLINJECTDlg::OnNMClickList1(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
	
	NMLISTVIEW *pNMListView = (NMLISTVIEW*)pNMHDR; 
	if (-1 != pNMListView->iItem)
	{
		//1.获取选中行的行号
		nIndex_1 = m_1.GetSelectionMark();

		//2.GetItemText：获取选中行的文本内容 （因为PID是在每一行的第2列，所以参数2为1）
		//参数1：行号
		//参数2：这一行的第几列   //0为第1列，1为第2列
		CString str = m_1.GetItemText(nIndex_1, 1);

		//3.将文本内容转化为整数
		PID = _tstoi(LPCTSTR(str));

		//4.遍历模块
		EnumModule(PID);

	}
	
	*pResult = 0;
}

//进程列表中鼠标右键单击某个进程时触发
void CDLLINJECTDlg::OnNMRClickList1(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
	

	CMenu* pSubMenu = m_menu.GetSubMenu(0);
	CPoint pos;
	GetCursorPos(&pos);
	pSubMenu->TrackPopupMenu(0, pos.x, pos.y, this);
	
	*pResult = 0;
}

//进程列表右键刷新
void CDLLINJECTDlg::OnReNew()
{
	//1.首先清空所有列表内容
	m_1.DeleteAllItems();

	//2.设置列表控件的样式
	m_1.SetExtendedStyle(LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES);

	//3.给列表添加标题（3列）
	CRect rect1;
	m_1.GetClientRect(rect1);  //获取列表的长度，以便均匀划分
	m_1.InsertColumn(0, L"", LVCFMT_CENTER, 100);
	m_1.InsertColumn(1, L"进程", LVCFMT_LEFT, rect1.right / 4);
	m_1.InsertColumn(2, L"PID", LVCFMT_CENTER, rect1.right / 4);
	m_1.InsertColumn(3, L"进程路径", LVCFMT_LEFT, rect1.right / 2);
	m_1.DeleteColumn(0);

	//4.设置列表控件的样式
	m_2.SetExtendedStyle(LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES);

	//5.给列表添加标题（2列）
	CRect rect2;
	m_2.GetClientRect(rect2);
	m_2.InsertColumn(0, L"", LVCFMT_CENTER, 100);
	m_2.InsertColumn(1, L"DLL模块", LVCFMT_LEFT, rect2.right / 3);
	m_2.InsertColumn(2, L"DLL模块位置", LVCFMT_LEFT, 2 * rect2.right / 3);
	m_2.DeleteColumn(0);

	//6.遍历进程
	EnumProcess();
}


//--------------------------------------模块列表相关-----------------------------------
//模块遍历
void CDLLINJECTDlg::EnumModule(DWORD dwPId)
{
	//1.清空列表的所有内容
	m_2.DeleteAllItems();

	//2.创建指定进程的所有模块快照
	MODULEENTRY32 me32 = { sizeof(MODULEENTRY32) };
	HANDLE hModuleSnap = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, dwPId);
	if (hModuleSnap == INVALID_HANDLE_VALUE)
		return;

	//3.通过模块快照句柄获取第一个模块信息
	if (!Module32First(hModuleSnap, &me32))
	{
		CloseHandle(hModuleSnap);
		return;
	}

	//4.循环获取模块信息
	do {
		//4.1 申请一行，并且插入本行的第1列
		m_2.InsertItem(0, me32.szModule);

		//4.2 插入本行的第2列
		m_2.SetItemText(0, 1, me32.szExePath);

	} while (Module32Next(hModuleSnap, &me32));

	//5.关闭句柄并退出函数
	CloseHandle(hModuleSnap);
	return;
}

//模块列表鼠标左键单击事件   //获取要卸载DLL的路径，存放到szDLLPATH
void CDLLINJECTDlg::OnNMClickList2(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
	
	//1.获取选中行的行号
	int nIndex = m_2.GetSelectionMark();

	//2.GetItemText：获取选中行的文本内容 （因为PID是在每一行的第2列，所以参数2为1）
	//参数1：行号
	//参数2：这一行的第几列   //0为第1列，1为第2列
	szDLLPATH = m_2.GetItemText(nIndex, 1);     

	*pResult = 0;
}

//-----------------------------------控件焦点----------------------------------
//当控件失去焦点时
void CDLLINJECTDlg::OnNMKillfocusList1(NMHDR *pNMHDR, LRESULT *pResult)
{	
	m_nSelectedItemIndex = m_1.GetSelectionMark();
	m_1.SetItemState(m_nSelectedItemIndex, LVIS_DROPHILITED, LVIS_DROPHILITED);

	*pResult = 0;
}

//当控件获得焦点时
void CDLLINJECTDlg::OnNMSetfocusList1(NMHDR *pNMHDR, LRESULT *pResult)
{
	m_1.SetItemState(m_nSelectedItemIndex, FALSE, LVIF_STATE);
	*pResult = 0;
}
