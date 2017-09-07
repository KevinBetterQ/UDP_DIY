#include "2410lib.h"
#include "mac.h"
#include "utils.h"

#define DM9000_ID  0x90000A46
#define PACKET_TYPE_ALL_MULTICAST      0x0004
#define PACKET_TYPE_BROADCAST          0x0008
#define PACKET_TYPE_PROMISCUOUS        0x0020
UINT16T MacAddr[3]={0x8000,0x1248,0x5634};
// Hash creation constants.
//
#define CRC_PRIME               0xFFFFFFFF;
#define CRC_POLYNOMIAL          0x04C11DB6;
static UINT8T OurEmacAddr[6] = {0x00,0x00,0x32,0x12,0x33,0x12} ;


#define IOREAD(o)     ((UINT8T)*((volatile UINT8T *)(o)))
#define IOWRITE(o, d)    *((volatile UINT8T *)(o)) = (UINT8T)(d)

#define IOREAD16(o)     ((UINT16T)*((volatile UINT16T *)(o)))
#define IOWRITE16(o, d)    *((volatile UINT16T *)(o)) = (UINT16T)(d)

#define IOREAD32(o)     ((UINT32T)*((volatile UINT32T *)(o)))
#define IOWRITE32(o, d)    *((volatile UINT32T *)(o)) = (UINT32T)(d)

static UINT32T dwEthernetIOBase;
static UINT32T dwEthernetDataPort;

static UINT8T DM9000_iomode;
static UINT16T hash_table[4];


static UINT8T DM9000_rev; 


#define DM9000_DWORD_MODE  1
#define DM9000_BYTE_MODE  2
#define DM9000_WORD_MODE  0

//#define DM9000_MEM_MODE
#ifdef DM9000_MEM_MODE

#define READ_REG1     ReadReg
#define READ_REG2     MEMREAD

#define WRITE_REG1     WriteReg
#define WRITE_REG2     MEMWRITE

#else

#define READ_REG1     ReadReg
#define READ_REG2     ReadReg

#define WRITE_REG1     WriteReg
#define WRITE_REG2     WriteReg

#endif


static UINT8T
ReadReg(UINT16T offset)
{
 IOWRITE(dwEthernetIOBase, offset);
 return IOREAD(dwEthernetDataPort);
}

static void
WriteReg(UINT16T offset, UINT8T data)
{
 IOWRITE(dwEthernetIOBase, offset);
 IOWRITE(dwEthernetDataPort, data);
}


/*
    @func   UINT8T | CalculateHashIndex | Computes the logical addres filter hash index value.  This used when there are multiple
                                     destination addresses to be filtered.
    @rdesc  Hash index value.
    @comm   
    @xref  
*/
UINT8T CalculateHashIndex( UINT8T  *pMulticastAddr )
{
   UINT32T CRC;
   UINT8T  HashIndex;
   UINT8T  AddrByte;
   UINT32T HighBit;
   int   Byte;
   int   Bit;

   // Prime the CRC.
   CRC = CRC_PRIME;

   // For each of the six bytes of the multicast address.
   for ( Byte=0; Byte<6; Byte++ )
   {
      AddrByte = *pMulticastAddr++;

      // For each bit of the byte.
      for ( Bit=8; Bit>0; Bit-- )
      {
         HighBit = CRC >> 31;
         CRC <<= 1;

         if ( HighBit ^ (AddrByte & 1) )
         {
            CRC ^= CRC_POLYNOMIAL;
            CRC |= 1;
         }

         AddrByte >>= 1;
      }
   }

   // Take the least significant six bits of the CRC and copy them
   // to the HashIndex in reverse order.
   for( Bit=0,HashIndex=0; Bit<6; Bit++ )
   {
      HashIndex <<= 1;
      HashIndex |= (UINT8T)(CRC & 1);
      CRC >>= 1;
   }

   return(HashIndex);
}

void DM9000_Delay(UINT32T dwCounter)
{
 // Simply loop...
 while (dwCounter--);
} 

void dm9000_hash_table(UINT16T *mac)
{
 UINT16T i, oft;

 uart_printf("dm9000_hash_table\r\n");
 /* Set Node address */
 WRITE_REG1(0x10, (UINT8T)(mac[0] & 0xFF));
 WRITE_REG1(0x11, (UINT8T)(mac[0] >> 8));
 WRITE_REG1(0x12, (UINT8T)(mac[1] & 0xFF));
 WRITE_REG1(0x13, (UINT8T)(mac[1] >> 8));
 WRITE_REG1(0x14, (UINT8T)(mac[2] & 0xFF));
 WRITE_REG1(0x15, (UINT8T)(mac[2] >> 8)); 

 /* Clear Hash Table */
 for (i = 0; i < 4; i++)
  hash_table[i] = 0x0;

 /* broadcast address */
 hash_table[3] = 0x8000;
 /* Write the hash table to MAC MD table */
 for (i = 0, oft = 0x16; i < 4; i++)
 {
  WRITE_REG1(oft++, (UINT8T)(hash_table[i] & 0xff));
  WRITE_REG1(oft++, (UINT8T)((hash_table[i] >> 8) & 0xff));
 }
  

}

/*
 * This function is used to detect DM9000 chip
 * input : void
 * return : TRUE, detect DM9000
 *          FALSE, Not find DM9000
 */
static int Probe(void)
{
 int r = FALSE;
 UINT32T id_val;


 uart_printf("detected DM9000...\r\n"); 
 uart_printf ( "dwEthernetIOBase =  %x\r\n",dwEthernetIOBase);
 uart_printf ( "dwEthernetDataPort =  %x\r\n",dwEthernetDataPort);
 
 id_val  = READ_REG1(0x28);
 id_val |= READ_REG1(0x29) << 8;
 id_val |= READ_REG1(0x2a) << 16;
 id_val |= READ_REG1(0x2b) << 24;

uart_printf ( "id_val =  %x\r\n",id_val);
 
 if (id_val == DM9000_ID) {
  uart_printf("INFO: Probe: DM9000 is detected.\r\n");
  DM9000_rev = READ_REG1(0x2c);
  uart_printf("INFO:CHIP Revision is:%d\n",DM9000_rev);

  
  r = TRUE;
 }
 else {
  uart_printf("ERROR: Probe: Can not find DM9000.\r\n");
 }

 return r;
}


/*
 * This function enables TX/RX interrupt mask
 * input : void
 * return : viod
 */
void DM9000DBG_EnableInts(void)
{
 /*only enable RX interrupt*/
 WRITE_REG1(0xff, 0x81);
}
/*
 * This function disables TX/RX interrupt mask
 * input : void
 * return void
 */

void DM9000DBG_DisableInts(void)
{
 WRITE_REG1(0xff, 0x80);
}

/* Send a data block via Ethernet. */

 int dm9000_send (UINT8T *pbData,unsigned int length)
{ 
 int i;
 int tmplen;


 IOWRITE(dwEthernetIOBase, 0xf8); /* data copy ready set */
 /* copy data to FIFO */
 switch (DM9000_iomode)
 {
  case DM9000_BYTE_MODE:
   tmplen = length ;  
   for (i = 0; i < tmplen; i++)
   IOWRITE(dwEthernetDataPort, ((UINT8T *)pbData)[i]);
   break;
  case DM9000_WORD_MODE:
   tmplen = (length+1)/2;
   for (i = 0; i < tmplen; i++)
           IOWRITE16(dwEthernetDataPort, ((UINT16T *)pbData)[i]);
   break;
  case DM9000_DWORD_MODE:
   tmplen = (length+3)/4;
   for (i = 0; i < tmplen; i++)
           IOWRITE32(dwEthernetDataPort, ((UINT32T *)pbData)[i]);
  default:
   uart_printf("[DM9000][TX]Move data error!!!");
   break;
 }
 /*set packet leng */
 WRITE_REG1(0xfd, (length >> 8) & 0xff); 
 WRITE_REG1(0xfc, length & 0xff);
 /* start transmit */
 WRITE_REG1(0x02, 1);
 
 /*wait TX complete*/
 while(1)
 {
  if (READ_REG1(0xfe) & 2) { //TX completed
   WRITE_REG1(0xfe, 2);
   break;
  }
  DM9000_Delay(1000);
 }
 return 0;
}

/*::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::*/
/*::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::*/
/*::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::*/
/*::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::*/

int DM9000DBG_Init(void )
{

    int r = FALSE;
	UINT32T temp;
	// Core DM9000

	dwEthernetIOBase   = 0x20000000;
	dwEthernetDataPort = 0x20100000;
   
     r = Probe(); /*Detect DM9000 */

	 
    MacAddr[0] = 0x0000;
	MacAddr[1] = 0x1232;
	MacAddr[2] = 0x1233;
    uart_printf("DM9000: MAC Address: %u:%u:%u:%u:%u:%u\r\n",
    MacAddr[0] & 0x00FF, MacAddr[0] >> 8,
    MacAddr[1] & 0x00FF, MacAddr[1] >> 8,
    MacAddr[2] & 0x00FF, MacAddr[2] >> 8);

	if(r == FALSE)
	{
	    return FALSE; 
	}

	 /* set the internal PHY power-on, GPIOs normal */
	 WRITE_REG1(0x1f, 0); /* GPR (reg_1Fh)bit GPIO0=0 pre-activate PHY */ 
	 DM9000_Delay(1000);

	 /* do a software reset */
	 WRITE_REG1(0x0, 3); /* NCR (reg_00h) bit[0] RST=1 & Loopback=1, reset on */
	 DM9000_Delay(1000);
	 WRITE_REG1(0x0, 3); /* NCR (reg_00h) bit[0] RST=1 & Loopback=1, reset on */ 
	 DM9000_Delay(1000);

	 /* I/O mode */
	 DM9000_iomode = READ_REG1(0xfe) >> 6; /* ISR bit7:6 keeps I/O mode */

	 /* Program operating register */
	 WRITE_REG1(0x0, 0);
	 WRITE_REG1(0x02, 0); /* TX Polling clear */
	 WRITE_REG1(0x2f, 0); /* Special Mode */
	 WRITE_REG1(0x01, 0x2c); /* clear TX status */
	 WRITE_REG1(0xfe, 0x0f); /* Clear interrupt status */

	 /* Set address filter table */
	 dm9000_hash_table(MacAddr);

	 /* Activate DM9000A/DM9010 */
	 WRITE_REG1(0x05, 0x30 | 1); /* Discard long packet and CRC error packets*//* RX enable */     
	 WRITE_REG1(0xff, 0x80);  /* Enable SRAM automatically return */
	    
	 /* wait link ok */
	 

	 while(1)
	 {
	         temp=READ_REG1(0x01)&0x40;
	         if(temp)
	         	{
	         uart_printf ("link stauts = %x\r\n", READ_REG1(0x01));
	         uart_printf("INFO: Init: DM9000_Init OK.\r\n");
	            break;	
	         	}
		//uart_printf("wait for Ethernet link ...\r\n");
	 }   

	 return r;
}

//----------------------------------------------------------------------

UINT32T
DM9000DBG_GetPendingInts(void)
{
 UINT8T intr_state;
 
 intr_state = READ_REG1(0xfe);
 WRITE_REG1(0xfe, intr_state); /*clean ISR*/
 return(1);

}


/*::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::*/
/*::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::*/
int DM9000DBG_GetFrame(UINT8T *pbData,  unsigned int *pwLength)
{
 
 int i;
 unsigned long tmp32;
 unsigned short rxlen, tmplen;
 unsigned short status;
 UINT8T RxRead;

 RxRead=READ_REG1(0xFE);
 if(RxRead&0x01==0) return -1;

 READ_REG1(0x3); 

 READ_REG1(0x4);  /*Try & work run*/
 
 /* read the first byte*/
 RxRead = READ_REG1(0xf0);
 RxRead = IOREAD(dwEthernetDataPort);
 RxRead = IOREAD(dwEthernetDataPort);

 /* the fist byte is ready or not */
 if ((RxRead & 3) != 1)  /* no data */
 	{
    return -1;
	}																			    
 IOWRITE(dwEthernetIOBase, 0xf2); /* set read ptr ++ */
 switch (DM9000_iomode)
 {
  case DM9000_BYTE_MODE:
   status = IOREAD(dwEthernetDataPort)+(IOREAD(dwEthernetDataPort)<<8);
   rxlen = IOREAD(dwEthernetDataPort) + (IOREAD(dwEthernetDataPort)<<8);
   break;
  case DM9000_WORD_MODE:
   status = IOREAD16(dwEthernetDataPort);  
   rxlen = IOREAD16(dwEthernetDataPort);  
   break;
  case DM9000_DWORD_MODE:
   tmp32 = IOREAD32(dwEthernetDataPort);
   status = (unsigned short)(tmp32&0xffff);
   rxlen = (unsigned short)((tmp32>>16)&0xffff);
  default:
   uart_printf("[DM9000]Get status and rxlen error!!!");
   break;
 }  

 if (status & 0xbf00)
  uart_printf("[DM9000]RX status error!!!=[%x]",(status>>8) );

 /* move data from FIFO to memory */
 switch (DM9000_iomode)
 {
  case DM9000_BYTE_MODE:
   tmplen = rxlen ;
   for (i = 0; i < tmplen; i++)
    ((UINT8T *)pbData)[i] = IOREAD(dwEthernetDataPort);
   break;
  case DM9000_WORD_MODE:
   tmplen = (rxlen+1)/2;
   for (i = 0; i < tmplen; i++)
    ((UINT16T *)pbData)[i] = IOREAD16(dwEthernetDataPort);
   break;
  case DM9000_DWORD_MODE:
   tmplen = (rxlen+3)/4;
   for (i = 0; i < tmplen; i++)
    ((UINT32T *)pbData)[i] = IOREAD32(dwEthernetDataPort);
  default:
   uart_printf("[DM9000][RX]Move data error!!!");
   break;
 }

 *pwLength = rxlen;
 /* clean ISR */
 WRITE_REG1(0xfe,READ_REG1(0xfe));
 return rxlen; 
}



/*::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::*/
/*::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::*/
UINT16T DM9000DBG_SendFrame( UINT8T *pbData, UINT32T dwLength )
{
// uart_printf("[DM9000A]: DM9000DBG_SendFrame()..........\r\n");
 return dm9000_send(pbData, (UINT16T)dwLength);
}


/*
    @func   void | DM9000DBG_CurrentPacketFilter | Sets a receive packet h/w filter.
    @rdesc  N/A.
    @comm   
    @xref  
*/
void DM9000DBG_CurrentPacketFilter(UINT32T dwFilter)
{
 UINT8T uTemp;
 UINT16T i, oft;

 uart_printf("[DM9000A]: DM9000DBG_CurrentPacketFilter()..........\r\n");  
 // What kind of filtering do we want to apply?
 //
 // NOTE: the filter provided might be 0, but since this EDBG driver is used for KITL, we don't want
 // to stifle the KITL connection, so broadcast and directed packets should always be accepted.
 //
 if (dwFilter & PACKET_TYPE_ALL_MULTICAST)
 { // Accept *all* multicast packets.
  uTemp = READ_REG1(0x05);
  WRITE_REG1(0x05, uTemp | 0x08);  //Enable pass all multicast
 }

#if 0  //Always can receive multicast address according to hash table.
 if (dwFilter & PACKET_TYPE_MULTICAST)
 { // Accept multicast packets.
  
 }
#endif

 if (dwFilter & PACKET_TYPE_BROADCAST)
 {
  /* broadcast address */
  hash_table[3] = 0x8000;
  /* Write the hash table to MAC MD table */
  for (i = 0, oft = 0x16; i < 4; i++) {
   WRITE_REG1(oft++, hash_table[i] & 0xff);
   WRITE_REG1(oft++, (hash_table[i] >> 8) & 0xff);
  }
  
 }
 
 // Promiscuous mode is causing random hangs - it's not strictly needed.
 if (dwFilter & PACKET_TYPE_PROMISCUOUS)
 { // Accept anything.
  uTemp = READ_REG1(0x05);
  WRITE_REG1(0x05, uTemp | 0x02);  //Enable pass all multicast
 }

    uart_printf("DM9000: Set receive packet filter [Filter=0x%x].\r\n", dwFilter);


} // DM9000DBG_CurrentPacketFilter().


/*
    @func   int | DM9000DBG_MulticastList | Sets a multicast address filter list.
    @rdesc  TRUE = Success, FALSE = Failure.
    @comm   
    @xref  
*/
int DM9000DBG_MulticastList(UINT8T *pucMulticastAddresses, UINT32T dwNumAddresses)
{
 UINT8T nCount;
 UINT8T nIndex;
 UINT8T i, oft;
 UINT8T Reg5;

 //Stop RX
 Reg5 = READ_REG1(0x05);
 WRITE_REG1(0x05, Reg5 & 0xfe);

 // Compute the logical address filter value.
 //
 for (nCount = 0 ; nCount < dwNumAddresses ; nCount++)
 {
         uart_printf("DM9000: Multicast[%d of %d]  = %x-%x-%x-%x-%x-%x\r\n",
                             (nCount + 1),
        dwNumAddresses,
                             pucMulticastAddresses[6*nCount + 0],
                             pucMulticastAddresses[6*nCount + 1],
                             pucMulticastAddresses[6*nCount + 2],
                             pucMulticastAddresses[6*nCount + 3],
                             pucMulticastAddresses[6*nCount + 4],
                             pucMulticastAddresses[6*nCount + 5]);

  nIndex = CalculateHashIndex(&pucMulticastAddresses[6*nCount]);
         hash_table[nIndex/16]  |=  1 << (nIndex%16);
 }

 uart_printf("DM9000: Logical Address Filter = %x.%x.%x.%x.\r\n", hash_table[3], hash_table[2], hash_table[1], hash_table[0]);
 
 /* Write the hash table to MAC MD table */
 for (i = 0, oft = 0x16; i < 4; i++) {
  WRITE_REG1(oft++, hash_table[i] & 0xff);
  WRITE_REG1(oft++, (hash_table[i] >> 8) & 0xff);
 }

 //Start RX
 WRITE_REG1(0x05, Reg5);
 
    return(TRUE);

} // DM9000DBG_MulticastList

 int board_eth_get_addr(unsigned char *addr)
{
	memcpy(addr, OurEmacAddr, 6);
	return 0;
}
