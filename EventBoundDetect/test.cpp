#include "stdafx.h"

struct test{
	int* aa;
};

void main(){
	test* tt = new test;
	tt->aa = (int*)malloc(10 * sizeof(int));
	for (int i = 0; i < 10; i++){
		cout << i;
		tt->aa[i] = 0;
	}
}