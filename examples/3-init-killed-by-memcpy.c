#include <stdio.h>
#include <string.h>

#define LEN 5

void main()
{
    char pwd[LEN];
    // init
	memset(pwd, 0, sizeof(pwd));

    // rewrite
    char str[] = "password";
    memcpy(pwd, str, LEN);
 	printf("pwd is %s\n", pwd);
	return;	
}
