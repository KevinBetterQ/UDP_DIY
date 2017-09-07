#ifndef	__CHANGEFORMAT_H
#define	__CHANGEFORMAT_H

unsigned short ntohs(unsigned short s)
{
	return (s >> 8) | (s << 8);
}

unsigned long ntohl(unsigned long l)
{
	return  ((l >> 24) & 0x000000ff) |
		((l >>  8) & 0x0000ff00) |
		((l <<  8) & 0x00ff0000) |
		((l << 24) & 0xff000000);
}

unsigned short htons(unsigned short s)
{
	return (s >> 8) | (s << 8);
}

unsigned long htonl(unsigned long l)
{
	return ntohl(l);
}

#endif

