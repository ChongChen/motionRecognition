#include <stdio.h>
#include "uno_motionRecog.h"
#include <math.h>

void printArr(float x[], int N){
	for (int i = 0; i < N; i++)
		printf("%f  ", x[i]);
	printf("\n");
}


int main(){
	const int N = 5;
	float x[N] = {1.0, 2.0, 3, 4, 5};
	float y[N] = {2, 3, 1, 1, 1};
	float z[N] = {1.0, -1.0, -2.0, 3.0, -4.0};

	float energy[N] = {1.3, 2.2, 5.6, 4.5, 9};
	float entrophy = calEntrophy(energy, N);
	float std = arr_std(x, N);
	printf("%f\n", std);

	/*
	int crossZeroRate = calZeroCrossRate(z, N);
	printf("%d\n", crossZeroRate);
	*/


	/*
	float energy[N];
	getEnergy(x, y, z, energy, N);
	printArr(energy, N);

	printf("%f\n", log(2.7));
	*/


	/*
	float res1 = arr_sum(x, N);
	float res2 = arr_max(x, N);
	float res3 = arr_min(x, N);
	
	float z[N];
	arr_product(x, z, 2, N);

	float cc[N];
	arr2_product(x,y, cc, N);

	float dd[N];
	arr_divide(x, dd, 2, N);

	printf("sum %f\n", res1);
	printf("max %f\n", res2);
	printf("min %f\n", res3);
	printArr(z, N);
	printArr(cc, N);
	printArr(dd, N);
	*/

	/*
	printArr(x, N);
	arr_add(x, 1.0, N);
	printArr(x, N);
	arr_sub(x, 1.0, N);
	printArr(x, N);
	*/
}
