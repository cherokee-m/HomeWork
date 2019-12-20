
// FTPServerDlg.cpp: 实现文件
//

#include "stdafx.h"
#include "FTPServer.h"
#include "FTPServerDlg.h"
#include "afxdialogex.h"
#include <string>
#include <io.h>
using namespace std;

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// 用于应用程序“关于”菜单项的 CAboutDlg 对话框
CString ClientIP;
UINT ClientPort;
CString UploadFileName;
CString DownloadFileName;
CString m_addr;
UINT SendUpload(PVOID hWnd);
UINT SendDownload(PVOID hWnd);
class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_ABOUTBOX };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

// 实现
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


// CFTPServerDlg 对话框



CFTPServerDlg::CFTPServerDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_FTPSERVER_DIALOG, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CFTPServerDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_FILELIST, m_flist);
	DDX_Text(pDX, IDC_FILEADDR, m_addr);
	DDX_Control(pDX, IDC_LOG, m_log);
}

BEGIN_MESSAGE_MAP(CFTPServerDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_REFRESH, &CFTPServerDlg::OnBnClickedRefresh)
	ON_BN_CLICKED(IDC_SELCFILE, &CFTPServerDlg::OnBnClickedSelcfile)
	ON_EN_CHANGE(IDC_FILEADDR, &CFTPServerDlg::OnEnChangeFileaddr)
	ON_WM_DESTROY()
END_MESSAGE_MAP()


// CFTPServerDlg 消息处理程序

BOOL CFTPServerDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// 将“关于...”菜单项添加到系统菜单中。

	// IDM_ABOUTBOX 必须在系统命令范围内。
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

	// 设置此对话框的图标。  当应用程序主窗口不是对话框时，框架将自动
	//  执行此操作
	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);		// 设置小图标

	// TODO: 在此添加额外的初始化代码
	// 获取本地文件夹文件列表
	OnBnClickedRefresh();
	
	//创建套接字
	m_sock.dlg = this;
	m_sock.num = 1;
	if (m_sock.Create(2333, SOCK_DGRAM, FD_READ | FD_WRITE, nullptr)) {
		m_log.AddString(L"----套接字创建成功----");
	}
	else {
		MessageBoxA(NULL, "FAIL", "创建套接字错误", MB_OK);
		return TRUE;
	}

	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}

void CFTPServerDlg::OnSysCommand(UINT nID, LPARAM lParam)
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

// 如果向对话框添加最小化按钮，则需要下面的代码
//  来绘制该图标。  对于使用文档/视图模型的 MFC 应用程序，
//  这将由框架自动完成。

void CFTPServerDlg::OnPaint()
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
HCURSOR CFTPServerDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

void CFTPServerDlg::OnBnClickedRefresh()
{
	// TODO: 在此添加控件通知处理程序代码
	dir = L"";
	UpdateData(TRUE);
	m_flist.ResetContent();
	USES_CONVERSION;
	string p(W2A(m_addr));
	long hFile = 0;
	_finddata_t fileinfo;
	if ((hFile = _findfirst(p.append("*").c_str(), &fileinfo)) != -1)
	{
		do
		{
			if (!(fileinfo.attrib & _A_SUBDIR))
			{
				CString str(fileinfo.name);
				m_flist.AddString(str);
				dir += fileinfo.name;
				dir += "\n";
			}
		} while (_findnext(hFile, &fileinfo) == 0);
	}
	_findclose(hFile);
}

CString CFTPServerDlg::GetDirectory() {
	TCHAR szFolderPath[MAX_PATH] = { 0 };
	CString strFolderPath = TEXT("");
	BROWSEINFO sInfo;
	::ZeroMemory(&sInfo, sizeof(BROWSEINFO));
	sInfo.pidlRoot = 0;
	sInfo.lpszTitle = L"请选择本地文件服务器路径";
	sInfo.ulFlags = BIF_RETURNONLYFSDIRS | BIF_EDITBOX | BIF_DONTGOBELOWDOMAIN;
	sInfo.lpfn = NULL;
	LPITEMIDLIST lpidlBrowse = ::SHBrowseForFolder(&sInfo);
	if (lpidlBrowse != NULL)
		if (::SHGetPathFromIDList(lpidlBrowse, szFolderPath))
			strFolderPath = szFolderPath;
	if (lpidlBrowse != NULL)
		::CoTaskMemFree(lpidlBrowse);
	return strFolderPath;
}

void CFTPServerDlg::OnBnClickedSelcfile()
{
	m_addr = GetDirectory() + L"\\";
	UpdateData(FALSE);
	OnBnClickedRefresh();
	// TODO: 在此添加控件通知处理程序代码
}

void CFTPServerDlg::OnEnChangeFileaddr()
{
	// TODO:  如果该控件是 RICHEDIT 控件，它将不
	// 发送此通知，除非重写 CDialogEx::OnInitDialog()
	// 函数并调用 CRichEditCtrl().SetEventMask()，
	// 同时将 ENM_CHANGE 标志“或”运算到掩码中。

	// TODO:  在此添加控件通知处理程序代码
	OnBnClickedRefresh();
}

void CFTPServerDlg::OnReceive() {

	Pack RecvPack;
	int nLength = m_sock.ReceiveFrom(&RecvPack, sizeof(RecvPack), ClientIP, ClientPort, 0);
	if (nLength != 0) {
		switch (RecvPack.Operation) {
			case 200: {
				CString str1;
				str1.Format(L"收到--%d 文件列表请求", ClientPort);
				m_log.AddString(str1);
				Pack pack;
				pack.Operation = 299;
				int length = dir.GetLength();
				int i;
				for (i = 0; i < length; i++) {
					pack.Content[i] = dir.GetAt(i);
				}
				pack.Content[i++] = 0;
				if (m_sock.SendTo(&pack, sizeof(pack), ClientPort, ClientIP) != -1) {
					m_log.AddString(L"发送目录成功");
				}
				break;
			}
			case 800: {
				CString str1;
				CString str2;
				int i;
				for (i = 0; RecvPack.Content[i] != 0; i++) {
					str2 += wchar_t(RecvPack.Content[i]);
				}
				str2 += wchar_t(RecvPack.Content[i]);
				str1.Format(L"收到--%d 上传文件 %s 请求", ClientPort, str2);
				m_log.AddString(str1);
				UploadFileName = str2;
				WORD m = 801;
				CWinThread *m_thread;
				m_thread = AfxBeginThread(SendUpload, &m, THREAD_PRIORITY_NORMAL);
				if (!m_thread) {
					MessageBox(L"上传文件线程创建失败！");
					return;
				}
				break;
			}
			case 300: {
				CString str1;
				CString str2;
				int i;
				for (i = 0; RecvPack.Content[i] != 0; i++) {
					str2 += wchar_t(RecvPack.Content[i]);
				}
				str2 += wchar_t(RecvPack.Content[i]);
				str1.Format(L"收到--%d 下载文件 %s 请求", ClientPort, str2);
				m_log.AddString(str1);
				DownloadFileName = str2;
				CWinThread *m_thread;
				WORD m = 301;
				m_thread = AfxBeginThread(SendDownload, &m, THREAD_PRIORITY_NORMAL);
				if (!m_thread) {
					MessageBox(L"下载文件线程创建失败！");
					return;
				}
				break;
			}
		}
	}
}

UINT SendUpload(PVOID hWnd) {
	CProDlg dlg;
	WORD* op = (WORD*)(hWnd);
	dlg.operation = *op;
	dlg.ClientIP = ClientIP;
	dlg.ClientPort = ClientPort;
	dlg.UploadFileName = UploadFileName;
	dlg.LocalAddr = m_addr;
	dlg.DoModal();
	return 0;
}

UINT SendDownload(PVOID hWnd) {
	CProDlg dlg;
	WORD* op = (WORD*)(hWnd);
	dlg.operation = *op;
	dlg.ClientIP = ClientIP;
	dlg.ClientPort = ClientPort;
	DownloadFileName = DownloadFileName.Left(DownloadFileName.Find(L'\n'));
	dlg.DownloadFileName = DownloadFileName;
	dlg.LocalAddr = m_addr;
	dlg.DoModal();
	return 0;
}

void CFTPServerDlg::OnDestroy()
{
	CDialogEx::OnDestroy();
	m_sock.Close();
	// TODO: 在此处添加消息处理程序代码
}
