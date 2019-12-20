#include "stdafx.h"
#include "MySocket.h"
#include "FTPServerDlg.h"
#include "ProDlg.h"

CMySocket::CMySocket()
{
}


CMySocket::~CMySocket()
{
}


void CMySocket::OnReceive(int nErrorCode)
{
	// TODO: 在此添加专用代码和/或调用基类

	if (nErrorCode == 0) {
		if(num == 1)
			((CFTPServerDlg*)dlg)->OnReceive();
		else if(num == 2)
			((CProDlg*)dlg)->OnReceive();
	}
	CAsyncSocket::OnReceive(nErrorCode);
}
