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
#include "BackoffWin.c" 
using namespace std;

int sim_time=100;				//系統模擬時間

int sim_device_random_round=1;	//每個cell內的devices數目要random幾次
int	sim_round=1;				//CELL內devices數目的每種組合 要各模擬幾回合

const int cell=1;
const int list=1001;				//Devices Number+1=M+1
const int parameter=16;

int RACH[cell][list][parameter]={0};	//儲存模擬資料 RACH[x][y][z]

const int MM=100;				//the amount of MTC devices (Device number)
const int PREM=54;				//preambles
const int N=cell;				//cell number

const int inter_random_access_cycle=10;	//每一次RACH機會間隔多久
const int upbound_preamble=3;	//preamble重新傳送次數上限

const int RaResponseTime=20;				//RAR listen time, 20ms
const int RAR_amount=5; 					//一次可以回的RAR數目

const int Wbo=20;
float Imaxcomponent=((Wbo+7)/inter_random_access_cycle)+1;
int OverlapRegion=Imaxcomponent;

int BOX[cell]={0};	//儲存每個CELL的devices數目


/*Random產生每個CELL的Devices數目*/
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
		stringstream filePtr_name, filePtr_name_t;
		filePtr_name << "Test Simulation RACH Capacity N=" << simulation_case_M[simulation_case_index_N] 
					<< ", Prms=" << simulation_case_N[simulation_case_index_N] 
					<< ", T=" << sim_time;
		filePtr_name_t << filePtr_name.str() << ".csv";
		string filePtr_filename = filePtr_name_t.str();
		filePtr.open(filePtr_filename.c_str(), ios::out);
		
		// Time for each ms
		ofstream timePtr;
		stringstream timePtr_name;
		timePtr_name << filePtr_name.str() << " Time.csv";
		string timePtr_filename = timePtr_name.str();
		timePtr.open(timePtr_filename.c_str(), ios::out);
		
		
		const int preambles=simulation_case_N[simulation_case_index_N];
		//const int preambles=54;
		cout<<endl;

		for(int simulation_case_index_M=0; simulation_case_index_M<=0; simulation_case_index_M++)
		{
			
			int M=simulation_case_M[simulation_case_index_M];
			//int M=100;				

			for(int sim_device_random_round_count=1; sim_device_random_round_count<=sim_device_random_round; sim_device_random_round_count++)
			{
				int BOX[cell]={0};		//每個ROUND需要歸零的參數
				int num=0;				//每個ROUND需要歸零的參數
				
				for(int g=0; g<M; g++)
				{
					num=((int)(10000*lcgrand(6)))%N;
					BOX[num]=BOX[num]+1;
				}

				
				for(int sim_round_count=1; sim_round_count<=sim_round; sim_round_count++)	//每個cell devices數目的組合要跑的次數
				{
				
					int RACH[cell][list][parameter]={0};
				
		
					for(int xx=0; xx<cell; xx++)
					{
						for(int list1=0; list1<BOX[xx] ;list1++)	//把所有Devices preamble_send_time=22 初始化 皆由第22時間點傳送
						{
							RACH[xx][list1][2]=2+RaResponseTime;
						}
					}
					int flag[cell]={0};	//用來辨別只需印一次全部Devices完成的時間 //flag=0還未印出// flag =1印出
					
					
					//Main function
					for(int time=1; time<=sim_time; time++)	//共模擬多少時間
					{
						for(int x=0; x<cell; x++)	//每個cell分別 run simulation
						{
							int cell_diveces_number=0;	//抓出這個CELL devices的數目
							cell_diveces_number=BOX[x];
							RACH[x][cell_diveces_number][0]=0;//把統記(成功+失敗)的計數器歸零

							if(time%inter_random_access_cycle==2)	// 2st subframe 有RACH機會
							{
								int pre=0;				
								int buffer_pre[MM][2]={0};	//抓出preamble 出來比對是否碰撞
								int BOX_pre[PREM]={0};		//暫存preamble到 該編號的BOX
								
								for(int scan_2=0; scan_2<BOX[x]; scan_2++)
								{
									if((RACH[x][scan_2][2]==time) && (RACH[x][scan_2][1]==upbound_preamble)  && (RACH[x][scan_2] [7]==0) && (RACH[x][scan_2][8]==0) ) //preamble_send_time=time 且超過重傳上限=10 所以upbound_preamble=10實際就是第十一回合
									{
										RACH[x][scan_2][8]=1;
										RACH[x][cell_diveces_number][8]+=1;
									}
									
									if((RACH[x][scan_2][2]==time) && (RACH[x][scan_2][1]<upbound_preamble)  && (RACH[x][scan_2][7]==0) && (RACH[x][scan_2][8]==0) ) //preamble_send_time=time 且沒有超過重傳上限=10
									{
										buffer_pre[pre][0]=scan_2;	//哪幾列這次有重新抓preamble的	建立清單					
										RACH[x][scan_2][0]=(int)((lcgrand(3)*preambles));	//隨機產生preamble

										buffer_pre[pre][1]=RACH[x][scan_2][0];	//把preamble儲存

										int pre_serial=0;		//暫存preamble
										pre_serial=RACH[x][scan_2][0];
										BOX_pre[pre_serial]++;	//暫存preamble到 該編號的BOX  //假設preamble=36  BOX_pre[36]++

										pre++;
									
										RACH[x][scan_2][3]=time+RaResponseTime;	//傳送preamble後，在time+20的時間聽到RAR
										RACH[x][scan_2][1]++;			//preamble傳送次數+1   !!!!!!!  因為先跑完一次才+1  !!!!!!!
									}
								}

								
								double sum_preamble_collision_time=0;		//累計pream傳送發生碰撞的次數
								double sum_preamble_collision_num=0;		//累計pream傳送發生碰撞的數量 
								double sum_preamble_noncollision_time=0;	//累計pream傳送無發生碰撞的次數
								double sum_preamble_noncollision_num=0;		//累計pream傳送無發生碰撞的數量
								for(int scan_buffer_pre=0; scan_buffer_pre<preambles; scan_buffer_pre++)	//掃描BOX_pre內每個devices的preamble
								{
									if(BOX_pre[scan_buffer_pre]>=2)			//若該編號的BOX超過兩個DEVICEs則代表該PREAMBLE碰撞了
									{
										sum_preamble_collision_time++;		//累計幾次pream 的傳送發生碰撞
										sum_preamble_collision_num += BOX_pre[scan_buffer_pre];
									}
									else if(BOX_pre[scan_buffer_pre]==1)	//若該編號的BOX為1個DEVICEs則代表該PREAMBLE為預選目標
									{
										sum_preamble_noncollision_time++;
										sum_preamble_noncollision_num += BOX_pre[scan_buffer_pre];
									}
								}
								
							
								if(pre>=1)
								{			
									int count = sum_preamble_noncollision_time - RAR_amount;
									int fail_num = sum_preamble_collision_num + count;
									
									//計算 non-collision nodes 超過傳送限制的 backoff 
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
									}	//END//計算 non-collision nodes 超過傳送限制的 backoff 
									
									//計算 collision nodes 的backoff 
									for(int Token=0; Token<pre; Token++)
									{
										int buffer_EZ[1]={0};				//暫存器
										buffer_EZ[0]=buffer_pre[Token][1];	//暫存
										buffer_pre[Token][1]=100; 			//把搬出來的暫存preamble 塞回一個假的值100
										int countEZ=0;

										for(int member=0; member<pre; member++)
										{
											if((buffer_EZ[0]-buffer_pre[member][1])==0) //若preamble有與人重複 countEZ++
											{
												countEZ++;
											}
										}

										if (countEZ>0)	//拿出來暫存的device確定preamble有與人重複把該device 作backoff
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
										
										buffer_pre[Token][1]=buffer_EZ[0];	//把搬出來的暫存preambel放回原本位置
										buffer_EZ[0]=0;

									}	//END//計算 collision nodes 的backoff 
									
								} 
								
							}	//END// 2st subframe RACH process 

						} //END//每個cell分別 run simulation 
						
									
						/* debug show出所有的結果至Excel */ 
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
						
						
						/* Analyze data and record to excel*/
						if(time==1)
						{
							timePtr <<"Time(ms)"<<","<<"Success"<<","<<"TryOver"<<","<<"Timeout"<<","<<"M"<<"," \
									<<"Success Rate"<<","<<"Failure Rate"<<","<<"TAD"<<","<<"Access Delay"<<"," \
									<<endl;	
						}
						
						float success_rate=0;	// success rate
						float failure_rate=0;	// failure rate
						float average_success_access_delay=0;	// time-limited access delay(TAD)
						float average_access_delay=0;	// all devices access delay
						
						int HT_S_x=0;	//success device
						int TO_T_x=0;	//try over times device
						int TO_x=0;		//timeout device
						double sum_RA_success_time=0;	//calculate device success time
						double sum_RA_access_time=0;	//calculate device access time
									
						for(int xd=0; xd<cell; xd++) 
						{
							for(int yd=0; yd<BOX[xd]; yd++)
							{	
//								if((RACH[xd][yd][7]==0) && (RACH[xd][yd][8]==0))
								if(RACH[xd][yd][3]>sim_time)
								{
									TO_x++;
									sum_RA_access_time += sim_time;
								}
								else if(RACH[xd][yd][8]==1)
								{
									TO_T_x++;
									sum_RA_access_time = sum_RA_access_time + RACH[xd][yd][3] + RaResponseTime;
								}
								else if(RACH[xd][yd][7]==1)
								{
									HT_S_x++;
									sum_RA_success_time += RACH[xd][yd][3];
									sum_RA_access_time += RACH[xd][yd][3];
								}
							}
						}
						
						success_rate=(float)HT_S_x/M;
						failure_rate = 1 - success_rate;
						average_success_access_delay = (float)(sum_RA_success_time/HT_S_x);
						average_access_delay = (float)(sum_RA_access_time/M);
						
						timePtr <<time<<","<<HT_S_x<<","<<TO_T_x<<","<<TO_x<<","<<M<<"," \
								<<success_rate<<","<<failure_rate<<","<<average_success_access_delay<<"," \
								<<average_access_delay<<","<<endl;
								
								
					}//END//共模擬多少時間
					
				}//END//每個cell devices數目的組合要跑的次數
			
			}//END//每個cell devices數目 要random幾次

		}//end of simecase
		
		filePtr.close();
		timePtr.close();
		cout<< "================== End =================" << endl;
	
	}//END//Beginning for loop

	return 0;
			
}
