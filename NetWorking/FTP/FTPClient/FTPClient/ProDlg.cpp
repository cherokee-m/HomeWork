// ProDlg.cpp: 实现文件
//

#include "stdafx.h"
#include "FTPClient.h"
#include "ProDlg.h"
#include "afxdialogex.h"


// CProDlg 对话框
WORD Checksum(WORD * buffer, int size);
IMPLEMENT_DYNAMIC(CProDlg, CDialogEx)

CProDlg::CProDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_DIALOG1, pParent)
{
	window_size = 4;
	send_base = 0;
	nextseqnum = 0;
	ack = 0;
}

CProDlg::~CProDlg()
{
}

void CProDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LOG, m_log);
}


BEGIN_MESSAGE_MAP(CProDlg, CDialogEx)
	ON_WM_TIMER()
	ON_WM_CLOSE()
//	ON_WM_DESTROY()
//	ON_WM_DESTROYCLIPBOARD()
//ON_WM_NCDESTROY()
END_MESSAGE_MAP()


// CProDlg 消息处理程序


BOOL CProDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// TODO:  在此添加额外的初始化
	m_sock.dlg = this;
	m_sock.num = 2;
	srand((unsigned)time(NULL));
	m_port = ((UINT)rand()) % 1000 + 4001;
	if (m_sock.Create(m_port, SOCK_DGRAM, FD_READ | FD_WRITE, nullptr)) {
		CString str1;
		str1.Format(L"创建套接字%d", m_port);
		m_log.AddString(str1);
	}
	else {
		MessageBoxA(NULL, "FAIL", "创建套接字错误", MB_OK);
		return TRUE;
	}
	initPack();

	return TRUE;  // return TRUE unless you set the focus to a control
				  // 异常: OCX 属性页应返回 FALSE
}

void CProDlg::OnReceive() {
	Pack RecvPack;
	int nLength = m_sock.ReceiveFrom(&RecvPack, sizeof(RecvPack), ServerIP, ServerPort, 0);
	FilePack *filePack = (FilePack*)&RecvPack;
	if (nLength != 0) {
		switch (RecvPack.Operation) {
		case 801: {
			m_log.AddString(L"收到服务器确认接收文件 " + UploadFileName + L" 响应");
			KillTimer(2);
			OpenReadFile();
			SendFile();
			break;
		}
		case 811: {
			CString str1;
			str1.Format(L"收到ACK%d确认包", RecvPack.Content[0]);
			m_log.AddString(str1);
			if (RecvPack.Content[0] == 0) {
				KillTimer(1);
				CString str;
				str.Format(L"关闭计时器,全部文件已得到确认传输完毕");
				m_log.AddString(str);
				m_sock.Close();
			}
			else if (send_base + 511 == RecvPack.Content[0]) {
				send_base++;
				SendFile();
			}
			break;
		}
		case 311: {
			WORD check_buff[501];
			memcpy(check_buff, &(filePack->Checksum), filePack->Length + 2);
			if (Checksum(check_buff, filePack->Length + 2) == 0) {
				if (filePack->SeqNum == 0) {
					CString str;
					str.Format(L"成功接收最后一个数据包");
					m_log.AddString(str);
					ack++;
					file.SeekToEnd();
					file.Write(filePack->Content, filePack->Length);
					Pack pack;
					pack.Operation = 320;
					pack.Content[0] = 0;
					if (m_sock.SendTo(&pack, sizeof(pack), ServerPort, ServerIP, 0)) {
						CString str1;
						str1.Format(L"成功发送ACK0");
						m_log.AddString(str1);
					}
					file.Close();
					m_sock.Close();
				}
				else if (ack == filePack->SeqNum - 311) {
					CString str;
					str.Format(L"成功接收第%d个数据包", filePack->SeqNum - 310);
					m_log.AddString(str);
					ack++;
					file.SeekToEnd();
					file.Write(filePack->Content, filePack->Length);
					Pack pack;
					pack.Operation = 320;
					pack.Content[0] = filePack->SeqNum;
					if (m_sock.SendTo(&pack, sizeof(pack), ServerPort, ServerIP, 0)) {
						CString str1;
						str1.Format(L"成功发送ACK%d", pack.Content[0]);
						m_log.AddString(str1);
					}
				}
				else {
					Pack pack;
					pack.Operation = 320;
					pack.Content[0] = ack + 310;
					if (m_sock.SendTo(&pack, sizeof(pack), ServerPort, ServerIP, 0)) {
						CString str1;
						str1.Format(L"成功发送ACK%d", pack.Content[0]);
						m_log.AddString(str1);
					}
				}
			}
			else {
				Pack pack;
				pack.Operation = 320;
				pack.Content[0] = ack + 310;
				if (m_sock.SendTo(&pack, sizeof(pack), ServerPort, ServerIP, 0)) {
					CString str1;
					str1.Format(L"数据错误发送ACK%d", pack.Content[0]);
					m_log.AddString(str1);
				}
			}
			break;
		}
		}
	}
}

void CProDlg::initPack() {
	switch (operation) {
	case 800: {
		CString defaultDir = L"";
		CString filename = L"";
		CString filter = L"所有文件 (*.*)|*.*||";
		CFileDialog fileDlg(TRUE, defaultDir, filename, OFN_HIDEREADONLY | OFN_READONLY, filter, NULL);
		INT_PTR result = fileDlg.DoModal();
		if (result == IDOK) {
			UploadFileName = fileDlg.GetFileName();
			FileAddr = fileDlg.GetPathName();
			Pack pack;
			pack.Operation = 800;
			int length = UploadFileName.GetLength();
			int i;
			for (i = 0; i < length; i++) {
				pack.Content[i] = UploadFileName.GetAt(i);
			}
			pack.Content[i++] = 0;
			if (m_sock.SendTo(&pack, sizeof(pack), (UINT)2333, L"127.0.0.1", 0) != -1) {
				m_log.AddString(L"----正在向服务器发送上传文件" + UploadFileName + L"请求----");
				SetTimer(2, 1000, NULL);
			}
			else {
				MessageBoxA(NULL, "上传文件请求失败", "FAIL", MB_OK);
			}
		}
		else {
			CProDlg::EndDialog(0);
		}
		break;
	}
	case 310: {
		TCHAR szFolderPath[MAX_PATH] = { 0 };
		CString strFolderPath = TEXT("");
		BROWSEINFO sInfo;
		::ZeroMemory(&sInfo, sizeof(BROWSEINFO));
		sInfo.pidlRoot = 0;
		sInfo.lpszTitle = L"请选择本地存储位置";
		sInfo.ulFlags = BIF_RETURNONLYFSDIRS | BIF_EDITBOX | BIF_DONTGOBELOWDOMAIN;
		sInfo.lpfn = NULL;
		LPITEMIDLIST lpidlBrowse = ::SHBrowseForFolder(&sInfo);
		if (lpidlBrowse != NULL)
			if (::SHGetPathFromIDList(lpidlBrowse, szFolderPath))
				strFolderPath = szFolderPath;
		if (lpidlBrowse != NULL)
			::CoTaskMemFree(lpidlBrowse);
		StoreAddr = strFolderPath + L"\\" + DownLoadFileName;
		
		Pack pack;
		pack.Operation = 310;
		int length = DownLoadFileName.GetLength();
		int i;
		for (i = 0; i < length; i++) {
			pack.Content[i] = DownLoadFileName.GetAt(i);
		}
		pack.Content[i++] = 0;
		if (m_sock.SendTo(&pack, sizeof(pack), ServerPort, ServerIP)) {
			file.Open(StoreAddr, CFile::modeCreate | CFile::modeWrite | CFile::typeBinary);
			m_log.AddString(L"发送下载确认310成功");
		}

		break;
	}
	}
}

void CProDlg::OpenReadFile() {
	CFile t_file;
	t_file.Open(FileAddr, CFile::modeRead | CFile::typeBinary);
	// 二进制读
	packnum = t_file.GetLength() / 1000 + 1;
	CString str3;
	str3.Format(L"本次文件将分为%d个数据包传输", packnum);
	m_log.AddString(str3);
	dataLength = t_file.GetLength();
	pBuf = new char[t_file.GetLength() + 1];
	t_file.Read(pBuf, t_file.GetLength());
	t_file.Close();
}

void CProDlg::SendFile() {
	while (nextseqnum < send_base + window_size && nextseqnum < packnum) {
		FilePack SendPack;
		SendPack.Operation = 810;

		if (nextseqnum + 1 == packnum) {
			SendPack.SeqNum = 0;
			SendPack.Length = dataLength - nextseqnum * 1000;
			int i;
			for (i = 0; i < SendPack.Length; i++) {
				SendPack.Content[i] = pBuf[nextseqnum * 1000 + i];
			}
		}
		else {
			SendPack.SeqNum = 511 + nextseqnum;
			SendPack.Length = 1000;
			for (int i = 0; i < 1000; i++) {
				SendPack.Content[i] = pBuf[nextseqnum * 1000 + i];
			}
		}
		SendPack.Checksum = 0;
		WORD check_buff[501];
		memcpy(check_buff, &SendPack.Checksum, SendPack.Length + 2);
		SendPack.Checksum = Checksum(check_buff, SendPack.Length + 2);
		int nLength = m_sock.SendTo(&SendPack, sizeof(SendPack), ServerPort, ServerIP, 0);
		if (nLength) {
			CString str;
			if (send_base == nextseqnum) {
				if (SetTimer(1, 1000, NULL)) {
					m_log.AddString(L"启动定时器");
				}
			}
			str.Format(L"已经成功发送第%d个数据包", nextseqnum + 1);
			m_log.AddString(str);
			nextseqnum++;
		}
	}
}

void CProDlg::OnTimer(UINT_PTR nIDEvent)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	if (nIDEvent == 1) {
		m_log.AddString(L"定时器超时，重新发送缓冲区中数据包");
		for (int i = send_base; i < nextseqnum; i++) {
			FilePack SendPack;
			SendPack.Operation = 810;
			SendPack.SeqNum = i + 511;
			if (i + 1 == packnum) {
				SendPack.Length = dataLength - i * 1000;
				for (int j = 0; j < SendPack.Length; j++) {
					SendPack.Content[j] = pBuf[i * 1000 + j];
				}
			}
			else {
				SendPack.Length = 1000;
				for (int j = 0; j < 1000; j++) {
					SendPack.Content[j] = pBuf[i * 1000 + j];
				}
			}
			SendPack.Checksum = 0;
			WORD check_buff[501];
			memcpy(check_buff, &SendPack.Checksum, SendPack.Length + 2);
			SendPack.Checksum = Checksum(check_buff, SendPack.Length + 2);
			if (m_sock.SendTo(&SendPack, sizeof(SendPack), ServerPort, ServerIP, 0)) {
				CString str;
				str.Format(L"已重新发送第%d个数据包", i + 1);
				m_log.AddString(str);
			}
		}
	}
	if (nIDEvent == 2) {
		initPack();
	}
	CDialogEx::OnTimer(nIDEvent);
}

WORD Checksum(WORD * buffer, int size) {
	ULONG cksum = 0;
	while (size > 1) {
		cksum += *buffer++;
		size -= sizeof(WORD);
	}
	if (size) {
		// 最后可能单独一个字节
		cksum += *(WORD*)buffer;
	}
	cksum = (cksum >> 16) + (cksum & 0xffff);
	cksum += (cksum >> 16);

	return (WORD)(~cksum);
}


void CProDlg::PostNcDestroy()
{
	// TODO: 在此添加专用代码和/或调用基类
	delete this;
	CDialogEx::PostNcDestroy();
}
