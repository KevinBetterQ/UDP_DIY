#ifndef	__UTILS_H
#define	__UTILS_H

void memcpy(void *s1, const void *s2, int n);
void memset(void *s, const char ch, int n);
unsigned short ntohs(unsigned short s);
unsigned long ntohl(unsigned long l);
unsigned short htons(unsigned short s);
unsigned long htonl(unsigned long l);
//unsigned long strtoul(char *s);
void ultostr(char *s, unsigned long data);
unsigned long strtobcd(char *s);
unsigned long strtodec(char *str, int cnt);
int strlen(const char *s);
int strncmp(const char *s1, const char *s2, int maxlen);

int getyorn(void);
void printf(char *f, ...);
void sprintf(char *mem, char *fmt, ...);

int GetParameter(char *str, int cnt);

#endif
