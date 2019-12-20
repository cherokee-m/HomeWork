// ProDlg.cpp: 实现文件
//

#include "stdafx.h"
#include "FTPServer.h"
#include "ProDlg.h"
#include "afxdialogex.h"


// CProDlg 对话框
WORD Checksum(WORD * buffer, int size);
IMPLEMENT_DYNAMIC(CProDlg, CDialogEx)

CProDlg::CProDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_DIALOG1, pParent)
{
	ack = 0;
	window_size = 4;
	send_base = 0;
	nextseqnum = 0;
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
END_MESSAGE_MAP()


// CProDlg 消息处理程序


BOOL CProDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// TODO:  在此添加额外的初始化
	m_sock.dlg = this;
	m_sock.num = 2;
	srand((unsigned)time(NULL));
	m_port = ((UINT)rand()) % 1000 + 9001;
	if (m_sock.Create(m_port, SOCK_DGRAM, FD_READ | FD_WRITE, nullptr)) {
		CString str1;
		str1.Format(L"创建套接字%d", m_port);
		m_log.AddString(str1);
	}
	else {
		MessageBoxA(NULL, "FAIL", "创建套接字错误", MB_OK);
		return TRUE;
	}
	doOperation();
	return TRUE;  // return TRUE unless you set the focus to a control
				  // 异常: OCX 属性页应返回 FALSE
}

void CProDlg::OnReceive() {
	Pack RecvPack;
	int nLength = m_sock.ReceiveFrom(&RecvPack, sizeof(RecvPack), ClientIP, ClientPort);
	FilePack *filePack = (FilePack*)&RecvPack;
	if (nLength) {
		switch (RecvPack.Operation) {
		case 810: {
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
					pack.Operation = 811;
					pack.Content[0] = 0;
					if (m_sock.SendTo(&pack, sizeof(pack), ClientPort, ClientIP, 0)) {
						CString str1;
						str1.Format(L"成功发送ACK0");
						m_log.AddString(str1);
					}
					file.Close();
					m_sock.Close();
				}
				else if (ack == filePack->SeqNum - 511) {
					CString str;
					str.Format(L"成功接收第%d个数据包", filePack->SeqNum - 510);
					m_log.AddString(str);
					ack++;
					file.SeekToEnd();
					file.Write(filePack->Content, filePack->Length);
					Pack pack;
					pack.Operation = 811;
					pack.Content[0] = filePack->SeqNum;
					if (m_sock.SendTo(&pack, sizeof(pack), ClientPort, ClientIP, 0)) {
						CString str1;
						str1.Format(L"成功发送ACK%d", pack.Content[0]);
						m_log.AddString(str1);
					}
				}
				else {
					Pack pack;
					pack.Operation = 811;
					pack.Content[0] = ack + 510;
					if (m_sock.SendTo(&pack, sizeof(pack), ClientPort, ClientIP, 0)) {
						CString str1;
						str1.Format(L"成功发送ACK%d", pack.Content[0]);
						m_log.AddString(str1);
					}
				}
			}
			else {
				Pack pack;
				pack.Operation = 811;
				pack.Content[0] = ack + 510;
				if (m_sock.SendTo(&pack, sizeof(pack), ClientPort, ClientIP, 0)) {
					CString str1;
					str1.Format(L"数据错误发送ACK%d", pack.Content[0]);
					m_log.AddString(str1);
				}
			}
			break;
		}
		case 310: {
			m_log.AddString(L"收到客户端器确认接收文件 " + DownloadFileName + L" 响应");
			OpenReadFile();
			SendFile();
			break;
		}
		case 320: {
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
			else if (RecvPack.Content[0] == send_base + 311) {
				send_base++;
				SendFile();
			}
			break;
		}
		}
	}
}

void CProDlg::doOperation() {
	switch (operation) {
		case 801: {
			Pack pack;
			pack.Operation = 801;
			if (m_sock.SendTo(&pack, sizeof(pack), ClientPort, ClientIP)) {
				m_log.AddString(L"发送上传确认801成功");
				file.Open(LocalAddr + UploadFileName, CFile::modeCreate | CFile::modeWrite | CFile::typeBinary);
			}
			break;
		}
		case 301: {
			Pack pack;
			pack.Operation = 301;
			int length = DownloadFileName.GetLength();
			int i;
			for (i = 0; i < length; i++) {
				pack.Content[i] = DownloadFileName.GetAt(i);
			}
			pack.Content[i++] = 0;
			if (m_sock.SendTo(&pack, sizeof(pack), ClientPort, ClientIP)) {
				m_log.AddString(L"发送下载确认301成功");	
			}
			break;
		}
	}
}

void CProDlg::OpenReadFile() {
	CFile t_file;
	if (t_file.Open(LocalAddr + DownloadFileName, CFile::modeRead | CFile::typeBinary)) {
		packnum = t_file.GetLength() / 1000 + 1;
		CString str3;
		str3.Format(L"本次文件将分为%d个数据包传输", packnum);
		m_log.AddString(str3);
		dataLength = t_file.GetLength();
		pBuf = new char[t_file.GetLength() + 1];
		t_file.Read(pBuf, t_file.GetLength());
		t_file.Close();
	}
}

void CProDlg::SendFile() {
	while (nextseqnum < send_base + window_size && nextseqnum < packnum) {
		FilePack SendPack;
		SendPack.Operation = 311;

		if (nextseqnum + 1 == packnum) {
			SendPack.SeqNum = 0;
			SendPack.Length = dataLength - nextseqnum * 1000;
			int i;
			for (i = 0; i < SendPack.Length; i++) {
				SendPack.Content[i] = pBuf[nextseqnum * 1000 + i];
			}
		}
		else {
			SendPack.SeqNum = 311 + nextseqnum;
			SendPack.Length = 1000;
			for (int i = 0; i < 1000; i++) {
				SendPack.Content[i] = pBuf[nextseqnum * 1000 + i];
			}
		}
		SendPack.Checksum = 0;
		WORD check_buff[501];
		memcpy(check_buff, &SendPack.Checksum, SendPack.Length + 2);
		SendPack.Checksum = Checksum(check_buff, SendPack.Length + 2);
		int nLength = m_sock.SendTo(&SendPack, sizeof(SendPack), ClientPort, ClientIP, 0);
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
			SendPack.Operation = 311;
			SendPack.SeqNum = i + 311;
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
			if (m_sock.SendTo(&SendPack, sizeof(SendPack), ClientPort, ClientIP, 0)) {
				CString str;
				str.Format(L"已重新发送第%d个数据包", i + 1);
				m_log.AddString(str);
			}
		}
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