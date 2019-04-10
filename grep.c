#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <ctype.h>
#include "grep.h"
/* make BLKSIZE and LBSIZE 512 for smaller machines 4096 */
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

int	peekc;
int	lastc;
char	file[FNSIZE];
char	linebuf[LBSIZE];
char	expbuf[ESIZE+4];
int	given;
unsigned int	*addr1, *addr2;
unsigned int	*dot, *dol, *zero;
char	genbuf[LBSIZE];
char	*nextip;
int	ninbuf;
int	io;
int	open(char *, int);
int	creat(char *, int);
int	read(int, char*, int);
int	write(int, char*, int);
int	close(int);
int	unlink(char *);


char	*globp;
int	tfile	= -1;
int	tline;
char	*tfname;
char	*loc2;
char	obuff[BLKSIZE];
int	nleft;
int	names[26];
int	nbra;
unsigned nlall = 128;

char	*mkdtemp(char *);
char	tmpXXXXX[50] = "/tmp/eXXXXX";

#define MAXFILES 10
char* filenames[MAXFILES];
int ufile;
char* pattern;
char* pstart;
char* end = "q\n";
char* estart;
int gt1 = 0;
char* curfile;

void reset(int uf){
	ufile = uf;
	pstart = pattern;
	estart = end;
}

void gvars(void){
	zero = (unsigned *)malloc(nlall*sizeof(unsigned));
	tfname = mkdtemp(tmpXXXXX);
}

void pattern_init(int arc, char* arv[]){
    pattern = malloc(1 + strlen("g/") + strlen(arv[1])+ strlen("\n") );
    strcpy(pattern, "g/");
	strcat(pattern, arv[1]);
    strcat(pattern, "\n");
	pstart = pattern;
}

int dir_list(char* dirv[]){
	char* dl[MAXFILES+2];
	dl[0] = "0"; dl[1] = "0";
    int i = 2, ac = 2;
	DIR *dir;
	struct dirent *d;
    dir = opendir(dirv[2]);
	if(dir != NULL){
        while ((d = readdir(dir)) != NULL && i < MAXFILES+2){
            if(isalnum(*(d->d_name)) && (strstr(d->d_name, ".exe") == NULL)){
				dl[i] = d->d_name; ac++; i++;
			}
		}
		closedir(dir);
	} else{
		puts("directory does not exist.");
		exit(0);
	}
	return file_init(ac, dl);
}

int file_init(int arc, char* arv[]){
	int i, n, fc = 0;
	for(i = 2, n = 0; i < arc; i++, n++){
		char* filen = malloc(1 + strlen("e ") + strlen(arv[i])+ strlen("\n") );
      	strcpy(filen, "e ");
		strcat(filen, arv[i]);
     	strcat(filen, "\n");
      	filenames[n] = filen;
		fc++;
	}
	if (fc > 1){gt1 = 1;}
	return fc;
}

void commands(void) {
	unsigned int *a1;
	char c;
	char lastsep;

	for (;;) {
	c = '\n';
	for (addr1 = 0;;) {
		lastsep = c;
		a1 = 0;		 
		c = getchr();
		if (c!=',' && c!=';'){break;}
		if (a1==0) {
			a1 = zero+1;
			if (a1>dol){a1--;}
		}
		addr1 = a1;
		if (c==';'){dot = a1;}
	}
	if (lastsep!='\n' && a1==0){a1 = dol;}
	if ((addr2=a1)==0) {
		given = 0;
		addr2 = dot;	
	}
	else{given = 1;}
	if (addr1==0){addr1 = addr2;}
	switch(c) {

	case 'e':
		curfile = filenames[ufile];
		filename(c);
		init();
		addr2 = zero;
		io = open(file, 0);
		setwide();
		ninbuf = 0;
		c = zero != dol;
		append(getfile, addr2);
		continue;

	case 'g':
		global(1);
		continue;

	case 'p':
		print();
		continue;

	case 'q':
		unlink(tfname);
		return;

	case EOF:
		return;

	}
	}
}

void print(void) {
	unsigned int *a1 = addr1;
	do {
		if(gt1){
		write(1, (curfile+1), strlen(curfile)-2);
		write(1, ":", sizeof(":"));
		}
		puts(getline(*a1++));
	} while (a1 <= addr2);
	dot = addr2;
}

void setwide(void) {
	if (!given) {
		addr1 = zero + (dol>zero);
		addr2 = dol;
	}
}

void filename(int comm) {
	char *p1, *p2;
	int c;

	while ((c = getchr()) == ' '){}
	p1 = file;
	do {*p1++ = c;} while ((c = getchr()) != '\n');
	*p1++ = 0;
}

int getchr(void) {
	char c;
	if ((lastc=peekc)) {
		peekc = 0;
		return(lastc);
	}
	if (globp) {
		if ((lastc = *globp++) != 0){return(lastc);}
		globp = 0;
		return(EOF);
	}
	if(*(filenames[ufile]) != '\0'){
		c = *(filenames[ufile]);
		(filenames[ufile])++;
	}
	else if(*pstart != '\0'){
		c = *pstart;
		pstart++;
	}
	else if(*pstart == '\0'){c = *estart; estart++;}
	//if (read(0, &c, 1) <= 0){return(lastc = EOF);}
	lastc = c&0177;
	return(lastc);
}

int getfile(void) {
	int c;
	char *lp, *fp;

	lp = linebuf;
	fp = nextip;
	do {
		if (--ninbuf < 0) {
			if ((ninbuf = read(io, genbuf, LBSIZE)-1) < 0){
				if (lp>linebuf) {
					*genbuf = '\n';
				} else{return(EOF);}
			}
			fp = genbuf;
			while(fp < &genbuf[ninbuf]) {
				if (*fp++ & 0200){break;}
			}
			fp = genbuf;
		}
		c = *fp++;
		if (c=='\0'){continue;}
		*lp++ = c;
	} while (c != '\n');
	*--lp = 0;
	nextip = fp;
	return(0);
}

int append(int (*f)(void), unsigned int *a) {
	unsigned int *a1, *a2, *rdot;
	int nline, tl;

	nline = 0;
	dot = a;
	while ((*f)() == 0) {
		tl = putline();
		nline++;
		a1 = ++dol;
		a2 = a1+1;
		rdot = ++dot;
		while (a1 > rdot){*--a2 = *--a1;}
		*rdot = tl;
	}
	return(nline);
}

char *getline(unsigned int tl) {
	char *bp, *lp;

	lp = linebuf;
	bp = getblock(tl, READ);
	while ((*lp++ = *bp++)){}
	return(linebuf);
}

int putline(void) {
	char *bp, *lp;
	int nl;
	unsigned int tl;

	lp = linebuf;
	tl = tline;
	bp = getblock(tl, WRITE);
	nl = nleft;
	tl &= ~((BLKSIZE/2)-1);
	while ((*bp = *lp++)) {
		if (*bp++ == '\n') {
			*--bp = 0;
			break;
		}
		if (--nl == 0) {
			bp = getblock(tl+=(BLKSIZE/2), WRITE);
			nl = nleft;
		}
	}
	nl = tline;
	tline += (((lp-linebuf)+03)>>1)&077776;
	return(nl);
}

char *getblock(unsigned int atl, int iof) {
	int off;
	
	off = (atl<<1) & (BLKSIZE-1) & ~03;
	return(obuff+off);
}

void init(void) {
	int *markp;

	close(tfile);
	tline = 2;
	for (markp = names; markp < &names[26]; ){*markp++ = 0;}
	close(creat(tfname, 0600));
	tfile = open(tfname, 2);
	dot = dol = zero;
}

void global(int k) {
	int c;
	unsigned int *a1;

	setwide();
	c = getchr();
	compile(c);
	c = getchr();
	for (a1=zero; a1<=dol; a1++) {
		*a1 &= ~01;
		if (a1>=addr1 && a1<=addr2 && execute(a1)==k){*a1 |= 01;}
	}
	for (a1=zero; a1<=dol; a1++) {
		if (*a1 & 01) {
			*a1 &= ~01;
			dot = a1;
			globp = "p\n";
			commands();
			a1 = zero;
		}
	}
}

void compile(int eof) {
	int c;
	char *ep;
	char *lastep;
	char bracket[NBRA], *bracketp;
	int cclcnt;

	ep = expbuf;
	bracketp = bracket;
	c = getchr();
	nbra = 0;
	if (c=='^') {
		c = getchr();
		*ep++ = CCIRC;
	}
	peekc = c;
	lastep = 0;
	for (;;) {
		c = getchr();
		if (c == '\n') {
			peekc = c;
			*ep++ = CEOF;
			return;
		}
		if (c!='*'){lastep = ep;}
		switch (c) {

		case '\\':
			if ((c = getchr())=='(') {
				*bracketp++ = nbra;
				*ep++ = CBRA;
				*ep++ = nbra++;
				continue;
			}
			if (c == ')') {
				*ep++ = CKET;
				*ep++ = *--bracketp;
				continue;
			}
			if (c>='1' && c<'1'+NBRA) {
				*ep++ = CBACK;
				*ep++ = c-'1';
				continue;
			}
			*ep++ = CCHR;
			*ep++ = c;
			continue;

		case '.':
			*ep++ = CDOT;
			continue;

		case '*':
			if (lastep==0 || *lastep==CBRA || *lastep==CKET){
				*ep++ = CCHR;
				*ep++ = c;
			}
			*lastep |= STAR;
			continue;

		case '$':
			if (peekc!='\n'){
				*ep++ = CCHR;
				*ep++ = c;
			}
			*ep++ = CDOL;
			continue;

		case '[':
			*ep++ = CCL;
			*ep++ = 0;
			cclcnt = 1;
			if ((c=getchr()) == '^') {
				c = getchr();
				ep[-2] = NCCL;
			}
			do {
				if (c=='-' && ep[-1]!=0) {
					if ((c=getchr())==']') {
						*ep++ = '-';
						cclcnt++;
						break;
					}
					while (ep[-1]<c) {
						*ep = ep[-1]+1;
						ep++;
						cclcnt++;
					}
				}
				*ep++ = c;
				cclcnt++;
			} while ((c = getchr()) != ']');
			lastep[1] = cclcnt;
			continue;

		default:
			*ep++ = CCHR;
			*ep++ = c;
		}
	}
}

int execute(unsigned int *addr) {
	char *p1, *p2;
	int c;

	p2 = expbuf;
	p1 = getline(*addr);
	if (*p2==CCHR) {
		c = p2[1];
		do {
			//if (*p1!=c){continue;}
			if (advance(p1, p2)) {return(1);}
		} while (*p1++);
		return(0);
	}
	/* regular algorithm */
	do {
		if (advance(p1, p2)) {
			return(1);
		}
	} while (*p1++);
	return(0);
}

int advance(char *lp, char *ep) {
	char *curlp;
	int i;

	for (;;) switch (*ep++) {

	case CCHR:
		if (*ep++ == *lp++){continue;}
		return(0);

	case CDOT:
		if (*lp++){continue;}
		return(0);

	case CDOL:
		if (*lp==0){continue;}
		return(0);

	case CEOF:
		loc2 = lp;
		return(1);

	case CCL:
		if (cclass(ep, *lp++, 1)) {
			ep += *ep;
			continue;
		}
		return(0);
	}
}

int cclass(char *set, int c, int af) {
	int n;

	if (c==0){return(0);}
	n = *set++;
	while (--n){
		if (*set++ == c){return(af);}
	}
	return(!af);
}
