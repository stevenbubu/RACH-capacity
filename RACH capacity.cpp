#include <iostream>
#include <stdlib.h>
#include <stdio.h>
#include <sstream>
#include <string>
#include <math.h>
#include <time.h>
#include <fstream>
#include "lcgrand.c"
#include "BackoffWin.c" 
using namespace std;


int** assign_mem2(int **pArray, unsigned int size1, unsigned int size2);
int*** assign_mem3(int ***pArray, unsigned int size1, unsigned int size2, unsigned int size3);
void release_mem2(int **pArray, unsigned int size1);
void release_mem3(int ***pArray, unsigned int size1, unsigned int size2);


// int sim_time=850;		//�t�μ����ɶ�
// const int MM=1750;		//the amount of MTC devices (Device number)
// const int PREM=54;		//preambles
// const int CWin=5;		//CW window

const int inter_random_access_cycle=10;	//�C�@��RACH���|���j�h�[
const int upbound_preamble=10;			//preamble���s�ǰe���ƤW��
const int RaResponseTime=20;			//RAR listen time, 20ms
const int RAR_amount=15; 				//�@���i�H�^��RAR�ƥ�


/*
   [0]	||			[1]			||		[2]				||		[3]			||		[4]			||	     [5]		||	     [6]	 ||  [7]  ||		[8]				||				[9]				||		[10]	||		[11]		||		[12]		||		 [13]		||	 [14]	||	 [15]	||
preamble|| preamble tran round	|| preamble send time	|| RAR listen window|| Msg3 tran round	||	Msg3 send time	|| Msg3 ACK time || stage ||over preamble tran round||mac-ContentionResolutionTimer	||   real-timer	|| Msg4 listen time	|| Msg4 tran round	||   Msg4 Ack time	||  P_time	|| M34_time	||
*/


int main(void)
{
	//simulation time
	int simulation_case_time[3]={250, 550, 850};
	int time_index = sizeof(simulation_case_time)/sizeof(simulation_case_time[0]);
	//the amount of MTC devices (Device number)
	int simulation_case_M[12]={	250, 500, 750, 1000, \
								1250, 1500, 1750, 2000, \
								2250, 2500, 2750, 3000};
	int sim_index = sizeof(simulation_case_M)/sizeof(simulation_case_M[0]);
	//preambles
	int simulation_case_PREM[1]={54};
	int PREM_index = sizeof(simulation_case_PREM)/sizeof(simulation_case_PREM[0]);

	int i=1;
	for(int simulation_case_index_time=0; simulation_case_index_time<time_index; simulation_case_index_time++)	// sim_time loop
	{
		for(int simulation_case_index_M=0; simulation_case_index_M<sim_index; simulation_case_index_M++)	// M loop
		{
			cout << "G=" << i \
				<< ", T=" << simulation_case_time[simulation_case_index_time] \
				<< ", M=" << simulation_case_M[simulation_case_index_M] 
				<< " --- Start" << endl;

			// All MTC detail for each ms
			ofstream filePtr;
			stringstream filePtr_name, filePtr_name_t;
			filePtr_name << "RACH " \
						<< "T=" << simulation_case_time[simulation_case_index_time] \
						<< ", M=" << simulation_case_M[simulation_case_index_M] \
						<< ", Prms=" << simulation_case_PREM[0];
			filePtr_name_t << filePtr_name.str() << ".csv";
			string filePtr_filename = filePtr_name_t.str();
			filePtr.open(filePtr_filename.c_str(), ios::out);
			
			// Time for each ms
			ofstream timePtr;
			stringstream timePtr_name;
			timePtr_name << filePtr_name.str() << " Time.csv";
			string timePtr_filename = timePtr_name.str();
			timePtr.open(timePtr_filename.c_str(), ios::out);
			
			// Parameters
			int preambles=simulation_case_PREM[0];
			int M=simulation_case_M[simulation_case_index_M];
			int cell=1;
			int list=M+1;		//Devices Number+1=M+1
			int parameter=16;
			int sim_time=simulation_case_time[simulation_case_index_time];
			int start_time=1;
			int end_time=sim_time;
			
			float success_rate=0;	// success rate
			float failure_rate=0;	// failure rate
			float average_success_access_delay=0;	// time-limited access delay(TAD)
			float average_access_delay=0;	// all devices access delay
			
			int HT_S_x=0;		//success device
			int TO_T_x=0;		//try over times device
			int TO_x=0;			//timeout device	
			double sum_RA_success_time=0;	//calculate device success time
			double sum_RA_access_time=0;	//calculate device access time
			

			int BOX[cell]={0};		//�x�s�C��CELL��devices�ƥ�
			for(int g=0; g<M; g++)
			{
				BOX[0]=BOX[0]+1;
			};
	
	
			// int RACH[cell][list][parameter]={0};
			int*** RACH;
			RACH = assign_mem3(&RACH[0], cell, list, parameter);
		
			for(int xx=0; xx<cell; xx++)
			{
				// �x�}�M�� 
				for(int list1=0; list1<list; list1++)
				{
					for(int p=0; p<parameter; p++)
					{
						RACH[xx][list1][p]=0;
					} 			
				}
				//2st subframe send RAR request
				for(int list1=0; list1<BOX[xx]; list1++)	//��Ҧ�Devices preamble_send_time=22 ��l�� �ҥѲ�22�ɶ��I�ǰe
				{
					RACH[xx][list1][2]=2+start_time-1;
				}
			}
				
			int** buffer_pre;	//��Xpreamble�X�Ӥ��O�_�I��
			buffer_pre = assign_mem2(&buffer_pre[0], M, 2);
			// int buffer_pre[M][2]={0};	//��Xpreamble�X�Ӥ��O�_�I��
			int BOX_pre[preambles]={0};	//�Ȧspreamble��ӽs����BOX
			//Main function
			for(int time=start_time; time<=end_time; time++)	//�@�Ӽ����϶��ɶ�
			{		
				for(int x=0; x<cell; x++)	//�C��cell���O run simulation
				{
					int cell_diveces_number=BOX[x];	//��X�o��CELL devices���ƥ�
	
					if(time%inter_random_access_cycle==2)	// 2st subframe ��RACH���|
					{
						int pre=0;
						for(int i=0; i<M; i++)
						{
							for(int j=0; j<2; j++)
							{
								buffer_pre[i][j]=0;
							}
						}
						for(int i=0; i<preambles; i++)
						{
							BOX_pre[i]=0;
						}					
						
						for(int scan_2=0; scan_2<cell_diveces_number; scan_2++)
						{
							if((RACH[x][scan_2][2]==time) && (RACH[x][scan_2][1]==upbound_preamble) && (RACH[x][scan_2][7]==0) && (RACH[x][scan_2][8]==0)) //preamble_send_time=time �B�W�L���ǤW��=10 �ҥHupbound_preamble=10��ڴN�O�ĤQ�@�^�X
							{
								RACH[x][scan_2][8]=1;
								RACH[x][cell_diveces_number][8]+=1;
								RACH[x][scan_2][3]=time+RaResponseTime;	//�ǰepreamble��A�btime+20���ɶ�ť��RAR
							}
							
							if((RACH[x][scan_2][2]==time) && (RACH[x][scan_2][1]<upbound_preamble) && (RACH[x][scan_2][7]==0) && (RACH[x][scan_2][8]==0)) //preamble_send_time=time �B�S���W�L���ǤW��=10
							{
								buffer_pre[pre][0]=scan_2;	//���X�C�o�������s��preamble��	�إ߲M��					
								RACH[x][scan_2][0]=(int)((lcgrand(3)*preambles));	//�H������preamble
								buffer_pre[pre][1]=RACH[x][scan_2][0];	//��preamble�x�s
	
								int pre_serial=0;		//�Ȧspreamble
								pre_serial=RACH[x][scan_2][0];
								BOX_pre[pre_serial]++;	//�Ȧspreamble��ӽs����BOX�A���]preamble=36  BOX_pre[36]++
	
								pre++;
							
								RACH[x][scan_2][3]=time+RaResponseTime;	//�ǰepreamble��A�btime+20���ɶ�ť��RAR
								RACH[x][scan_2][1]++;	//preamble�ǰe����+1   !!!!!!!  �]�����]���@���~+1  !!!!!!!
							}
						}
	
						
						double sum_preamble_collision_time=0;		//�֭ppream�ǰe�o�͸I��������
						double sum_preamble_collision_num=0;		//�֭ppream�ǰe�o�͸I�����ƶq 
						double sum_preamble_noncollision_time=0;	//�֭ppream�ǰe�L�o�͸I��������
						double sum_preamble_noncollision_num=0;		//�֭ppream�ǰe�L�o�͸I�����ƶq
						for(int scan_buffer_pre=0; scan_buffer_pre<preambles; scan_buffer_pre++)	//���yBOX_pre���C��devices��preamble
						{
							if(BOX_pre[scan_buffer_pre]>=2)			//�Y�ӽs����BOX�W�L���DEVICEs�h�N����PREAMBLE�I���F
							{
								sum_preamble_collision_time++;		//�֭p�X��pream ���ǰe�o�͸I��
								sum_preamble_collision_num += BOX_pre[scan_buffer_pre];
							}
							else if(BOX_pre[scan_buffer_pre]==1)	//�Y�ӽs����BOX��1��DEVICEs�h�N����PREAMBLE���w��ؼ�
							{
								sum_preamble_noncollision_time++;
								sum_preamble_noncollision_num += BOX_pre[scan_buffer_pre];
							}
						}
						
					
						if(pre>=1)
						{			
							int count = sum_preamble_noncollision_time - RAR_amount;
							int fail_num = sum_preamble_collision_num + count;
							
							//�p�� non-collision nodes �W�L�ǰe��� backoff 
							for(int scan_buffer_pre=0; scan_buffer_pre<preambles; scan_buffer_pre++)
							{
								if(BOX_pre[scan_buffer_pre]==1)
								{
									for(int member=0; member<pre; member++)
									{
										if((count>0) && (buffer_pre[member][1]==scan_buffer_pre))
										{
											count = count - 1;
											int index_1=1000;
											index_1=buffer_pre[member][0];
	
											int backoff_6=0;
											int back_win=0;
											back_win = BackoffWin(fail_num, ((sim_time/inter_random_access_cycle)-(time/inter_random_access_cycle)-2));
											backoff_6=RACH[x][index_1][2]+RaResponseTime+(int)(lcgrand(4)*back_win)*inter_random_access_cycle;
										
											if(backoff_6%inter_random_access_cycle==2)
											{
												RACH[x][index_1][2]=backoff_6;
											}
											else
											{
												if(backoff_6%inter_random_access_cycle==1)
												{
													RACH[x][index_1][2]=backoff_6+1;
												}
												else
												{
													RACH[x][index_1][2]=backoff_6+((inter_random_access_cycle+1)-(backoff_6-1)%inter_random_access_cycle);
												}
											}
											RACH[x][index_1][7]=0;
										}
										else if(buffer_pre[member][1]==scan_buffer_pre)
										{
											int index_2=1000;
											index_2=buffer_pre[member][0];
											RACH[x][index_2][7]=1;
											RACH[x][cell_diveces_number][7]+=1;
										}	
									} 
								} 
							}	//END//�p�� non-collision nodes �W�L�ǰe��� backoff 
							
							//�p�� collision nodes ��backoff 
							for(int Token=0; Token<pre; Token++)
							{
								int buffer_EZ[1]={0};				//�Ȧs��
								buffer_EZ[0]=buffer_pre[Token][1];	//�Ȧs
								buffer_pre[Token][1]=100; 			//��h�X�Ӫ��Ȧspreamble ��^�@�Ӱ�����100
								int countEZ=0;
	
								for(int member=0; member<pre; member++)
								{
									if((buffer_EZ[0]-buffer_pre[member][1])==0) //�Ypreamble���P�H���� countEZ++
									{
										countEZ++;
									}
								}
	
								if (countEZ>0)	//���X�ӼȦs��device�T�wpreamble���P�H���Ƨ��device �@backoff
								{
									int index_1=1000;
									index_1=buffer_pre[Token][0];
	
									int backoff_6=0;
									int back_win=0;
									back_win = BackoffWin(fail_num, ((sim_time/inter_random_access_cycle)-(time/inter_random_access_cycle)-2));
									backoff_6=RACH[x][index_1][2]+RaResponseTime+(int)(lcgrand(4)*back_win)*inter_random_access_cycle;			
								
									if(backoff_6%inter_random_access_cycle==2)
									{
										RACH[x][index_1][2]=backoff_6;
									}
									else
									{
										if(backoff_6%inter_random_access_cycle==1)
										{
											RACH[x][index_1][2]=backoff_6+1;
										}
										else
										{
											RACH[x][index_1][2]=backoff_6+((inter_random_access_cycle+1)-(backoff_6-1)%inter_random_access_cycle);
										}
									}
								}
								
								buffer_pre[Token][1]=buffer_EZ[0];	//��h�X�Ӫ��Ȧspreambel��^�쥻��m
								buffer_EZ[0]=0;
	
							}	//END//�p�� collision nodes ��backoff 
							
						} 
						
					}	//END// 2st subframe RACH process 
	
				} //END//�C��cell���O run simulation 
				
							
				/* debug show�X�Ҧ������G��Excel */ 
				for(int xd=0; xd<cell; xd++) 
				{
					filePtr <<"time="<<time<<","<<"cell="<<xd<<endl;
					filePtr <<"pream"<<","<<"p tr round"<<","<<"p send t"<<","<<"RAR lis W"<<"," \
							<<"M3 tr round"<<","<<"M3 send T"<<","<<"M3_ACK_T"<<","<<"Stage"<<"," \
							<<"over p t round"<<","<<"macTimer"<<","<<"real-timer"<<","<<"M4_listen_T"<<"," \
							<<"M4_t_round"<<","<<"M4_ACK_T"<<","<<"P_time"<<","<<"M34_time"<<endl;
					
					for(int yd=0; yd<BOX[xd]+1; yd++)
					{
						for(int zd=0; zd<parameter; zd++)
						{
							filePtr<<RACH[xd][yd][zd]<<",";
						}
						filePtr<<endl;
					}
					filePtr<<endl<<endl<<endl;
				}
				
				
				/* Analyze data and record to excel*/
				if(time==1)
				{
					timePtr <<"Time(ms)"<<","<<"Success"<<","<<"TryOver"<<","<<"Timeout"<<","<<"M"<<","<<"Success Rate"<<"," \
							<<"Failure Rate"<<","<<"TAD"<<","<<"Access Delay"<<"," \
							<<endl;	
				}
				
				
				for(int xd=0; xd<cell; xd++) 
				{
					TO_x=0;
					TO_T_x=0;
					HT_S_x=0;
					sum_RA_success_time=0;
					sum_RA_access_time=0; 
					
					for(int yd=0; yd<BOX[xd]; yd++)
					{	
						if((RACH[xd][yd][7]==0) && (RACH[xd][yd][8]==0))	//timeout
						{
							TO_x++;
							sum_RA_access_time += sim_time;
						}
						else if(RACH[xd][yd][7]==1)	//success 
						{
							HT_S_x++;
							sum_RA_success_time += RACH[xd][yd][3];
							sum_RA_access_time += RACH[xd][yd][3];	
						}
						else if(RACH[xd][yd][8]==1)	//try over times
						{
							TO_T_x++;
							sum_RA_access_time = sum_RA_access_time + RACH[xd][yd][3];
						}	
					}
				}
				
				success_rate=(float)HT_S_x/M;
				failure_rate = 1 - success_rate;
				average_success_access_delay = (float)(sum_RA_success_time/HT_S_x);
				average_access_delay = (float)(sum_RA_access_time/M);
				
				timePtr <<time<<","<<HT_S_x<<","<<TO_T_x<<","<<TO_x<<","<<M<<","<<success_rate<<"," \
						<<failure_rate<<","<<average_success_access_delay<<","<<average_access_delay<<"," \
						<<endl;
		
			} //END//�@�Ӽ����϶��ɶ�

			release_mem2(&buffer_pre[0], M);
			delete[] buffer_pre;
			buffer_pre = NULL;

			release_mem3(&RACH[0], cell, list);
			delete[] RACH;
			RACH = NULL;

			filePtr.close();
			timePtr.close();	
			cout << "G=" << i \
				<< ", T=" << simulation_case_time[simulation_case_index_time] \
				<< ", M=" << simulation_case_M[simulation_case_index_M] 
				<< " --- End" << endl << endl;
			i+=1;
		
		} // M loop
	} //END//sim_time loop

	return 0;			
}



int** assign_mem2(int **pArray, unsigned int size1, unsigned int size2)
{
    pArray = new int*[size1];
    for(int i = 0; i<size1; i++)
    {
        pArray[i] = new int[size2];
    }
    return pArray;
}

int*** assign_mem3(int ***pArray, unsigned int size1, unsigned int size2, unsigned int size3)
{
    pArray = new int**[size1];
    for(int i=0; i<size1; i++)
    {
        pArray[i] = new int*[size2];
		for(int j= 0; j<size2; j++)
		{
			pArray[i][j] = new int[size3];
		}
    }
    return pArray;
}

void release_mem2(int **pArray, unsigned int size1)
{
    for(int i = 0; i<size1; i++)
    {
        delete pArray[i];
    }
}

void release_mem3(int ***pArray, unsigned int size1, unsigned int size2)
{
	for(int i=0; i<size1; i++)
    {
		for(int j= 0; j<size2; j++)
		{
			delete pArray[i][j];
		}
    }
}
