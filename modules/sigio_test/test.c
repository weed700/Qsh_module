#include <stdio.h>
#include <string.h>
#include <stdlib.h>

void main()
{
    char* a = "/var/lib/docker/overlay2/0118b543d964bf05465b2870f72f7102ef3c9f343c552850c91aae9c0c2ecba4";

    char* temp = (char*)malloc(100*sizeof(char));

    sprintf(temp,"mount $path %s\n",a);

    printf("%s\n",temp);

    free(temp);
}
