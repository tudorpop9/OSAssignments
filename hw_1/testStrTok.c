#include <stdio.h>
#include <string.h>

int main(){
	char c[100],*r;
	scanf("%s",c);
	r=strtok(c,"-");
	int x=1;
	printf("%s\n",r);
	return 0;

}