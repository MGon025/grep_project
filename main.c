#include <stdio.h>
#include <stdlib.h>
#include "grep.h"

int main(int argc, char *argv[]) {
	if (argc > MAXFILES+2){
		puts("too many arguments");
		return 0;
	}
	if (argc > 1){
		pattern_init(argv[1]);
		zero = (unsigned *)malloc(nlall*sizeof(unsigned));
		tfname = mkdtemp(tmpXXXXX);
		init();
		if(argc == 2){ //use funtions with stdin and quit with ^C
			puts("use with stdin not implemented");
			return 0;
		}
		if (argc > 2){
			int usingfile = 0, filecount = 0;
			if(*argv[2] == '/'){filecount = dir_list(argv);}
     		else{filecount = file_init(argc, argv);}
			while(usingfile < filecount){
				reset(usingfile);
				commands();
				usingfile++;
			}
		}
    } else{puts("too few arguments");}
	return 0;
}
