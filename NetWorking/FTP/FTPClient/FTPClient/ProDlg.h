#pragma once
#include"MySocket.h"

// CProDlg 对话框

class CProDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CProDlg)

public:
	CProDlg(CWnd* pParent = nullptr);   // 标准构造函数
	virtual ~CProDlg();

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DIALOG1 };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
public:
	CMySocket m_sock;
	UINT m_port;
	virtual BOOL OnInitDialog();
	CListBox m_log;
	void OnReceive();
	WORD operation;
	void initPack();
	CString ServerIP;
	UINT ServerPort;
	CString UploadFileName;
	CString FileAddr;
	CString DownLoadFileName;
	CString StoreAddr;
	int window_size;//滑动窗口大小
	void SendFile();
	int nextseqnum;
	int send_base;
	char* pBuf;
	void OpenReadFile();
	int packnum;
	int dataLength;
	int ack;
	CFile file;
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	virtual void PostNcDestroy();
};
