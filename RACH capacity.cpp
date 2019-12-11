#include <iostream>
#include <stdlib.h>
#include <stdio.h>
#include <sstream>
#include <string>
#include <math.h>
#include <time.h>
#include <fstream>
#include "lcgrand.c"
#include <assert.h>
using namespace std;

int sim_time=800;				//�t�μ����ɶ�

int sim_device_random_round=1;	//�C��cell����devices�ƥحnrandom�X��
int	sim_round=1;				//CELL��devices�ƥت��C�زզX �n�U�����X�^�X

const int cell=1;
const int list=1001;				//Devices Number+1=M+1
const int parameter=16;

int RACH[cell][list][parameter]={0};	//�x�s������� RACH[x][y][z]
//int HT_success[10000000]={0};	//�x�sRA���\device�����\�ɶ�

const int MM=1000;				//the amount of MTC devices (Device number)
const int PREM=54;				//preambles
const int N=cell;				//cell number

const int inter_random_access_cycle=10;	//�C�@��RACH���|���j�h�[
const int upbound_preamble=10;	//preamble���s�ǰe���ƤW��

const int RaResponseTime=20;				//RAR listen time, 20ms
const int RAR_amount=15; 					//�@���i�H�^��RAR�ƥ�

const int Wbo=20;
float Imaxcomponent=((Wbo+7)/inter_random_access_cycle)+1;
int OverlapRegion=Imaxcomponent;

int BOX[cell]={0};	//�x�s�C��CELL��devices�ƥ�


/*Random���ͨC��CELL��Devices�ƥ�*/
int num=0;


/*
   [0]	||			[1]			||		[2]				||		[3]			||		[4]			||	     [5]		||	     [6]	 ||  [7]  ||		[8]				||				[9]				||		[10]	||		[11]		||		[12]		||		 [13]		||	 [14]	||	 [15]	||
preamble|| preamble tran round	|| preamble send time	|| RAR listen window|| Msg3 tran round	||	Msg3 send time	|| Msg3 ACK time || stage ||over preamble tran round||mac-ContentionResolutionTimer	||   real-timer	|| Msg4 listen time	|| Msg4 tran round	||   Msg4 Ack time	||  P_time	|| M34_time	||
*/


int main(void)
{

	//int simulation_case_M[7]={10,20,50,100,200,500,1000};
	//int simulation_case_M[11]={10,20,50,100,200,500,600,700,800,900,1000};
	//int simulation_case_M[11]={70,140,350,700,1400,3500,4200,4900,5600,6300,7000};
	//int simulation_case_M[7]={70,140,350,700,1400,3500,7000};
	//int simulation_case_M[1]={1000};
	//int simulation_case_M[9]={10,20,50,100,200,300,400,500,600};

	int simulation_case_N[1]={PREM};	//preambles
	int simulation_case_M[1]={MM};		//the amount of MTC devices (Device number)

	for(int simulation_case_index_N=0; simulation_case_index_N<=0; simulation_case_index_N++) //Beginning for loop
	{
		// All detail content
		ofstream filePtr;
		stringstream filePtr_name;
		filePtr_name << "Test Simulation RACH Capacity N=" << simulation_case_M[simulation_case_index_N] 
					<< ", Prms=" << simulation_case_N[simulation_case_index_N] 
					<< ", T=" << sim_time << ".csv";
		string filePtr_filename = filePtr_name.str();
		filePtr.open(filePtr_filename.c_str(), ios::out);
		
		// Time for each ms
		ofstream timePtr;
		stringstream timePtr_name;
		timePtr_name << "Time.csv";
		string timePtr_filename = timePtr_name.str();
		timePtr.open(timePtr_filename.c_str(), ios::out);
		timePtr <<"time(ms)"<<","<<"sum_RA_success_time"<<","<<"HT_S_x"<<","<<"Average Delay(ms)"<<"," \
				<<"Access Success"<<","<<endl;
		
		
		const int preambles=simulation_case_N[simulation_case_index_N];
		//const int preambles=54;
		cout<<endl;

		for(int simulation_case_index_M=0; simulation_case_index_M<=0; simulation_case_index_M++)
		{
			
			int M=simulation_case_M[simulation_case_index_M];
			//int M=100;				

			for(int sim_device_random_round_count=1; sim_device_random_round_count<=sim_device_random_round; sim_device_random_round_count++)
			{
				int BOX[cell]={0};		//�C��ROUND�ݭn�k�s���Ѽ�
				int num=0;				//�C��ROUND�ݭn�k�s���Ѽ�
				
				for(int g=0; g<M; g++)
				{
					num=((int)(10000*lcgrand(6)))%N;
					BOX[num]=BOX[num]+1;
				}

				
				for(int sim_round_count=1; sim_round_count<=sim_round; sim_round_count++)	//�C��cell devices�ƥت��զX�n�]������
				{
				
					int RACH[cell][list][parameter]={0};
				
		
					for(int xx=0; xx<cell; xx++)
					{
						for(int list1=0; list1<BOX[xx] ;list1++)	//��Ҧ�Devices preamble_send_time=22 ��l�� �ҥѲ�22�ɶ��I�ǰe
						{
							RACH[xx][list1][2]=2+RaResponseTime;
						}
					}
					int flag[cell]={0};	//�Ψӿ�O�u�ݦL�@������Devices�������ɶ� //flag=0�٥��L�X// flag =1�L�X
					
					
					//Main function
					for(int time=1; time<=sim_time; time++)	//�@�����h�֮ɶ�
					{
						
						for(int x=0; x<cell; x++)	//�C��cell���O run simulation
						{
						
							int cell_diveces_number=0;	//��X�o��CELL devices���ƥ�
							cell_diveces_number=BOX[x];

							RACH[x][cell_diveces_number][0]=0;//��ΰO(���\+����)���p�ƾ��k�s


							if(time%inter_random_access_cycle==2)	// 2st subframe ��RACH���|
							{
								int pre=0;				
								int buffer_pre[MM][2]={0};	//��Xpreamble �X�Ӥ��O�_�I��
								int BOX_pre[PREM]={0};		//�Ȧspreamble�� �ӽs����BOX
								
								for(int scan_2=0; scan_2<BOX[x]; scan_2++)
								{
									
									if((RACH[x][scan_2][2]==time) && (RACH[x][scan_2][1]==upbound_preamble)  && (RACH[x][scan_2] [7]==0) && (RACH[x][scan_2][8]==0) ) //preamble_send_time=time �B�W�L���ǤW��=10 �ҥHupbound_preamble=10��ڴN�O�ĤQ�@�^�X
									{
										RACH[x][scan_2][8]=1;
										RACH[x][cell_diveces_number][8]+=1;
									}
									
								
									if((RACH[x][scan_2][2]==time) && (RACH[x][scan_2][1]<upbound_preamble)  && (RACH[x][scan_2][7]==0) && (RACH[x][scan_2][8]==0) ) //preamble_send_time=time �B�S���W�L���ǤW��=10
									{
										buffer_pre[pre][0]=scan_2;	//���X�C�o�������s��preamble��	�إ߲M��					
										RACH[x][scan_2][0]=(int)((lcgrand(3)*preambles));;	//�H������preamble

										buffer_pre[pre][1]=RACH[x][scan_2][0];	//��preamble�x�s

										int pre_serial=0;		//�Ȧspreamble
										pre_serial=RACH[x][scan_2][0];
										BOX_pre[pre_serial]++;	//�Ȧspreamble�� �ӽs����BOX  //���]preamble=36  BOX_pre[36]++

										pre++;
									
										RACH[x][scan_2][3]=time+RaResponseTime;	//�ǰepreamble��A�btime+20���ɶ�ť��RAR
										RACH[x][scan_2][1]++;			//preamble�ǰe����+1   !!!!!!!  �]�����]���@���~+1  !!!!!!!
									}
								}

								
								double sum_preamble_collision_time=0;	//�֭ppream�ǰe�o�͸I��������
								for(int scan_buffer_pre=0; scan_buffer_pre<preambles; scan_buffer_pre++)	//���yBOX_pre���C��devices��preamble
								{
									if(BOX_pre[scan_buffer_pre]>=2)//�Y�ӽs����BOX�W�L���DEVICEs�h�N����PREAMBLE�I���F
									{
										sum_preamble_collision_time++;//�֭p�X��pream ���ǰe�o�͸I��
									}
								}


								if(pre>1)	//�p�� collision nodes ��backoff 
								{
									for(int Token=0; Token<pre; Token++) //��C�ӽ��y�h�X�ӼȦs �۴��s �ΥH�P�_preamble�O�_����
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
											backoff_6=RACH[x][index_1][2]+RaResponseTime+(int)((lcgrand(4)*(1+Wbo)));
										
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

									}
								}
								
								double sum_preamble_noncollision_time=0;
								if(pre>1)	//�p�� non-collision nodes �W�L�ǰe��� backoff 
								{
									for(int scan_buffer_pre=0; scan_buffer_pre<preambles; scan_buffer_pre++)	//���yBOX_pre���C��devices��preamble
									{
										if(BOX_pre[scan_buffer_pre]==1)	//�Y�ӽs����BOX��1��DEVICEs�h�N����PREAMBLE���w��ؼ�
										{
											sum_preamble_noncollision_time++;
										}
									}
								
									int count = sum_preamble_noncollision_time - RAR_amount;
									for(int scan_buffer_pre=preambles-1; scan_buffer_pre>=0; scan_buffer_pre--)	//���yBOX_pre���C��devices��preamble
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
													backoff_6=RACH[x][index_1][2]+RaResponseTime+(int)((lcgrand(4)*(1+Wbo)));
												
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
										
									} //END//���yBOX_pre���C��devices��preamble
									
								} //END//�p�� non-collision nodes �W�L�ǰe��� backoff 
							}

						} //END//�C��cell���O run simulation 
						
									
						/* debug show�X�Ҧ������G��Excel */ 
						for(int xd=0; xd<cell; xd++) 
						{
							filePtr<<"time="<<time<<","<<"cell="<<xd<<endl;
							filePtr<<"pream"<<","<<"p tr round"<<","<<"p send t"<<","<<"RAR lis W"<<","<<"M3 tr round"<<","<<"M3 send T"<<","<<"M3_ACK_T"<<","<<"Stage"<<","<<"over p t round"<<","<<"macTimer"<<","<<"real-timer"<<","<<"M4_listen_T"<<","<<"M4_t_round"<<","<<"M4_ACK_T"<<","<<"P_time"<<","<<"M34_time"<<endl;
							
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
						
						
						
						double average_delay=0;			//HT_success[HT_S_x]�����֥[/HT_S_x
						double random_access_success=0;	//�έp������
//						double average_pream_collision_prob=0;	//collision pream������/�`�@���h�֦�pream�ǰe���|
//						float RACH_round_pream=0; 		//��ӧ���RA�� �ɶ������h�֦�RACH���|*preamble�ƥ�
						
						float HT_S_x=0;
						float sum_RA_success_time=0;	//�֭pRA���\device�����\�ɶ�
									
						for(int xd=0; xd<cell; xd++) 
						{
							for(int yd=0; yd<BOX[xd]; yd++)
							{	
								if(RACH[xd][yd][7]==1)
								{
									HT_S_x++;
									sum_RA_success_time += RACH[xd][yd][3];
								}
							}
						}
						average_delay=sum_RA_success_time/HT_S_x;
						random_access_success=(float)HT_S_x/(M*sim_device_random_round*sim_round);	//�������\�H�����
						
						timePtr <<time<<","<<sum_RA_success_time<<","<<HT_S_x<<","<<average_delay<<"," \
								<<random_access_success<<","<<endl;
						
//					 	average_pream_collision_prob=sum_preamble_collision_time/(RACH_round_pream);
					}//END//�@�����h�֮ɶ�
					
				}//END//�C��cell devices�ƥت��զX�n�]������
			
			}//END//�C��cell devices�ƥ� �nrandom�X��

		}//end of simecase
		
		filePtr.close();
		cout<< "================== End =================" << endl;
	
	}//END//Beginning for loop

	return 0;
			
}