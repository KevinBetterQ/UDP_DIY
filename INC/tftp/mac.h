#ifndef __MAC_H
#define __MAC_H

int DM9000DBG_Init(void);
int dm9000_send(unsigned char *data, unsigned int len);	   
int DM9000DBG_GetFrame(unsigned char *data, unsigned int *len);	  
int board_eth_get_addr(unsigned char *addr);

#endif /* __MAC_H */
