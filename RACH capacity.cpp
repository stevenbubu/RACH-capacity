#include <iostream>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <time.h>
#include <fstream>
#include "lcgrand.c"
using namespace std;
//改WBO 1.修正Backoff的餘數 2.修正Pc統計時會用到的計算幾次RA 中Imax的算法
int sim_time=500;	//系統模擬時間

int sim_device_random_round=1;	//每個cell內的devices數目要random幾次
int	sim_round=100;	//CELL內devices數目的每種組合 要各模擬幾回合

double average_delay=0;	//HT_success[HT_S_x]全部累加/HT_S_x
double average_pream_collision_prob=0;	//collision pream的次數/總共有多少次pream傳送機會
float RACH_round_pream=0; //整個完成RA後 時間內有多少次RACH機會*preamble數目

const int cell=1;
const int list=1001;	//Devices Number+1=M+1
const int parameter=16;

int RACH[cell][list][parameter]={0};	//儲存模擬資料 RACH[x][y][z]
//int HT_success[10000000]={0};	//儲存RA成功device的成功時間
float HT_S_x=0;
float sum_RA_success_time=0;	//累計RA成功device的成功時間
float everyRACHdevicesnum[300]={0}; //儲存每次RACH 機會有多少devices
float everyRACHdevicesnum_1[300][10]={0};// 儲存每次RACH 機會有多少devices 並分類是歸類於第幾次傳送preamble


int phy=1;	//phy層因素加成數字越大 preamble被detect到的機會越高//1=不變更

int MM=3500;  //the amount of MTC devices (Device number)
const int N=cell;	//cell number


const int inter_random_access_cycle=10;	//每一次RACH機會間隔多久
const int upbound_preamble=10;	//preamble重新傳送次數上限
const int msg3_tran_bound=5;	//Msg3 重新傳送次數上限
const int msg4_tran_bound=5;	//Msg4 重新傳送次數上限

const int msg3_ACK_offset=4;	//Msg3 送出後預期在何時後收到ACK
const int msg4_ACk_offset=4;	//Msg4 送出後預期在何時後收到ACK


const int msg3_BF_offset=4;		//Msg3 失敗Backoff多久後重送
const int msg4_BF_offset=1;		//Msg4 失敗(M4_NACK送出的時間點後多久，實際不會送)Backoff多久後重送 M4↓ +msg4_ACk_offset   M4_NACK(不會送出)   +msg4_BF_offset   M4↓重送

const int msg4_receive_offset=1;	//Msg3 ACK正確接受後 多久收到Msg4


//const int ACK_offset=4;	//4 // msg3送出以後 要delay幾個subframe才會聽到msg4 (+3 msg3's ack)

const int RaResponseTime=7;	//preamble送出去後 第幾個subframe會聽到RAR 1+7=8
const int mac_ContentionResolutionTimer=48;	//48//	msg3送出去後要在48八個subframe內收到msg4
const int msg3_percent=9;		//msg3 被正確收到的機率1~9(成)
const int msg4_percent=9;		//msg4 被正確收到的機率1~9(成)
const int RAR_amount=15; //一次可以回的RAR數目

int checkRARamount[2][54]={0}; //儲存那些人 not collided & detected[有無被Random選上回RAR的 標記][哪一位User]

const int Wbo=20;
float Imaxcomponent=((Wbo+7)/inter_random_access_cycle)+1;
int OverlapRegion=Imaxcomponent;

int BOX[cell]={0};	//儲存每個CELL的devices數目


/*Random產生每個CELL的Devices數目*/
int num=0;
int total_Devices=0;

int stop=0;


/*
   [0]	||			[1]			||		[2]				||		[3]			||		[4]			||	     [5]		||	     [6]	 ||  [7]  ||		[8]				||				[9]				||		[10]	||		[11]		||		[12]		||		 [13]		||	 [14]	||	 [15]	||
preamble|| preamble tran round	|| preamble send time	|| RAR listen window|| Msg3 tran round	||	Msg3 send time	|| Msg3 ACK time || stage ||over preamble tran round||mac-ContentionResolutionTimer	||   real-timer	|| Msg4 listen time	|| Msg4 tran round	||   Msg4 Ack time	||  P_time	|| M34_time	||
*/

int main(void)
{
	ofstream filePtr;
	//filePtr.open("test Simulation RACH Capacity.csv", ios::out);
	ofstream filePtrX;
	//filePtrX.open("Tran Simulation RACH Capacity_average_delay cell=1 Wbo=10 M=10 1000 N=54 Nptmax=10.csv", ios::out);
	//filePtrX.open("test.csv", ios::out);

	ofstream filePtrY;
	//filePtrY.open("Tran Simulation RACH Capacity RAD CDF cell=1 Wbo=10 M=10 1000 N=54 Nptmax=10.csv", ios::out);

	ofstream filePtrZ;
	//filePtrZ.open("Tran Simulation RACH Capacity PT CDF cell=1 Wbo=10 M=10 1000 N=54 Nptmax=10.csv", ios::out);
	
	ofstream filePtrA;
	//filePtrA.open("Tran Simulation RACH Capacity Average PT cell=1 Wbo=10 M=10 1000 N=54 Nptmax=10.csv", ios::out);

	ofstream filePtrUs;
	//filePtrUs.open("Tran Us Ps M=100 150 300 N=18 27 54 Nptmax=5.csv");

	ofstream filePtr_every_RACH_devices_num;
	//filePtr_every_RACH_devices_num.open("every_RACH_devices_num.csv", ios::out);

	ofstream every_RACH_devices_num_pream_tran_round;
	//every_RACH_devices_num_pream_tran_round.open("every_RACH_devices_num_pream_tran_round.csv", ios::out);

	ofstream filePtr_CheckRARIndex;
	//filePtr_CheckRARIndex.open("CheckRARIndex.csv", ios::out);


	//int simulation_case_M[7]={10,20,50,100,200,500,1000};
	//int simulation_case_M[11]={10,20,50,100,200,500,600,700,800,900,1000};
	//int simulation_case_M[11]={70,140,350,700,1400,3500,4200,4900,5600,6300,7000};
	//int simulation_case_M[7]={70,140,350,700,1400,3500,7000};
	//int simulation_case_M[1]={1000};
	//int simulation_case_M[9]={10,20,50,100,200,300,400,500,600};

	int simulation_case_N[1]={54};
	int simulation_case_M[1]={1000};

		for(int simulation_case_index_N=0; simulation_case_index_N<=0; simulation_case_index_N++)
		{
			const int preambles=simulation_case_N[simulation_case_index_N];
			//const int preambles=54;
			cout<<endl;

			for(int simulation_case_index_M=0; simulation_case_index_M<=0; simulation_case_index_M=simulation_case_index_M++)
			{
				int M=0;
				M=simulation_case_M[simulation_case_index_M];
				//int M=100;				

				double sum_RA_success_time=0;
				int HT_S_x=0;
				double sum_preamble_collision_time=0;	//累計pream傳送發生碰撞的次數
				float RACH_round_pream=0;
				double random_access_success=0;	//統計模擬的
				int success_table_RAD[301]={0};	//歸零//儲存成功RA的DEVICE 的RA delay time  //delay=33→ success_table_RAD[33]++
				int success_table_PT[11]={0};	//歸零//儲存多少成功RA的DEVICE的pream tran round //pream round=8→success_table_PT[8]++

				int st_1=0;


				for(int sim_device_random_round_count=1; sim_device_random_round_count<=sim_device_random_round; sim_device_random_round_count++)
				//if(sim_device_random_round>0)//每個cell devices數目 要random幾次
				{
						int BOX[cell]={0};		//每個ROUND需要歸零的參數
						int num=0;				//每個ROUND需要歸零的參數
						
						for(int g=0; g<M; g++)
						{
							num=((int)(10000*lcgrand(6)))%N;
							BOX[num]=BOX[num]+1;
						}


					
					for(int sim_round_count=1; sim_round_count<=sim_round; sim_round_count++ )	//每個cell devices數目的組合要跑的次數
					{
						int RACH_cycle=0; //第幾次RACH機會
						
						int total_Devices=0;	//每個ROUND需要歸零的參數

						int RACH[cell][list][parameter]={0};
						

		/*		
						for(int p=0; p<N; p++)
						{
							cout<<"Cell "<<p<<"="<<BOX[p]<<endl;
							total_Devices=total_Devices+BOX[p];
						}


						cout<<"total_Devices="<<total_Devices<<endl;
						cout<<"sim_device_random_round="<<sim_device_random_round<<endl;
						cout<<"sim_round_count="<<sim_round_count<<endl;
		*/	
						for(int xx=0; xx<cell; xx++)
						{
							for(int list1=0; list1<BOX[xx] ;list1++)	//把所有Devices preamble_send_time=2 初始化 皆由第2時間點傳送
							{
								RACH[xx][list1][2]=2;
							}
						}
						int flag[cell]={0};	//用來辨別只需印一次全部Devices完成的時間 //flag=0還未印出// flag =1印出
						for(int time=1; time<=sim_time; time++)	//共模擬多少時間
						{
							int Finish_Device_Number=0;	//統計每個時間點 "每個CELL"(成功+失敗)的計數器歸零

							for(int x=0; x<cell; x++)	//每個cell分別 run simulation
							{
							
								int cell_diveces_number=0;	//抓出這個CELL devices的數目
								cell_diveces_number=BOX[x];

								RACH[x][cell_diveces_number][0]=0;//把統記(成功+失敗)的計數器歸零

								//////////////////////////////////////////////////////
	/*							for(int list_48=0; list_48<BOX[x]; list_48++)
								{
									int r_1=0;
									if(RACH[x][list_48][9]!=0)	//在msg3還未第一次傳送之前 MAC contention timer 不會被寫入第一次傳送的時間 所以也就不用去判斷 是否超過48
									{
										r_1=(time-RACH[x][list_48][9]);
									}
									

										if(RACH[x][list_48][5]!=0 && (r_1>mac_ContentionResolutionTimer) && (RACH[x][list_48][7]!=1) && (RACH[x][list_48][8]!=1))//超過48個subframe上限
										{ //判斷是否落在msg3傳送階段 若非msg3收送階段則不需要啟動 MAC contention timer 超過48個subframe的判斷
											
											if(RACH[x][list_48][1]==upbound_preamble)//若MAC TIMEOUT pream tran round又達到上限 則不需要將pream tran randomback 
											{
												RACH[x][list_48][5]=0;	//若msg3~msg4超過48個subframe則不繼續等待
												RACH[x][list_48][6]=0;	//若msg3~msg4超過48個subframe則不繼續等待
												RACH[x][list_48][8]=1;
												RACH[x][list_48][9]=0;	//若msg3~msg4超過48個subframe則不繼續等待 把msg3第一次傳送時間歸零
												RACH[x][list_48][10]=0;	//若msg3~msg4超過48個subframe則不繼續等待

											}

											else
											{
											
											
												RACH[x][list_48][4]=0;	//若msg3~msg4超過48個subframe則不繼續等待
												RACH[x][list_48][5]=0;	//若msg3~msg4超過48個subframe則不繼續等待
												RACH[x][list_48][6]=0;	//若msg3~msg4超過48個subframe則不繼續等待
												RACH[x][list_48][9]=0;	//若msg3~msg4超過48個subframe則不繼續等待 把msg3第一次傳送時間歸零
												RACH[x][list_48][10]=0;	//若msg3~msg4超過48個subframe則不繼續等待



												int backoff=0;
												backoff=time+(((int)(1000*lcgrand(3)))%21);
												RACH[x][list_48][4]=0;	//msg3 tran round歸零
															
												//在msg3階段失敗也是20ms random backoff
												if(backoff%5==1)
												{
													RACH[x][list_48][2]=backoff;
												}
												else
												{
													if(backoff%5==0)
													{
														RACH[x][list_48][2]=backoff+1;
													}
													else
													{
														RACH[x][list_48][2]=backoff+(6-backoff%5);
													}
												}
											}

										}
								}
								
	*/
								if(time%inter_random_access_cycle==2)	// 2st & 7st subframe 有RACH機會
								{
									int pre=0;/////////////////////
									int buffer_pre[10000][2]={0};	//抓出preamble 出來比對是否碰撞
									int BOX_pre[60]={0};		//暫存preamble到 該編號的BOX
									for(int scan_2=0; scan_2<BOX[x]; scan_2++)
									{
										//目前若preamble碰撞所造成的Backoff preamble retransmission 是先BF之後 到了可以傳送pream的機會時 才去判斷是否已經到達pream可以傳送次數的上限
										//不過因為 目前統計RANDOM ACCESS DELAY只有針對成功的device 若走到這一步 也不會列入統計 所以不影響 最後多一段BF 才判斷失敗的時間
										
										if(RACH[x][scan_2][2]==time) // 從M3/M4階段 跳回Pream階段後
										{
											RACH[x][scan_2][15]=RACH[x][scan_2][15]+(time-1)-(RACH[x][scan_2][14]+RACH[x][scan_2][15]); 
											// 把(現在時間-1)扣掉前面分別已經花在( M3/M4階段+ 與Pream階段)的時間總和 再累加進 M34_time
										}
										

										if((RACH[x][scan_2][2]==time) && (RACH[x][scan_2][1]==upbound_preamble)  && (RACH[x][scan_2][7]==0) && (RACH[x][scan_2][8]==0) ) //preamble_send_time=time 且超過重傳上限=10 所以upbound_preamble=10實際就是第十一回合
										{
											RACH[x][scan_2][8]=1;
										}
										
									
										if((RACH[x][scan_2][2]==time) && (RACH[x][scan_2][1]<upbound_preamble)  && (RACH[x][scan_2][7]==0) && (RACH[x][scan_2][8]==0) ) //preamble_send_time=time 且沒有超過重傳上限=10
										{
											buffer_pre[pre][0]=scan_2;	//哪幾列這次有重新抓preamble的	建立清單					
											RACH[x][scan_2][0]=(int)((lcgrand(3)*preambles)+1);;	//隨機產生preamble


											buffer_pre[pre][1]=RACH[x][scan_2][0];	//把preamble儲存

											int pre_serial=0;		//暫存preamble
											pre_serial=RACH[x][scan_2][0];
											BOX_pre[pre_serial]++;	//暫存preamble到 該編號的BOX  //假設preamble=36  BOX_pre[36]++


											pre++;
											
											//把每次Backoff Window內 屬於第幾次preamble傳送的人分別儲存
											int d_pream_tran_round=0;
											d_pream_tran_round=RACH[x][scan_2][1];
											everyRACHdevicesnum_1[RACH_cycle][d_pream_tran_round]++;
									
											RACH[x][scan_2][3]=time+7; //傳送preamble後，在time+7的時間聽到RAR
											RACH[x][scan_2][1]++;	//preamble傳送次數+1   !!!!!!!  因為先跑完一次才+1  !!!!!!!

											
										}
									}


									//把每一個RACH機會有多少devices參與RA的值 統計下來
									
									everyRACHdevicesnum[RACH_cycle]+=pre; //每模擬一次 就把當次模擬中 該RACH機會 所參與的devices累加進
									RACH_cycle++;

									for(int scan_buffer_pre=0; scan_buffer_pre<preambles; scan_buffer_pre++)	//掃描BOX_pre內每個devices的preamble
									{
										if(BOX_pre[scan_buffer_pre]>=2)//若該編號的BOX超過兩個DEVICEs則代表該PREAMBLE碰撞了
										{
											sum_preamble_collision_time++;//累計幾次pream 的傳送發生碰撞
										}

									}


									if(pre>1)
									{
										//if(time=15)
										for(int Token=0; Token<pre; Token++) //把每個輪流搬出來暫存 相減等於零 用以判斷preamble是否重複
										{
											int buffer_EZ[1]={0};		//暫存器
											buffer_EZ[0]=buffer_pre[Token][1];	//暫存
											buffer_pre[Token][1]=100; //把搬出來的暫存preamble 塞回一個假的值100
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
												backoff_6=RACH[x][index_1][2]+RaResponseTime+(int)((lcgrand(4)*(1+Wbo))+1);
												
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

										}
									}
									
								
								}

								int RAR_amount_count=1;	//目前回了幾個RAR
								/*新的回RAR機制*/
								int checkRAR_index=0;	//checkRARamount陣列中 從零的位置開始儲存
								
								for(int clean_checkRARamount_x=0; clean_checkRARamount_x<2; clean_checkRARamount_x++)
								{
									for(int clean_checkRARamount_y=0; clean_checkRARamount_y<54; clean_checkRARamount_y++)
									{
										checkRARamount[clean_checkRARamount_x][clean_checkRARamount_y]=0;
									}
								}

								for(int scanRAR=0; scanRAR<BOX[x]; scanRAR++)//將no collided & detected的人丟進checkRARamount[有無被Random選上回RAR的 標記][哪一位User] 的array
								{
									if((RACH[x][scanRAR][3]==time) && (RACH[x][scanRAR][2]<RACH[x][scanRAR][3]) && (RACH[x][scanRAR][4]<=msg3_tran_bound))	
									//判斷RAR listen window是否 > preamble send time
									//用以區別no preamble collision的devices
									{
										int phy_i=0;
										phy_i=RACH[x][scanRAR][1];	//phy_i means 第phy_i次傳送preamble
										
										if( phy*(1-exp(-phy_i))>lcgrand(5) ) //判斷有通過PHY
										{
											checkRARamount[1][checkRAR_index]=scanRAR;
											checkRAR_index++;//儲存array位置往下一個
										}
										else //未通過PHY 進行BACK OFF
										{
											if(RACH[x][scanRAR][1]==upbound_preamble) // 若未通過PHY因素，且若pream tran round同時已經達上限 則不需要random backoff
											{
												RACH[x][scanRAR][8]=1;
												RACH[x][scanRAR][4]=0;
												RACH[x][scanRAR][5]=0;
												RACH[x][scanRAR][6]=0;
												RACH[x][scanRAR][9]=0;
												RACH[x][scanRAR][10]=0;
											}
											else
											{
												//preamble未通過1-exp(-i)的因素
												//從RAR listen window backoff
												int backoff_3=0;
												backoff_3=time+(int)((lcgrand(6)*(1+Wbo))+1);
																						
												if(backoff_3%inter_random_access_cycle==2)
												{
													RACH[x][scanRAR][2]=backoff_3;
												}
												else
												{
													if(backoff_3%inter_random_access_cycle==1)
													{
														RACH[x][scanRAR][2]=backoff_3+1;
													}
													else
													{
														RACH[x][scanRAR][2]=backoff_3+((inter_random_access_cycle+1)-(backoff_3-1)%inter_random_access_cycle);
													}
												}
											}			
										}//未通過PHY 進行BACK OFF
									}
								}
								filePtr_CheckRARIndex<<checkRAR_index<<",";
								int sumRAR=0;  //儲存 目前已經Random 出多少位會被回RAR的user
								if(checkRAR_index>0)
								{					
									while( (sumRAR<checkRAR_index)&&(sumRAR<RAR_amount) )//從checkRARamount[標記][]array中隨機選取要回RAR的人並把"標記"設為1
									{
										sumRAR=0;
										int RAR_reply_user=0; //儲存RANDOM出來誰要被回RAR的USER
										RAR_reply_user=(int)(lcgrand(7)*checkRAR_index); //random 0 ~ (checkRAR_index-1)
										checkRARamount[0][RAR_reply_user]=1; //RANDOM選中的user 標註設為1
										for(int sumR_i=0; sumR_i<checkRAR_index; sumR_i++) //累加 目前已經Random 出多少位 要被回RAR的user
										{
											sumRAR=sumRAR+checkRARamount[0][sumR_i];
										}
										
									}

									for(int identify_RAR=0; identify_RAR<checkRAR_index; identify_RAR++) //將 Random 回RAR的結果 進行處理							
									{
										filePtr_CheckRARIndex<<checkRARamount[0][identify_RAR]<<","<<checkRARamount[1][identify_RAR]<<endl;
										if(checkRARamount[0][identify_RAR]==1)//checkRARamount[標記][]==1 ;設立Msg3傳送時間
										{
											int whois1=0;
											whois1=checkRARamount[1][identify_RAR]; //哪位user 在checkRARamount[標記][]==1
											RACH[x][whois1][5]=time+1; //收到RAR後下一個subframe就送出msg3
											//RACH[x][whois1][4]++;	//msg3 tran round+1
										}
										else//checkRARamount[標記][]==0 ;進行Backoff
										{
											int whois0=0;
											whois0=checkRARamount[1][identify_RAR]; //哪位user 在checkRARamount[標記][]==0

											if(RACH[x][whois0][1]==upbound_preamble) // 若未通過PHY因素，且若pream tran round同時已經達上限 則不需要random backoff
											{
												RACH[x][whois0][8]=1;
												RACH[x][whois0][4]=0;
												RACH[x][whois0][5]=0;
												RACH[x][whois0][6]=0;
												RACH[x][whois0][9]=0;
												RACH[x][whois0][10]=0;
											}
											else
											{
												//preamble未通過1-exp(-i)的因素
												//從RAR listen window backoff
												int backoff_9=0;
												backoff_9=time+(int)((lcgrand(8)*(1+Wbo))+1);
																						
												if(backoff_9%inter_random_access_cycle==2)
												{
													RACH[x][whois0][2]=backoff_9;
												}
												else
												{
													if(backoff_9%inter_random_access_cycle==1)
													{
														RACH[x][whois0][2]=backoff_9+1;
													}
													else
													{
														RACH[x][whois0][2]=backoff_9+((inter_random_access_cycle+1)-(backoff_9-1)%inter_random_access_cycle);
													}
												}
											}

										}//checkRARamount[標記][]==0 ;進行Backoff

									}//將 Random 回RAR的結果 進行處理
								} //end of 新的回RAR機制

							
								
								for(int list2=0; list2<BOX[x]; list2++)//判斷是否收到RAR
								{
									
									/*	舊的回RAR機制 依照順序 由上而下
									if(RACH[x][list2][5]!=0 && RACH[x][list2][9]>=1 && RACH[x][list2][7]==0 && RACH[x][list2][8]==0)//device未成功未失敗之前 pream傳送時間在 msg3第一次傳送之後 用以界定real time是否需要停止計數
									{
										RACH[x][list2][10]=time-RACH[x][list2][9];	//記錄即時的mac contention timer
									}


									if((RACH[x][list2][3]==time) && (RACH[x][list2][2]<RACH[x][list2][3]) && (RACH[x][list2][4]<=msg3_tran_bound))	
										//判斷RAR listen window是否 > preamble send time
										//用以區別no preamble collision的devices
									{
										int r_2=0;
										r_2=RACH[x][list2][1];	//i means 第i次傳送preamble
										if(( phy*(1-exp(-r_2))>lcgrand(4) )&&(RAR_amount_count<=RAR_amount))	//無與人碰撞的preamble，並且通過1-exp(-i)的因素
										{
											RACH[x][list2][5]=time+1; //收到RAR後下一個subframe就送出msg3
											//RACH[x][list2][4]++;	//msg3 tran round+1
											RAR_amount_count++;
										}
										else
										{
											if(RACH[x][list2][1]==upbound_preamble) // 若未通過PHY因素，且若pream tran round同時已經達上限 則不需要random backoff
											{
												RACH[x][list2][8]=1;
												RACH[x][list2][4]=0;
												RACH[x][list2][5]=0;
												RACH[x][list2][6]=0;
												RACH[x][list2][9]=0;
												RACH[x][list2][10]=0;
											}
											else
											{
												//preamble未通過1-exp(-i)的因素
												//從RAR listen window backoff

												int backoff_3=0;
												backoff_3=time+(int)((lcgrand(6)*(1+Wbo))+1);
																						
												if(backoff_3%inter_random_access_cycle==2)
												{
													RACH[x][list2][2]=backoff_3;
												}
												else
												{
													if(backoff_3%inter_random_access_cycle==1)
													{
														RACH[x][list2][2]=backoff_3+1;
													}
													else
													{
														RACH[x][list2][2]=backoff_3+((inter_random_access_cycle+1)-(backoff_3-1)%inter_random_access_cycle);
													}
												}
											}
										
											
										}
									} end of 舊的回RAR機制 依照順序 由上而下*/
									
									if(RACH[x][list2][5]==time)//時間點到了Msg3傳送時間代表切換到M3/M4階段
									{
										RACH[x][list2][14]=RACH[x][list2][14]+(time-1)-(RACH[x][list2][14]+RACH[x][list2][15]);
										//把(現在時間-1)扣掉前面分別已經花在( M3/M4階段+ 與Pream階段)的時間總和 再累加進 P_time
									}

									if((RACH[x][list2][5]==time) && (RACH[x][list2][4]<msg3_tran_bound))	//判斷時間是否來到 該"送"msg3的時候 msg3重傳次數是否超過上限5次
									{
										RACH[x][list2][6]=time+msg3_ACK_offset;	//設立Msg3 ACK的時間
										RACH[x][list2][9]=time;	//紀錄Msg3發送的時間點
										RACH[x][list2][4]++;	//msg3 tran round+1
										
									}

									if((time>=RACH[x][list2][5]) &&(RACH[x][list2][5]!=0))	//在M3有送出後才開始記錄Timer即時的值
									{
										RACH[x][list2][10]=time-RACH[x][list2][9];	//把M3送出後Timer 即時的值儲存起來
									}

									
									if(RACH[x][list2][10]<=mac_ContentionResolutionTimer)	
									{	//Timer在送出Msg3後啟動 要在expierd之前完成以下
										if(RACH[x][list2][6]==time)		//到了Msg3_ACK接收的時間點
										{
											if(msg3_percent<((int)((lcgrand(11)*10)+1)))	//msg3 有10%的機率會接收"失敗"
											{
												if(RACH[x][list2][4]<msg3_tran_bound)		//若沒超過Msg3可以重傳次數的上限
												{
													RACH[x][list2][5]=time+msg3_BF_offset;	//設立Msg3 重新傳送的時間點

												}
												else//超過Msg3可以重傳次數的上限 UE會一直等到Timer expired才會做Preamble retransmission
												{
													//需要注意Backoff至preamble retransmission時 pream可以重傳的次數是否已經用完 若已用完則
												}
											}
											else		
											{	//msg3傳送成功
												RACH[x][list2][11]=time+msg4_receive_offset;	//設立Msg4 會接受到的的時間
											}

										}


										
										if((RACH[x][list2][11]==time) && (RACH[x][list2][12]<msg4_tran_bound))	//判斷時間是否來到 該"收"msg4的時候 且未超過Msg4重傳次數上限
										{
											RACH[x][list2][13]=time+msg4_ACk_offset;	//設立Msg4_ACK接收的時間點
											RACH[x][list2][12]++;	//Msg4傳送次數+1
										}

										if(RACH[x][list2][13]==time)
										{
											RACH[x][list2][15]=RACH[x][list2][15]+time-(RACH[x][list2][14]+RACH[x][list2][15]);	
											//到了RA成功階段 不會再回到pream 傳送階段 所以從Msg3送出去後到 RA成功這一單趟的時間需要靠現在額外累加進 M34_time
											if(msg4_percent<((int)((lcgrand(12)*10)+1)))	//msg4 有10%的機率會接收失敗
											{
												if(RACH[x][list2][12]<msg4_tran_bound)	//是否有超過Msg4可以重新傳送的次數上限 
												{
													RACH[x][list2][11]=time+msg4_BF_offset;	//若未超過重新設立Msg4傳送時間點
												}
												else	//超過Msg4可以重新傳送的次數上限，所以要等到Timer expired才重新傳送Preamble
												{
													//需要注意Backoff至preamble retransmission時 pream可以重傳的次數是否已經用完 若已用完則

												}
											}
											else
											{
												RACH[x][list2][7]=1;	//若正確接收到msg4 設立stage=1
												//HT_success[HT_S_x]=time;	//把RA成功的時間點記錄下來
												//cout<<"HT_success["<<HT_S_x<<"]="<<HT_success[HT_S_x]<<endl;
												sum_RA_success_time=sum_RA_success_time+time;


												success_table_RAD[time]=success_table_RAD[time]+1;	//delay=33→ success_table_RAD[33]++
												int PT_time=0;
												PT_time=RACH[x][list2][1];	//把該RA 成功的deive 之pream tran round 抓出來
												success_table_PT[PT_time]=success_table_PT[PT_time]+1;	//pream round=8→success_table_PT[8]++

												
												HT_S_x++;

												RACH[x][list2][0]=0;	//20110429強置停掉RA
												RACH[x][list2][1]=0;	//20110429強置停掉RA
												RACH[x][list2][2]=0;	//20110429強置停掉RA
												RACH[x][list2][3]=0;	//20110429強置停掉RA



												RACH[x][list2][4]=0;	//成功後把Msg3 Msg4傳送接收的相關時間點reset
												RACH[x][list2][5]=0;
												RACH[x][list2][6]=0;
												RACH[x][list2][9]=0;
												RACH[x][list2][10]=0;
												RACH[x][list2][11]=0;
												RACH[x][list2][12]=0;
												RACH[x][list2][13]=0;
											}
										}			
									} 	//end of if(time-RACH[x][list2][9]<mac_ContentionResolutionTimer)
									else	
									{	//mac_ContentionResolutionTimer expired 回到preamble重新傳送階段
										
									
										RACH[x][list2][4]=0;	//把Msg3 Msg4傳送接收的相關時間點reset
										RACH[x][list2][5]=0;
										RACH[x][list2][6]=0;
										RACH[x][list2][9]=0;	//把Timer 停掉
										RACH[x][list2][10]=0;
										RACH[x][list2][11]=0;
										RACH[x][list2][12]=0;
										RACH[x][list2][13]=0;

										//需要注意Backoff至preamble retransmission時 pream可以重傳的次數是否已經用完 若已用完則，若已達上限 直接判定RA失敗 而不 Random BF
										if (RACH[x][list2][1]==upbound_preamble)
										{
											RACH[x][list2][8]=1;
										}
										else //未達上限所以Random BF
										{
											int backoff_4=0;
											backoff_4=time+(int)((lcgrand(13)*(1+Wbo))+1);
												
											if(backoff_4%inter_random_access_cycle==2)
											{
												RACH[x][list2][2]=backoff_4;
											}
											else
											{
												if(backoff_4%inter_random_access_cycle==1)
												{
													RACH[x][list2][2]=backoff_4+1;
												}
												else
												{
													RACH[x][list2][2]=backoff_4+((inter_random_access_cycle+1)-(backoff_4-1)%inter_random_access_cycle);
												}
											}
										}
									} 
									
									RACH[x][cell_diveces_number][0]=RACH[x][cell_diveces_number][0]+RACH[x][list2][7]+RACH[x][list2][8]; //每經過一個subframe 即時統計有多少device(成功+失敗)

									
								}

							//	Finish_Device_Number=Finish_Device_Number+RACH[x][cell_diveces_number][0];


								////
								if(RACH[x][cell_diveces_number][0]==BOX[x] && flag[x]==0)//印出所有DEVICES完成的時間
									{
										//cout<<"CELL["<<x<<"全部Devices完成在Time="<<time<<endl;
										flag[x]=1;
										int RACH_round=0;
										RACH_round=(time/inter_random_access_cycle);
										RACH_round_pream=RACH_round_pream+(RACH_round*preambles);//整個完成RA後 換算經過了多少次的RACH
					
									}
								

								//HT_success[time]=HT_success[time]+HT_S_x;	//把每一個時間點 當時有多少devices成功的數據 記錄進該時間點的陣列
		
							} //END//每個cell分別 run simulation 
							//HT_success[time]=HT_success[time]+HT_S_x;	//把每一個時間點 當時有多少devices成功的數據 記錄進該時間點的陣列
		
							//cout<<"Finish devices at Time"<<time<<"="<<Finish_Device_Number<<endl;

										
			/*				for(int xd=0; xd<cell; xd++) //debug show出所有的結果至Excel
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

		*/
		/*			
							if(Finish_Device_Number==M && flag[x]==0)//印出所有DEVICES完成的時間
							{
								//cout<<"全部Devices完成在Time="<<time<<endl;
								flag[x]=1;
								int RACH_round=0;
								RACH_round=(time/5);
								RACH_round_pream=RACH_round_pream+(RACH_round*preambles);//整個完成RA後 換算經過了多少次的RACH
					
							}
		*/
				

					/*		
							int sum_s=0; //統計完成RA的時間


							for(int h=0; h<BOX[0]; h++)
							{
								sum_s=sum_s+RACH[0][h][7]+RACH[0][h][8];
							//	cout<<"RACH["<<x<<"]["<<h<<"][7]="<<RACH[0][h][7]<<endl;
							//	cout<<"RACH["<<x<<"]["<<h<<"][8]="<<RACH[0][h][8]<<endl;
								

							}
							//cout<<"sum_s="<<sum_s<<endl;
							

							int end_time=0;
							int r_5=BOX[0];
							

							if((sum_s==M) && (stop==0))
							{
								cout<<"sum_s="<<sum_s<<endl;
								cout<<"r_5="<<r_5<<endl;
								cout<<"stop="<<stop<<endl;
								end_time=time;
								stop=1;
								cout<<"end_time="<<end_time<<endl;
							}

					  */
							
						}//END//共模擬多少時間
						

					}//END//每個cell devices數目的組合要跑的次數
					
					
					//cout<<"===================="<<endl;
						
				}//END//每個cell devices數目 要random幾次
			

				int total_RA_success_time=0;

			//	for(int sum_RA_success_time=0; sum_RA_success_time<HT_S_x; sum_RA_success_time++)
			//	{
			//		total_RA_success_time=total_RA_success_time+HT_success[sum_RA_success_time];

			//	}
			//	for(int print_delay_CDF=0; print_delay_CDF<100; print_delay_CDF++)
			//	{
			//		cout<<"HT_success["<<print_delay_CDF<<"]="<<HT_success[print_delay_CDF]<<endl;
			//	}

				

				average_delay=sum_RA_success_time/HT_S_x;
				//average_pream_collision_prob=sum_preamble_collision_time/(RACH_round_pream);
				//Pc統計方式 分母改成 分析的preambles*(Imax/ T_RA_REP)
				average_pream_collision_prob=sum_preamble_collision_time/((float)sim_device_random_round*sim_round*cell*preambles*(1+(upbound_preamble-1)*OverlapRegion));
				random_access_success=(float)HT_S_x/(M*sim_device_random_round*sim_round);//平均成功人的比例
				filePtrUs<<(float)HT_S_x/(sim_device_random_round*sim_round*cell*preambles*(1+(upbound_preamble-1)*6))<<","<<random_access_success<<","; //平均成功的人數/(Ni*Imax)

				

				cout<<"Ni="<<preambles<<" M="<<M<<endl;
				cout<<"average_delay="<<average_delay<<endl;
				cout<<"HT_S_x="<<HT_S_x<<endl;

				cout<<sum_preamble_collision_time<<"/"<<(RACH_round_pream)<<endl;

				cout<<"average_pream_collision_prob="<<average_pream_collision_prob<<endl;

				cout<<"random_access_success="<<random_access_success<<endl;
				filePtrX<<M<<","<<average_delay<<","<<average_pream_collision_prob<<","<<random_access_success<<endl;


				float CDF_RA=0;
				for(int finish_RA_time=1; finish_RA_time<=300; finish_RA_time++)//統計 RAD的CDF
				{
					CDF_RA=CDF_RA+(float)success_table_RAD[finish_RA_time]/(float)HT_S_x;
					filePtrY<<CDF_RA<<",";
				}
				filePtrY<<endl;

		
				float CDF_PTR=0;
				for(int pre_tran_tound=1; pre_tran_tound<=10; pre_tran_tound++)//統計 PTR的CDF
				{
					CDF_PTR=CDF_PTR+(float)success_table_PT[pre_tran_tound]/(float)HT_S_x;
					filePtrZ<<CDF_PTR<<",";
				}
				filePtrZ<<endl;

				float average_PTR=0;
				for(int pre_tran_tound_average=1; pre_tran_tound_average<=10; pre_tran_tound_average++)// 平均preamble傳送次數
				{
					average_PTR=average_PTR+(float)(pre_tran_tound_average*success_table_PT[pre_tran_tound_average])/(float)HT_S_x;
				}
				filePtrA<<average_PTR<<",";
	/*
				cout<<"average_PTR="<<average_PTR<<endl;

				for(int RACH_seq=0; RACH_seq<60; RACH_seq++)
				{
					filePtr_every_RACH_devices_num<<everyRACHdevicesnum[RACH_seq]/sim_round<<endl;
				}

				for(int RACH_seq_1=0; RACH_seq_1<60; RACH_seq_1++)
				{
					for(int RACH_seq_2=0; RACH_seq_2<10; RACH_seq_2++)
					{
						every_RACH_devices_num_pream_tran_round<<everyRACHdevicesnum_1[RACH_seq_1][RACH_seq_2]/sim_round<<",";
					}
					every_RACH_devices_num_pream_tran_round<<endl;
				}
			
	*/

			}//end of simecase
			filePtrUs<<endl;
		}

			filePtrX.close();
		//	filePtr.close();
			filePtrY.close();
			filePtrZ.close();
			filePtrA.close();
			filePtrUs.close();
			filePtr_every_RACH_devices_num.close();
			every_RACH_devices_num_pream_tran_round.close();
  
			return 0;
			
}
