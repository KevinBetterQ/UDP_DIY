/*********************************************************************************************
* File£º	main.c
* Author:	Embest	
* Desc£º	C code entry  
* History:	
*********************************************************************************************/

/*------------------------------------------------------------------------------------------*/
/*							include files                                                   */
/*------------------------------------------------------------------------------------------*/
#include "2410lib.h"

unsigned long download_len;
unsigned long download_addr;


/*------------------------------------------------------------------------------------------*/
/*							function declare                                                */
/*------------------------------------------------------------------------------------------*/
extern int NetLoadFile(UINT32T addr, UINT32T give_ip, UINT32T a3, UINT32T a4);
/*********************************************************************************************
* name:		main
* func:		c code entry
* para:		none
* ret:		none
* modify:
* comment:		
*********************************************************************************************/
int main(int argc,char **argv)
{
    sys_init();                                                 //Initial 2410x's Interrupt,Port and UART
	change_value_MPLL(88,1,1);	// Fin=12MHz FCLK=192MHz
	uart_init(192000000/4, 115200, UART0);
	delay(100);
	
	for( ; ; )
	{
		uart_printf("FS2410XP TFTP Test,please Enter 'ESC' to exit\n");
		NetLoadFile(0,0,0,0);
		delay(500);
	}
	
}

