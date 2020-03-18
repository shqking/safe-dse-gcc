#include <stdio.h>
#include <string.h>

#define LEN 5

void main()
{
    char pwd[LEN];
    // init
	memset(pwd, 0, sizeof(pwd));

    // rewrite
    int inx;
    for(inx = 0; inx < LEN; ++ inx)
    {
        int var;
        scanf("%d", &var);
        pwd[inx] = var;
    }
 	printf("pwd is %s\n", pwd);
	return;	
}
