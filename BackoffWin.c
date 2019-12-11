/* Backoff window function*/
#include <iostream>
#include <math.h> 
//using namespace std;


const int m=23;		//default m


int BackoffWin(int fail_num, int section_val)
{
//	int RACH[cell][list][parameter]={0};
	int BW=0;	//select backoff window value
	int fail_BW=0;	//fail device backoff window value

	// Reshape RACH struct
//	for(int xx=0; xx<cell; xx++)
//	{
//		for(int list1=0; list1<list ;list1++)
//		{
//			for(int p=0; p<parameter; p++)
//			{
//				RACH[xx][list1][p] = arr[xx*list*parameter + list1*parameter + p];
//			}
//		}
//	}
	
	fail_BW = ceil((double)fail_num/m);
	
	if(fail_BW > section_val)
	{
		BW = section_val;
	}
	else
	{
		BW = fail_BW;
	}
		
	return BW;
}
