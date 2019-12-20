
// FTPClientDlg.h: 头文件
//
#include"MySocket.h"
#include"ProDlg.h"
#pragma once


// CFTPClientDlg 对话框
class CFTPClientDlg : public CDialogEx
{
// 构造
public:
	CFTPClientDlg(CWnd* pParent = nullptr);	// 标准构造函数

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_FTPCLIENT_DIALOG };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 支持


// 实现
protected:
	HICON m_hIcon;

	// 生成的消息映射函数
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedCreate();
	UINT clientPort;
	CString clientIP;
	CIPAddressCtrl m_ip;
	CMySocket m_sock;
	CListBox m_log;
	afx_msg void OnBnClickedUpload();
	afx_msg void OnBnClickedGet();
	UINT m_port;
	afx_msg void OnBnClickedDownload();
	void OnReceive();
	CListBox m_fileList;
	afx_msg void OnDestroy();
	afx_msg void OnTimer(UINT_PTR nIDEvent);
};
