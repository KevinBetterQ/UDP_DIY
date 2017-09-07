//*----------------------------------------------------------------------------
//*         ATMEL Microcontroller Software Support  -  ROUSSET  -
//*----------------------------------------------------------------------------
//* The software is delivered "AS IS" without warranty or condition of any
//* kind, either express, implied or statutory. This includes without
//* limitation any warranty or condition with respect to merchantability or
//* fitness for any particular purpose, or against the infringements of
//* intellectual property rights of others.
//*----------------------------------------------------------------------------
//* File Name           : Emac.h
//* Object              : Emac header file
//* Creation            : Hi   11/18/2002
//*
//*----------------------------------------------------------------------------
#ifndef AT91C_EMAC_H
#define AT91C_EMAC_H

//* Allows to display all IP header in the main.c
//* If not defined, only ICMP packets are displayed
#define AT91C_DISPLAY_ALL_IPHEADER		1

#define NB_ETH_RX_PACKETS		8			//* Number of ethernet packets buffer
#define ETH_PACKET_SIZE 		1536        //* Size of an ethernet packet
#define AT91C_NO_IPPACKET		0
#define AT91C_IPPACKET	        1

#define ARP_REQUEST				0x0001
#define ARP_REPLY				0x0002
#define PROT_ARP				0x0806
#define PROT_IP					0x0800
#define PROT_ICMP				0x01
#define ICMP_ECHO_REQUEST		0x08
#define ICMP_ECHO_REPLY			0x00

#define SWAP16(x)	(((x & 0xff) << 8) | (x >> 8))

//* Transfer descriptor structure
typedef struct _AT91S_TdDescriptor {
	unsigned int addr;
	unsigned int size;
}AT91S_TdDescriptor, *AT91PS_TdDescriptor;


//* IP Header structure
typedef struct _AT91S_IPheader {
	unsigned char	ip_hl_v;	/* header length and version	*/
	unsigned char	ip_tos;		/* type of service		*/
	unsigned short	ip_len;		/* total length			*/
	unsigned short	ip_id;		/* identification		*/
	unsigned short	ip_off;		/* fragment offset field	*/
	unsigned char	ip_ttl;		/* time to live			*/
	unsigned char	ip_p;		/* protocol			*/
	unsigned short	ip_sum;		/* checksum			*/
	unsigned char	ip_src[4];		/* Source IP address		*/
	unsigned char	ip_dst[4];		/* Destination IP address	*/
} AT91S_IPheader, *AT91PS_IPheader;

//* ICMP echo header structure
typedef struct _AT91S_IcmpEchoHdr {
    unsigned char   type;       /* type of message */
    unsigned char   code;       /* type subcode */
    unsigned short  cksum;      /* ones complement cksum of struct */
    unsigned short  id;         /* identifier */
    unsigned short  seq;        /* sequence number */
}AT91S_IcmpEchoHdr, *AT91PS_IcmpEchoHdr;


/* MII registers definition */
#define MII_STS_REG	       0x01
#define MII_STS2_REG	   0x11

#endif //* AT91C_EMAC_H