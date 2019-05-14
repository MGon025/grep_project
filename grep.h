#ifndef GREP_H
#define GREP_H
char *getblock(unsigned int atl);
char *getline(unsigned int tl);
int advance(char *lp, char *ep);
int append(int (*f)(void), unsigned int *a);
int cclass(char *set, int c, int af);
void commands(void);
void compile(void);
int execute(unsigned int *addr);
void filename(void);
char getchr(void);
int getfile(void);
void global(int k);
void init(void);
void print(void);
void putchr(int ac);
int putline(void);
int puts(char *sp);
void setwide(void);
void gvars(void);
void pattern_init(char* p);
int file_init(int arc, char* arv[]);
int dir_list(char* dirv[]);
void reset(int uf);
int open(char *, int);
int creat(char *, int);
int read(int, char*, int);
int write(int, char*, int);
int close(int);
int unlink(char *);
char *mkdtemp(char *);

#define	BLKSIZE	16384
#define	NBLK	2047
#define	FNSIZE	128
#define	LBSIZE	16384
#define	ESIZE	256
#define	GBSIZE	256
#define	NBRA	5
#define	EOF	-1
#define	CBRA	1
#define	CCHR	2
#define	CDOT	4
#define	CCL	6
#define	NCCL	8
#define	CDOL	10
#define	CEOF	11
#define	CKET	12
#define	CBACK	14
#define	CCIRC	15
#define	STAR	01
#define	READ	0
#define	WRITE	1
#define MAXFILES 10

extern int	tfile, nbra, nleft, peekc, lastc, given, io, ninbuf, tline, ufile, gt1;
extern char file[FNSIZE], linebuf[LBSIZE], expbuf[ESIZE+4], genbuf[LBSIZE], obuff[BLKSIZE];
extern unsigned int *addr1, *addr2, *dot, *dol, *zero;
extern char *nextip, *globp, *tfname, *loc2, *pattern, *pstart, *end, *estart, *curfile;
extern unsigned nlall;
extern int	names[26];
extern char	tmpXXXXX[50];
extern char* filenames[MAXFILES];
#endif
