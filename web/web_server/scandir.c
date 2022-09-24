/*************************************************************************
  > File Name: scandir.c
  > Author: ma6174
  > Mail: ma6174@163.com 
  > Created Time: 2022年09月22日 星期四 16时03分48秒
 ************************************************************************/

#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>

	int
main(int argc,char *argv[])
{
	struct dirent **namelist;
	int n;

	n = scandir(argv[1], &namelist, NULL, alphasort);
	if (n == -1 ) {
		perror("scandir");
		exit(EXIT_FAILURE);
	}

	while (n--) {
		printf("%s\n", namelist[n]->d_name);
		free(namelist[n]);
	}
	free(namelist);

	exit(EXIT_SUCCESS);
}

