
// FTPClientDlg.cpp: 实现文件
//

#include "stdafx.h"
#include "FTPClient.h"
#include "FTPClientDlg.h"
#include "afxdialogex.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

CString filename;
CString ServerIP;
UINT ServerPort;
//UINT SendUpload(PVOID hWnd);
//UINT SendDownload(PVOID hWnd);
// 用于应用程序“关于”菜单项的 CAboutDlg 对话框

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


// CFTPClientDlg 对话框



CFTPClientDlg::CFTPClientDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_FTPCLIENT_DIALOG, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CFTPClientDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_IPADDRESS, m_ip);
	DDX_Control(pDX, IDC_LOG, m_log);
	DDX_Text(pDX, IDC_PORT, m_port);
	DDX_Control(pDX, IDC_FILELIST, m_fileList);
}

BEGIN_MESSAGE_MAP(CFTPClientDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_CREATE, &CFTPClientDlg::OnBnClickedCreate)
	ON_BN_CLICKED(IDC_UPLOAD, &CFTPClientDlg::OnBnClickedUpload)
	ON_BN_CLICKED(IDC_GET, &CFTPClientDlg::OnBnClickedGet)
	ON_BN_CLICKED(IDC_DOWNLOAD, &CFTPClientDlg::OnBnClickedDownload)
	ON_WM_DESTROY()
	ON_WM_TIMER()
END_MESSAGE_MAP()


// CFTPClientDlg 消息处理程序

BOOL CFTPClientDlg::OnInitDialog()
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
	m_ip.SetAddress(BYTE(127), BYTE(0), BYTE(0), BYTE(1));
	srand((unsigned)time(NULL));
	m_port = ((UINT)rand()) % 1000 + 2000;
	UpdateData(FALSE);

	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}

void CFTPClientDlg::OnSysCommand(UINT nID, LPARAM lParam)
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

void CFTPClientDlg::OnPaint()
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
HCURSOR CFTPClientDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}



void CFTPClientDlg::OnBnClickedCreate()
{
	// TODO: 在此添加控件通知处理程序代码
	UpdateData(TRUE);
	CString temp;
	GetDlgItemText(IDC_PORT, temp);
	clientPort = _ttoi(temp);
	m_sock.dlg = this;
	m_sock.num = 1;
	if (m_sock.Create(clientPort, SOCK_DGRAM, FD_READ | FD_WRITE, nullptr)) {
		m_log.AddString(L"----套接字创建成功----");
	}
	else {
		MessageBoxA(NULL, "创建套接字错误", "FAIL", MB_OK);
		return;
	}
	BYTE a1, a2, a3, a4;
	m_ip.GetAddress(a1, a2, a3, a4);
	clientIP.Format(CString("%d.%d.%d.%d"), a1, a2, a3, a4);
}


void CFTPClientDlg::OnBnClickedUpload()
{
	// TODO: 在此添加控件通知处理程序代码
	//CWinThread *m_thread;
	//WORD m = 800;
	//m_thread = AfxBeginThread(SendUpload, &m, THREAD_PRIORITY_NORMAL);
	//if (!m_thread) {
	//	MessageBox(L"上传文件线程创建失败！");
	//	return;
	//}

	CProDlg *dlg;
	dlg = new CProDlg();
	dlg->operation = 800;
	dlg->Create(IDD_DIALOG1);
	dlg->ShowWindow(SW_SHOW);
}


void CFTPClientDlg::OnBnClickedGet()
{
	// TODO: 在此添加控件通知处理程序代码
	Pack pack;
	pack.Operation = 200;
	int nLength = m_sock.SendTo(&pack, sizeof(pack), 2333, L"127.0.0.1", 0);
	if (nLength != -1) {
		SetTimer(2, 1000, NULL);
		m_log.AddString(L"----正在向服务器发送获取文件列表请求----");
	}
	else {
		MessageBoxA(NULL, "请求文件列表失败", "FAIL", MB_OK);
	}
}

void CFTPClientDlg::OnBnClickedDownload()
{
	// TODO: 在此添加控件通知处理程序代码
	Pack pack;
	pack.Operation = 300;
	m_fileList.GetText(m_fileList.GetCurSel(), filename);

	int length = filename.GetLength();
	int i;
	for (i = 0; i < length; i++) {
		pack.Content[i] = filename.GetAt(i);
	}
	pack.Content[i++] = 0;
	if (m_sock.SendTo(&pack, sizeof(pack), (UINT)2333, L"127.0.0.1", 0) != -1) {
		m_log.AddString(L"----正在向服务器发送下载文件" + filename + L"请求----");
		SetTimer(1, 1000, NULL);
	}
	else {
		MessageBoxA(NULL, "发送下载文件请求失败", "FAIL", MB_OK);
		return;
	}
}

void CFTPClientDlg::OnReceive() {
	Pack RecvPack;
	CString m_ip;
	UINT m_port;

	int nLength = m_sock.ReceiveFrom(&RecvPack, sizeof(RecvPack), m_ip, m_port);
	ServerPort = m_port;
	ServerIP = m_ip;
	if (nLength != 0) {
		switch (RecvPack.Operation) {
			case 299: {
				m_log.AddString(L"收到服务端的目录列表");
				KillTimer(2);
				m_fileList.ResetContent();
				CString str1;
				wchar_t * temp;
				temp = new wchar_t[100];
				memset(temp, 0, 100);
				int j = 0;
				for (int i = 0; RecvPack.Content[i]; i++) {
					temp[j] = RecvPack.Content[i];
					str1 += temp[j];
					j++;
					if (RecvPack.Content[i] == '\n') {
						if (!str1.IsEmpty())
							m_fileList.AddString(str1);
						memset(temp, 0, 100);
						j = 0;
						str1.Empty();
					}
				}
				break;
			}
			case 301: {
				CString str1;
				CString str2;
				int i;
				for (i = 0; RecvPack.Content[i] != 0; i++) {
					str2 += wchar_t(RecvPack.Content[i]);
				}
				str2 += wchar_t(RecvPack.Content[i]);
				str1.Format(L"收到--%d 传输文件 %s 确认请求", m_port, str2);
				KillTimer(1);
				filename = str2;
				//CWinThread *m_thread;
				//WORD m = 310;
				//m_thread = AfxBeginThread(SendDownload, &m, THREAD_PRIORITY_NORMAL);
				//if (!m_thread) {
				//	MessageBox(L"下载文件线程创建失败！");
				//	return;
				//}	
				CProDlg *dlg;
				dlg = new CProDlg();
//				WORD* op = (WORD*)(hWnd);
				dlg->operation = 310;
				dlg->DownLoadFileName = filename;
				dlg->ServerIP = ServerIP;
				dlg->ServerPort = ServerPort;
				dlg->Create(IDD_DIALOG1);
				dlg->ShowWindow(SW_SHOW);
				break;
			}
		}
	}
}

//UINT SendUpload(PVOID hWnd) {
//	CProDlg *dlg;
//	dlg = new CProDlg();
//	WORD* op = (WORD*)(hWnd);
//	dlg->operation = *op;
//	dlg->Create(IDD_DIALOG1);
//	dlg->ShowWindow(SW_SHOW);
//	return 0;
//}

//UINT SendDownload(PVOID hWnd) {
//
//	return 0;
//}

void CFTPClientDlg::OnDestroy()
{
	CDialogEx::OnDestroy();
	m_sock.Close();
	// TODO: 在此处添加消息处理程序代码
}


void CFTPClientDlg::OnTimer(UINT_PTR nIDEvent)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	if (nIDEvent == 1) {
		OnBnClickedDownload();
	}
	if (nIDEvent == 2) {
		OnBnClickedGet();
	}
	CDialogEx::OnTimer(nIDEvent);
}
