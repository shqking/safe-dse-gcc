#include <stdio.h>
#include <string.h>

#define LEN 5

void main()
{
    char pwd[LEN];
    // init
	memset(pwd, 0, sizeof(pwd));

    // rewrite
    int var;
    scanf("%d", &var);
    memset(pwd, var, sizeof(pwd));
 	printf("pwd is %s\n", pwd);
	return;	
}
