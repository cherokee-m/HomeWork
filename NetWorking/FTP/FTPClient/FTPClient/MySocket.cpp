#include "stdafx.h"
#include "MySocket.h"
#include "FTPClientDlg.h"
#include "ProDlg.h"


CMySocket::CMySocket()
{
}


CMySocket::~CMySocket()
{
}


void CMySocket::OnReceive(int nErrorCode)
{
	// TODO: �ڴ����ר�ô����/����û���
	if (nErrorCode == 0) {
		if (num == 1)
			((CFTPClientDlg*)dlg)->OnReceive();
		if (num == 2)
			((CProDlg*)dlg)->OnReceive();

	}
	CAsyncSocket::OnReceive(nErrorCode);
}
