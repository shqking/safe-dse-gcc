#include <stdio.h>
#include <string.h>

void main()
{
    char pwd[20];
    char str[] = "password";
    printf("str is %s\n", str);
    printf("str len is %d\n", strlen(str));

    // sensitive information, i.e. password, is stored in pwd array.
    memcpy(pwd, str, strlen(str));
    // use the password
    printf("pwd is %s\n", pwd);

    // clear the sensitive information.
    memset(pwd, 0, sizeof(pwd));
    return;
}
