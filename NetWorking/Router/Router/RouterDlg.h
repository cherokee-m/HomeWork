
// RouterDlg.h: 头文件
//

#pragma once
#include "pcap.h"
#include "afxtempl.h"

#pragma pack(1)
typedef struct FrameHeader_t {	  // 帧首部
	UCHAR	DesMAC[6];	          // 目的地址
	UCHAR	SrcMAC[6];	          // 源地址
	USHORT	FrameType;	          // 帧类型
} FrameHeader_t;

typedef struct ARPFrame_t {		  // ARP帧
	FrameHeader_t	FrameHeader;  // 帧首部
	WORD			HardwareType; // 硬件类型
	WORD			ProtocolType; // 协议类型
	BYTE			HLen;         // 硬件地址长度
	BYTE			PLen;         // 协议地址长度
	WORD			Operation;    // 操作值
	UCHAR			SendHa[6];    // 源MAC地址
	ULONG			SendIP;       // 源IP地址
	UCHAR			RecvHa[6];    // 目的MAC地址
	ULONG			RecvIP;       // 目的IP地址
} ARPFrame_t;

typedef struct IPHeader_t {		  // IP首部
	BYTE	Ver_HLen;             // 版本+头部长度
	BYTE	TOS;                  // 服务类型
	WORD	TotalLen;             // 总长度
	WORD	ID;                   // 标识
	WORD	Flag_Segment;         // 标志+片偏移
	BYTE	TTL;                  // 生存时间
	BYTE	Protocol;             // 协议
	WORD	Checksum;             // 头部校验和
	ULONG	SrcIP;                // 源IP地址
	ULONG	DstIP;                // 目的IP地址
} IPHeader_t;

typedef struct IPFrame_t {	      // IP帧
	FrameHeader_t	FrameHeader;  // 帧首部
	IPHeader_t		IPHeader;     // IP首部
} IPFrame_t;

typedef struct ICMPHeader_t {     // ICMP首部
	BYTE    Type;                 // 类型
	BYTE    Code;                 // 代码
	WORD    Checksum;             // 校验和
	WORD    Id;                   // 标识
	WORD    Sequence;             // 序列号
} ICMPHeader_t;
#pragma pack()

typedef struct ip_t {             // 网络地址
	ULONG			IPAddr;       // IP地址
	ULONG			IPMask;       // 子网掩码
} ip_t;

typedef struct RouteTable_t {	  // 路由表结构
	ULONG	Mask;                 // 子网掩码
	ULONG	DstIP;                // 目的地址
	ULONG	NextHop;              // 下一跳步
} RouteTable_t;

typedef struct IP_MAC_t {         // IP-MAC地址映射结构
	ULONG	IPAddr;               // IP地址
	UCHAR	MACAddr[6];           // MAC地址
} IP_MAC_t;

typedef struct SendPacket_t {	  // 发送数据包结构
	int				len;          // 长度
	BYTE			PktData[2000];// 数据缓存
	ULONG			TargetIP;     // 目的IP地址
	UINT_PTR		n_mTimer;     // 定时器
} SendPacket_t;

// CRouterDlg 对话框
class CRouterDlg : public CDialogEx
{
// 构造
public:
	CRouterDlg(CWnd* pParent = nullptr);	// 标准构造函数
	pcap_t * adhandle;
	pcap_if_t *alldevs;
	BOOL GetLocalDev();
	pcap_if_t* m_selectdevs;

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_ROUTER_DIALOG };
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
	CListBox m_devs;
	afx_msg void OnDestroy();
	afx_msg void OnBnClickedStart();
	CListBox m_log;
	CListBox m_rt;
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg void OnSelchangeDevs();
	afx_msg void OnBnClickedAdd();
	afx_msg void OnBnClickedDel();
	CIPAddressCtrl m_mask;
	CIPAddressCtrl m_net;
	CIPAddressCtrl m_nxthop;
	afx_msg void OnBnClickedRet();
};
