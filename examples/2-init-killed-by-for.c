#include <stdio.h>
#include <string.h>

#define LEN 5

void main()
{
    char str[LEN];
    // init
    memset(str, 0, sizeof(str));

    // rewrite
    int inx;
    for(inx = 0; inx < LEN; ++ inx)
    {
        int var;
        scanf("%d", &var);
        str[inx] = var;
    }
    printf("str is %s\n", str);
    return;
}
