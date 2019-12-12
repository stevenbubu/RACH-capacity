/* Backoff window function*/
//#include <iostream>
#include <math.h> 
//using namespace std;


const int m=23;		//default m

int power(int base, int exp)
{
	int p;
	if(exp == 0)
		return 1;
	for(p=base; exp>1; exp--)
		p *= base;
	return p;	
}


int BackoffWin(int trytime, int CWin)
{
	int BW=0;	//select backoff window value
	BW = power(2, (int)(CWin+trytime-1));
	return BW;
}
