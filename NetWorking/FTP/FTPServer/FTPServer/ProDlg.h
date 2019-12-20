﻿#pragma once
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
	void OnReceive();
	virtual BOOL OnInitDialog();
	CListBox m_log;
	UINT m_port;
	CString ClientIP;
	UINT ClientPort;
	WORD operation;
	CString UploadFileName;
	CString LocalAddr;
	CString DownloadFileName;
	CFile file;
	int ack;
	void doOperation();
	void OpenReadFile();
	int packnum;
	int dataLength;
	char* pBuf;
	void SendFile();
	int nextseqnum;
	int send_base;
	int window_size;//滑动窗口大小
	afx_msg void OnTimer(UINT_PTR nIDEvent);
};
