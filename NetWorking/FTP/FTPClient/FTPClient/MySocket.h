#pragma once
#include <afxsock.h>

struct Pack {
	WORD Operation;
	WORD Content[503];
};

struct FilePack
{
	WORD Operation;
	WORD SeqNum;
	WORD Length;
	WORD	Checksum;             // ����У���
	BYTE Content[1000];
};
class CMySocket :
	public CAsyncSocket
{
public:
	CMySocket();
	CDialog* dlg;
	int num;
	virtual ~CMySocket();
	virtual void OnReceive(int nErrorCode);
};
