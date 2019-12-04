
// RouterDlg.cpp: 实现文件
//

#include "stdafx.h"
#include "Router.h"
#include "RouterDlg.h"
#include "afxdialogex.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


UINT CaptureARP(PVOID hWnd);
CArray <ip_t, ip_t&> ip;       // IP地址列表
UCHAR MACAddr[6];			   // MAC 地址
CString IPntoa(ULONG nIPAddr);
BOOL flag = FALSE;
CString MACntoa(UCHAR *nMACAddr);
CList <RouteTable_t, RouteTable_t&> RouteTable;   // 路由表
UINT CaptureData(PVOID hWnd);
CRouterDlg	*pDlg;                // 对话框指针
CList <IP_MAC_t, IP_MAC_t&> IP_MAC;               // IP-MAC地址映射列表
BOOL IPFind(ULONG ipaddr, UCHAR *p);
void IPPacketProc(const u_char * pkt_data, struct pcap_pkthdr *header);
void ICMPPacketProc(BYTE type, BYTE code, const u_char * pkt_data);
WORD Checksum(WORD * buffer, int size);
CList <SendPacket_t, SendPacket_t&> SP;           // 发送数据包缓存队列
CMutex mMutex(0, 0, 0);

ULONG RouteFind(ULONG dstIP);
void ARPPacketProc(ARPFrame_t * ARPF);
UINT_PTR    TimerCount;           // 定时器个数
// 用于应用程序“关于”菜单项的 CAboutDlg 对话框

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_ABOUTBOX };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

// 实现
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogEx(IDD_ABOUTBOX)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()


// CRouterDlg 对话框



CRouterDlg::CRouterDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_ROUTER_DIALOG, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CRouterDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_DEVS, m_devs);
	DDX_Control(pDX, IDC_LOG, m_log);
	DDX_Control(pDX, IDC_ROUTETABLE, m_rt);
	DDX_Control(pDX, IDC_MASK, m_mask);
	DDX_Control(pDX, IDC_NET, m_net);
	DDX_Control(pDX, IDC_NEXTHOP, m_nxthop);
}

BEGIN_MESSAGE_MAP(CRouterDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_WM_DESTROY()
	ON_BN_CLICKED(IDC_START, &CRouterDlg::OnBnClickedStart)
	ON_WM_TIMER()
	ON_LBN_SELCHANGE(IDC_DEVS, &CRouterDlg::OnSelchangeDevs)
	ON_BN_CLICKED(IDC_ADD, &CRouterDlg::OnBnClickedAdd)
	ON_BN_CLICKED(IDC_DEL, &CRouterDlg::OnBnClickedDel)
	ON_BN_CLICKED(IDC_RET, &CRouterDlg::OnBnClickedRet)
END_MESSAGE_MAP()


// CRouterDlg 消息处理程序

BOOL CRouterDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// 将“关于...”菜单项添加到系统菜单中。

	// IDM_ABOUTBOX 必须在系统命令范围内。
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != nullptr)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// 设置此对话框的图标。  当应用程序主窗口不是对话框时，框架将自动
	//  执行此操作
	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);		// 设置小图标

	// TODO: 在此添加额外的初始化代码
	CRouterApp* pApp = (CRouterApp*)AfxGetApp();
	pDlg = (CRouterDlg*)pApp->m_pMainWnd;

	GetLocalDev();
	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}

void CRouterDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialogEx::OnSysCommand(nID, lParam);
	}
}

// 如果向对话框添加最小化按钮，则需要下面的代码
//  来绘制该图标。  对于使用文档/视图模型的 MFC 应用程序，
//  这将由框架自动完成。

void CRouterDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // 用于绘制的设备上下文

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// 使图标在工作区矩形中居中
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// 绘制图标
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

//当用户拖动最小化窗口时系统调用此函数取得光标
//显示。
HCURSOR CRouterDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

BOOL CRouterDlg::GetLocalDev() {
	char errbuf[PCAP_ERRBUF_SIZE];
	if (pcap_findalldevs_ex(PCAP_SRC_IF_STRING, NULL, &alldevs, errbuf) == -1) {
		MessageBox(_T("获取本机设备失败"), _T("ERROR"), MB_OK | MB_ICONQUESTION);
		return FALSE;
	}
	pcap_if_t *dev;

	for (dev = alldevs; dev != NULL; dev = dev->next) {
		CString name(dev->name);
		CString description(dev->description);
		m_devs.AddString(name + CString("\r\n") + description);
	}

	m_devs.SetCurSel(0);
	CString s;
	int i = m_devs.GetCurSel();
	m_devs.GetText(i, s);
	SetDlgItemText(IDC_EDIT1, s);
	return TRUE;
}


void CRouterDlg::OnDestroy()
{
	CDialogEx::OnDestroy();
	pcap_freealldevs(alldevs);
}

void CRouterDlg::OnBnClickedStart()
{
	pcap_addr_t *adrs;
	ip_t ipaddr;
	int j = 0;
	char errbuf[PCAP_ERRBUF_SIZE];
	int i = m_devs.GetCurSel();
	m_selectdevs = alldevs;
	while (i--)
		m_selectdevs = m_selectdevs->next;

	for (adrs = m_selectdevs->addresses; adrs; adrs = adrs->next) {
		if (adrs->addr->sa_family == AF_INET) {
			ipaddr.IPAddr = (((struct sockaddr_in *)adrs->addr)->sin_addr.s_addr);
			ipaddr.IPMask = (((struct sockaddr_in *)adrs->netmask)->sin_addr.s_addr);
			ip.Add(ipaddr);
			j++;
		}
	}
	if (j < 2) {
		MessageBox(L"本地主机至少应该具有2个IP地址");
		return;
	}
	if ((adhandle = pcap_open(m_selectdevs->name,
								65536,
								PCAP_OPENFLAG_PROMISCUOUS,
								1000,
								NULL,
								errbuf
								)) == NULL) {
		CString error(errbuf);
		MessageBox(error);
		return;
	}

	// 捕获ARP并获取本地MAC地址
	CWinThread *m_thread;
	m_thread = AfxBeginThread(CaptureARP, adhandle, THREAD_PRIORITY_NORMAL);
	if (!m_thread) {
		MessageBox(L"捕获ARP响应包线程创建失败！");
		return;
	}

	struct ARPFrame_t ARPFrame;
	memset(ARPFrame.FrameHeader.DesMAC, 0xff, 6);
	//本机MAC地址,假的
	memset(ARPFrame.FrameHeader.SrcMAC, 0x0f, 6);
	ARPFrame.FrameHeader.FrameType = htons(0x0806);

	ARPFrame.HardwareType = htons(0x0001);
	ARPFrame.ProtocolType = htons(0x0800);
	ARPFrame.HLen = 6;
	ARPFrame.PLen = 4;
	ARPFrame.Operation = htons(0x0001);

	//本机MAC地址，IP地址,假的
	memset(ARPFrame.SendHa, 0x0f, 6);
	ARPFrame.SendIP = inet_addr("112.112.112.112");

	memset(ARPFrame.RecvHa, 0x00, 6);
	ARPFrame.RecvIP = ip[0].IPAddr;
	if (pcap_sendpacket(adhandle, (u_char*)&ARPFrame, sizeof(ARPFrame_t)) != 0) {
		MessageBox(_T("发送ARP包失败"), _T("error"), MB_OK | MB_ICONERROR);
	}



	int time = 10;
	while (!flag && time--) {
		Sleep(3000);
	}
	if(!flag)
		MessageBox(L"MAC地址获取失败");
	m_log.InsertString(-1, L"接口设备名:" + CString(m_selectdevs->name));
	m_log.InsertString(-1, L"	设备描述:" + CString(m_selectdevs->description));
	m_log.InsertString(-1, L"	MAC地址:" + MACntoa(MACAddr));
	for (j = 0; j < ip.GetSize(); j++) {
		m_log.InsertString(-1, L"		IP地址:" + IPntoa(ip[j].IPAddr));
	}

	RouteTable_t rt;
	for (j = 0; j < ip.GetSize(); j++) {
		rt.DstIP = ip[j].IPAddr & ip[j].IPMask;
		rt.Mask = ip[j].IPMask;
		rt.NextHop = 0;//直接投递
		RouteTable.AddTail(rt);
		m_rt.InsertString(-1, IPntoa(rt.Mask) + " -- " + IPntoa(rt.DstIP) + " -- " + IPntoa(rt.NextHop) + "  (直接投递)");		
	}

	//设置过滤器
	CString filter, finfil;
	filter = "(";
	for (j = 0; j < ip.GetSize(); j++) {
		filter += L"(ip dst host " + IPntoa(ip[j].IPAddr) + L")";
		if (j != ip.GetSize() - 1) {
			filter += " or ";
		}
		else
			filter += ")";
	}
	finfil = L"(ether dst " + MACntoa(MACAddr) + L")"
		+ L" and ((arp and (ether[21]=0x2)) or (not" + filter + L"))";

	struct bpf_program fcode;
	char strbuf[1000];
	for (int i = 0; i < finfil.GetLength(); i++)
	{
		strbuf[i] = char(finfil[i]);
	}
	strbuf[finfil.GetLength()] = '\0';
	if (pcap_compile(adhandle, &fcode, strbuf, 1, ip[0].IPMask) < 0) {
		MessageBox(finfil + L"过滤规则编译不成功，请检查书写的规则语法是否正确！");
		return;
	}
	if (pcap_setfilter(adhandle, &fcode) < 0) {
		MessageBox(L"过滤器设置失败！");
		return;
	}

	m_thread = AfxBeginThread(CaptureData, adhandle, THREAD_PRIORITY_NORMAL);
	if (!m_thread) {
		MessageBox(L"数据包捕获线程创建失败！");
		return;
	}
	m_log.AddString(L">>>>>>转发数据报开始！");
//	m_devs.EnableWindow(FALSE);
//	GetDlgItem(IDC_START)->EnableWindow(FALSE);
}

// 数据包捕获线程
UINT CaptureData(PVOID hWnd) {
	int res;
	struct pcap_pkthdr *header;
	const u_char * pkt_data;

	while (true) {
		res = pcap_next_ex((pcap_t *)hWnd, &header, &pkt_data);
		if (res == 0)
			continue;
		if (res == 1) {
			FrameHeader_t *fh;
			fh = (FrameHeader_t *)pkt_data;

			switch (ntohs(fh->FrameType)) {
			case 0x0806:
				ARPFrame_t *ARPF;
				ARPF = (ARPFrame_t *)pkt_data;
				ARPPacketProc(ARPF);
				break;
			case 0x0800:
				IPPacketProc(pkt_data, header);
				break;
			default:
				break;
			}
		}
		if (res == -1) {
			return -1;
		}
	}
	return 0;
}

// 查询IP MAC地址映射表
BOOL IPFind(ULONG ipaddr, UCHAR *p) {
	if (IP_MAC.IsEmpty())return FALSE;
	POSITION head = IP_MAC.GetHeadPosition();
	IP_MAC_t ip_mac;
	for (int i = 0; i<IP_MAC.GetCount(); i++) {
		ip_mac = IP_MAC.GetNext(head);
		if (ipaddr == ip_mac.IPAddr) {
			p[0] = ip_mac.MACAddr[0];
			p[1] = ip_mac.MACAddr[1];
			p[2] = ip_mac.MACAddr[2];
			p[3] = ip_mac.MACAddr[3];
			p[4] = ip_mac.MACAddr[4];
			p[5] = ip_mac.MACAddr[5];
			return TRUE;
		}
	}
	return FALSE;

}

// 处理ARP数据报
void ARPPacketProc(ARPFrame_t * ARPF) {
	IP_MAC_t ip_mac;
	UCHAR macAddr[6];
	BOOL flagg;
	if (ARPF->Operation == ntohs(0x0002)) {
		pDlg->m_log.AddString(L"------------------------");
		pDlg->m_log.InsertString(-1, L"收到ARP响应包");
		pDlg->m_log.InsertString(-1, L"	ARP " + IPntoa(ARPF->SendIP)
			+ L" -- " + MACntoa(ARPF->SendHa));
		if (IPFind(ARPF->SendIP, macAddr)) {
			pDlg->m_log.InsertString(-1, L"	该对应关系已经存在于IP_MAC地址映射表中");
			return;
		}
		else {
			ip_mac.IPAddr = ARPF->SendIP;
			ip_mac.MACAddr[0] = ARPF->SendHa[0];
			ip_mac.MACAddr[1] = ARPF->SendHa[1];
			ip_mac.MACAddr[2] = ARPF->SendHa[2];
			ip_mac.MACAddr[3] = ARPF->SendHa[3];
			ip_mac.MACAddr[4] = ARPF->SendHa[4];
			ip_mac.MACAddr[5] = ARPF->SendHa[5];
			IP_MAC.AddHead(ip_mac);
			pDlg->m_log.InsertString(-1, L"	将该对应关系存入IP-MAC地址映射表中");
			
			mMutex.Lock(INFINITE);
			IPFrame_t		*IPF;
			do {
				flagg = FALSE;

				if (SP.IsEmpty())
					break;

				POSITION head;
				head = SP.GetHeadPosition();
				for (int i = 0; i < SP.GetCount(); i++) {
					POSITION tmp = head;
					SendPacket_t sPacket = SP.GetNext(head);
					if (sPacket.TargetIP == ARPF->SendIP) {
						IPF = (IPFrame_t*)sPacket.PktData;
						IPF->FrameHeader.DesMAC[0] = ARPF->SendHa[0];
						IPF->FrameHeader.DesMAC[1] = ARPF->SendHa[1];
						IPF->FrameHeader.DesMAC[2] = ARPF->SendHa[2];
						IPF->FrameHeader.DesMAC[3] = ARPF->SendHa[3];
						IPF->FrameHeader.DesMAC[4] = ARPF->SendHa[4];
						IPF->FrameHeader.DesMAC[5] = ARPF->SendHa[5];

						pcap_sendpacket(pDlg->adhandle, (u_char*)IPF, sPacket.len);
						SP.RemoveAt(tmp);

						pDlg->m_log.InsertString(-1, L"	转发缓存区中目的地址是该MAC地址的IP数据报");
						pDlg->m_log.InsertString(-1, L" 发送IP数据报："
							+ IPntoa(IPF->IPHeader.SrcIP)
							+ L" -> "
							+ IPntoa(IPF->IPHeader.DstIP)
							+ "    "
							+ MACntoa(IPF->FrameHeader.SrcMAC)
							+ L" -> "
							+ MACntoa(IPF->FrameHeader.DesMAC));
						flagg = TRUE;
						// 充满了疑惑
						break;
					}
				}
			} while (flagg);
			mMutex.Unlock();
		}
	}
}

// 处理IP数据报
void IPPacketProc(const u_char * pkt_data, struct pcap_pkthdr *header) {

	IPFrame_t *IPF;
	IPF = (IPFrame_t *)pkt_data;
	pDlg->m_log.AddString(L"-----------------------");
	SendPacket_t sPacket;

	pDlg->m_log.InsertString(-1, L"收到IP数据报:"
		+ IPntoa(IPF->IPHeader.SrcIP)
		+ L" -> "
		+ IPntoa(IPF->IPHeader.DstIP)
	);

	// 超时

	if (IPF->IPHeader.TTL <= 1) {
		ICMPPacketProc(11, 0, pkt_data);
		return;
	}

	IPHeader_t *IpHdr = &(IPF->IPHeader);
	WORD check_buff[sizeof(IPHeader_t)];
	memcpy(check_buff, IpHdr, sizeof(IPHeader_t));

	if (Checksum(check_buff, sizeof(IPHeader_t)) != 0) {
		ICMPPacketProc(12, 0, pkt_data);
		return;
	}

	ULONG nextHop;
	if ((nextHop = RouteFind(IPF->IPHeader.DstIP)) == -1) {
		ICMPPacketProc(3, 0, pkt_data); // 网络不可达
		return;
	}
	else {
		sPacket.TargetIP = nextHop;
		IPF->FrameHeader.SrcMAC[0] = MACAddr[0];
		IPF->FrameHeader.SrcMAC[1] = MACAddr[1];
		IPF->FrameHeader.SrcMAC[2] = MACAddr[2];
		IPF->FrameHeader.SrcMAC[3] = MACAddr[3];
		IPF->FrameHeader.SrcMAC[4] = MACAddr[4];
		IPF->FrameHeader.SrcMAC[5] = MACAddr[5];

		IPF->IPHeader.TTL -= 1;

		WORD check_buff[sizeof(IPHeader_t)];
		IPF->IPHeader.Checksum = 0;
		memcpy(check_buff, &(IPF->IPHeader), sizeof(IPHeader_t));

		IPF->IPHeader.Checksum = Checksum(check_buff, sizeof(IPHeader_t));

		if (IPFind(sPacket.TargetIP, IPF->FrameHeader.DesMAC)) {
			memcpy(sPacket.PktData, (u_char*)IPF, header->len);
			sPacket.len = header->len;
			if (pcap_sendpacket(pDlg->adhandle, 
				(u_char *)IPF, 
				header->len) != 0) {
				AfxMessageBox(L"发送IP数据报时出错！");
				return;
			}
			pDlg->m_log.InsertString(-1, L"	转发IP数据报： ");
			pDlg->m_log.InsertString(-1, L"		"
				+ IPntoa(IPF->IPHeader.SrcIP)
				+ L" -> "
				+ IPntoa(IPF->IPHeader.DstIP)
				+ "     "
				+ MACntoa(IPF->FrameHeader.SrcMAC)
				+ L" -> "
				+ MACntoa(IPF->FrameHeader.DesMAC));
		}
		else {
			if (SP.GetCount() < 65530) {
				sPacket.len = header->len;
				memcpy(sPacket.PktData, (u_char *)IPF, header->len);
				
				mMutex.Lock(INFINITE);
				sPacket.n_mTimer = TimerCount;
				if (TimerCount++ >= 65533)
					TimerCount = 1;
				//pDlg->SetTimer(sPacket.n_mTimer, 10000, NULL);
				SP.AddTail(sPacket);
				mMutex.Unlock();

				pDlg->m_log.InsertString(-1, L"	缺少目的MAC地址，将IP数据报存入转发缓冲区");
				pDlg->m_log.InsertString(-1, L"	存入转发缓冲区的数据包为：" 
					+ IPntoa(IPF->IPHeader.SrcIP)
					+ L" -> "
					+ IPntoa(IPF->IPHeader.DstIP)
					+ L" "
					+ MACntoa(IPF->FrameHeader.SrcMAC)
					+ L" -> xx:xx:xx:xx:xx:xx");
				pDlg->m_log.InsertString(-1, L" 发送ARP请求");

				ARPFrame_t ARPFrame;
				memset(ARPFrame.FrameHeader.DesMAC, 0xff, 6);

				ARPFrame.FrameHeader.SrcMAC[0] = MACAddr[0];
				ARPFrame.FrameHeader.SrcMAC[1] = MACAddr[1];
				ARPFrame.FrameHeader.SrcMAC[2] = MACAddr[2];
				ARPFrame.FrameHeader.SrcMAC[3] = MACAddr[3];
				ARPFrame.FrameHeader.SrcMAC[4] = MACAddr[4];
				ARPFrame.FrameHeader.SrcMAC[5] = MACAddr[5];

				ARPFrame.SendHa[0] = MACAddr[0];
				ARPFrame.SendHa[1] = MACAddr[1];
				ARPFrame.SendHa[2] = MACAddr[2];
				ARPFrame.SendHa[3] = MACAddr[3];
				ARPFrame.SendHa[4] = MACAddr[4];
				ARPFrame.SendHa[5] = MACAddr[5];

				memset(ARPFrame.RecvHa, 0x00, 6);

				ARPFrame.FrameHeader.FrameType = htons(0x0806);
				ARPFrame.HardwareType = htons(0x0001);
				ARPFrame.ProtocolType = htons(0x0800);
				ARPFrame.HLen = 6;
				ARPFrame.PLen = 4;
				ARPFrame.Operation = htons(0x0001);
				for (int i = 0; i < ip.GetCount(); i++) {
					if ((ip[i].IPAddr & ip[i].IPMask) == (sPacket.TargetIP & ip[i].IPMask)) {
						ARPFrame.SendIP = ip[i].IPAddr;
						break;
					}
				}
				ARPFrame.RecvIP = sPacket.TargetIP;

				if (pcap_sendpacket(pDlg->adhandle, (u_char*)&ARPFrame, sizeof(ARPFrame_t)) == -1) {
					AfxMessageBox(_T("发送ARP包失败"));
				}

			}
			else {
				ICMPPacketProc(4, 0, pkt_data);
				// 源站抑制报文
				pDlg->m_log.InsertString(-1, L"转发缓冲区溢出，丢弃IP数据报");
				pDlg->m_log.InsertString(-1, L"丢弃的数据报为："
					+ IPntoa(IPF->IPHeader.SrcIP)
					+ L" -> "
					+ IPntoa(IPF->IPHeader.DstIP)
					+ L" "
					+ MACntoa(IPF->FrameHeader.SrcMAC)
					+ L" -> xx:xx:xx:xx:xx:xx");
			}
		}
	}
}

// ICMP报文
void ICMPPacketProc(BYTE type, BYTE code, const u_char * pkt_data) {
	u_char * ICMPBuf = new u_char[70];
	IPFrame_t *IPF;
	IPF = (IPFrame_t *)pkt_data;
	// 帧首部
	memcpy(((FrameHeader_t *)ICMPBuf)->DesMAC, IPF->FrameHeader.SrcMAC, 6);
	memcpy(((FrameHeader_t*)ICMPBuf)->SrcMAC, IPF->FrameHeader.DesMAC, 6);
	((FrameHeader_t *)ICMPBuf)->FrameType = htons(0x0800);

	// 需要回去再了解一下ICMP
	// IP首部
	((IPHeader_t *)(ICMPBuf + 14))->Ver_HLen = IPF->IPHeader.Ver_HLen;
	((IPHeader_t *)(ICMPBuf + 14))->TOS = IPF->IPHeader.TOS;
	((IPHeader_t *)(ICMPBuf + 14))->TotalLen = htons(56);
	((IPHeader_t *)(ICMPBuf + 14))->ID = IPF->IPHeader.ID;
	((IPHeader_t *)(ICMPBuf + 14))->Flag_Segment = IPF->IPHeader.Flag_Segment;
	((IPHeader_t *)(ICMPBuf + 14))->TTL = 64;
	((IPHeader_t *)(ICMPBuf + 14))->Protocol = 1;
	//((IPHeader_t *)(ICMPBuf + 14))->SrcIP = IPF->IPHeader.DstIP;
	((IPHeader_t *)(ICMPBuf + 14))->SrcIP = ip[0].IPAddr;
	((IPHeader_t *)(ICMPBuf + 14))->DstIP = IPF->IPHeader.SrcIP;
	((IPHeader_t *)(ICMPBuf + 14))->Checksum = 0;
	((IPHeader_t *)(ICMPBuf + 14))->Checksum = Checksum((unsigned short *)(ICMPBuf + 14), 20);

	// ICMP首部
	((ICMPHeader_t *)(ICMPBuf + 34))->Id = 0;
	((ICMPHeader_t *)(ICMPBuf + 34))->Sequence = 0;
	((ICMPHeader_t *)(ICMPBuf + 34))->Type = type;
	((ICMPHeader_t *)(ICMPBuf + 34))->Code = code;
	((ICMPHeader_t *)(ICMPBuf + 34))->Checksum = 0;

	// 填充数据
	memcpy((u_char *)(ICMPBuf + 42), (u_char *)(pkt_data + 14), 28);
	((ICMPHeader_t *)(ICMPBuf + 34))->Checksum = Checksum((unsigned short *)(ICMPBuf + 34), 36);

	// 发送数据包
	pcap_sendpacket(pDlg->adhandle, (u_char *)ICMPBuf, 70);

	if(type == 0)
		pDlg->m_log.InsertString(-1, L"	发送ICMP响应数据报：");
	if (type == 11)
		pDlg->m_log.InsertString(-1, L"	发送ICMP超时数据报：");
	if (type == 3)
		pDlg->m_log.InsertString(-1, L"	发送ICMP网络不可达数据报：");
	if (type == 12)
		pDlg->m_log.InsertString(-1, L"	发送ICMP IP包头损坏数据报：");
	if (type == 4)
		pDlg->m_log.InsertString(-1, L"	发送ICMP 源点抑制报文：");
	pDlg->m_log.InsertString(-1, (L"   ICMP ->" + IPntoa(((IPHeader_t *)(ICMPBuf + 14))->DstIP)
		+ L"-" + MACntoa(((FrameHeader_t *)ICMPBuf)->DesMAC)));
	delete[] ICMPBuf;
}

// 查路由表
ULONG RouteFind(ULONG dstIP) {
	ULONG MaxMask = 0; //最大的掩码地址
	int Index = -1; // 路由表索引

	POSITION head;
	RouteTable_t rt;
	ULONG tmp;

	head = RouteTable.GetHeadPosition();
	for (int i = 0; i < RouteTable.GetCount(); i++) {
		rt = RouteTable.GetNext(head);
		if ((dstIP & rt.Mask) == rt.DstIP) {
			Index = i;
			if (rt.Mask >= MaxMask) {
				if (rt.NextHop == 0)
					tmp = dstIP;
				else
					tmp = rt.NextHop;
			}
		}
	}

	if (Index == -1)
		return -1;
	else
		return tmp;

}

// 计算检验和
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

// 获取本地接口MAC地址线程
UINT CaptureARP(PVOID hWnd) {
	int res;
	struct pcap_pkthdr *header;
	const u_char * pkt_data;
	ARPFrame_t *ARPFrame;
	while (true) {
		res = pcap_next_ex((pcap_t *)hWnd, &header, &pkt_data);
		if(res == 0)
			continue;
		if (res > 0) {
			ARPFrame = (ARPFrame_t *)pkt_data;

			if ((ARPFrame->FrameHeader.FrameType == htons(0x0806))
				&& (ARPFrame->Operation == htons(0x0002))
				&& (ARPFrame->SendIP == ip[0].IPAddr)) {
				MACAddr[0] = ARPFrame->SendHa[0];
				MACAddr[1] = ARPFrame->SendHa[1];
				MACAddr[2] = ARPFrame->SendHa[2];
				MACAddr[3] = ARPFrame->SendHa[3];
				MACAddr[4] = ARPFrame->SendHa[4];
				MACAddr[5] = ARPFrame->SendHa[5];
				flag = TRUE;
				return 0;
			}
		}
		if (res == -1)
			return -1;
	}
	return 0;
}

// 把IP地址转换成点分十进制形式
CString IPntoa(ULONG nIPAddr)
{
	char		strbuf[50];
	u_char		*p;
	CString		str;

	p = (u_char *)&nIPAddr;
	sprintf_s(strbuf, "%03d.%03d.%03d.%03d", p[0], p[1], p[2], p[3]);
	// 我怕不是个傻子，这不就是网络序转主机序？？？
	str = strbuf;
	return str;
}

// 把MAC地址转换成“%02X:%02X:%02X:%02X:%02X:%02X”的格式
CString MACntoa(UCHAR *nMACAddr)
{
	char		strbuf[50];
	CString		str;

	sprintf_s(strbuf, "%02X:%02X:%02X:%02X:%02X:%02X", nMACAddr[0], nMACAddr[1],
		nMACAddr[2], nMACAddr[3], nMACAddr[4], nMACAddr[5]);
	str = strbuf;
	return str;
}

void CRouterDlg::OnTimer(UINT_PTR nIDEvent)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	SendPacket_t sPacket;
	POSITION pos, tmp;
	IPFrame_t *IPF;

	if (SP.IsEmpty())
		return;

	mMutex.Lock(INFINITE);
	pos = SP.GetHeadPosition();
	for (int i = 0; i < SP.GetCount(); i++) {
		tmp = pos;
		sPacket = SP.GetNext(pos);
		if (sPacket.n_mTimer == nIDEvent) {
			IPF = (IPFrame_t *)sPacket.PktData;

			m_log.InsertString(-1, L"IP数据报再转发队列中等待10s还未被转发");
			m_log.InsertString(-1, L" 定时器中删除该IP数据报:"
				+ IPntoa(IPF->IPHeader.SrcIP)
				+ L" -> "
				+ IPntoa(IPF->IPHeader.DstIP)
				+ "    "
				+ MACntoa(IPF->FrameHeader.SrcMAC)
				+ " -> xx:xx:xx:xx:xx:xx");
			KillTimer(sPacket.n_mTimer);
			SP.RemoveAt(tmp);
		}
	}
	CDialogEx::OnTimer(nIDEvent);
}

void CRouterDlg::OnSelchangeDevs()
{
	// TODO: 在此添加控件通知处理程序代码
	CString s;
	int i = m_devs.GetCurSel();
	m_devs.GetText(i, s);
	SetDlgItemText(IDC_EDIT1, s);
	return;
}


void CRouterDlg::OnBnClickedAdd()
{
	// TODO: 在此添加控件通知处理程序代码
	ULONG ipaddr;
	RouteTable_t rt;
	BOOL er = FALSE;
	m_nxthop.GetAddress(ipaddr);
	ipaddr = htonl(ipaddr);
	ULONG mask_f;
	m_mask.GetAddress(mask_f);
	mask_f = htonl(mask_f);
	ULONG dstip_f;
	m_net.GetAddress(dstip_f);
	dstip_f = htonl(dstip_f);
	if ((dstip_f & mask_f) != dstip_f) {
		MessageBox(L"目的网掩码不匹配，请重新输入");
		return;
	}

	for (int i = 0; i < ip.GetSize(); i++) {
		if ((ip[i].IPAddr & ip[i].IPMask) == (ip[i].IPMask & ipaddr)) {
			rt.NextHop = ipaddr;
			m_mask.GetAddress(ipaddr);
			rt.Mask = htonl(ipaddr);
			m_net.GetAddress(ipaddr);
			rt.DstIP = htonl(ipaddr);
			RouteTable.AddTail(rt);
			m_rt.InsertString(-1, IPntoa(rt.Mask)
				+ " -- "
				+ IPntoa(rt.DstIP)
				+ " -- "
				+ IPntoa(rt.NextHop));
			er = TRUE;
		}
	}

	if (!er) {
		MessageBox(L"输入错误，请重新输入");
	}
}


void CRouterDlg::OnBnClickedDel()
{
	// TODO: 在此添加控件通知处理程序代码
	int i;
	char str[100];
	char ipaddr[20];
	ULONG mask, destination, nexthop;


	if ((i = m_rt.GetCurSel()) == LB_ERR)
		return;
	CString STR;
	m_rt.GetText(i, STR);

	for (int j = 0; j < STR.GetLength(); j++) {
		str[j] = STR[j];
	}
	str[STR.GetLength()] = '\0';

	// 取得子网掩码选项
	strncat_s(ipaddr, str, 15);
	mask = inet_addr(ipaddr);
	// 取得目的地址选项
	ipaddr[0] = 0;
	strncat_s(ipaddr, &str[19], 15);
	destination = inet_addr(ipaddr);
	// 取得下一跳选项
	ipaddr[0] = 0;
	strncat_s(ipaddr, &str[38], 15);
	nexthop = inet_addr(ipaddr);

	if (nexthop == 0) {
		MessageBox(L"直接连接路由，不允许删除");
		return;
	}

	m_rt.DeleteString(i);

	if (RouteTable.IsEmpty())
		return;

	POSITION pos, tmp;
	pos = RouteTable.GetHeadPosition();
	RouteTable_t rt;

	for (int j = 0; j < RouteTable.GetCount(); j++) {
		tmp = pos;
		rt = RouteTable.GetNext(pos);
		if (rt.Mask == mask && rt.DstIP == destination && rt.NextHop == nexthop) {
			RouteTable.RemoveAt(pos);
			return;
		}
	}
}


void CRouterDlg::OnBnClickedRet()
{
	// TODO: 在此添加控件通知处理程序代码
	m_log.ResetContent();
	GetDlgItem(IDC_START)->EnableWindow(TRUE);
	m_devs.EnableWindow(TRUE);
	m_rt.ResetContent();
	SP.RemoveAll();
	IP_MAC.RemoveAll();
	RouteTable.RemoveAll();
	ip.RemoveAll();
	flag = FALSE;
}
