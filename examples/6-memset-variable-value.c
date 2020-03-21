#include <stdio.h>
#include <string.h>

void main()
{
    char pwd[20];
    char str[] = "password";
    printf("str is %s\n", str);
    printf("str len is %d\n", strlen(str));
    memcpy(pwd, str, strlen(str));
    printf("pwd is %s\n", pwd);

    int var;
    scanf("%d", &var);
    memset(pwd, var, sizeof(pwd));
    return;
}
