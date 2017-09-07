#include "changeformat.h"
#include "skbuff.h"
#include "eth.h"
#include "arp.h"
#include "ip.h"
#include "udp.h"
#include "utils.h"
#include "2410lib.h"
#include "uart_test.h"


char TftpLoadEnd;
char TftpPutBegin;
char TftpPutMark;

int net_handle(void)
{
	struct sk_buff *skb;
	struct ethhdr *eth_hdr;												
	skb = alloc_skb(ETH_FRAME_LEN);

	if (eth_rcv(skb) != -1) 
	{

		eth_hdr = (struct ethhdr *)(skb->data);					
		skb_pull(skb, ETH_HLEN);
		if (ntohs(eth_hdr->h_proto) == ETH_P_ARP)
		{
			arp_rcv_packet(skb);

		}

		else if(ntohs(eth_hdr->h_proto) == ETH_P_IP)						
		{

		 	ip_rcv_packet(skb);

		}
	 	
	}

	free_skb(skb);

	return 0;
}



#define	LOCAL_IP_ADDR	((192UL<<24)|(168<<16)|(2<<8)|111)

extern unsigned long download_len;
extern unsigned long download_addr;

int NetLoadFile(UINT32T addr, UINT32T give_ip, UINT32T a3, UINT32T a4)
{
	
	char cInput[256];
	UINT8T ucInNo=0;
	UINT32T	g_nKeyPress;
	char c;
	int ki=0;
	
	
	struct sk_buff *skbh;
	char *str = "hello";

	
	unsigned char eth_addr[ETH_ALEN];	
	unsigned char *s;
	int i;
	char *p;
	give_ip = LOCAL_IP_ADDR;
	s = (unsigned char *)&give_ip;
	
	//uart_printf("Mini TFTP Server 1.0 (IP : %d.%d.%d.%d PORT: %d)\n", s[3], s[2], s[1], s[0], TFTP);		
	//uart_printf("Type tftp -i %d.%d.%d.%d put filename at the host PC\n", s[3], s[2], s[1], s[0]);

	eth_init();		
	eth_get_addr(eth_addr);		
	ip_init(give_ip);
	udp_init();
		
	arp_add_entry(eth_addr, give_ip);	


	skbh = alloc_skb(ETH_FRAME_LEN);
	
	uart_printf(" Please input words, then press Enter:\n");
	uart_printf(" />");
	uart_printf(" ");
	g_nKeyPress = 1;
	
	for(ki=0;ki<5;ki++){
		net_handle();
		udp_skb_reserve(skbh);
		memcpy(skbh->data,str,5);
		
		uart_printf("begin sending..%s...\n",str);
		udp_send(skbh, 3232236132, UDP, 45454);
		uart_printf("finish sending..\n");
	}

	
	while (1) {		

		while(g_nKeyPress==1)			// only for board test to exit
	{
		c=uart_getch();
		
		uart_printf("%c",c);
		if(c!='\r')
			cInput[ucInNo++]=c;
		else
		{
			cInput[ucInNo]='\0';
			break;
		}
	}
	delay(1000);	
	net_handle();
	udp_skb_reserve(skbh);
	memcpy(skbh->data,cInput,ucInNo+1);
		
	uart_printf("begin sending..%s...\n",cInput);
	udp_send(skbh, 3232236132, UDP, 45454);
	uart_printf("finish sending..\n");
	memcpy(cInput,"",ucInNo+1);
			ucInNo=0;
	}
	
	
		
	/*
	while (1) {		
		net_handle();
		udp_skb_reserve(skbh);
		memcpy(skbh->data,str,5);
		
		uart_printf("begin sending..%s...\n",str);
		//udp_send(skbh, 3232236132, UDP, 45454);
		uart_printf("finish sending..\n");
		
		delay(5000);
	}
	*/
  
	

	return 0;
}



