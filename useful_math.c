/* ----- in here we store math functions required for the code ----- */

#include "useful_math.h"

int min(int x, int y){
	if ( x < y )
		return x;
	return y;
}

int max(int x, int y){
	if ( x > y )
		return x;
	return y;
}

unsigned absolute(int x){
	if (x < 0)
		return -x;
	return x;
}

int eucleidian(int x1, int y1, int x2, int y2){
	int e1 = x1 - y1;
	int e2 = x2 - y2;
	return sqrt((e1 * e1) + (e2 * e2));
}

/* --- not math just an idea though it feels trivial --- */
void error_message(char * message){
	printf("Error: %s\n", message);
	assert(0);
}
