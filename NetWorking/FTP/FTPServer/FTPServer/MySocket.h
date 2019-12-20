#pragma once
#include <afxsock.h>

#pragma pack(1)
struct Pack
{
	WORD Operation;
	WORD Content[503];
};

struct FilePack
{
	WORD Operation;
	WORD SeqNum;
	WORD Length;
	WORD	Checksum;             // 数据校验和
	BYTE Content[1000];
};
#pragma pack()
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

