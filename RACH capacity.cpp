#include <iostream>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <time.h>
#include <fstream>
#include "lcgrand.c"
using namespace std;
//��WBO 1.�ץ�Backoff���l�� 2.�ץ�Pc�έp�ɷ|�Ψ쪺�p��X��RA ��Imax����k
int sim_time=500;	//�t�μ����ɶ�

int sim_device_random_round=1;	//�C��cell����devices�ƥحnrandom�X��
int	sim_round=100;	//CELL��devices�ƥت��C�زզX �n�U�����X�^�X

double average_delay=0;	//HT_success[HT_S_x]�����֥[/HT_S_x
double average_pream_collision_prob=0;	//collision pream������/�`�@���h�֦�pream�ǰe���|
float RACH_round_pream=0; //��ӧ���RA�� �ɶ������h�֦�RACH���|*preamble�ƥ�

const int cell=1;
const int list=1001;	//Devices Number+1=M+1
const int parameter=16;

int RACH[cell][list][parameter]={0};	//�x�s������� RACH[x][y][z]
//int HT_success[10000000]={0};	//�x�sRA���\device�����\�ɶ�
float HT_S_x=0;
float sum_RA_success_time=0;	//�֭pRA���\device�����\�ɶ�
float everyRACHdevicesnum[300]={0}; //�x�s�C��RACH ���|���h��devices
float everyRACHdevicesnum_1[300][10]={0};// �x�s�C��RACH ���|���h��devices �ä����O�k����ĴX���ǰepreamble


int phy=1;	//phy�h�]���[���Ʀr�V�j preamble�Qdetect�쪺���|�V��//1=���ܧ�

int MM=3500;  //the amount of MTC devices (Device number)
const int N=cell;	//cell number


const int inter_random_access_cycle=10;	//�C�@��RACH���|���j�h�[
const int upbound_preamble=10;	//preamble���s�ǰe���ƤW��
const int msg3_tran_bound=5;	//Msg3 ���s�ǰe���ƤW��
const int msg4_tran_bound=5;	//Msg4 ���s�ǰe���ƤW��

const int msg3_ACK_offset=4;	//Msg3 �e�X��w���b��ɫ᦬��ACK
const int msg4_ACk_offset=4;	//Msg4 �e�X��w���b��ɫ᦬��ACK


const int msg3_BF_offset=4;		//Msg3 ����Backoff�h�[�᭫�e
const int msg4_BF_offset=1;		//Msg4 ����(M4_NACK�e�X���ɶ��I��h�[�A��ڤ��|�e)Backoff�h�[�᭫�e M4�� +msg4_ACk_offset   M4_NACK(���|�e�X)   +msg4_BF_offset   M4�����e

const int msg4_receive_offset=1;	//Msg3 ACK���T������ �h�[����Msg4


//const int ACK_offset=4;	//4 // msg3�e�X�H�� �ndelay�X��subframe�~�|ť��msg4 (+3 msg3's ack)

const int RaResponseTime=7;	//preamble�e�X�h�� �ĴX��subframe�|ť��RAR 1+7=8
const int mac_ContentionResolutionTimer=48;	//48//	msg3�e�X�h��n�b48�K��subframe������msg4
const int msg3_percent=9;		//msg3 �Q���T���쪺���v1~9(��)
const int msg4_percent=9;		//msg4 �Q���T���쪺���v1~9(��)
const int RAR_amount=15; //�@���i�H�^��RAR�ƥ�

int checkRARamount[2][54]={0}; //�x�s���ǤH not collided & detected[���L�QRandom��W�^RAR�� �аO][���@��User]

const int Wbo=20;
float Imaxcomponent=((Wbo+7)/inter_random_access_cycle)+1;
int OverlapRegion=Imaxcomponent;

int BOX[cell]={0};	//�x�s�C��CELL��devices�ƥ�


/*Random���ͨC��CELL��Devices�ƥ�*/
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
				double sum_preamble_collision_time=0;	//�֭ppream�ǰe�o�͸I��������
				float RACH_round_pream=0;
				double random_access_success=0;	//�έp������
				int success_table_RAD[301]={0};	//�k�s//�x�s���\RA��DEVICE ��RA delay time  //delay=33�� success_table_RAD[33]++
				int success_table_PT[11]={0};	//�k�s//�x�s�h�֦��\RA��DEVICE��pream tran round //pream round=8��success_table_PT[8]++

				int st_1=0;


				for(int sim_device_random_round_count=1; sim_device_random_round_count<=sim_device_random_round; sim_device_random_round_count++)
				//if(sim_device_random_round>0)//�C��cell devices�ƥ� �nrandom�X��
				{
						int BOX[cell]={0};		//�C��ROUND�ݭn�k�s���Ѽ�
						int num=0;				//�C��ROUND�ݭn�k�s���Ѽ�
						
						for(int g=0; g<M; g++)
						{
							num=((int)(10000*lcgrand(6)))%N;
							BOX[num]=BOX[num]+1;
						}


					
					for(int sim_round_count=1; sim_round_count<=sim_round; sim_round_count++ )	//�C��cell devices�ƥت��զX�n�]������
					{
						int RACH_cycle=0; //�ĴX��RACH���|
						
						int total_Devices=0;	//�C��ROUND�ݭn�k�s���Ѽ�

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
							for(int list1=0; list1<BOX[xx] ;list1++)	//��Ҧ�Devices preamble_send_time=2 ��l�� �ҥѲ�2�ɶ��I�ǰe
							{
								RACH[xx][list1][2]=2;
							}
						}
						int flag[cell]={0};	//�Ψӿ�O�u�ݦL�@������Devices�������ɶ� //flag=0�٥��L�X// flag =1�L�X
						for(int time=1; time<=sim_time; time++)	//�@�����h�֮ɶ�
						{
							int Finish_Device_Number=0;	//�έp�C�Ӯɶ��I "�C��CELL"(���\+����)���p�ƾ��k�s

							for(int x=0; x<cell; x++)	//�C��cell���O run simulation
							{
							
								int cell_diveces_number=0;	//��X�o��CELL devices���ƥ�
								cell_diveces_number=BOX[x];

								RACH[x][cell_diveces_number][0]=0;//��ΰO(���\+����)���p�ƾ��k�s

								//////////////////////////////////////////////////////
	/*							for(int list_48=0; list_48<BOX[x]; list_48++)
								{
									int r_1=0;
									if(RACH[x][list_48][9]!=0)	//�bmsg3�٥��Ĥ@���ǰe���e MAC contention timer ���|�Q�g�J�Ĥ@���ǰe���ɶ� �ҥH�]�N���Υh�P�_ �O�_�W�L48
									{
										r_1=(time-RACH[x][list_48][9]);
									}
									

										if(RACH[x][list_48][5]!=0 && (r_1>mac_ContentionResolutionTimer) && (RACH[x][list_48][7]!=1) && (RACH[x][list_48][8]!=1))//�W�L48��subframe�W��
										{ //�P�_�O�_���bmsg3�ǰe���q �Y�Dmsg3���e���q�h���ݭn�Ұ� MAC contention timer �W�L48��subframe���P�_
											
											if(RACH[x][list_48][1]==upbound_preamble)//�YMAC TIMEOUT pream tran round�S�F��W�� �h���ݭn�Npream tran randomback 
											{
												RACH[x][list_48][5]=0;	//�Ymsg3~msg4�W�L48��subframe�h���~�򵥫�
												RACH[x][list_48][6]=0;	//�Ymsg3~msg4�W�L48��subframe�h���~�򵥫�
												RACH[x][list_48][8]=1;
												RACH[x][list_48][9]=0;	//�Ymsg3~msg4�W�L48��subframe�h���~�򵥫� ��msg3�Ĥ@���ǰe�ɶ��k�s
												RACH[x][list_48][10]=0;	//�Ymsg3~msg4�W�L48��subframe�h���~�򵥫�

											}

											else
											{
											
											
												RACH[x][list_48][4]=0;	//�Ymsg3~msg4�W�L48��subframe�h���~�򵥫�
												RACH[x][list_48][5]=0;	//�Ymsg3~msg4�W�L48��subframe�h���~�򵥫�
												RACH[x][list_48][6]=0;	//�Ymsg3~msg4�W�L48��subframe�h���~�򵥫�
												RACH[x][list_48][9]=0;	//�Ymsg3~msg4�W�L48��subframe�h���~�򵥫� ��msg3�Ĥ@���ǰe�ɶ��k�s
												RACH[x][list_48][10]=0;	//�Ymsg3~msg4�W�L48��subframe�h���~�򵥫�



												int backoff=0;
												backoff=time+(((int)(1000*lcgrand(3)))%21);
												RACH[x][list_48][4]=0;	//msg3 tran round�k�s
															
												//�bmsg3���q���Ѥ]�O20ms random backoff
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
								if(time%inter_random_access_cycle==2)	// 2st & 7st subframe ��RACH���|
								{
									int pre=0;/////////////////////
									int buffer_pre[10000][2]={0};	//��Xpreamble �X�Ӥ��O�_�I��
									int BOX_pre[60]={0};		//�Ȧspreamble�� �ӽs����BOX
									for(int scan_2=0; scan_2<BOX[x]; scan_2++)
									{
										//�ثe�Ypreamble�I���ҳy����Backoff preamble retransmission �O��BF���� ��F�i�H�ǰepream�����|�� �~�h�P�_�O�_�w�g��Fpream�i�H�ǰe���ƪ��W��
										//���L�]�� �ثe�έpRANDOM ACCESS DELAY�u���w�令�\��device �Y����o�@�B �]���|�C�J�έp �ҥH���v�T �̫�h�@�qBF �~�P�_���Ѫ��ɶ�
										
										if(RACH[x][scan_2][2]==time) // �qM3/M4���q ���^Pream���q��
										{
											RACH[x][scan_2][15]=RACH[x][scan_2][15]+(time-1)-(RACH[x][scan_2][14]+RACH[x][scan_2][15]); 
											// ��(�{�b�ɶ�-1)�����e�����O�w�g��b( M3/M4���q+ �PPream���q)���ɶ��`�M �A�֥[�i M34_time
										}
										

										if((RACH[x][scan_2][2]==time) && (RACH[x][scan_2][1]==upbound_preamble)  && (RACH[x][scan_2][7]==0) && (RACH[x][scan_2][8]==0) ) //preamble_send_time=time �B�W�L���ǤW��=10 �ҥHupbound_preamble=10��ڴN�O�ĤQ�@�^�X
										{
											RACH[x][scan_2][8]=1;
										}
										
									
										if((RACH[x][scan_2][2]==time) && (RACH[x][scan_2][1]<upbound_preamble)  && (RACH[x][scan_2][7]==0) && (RACH[x][scan_2][8]==0) ) //preamble_send_time=time �B�S���W�L���ǤW��=10
										{
											buffer_pre[pre][0]=scan_2;	//���X�C�o�������s��preamble��	�إ߲M��					
											RACH[x][scan_2][0]=(int)((lcgrand(3)*preambles)+1);;	//�H������preamble


											buffer_pre[pre][1]=RACH[x][scan_2][0];	//��preamble�x�s

											int pre_serial=0;		//�Ȧspreamble
											pre_serial=RACH[x][scan_2][0];
											BOX_pre[pre_serial]++;	//�Ȧspreamble�� �ӽs����BOX  //���]preamble=36  BOX_pre[36]++


											pre++;
											
											//��C��Backoff Window�� �ݩ�ĴX��preamble�ǰe���H���O�x�s
											int d_pream_tran_round=0;
											d_pream_tran_round=RACH[x][scan_2][1];
											everyRACHdevicesnum_1[RACH_cycle][d_pream_tran_round]++;
									
											RACH[x][scan_2][3]=time+7; //�ǰepreamble��A�btime+7���ɶ�ť��RAR
											RACH[x][scan_2][1]++;	//preamble�ǰe����+1   !!!!!!!  �]�����]���@���~+1  !!!!!!!

											
										}
									}


									//��C�@��RACH���|���h��devices�ѻPRA���� �έp�U��
									
									everyRACHdevicesnum[RACH_cycle]+=pre; //�C�����@�� �N��������� ��RACH���| �ҰѻP��devices�֥[�i
									RACH_cycle++;

									for(int scan_buffer_pre=0; scan_buffer_pre<preambles; scan_buffer_pre++)	//���yBOX_pre���C��devices��preamble
									{
										if(BOX_pre[scan_buffer_pre]>=2)//�Y�ӽs����BOX�W�L���DEVICEs�h�N���PREAMBLE�I���F
										{
											sum_preamble_collision_time++;//�֭p�X��pream ���ǰe�o�͸I��
										}

									}


									if(pre>1)
									{
										//if(time=15)
										for(int Token=0; Token<pre; Token++) //��C�ӽ��y�h�X�ӼȦs �۴��s �ΥH�P�_preamble�O�_����
										{
											int buffer_EZ[1]={0};		//�Ȧs��
											buffer_EZ[0]=buffer_pre[Token][1];	//�Ȧs
											buffer_pre[Token][1]=100; //��h�X�Ӫ��Ȧspreamble ��^�@�Ӱ�����100
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
											

											buffer_pre[Token][1]=buffer_EZ[0];	//��h�X�Ӫ��Ȧspreambel��^�쥻��m
											buffer_EZ[0]=0;

										}
									}
									
								
								}

								int RAR_amount_count=1;	//�ثe�^�F�X��RAR
								/*�s���^RAR����*/
								int checkRAR_index=0;	//checkRARamount�}�C�� �q�s����m�}�l�x�s
								
								for(int clean_checkRARamount_x=0; clean_checkRARamount_x<2; clean_checkRARamount_x++)
								{
									for(int clean_checkRARamount_y=0; clean_checkRARamount_y<54; clean_checkRARamount_y++)
									{
										checkRARamount[clean_checkRARamount_x][clean_checkRARamount_y]=0;
									}
								}

								for(int scanRAR=0; scanRAR<BOX[x]; scanRAR++)//�Nno collided & detected���H��icheckRARamount[���L�QRandom��W�^RAR�� �аO][���@��User] ��array
								{
									if((RACH[x][scanRAR][3]==time) && (RACH[x][scanRAR][2]<RACH[x][scanRAR][3]) && (RACH[x][scanRAR][4]<=msg3_tran_bound))	
									//�P�_RAR listen window�O�_ > preamble send time
									//�ΥH�ϧOno preamble collision��devices
									{
										int phy_i=0;
										phy_i=RACH[x][scanRAR][1];	//phy_i means ��phy_i���ǰepreamble
										
										if( phy*(1-exp(-phy_i))>lcgrand(5) ) //�P�_���q�LPHY
										{
											checkRARamount[1][checkRAR_index]=scanRAR;
											checkRAR_index++;//�x�sarray��m���U�@��
										}
										else //���q�LPHY �i��BACK OFF
										{
											if(RACH[x][scanRAR][1]==upbound_preamble) // �Y���q�LPHY�]���A�B�Ypream tran round�P�ɤw�g�F�W�� �h���ݭnrandom backoff
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
												//preamble���q�L1-exp(-i)���]��
												//�qRAR listen window backoff
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
										}//���q�LPHY �i��BACK OFF
									}
								}
								filePtr_CheckRARIndex<<checkRAR_index<<",";
								int sumRAR=0;  //�x�s �ثe�w�gRandom �X�h�֦�|�Q�^RAR��user
								if(checkRAR_index>0)
								{					
									while( (sumRAR<checkRAR_index)&&(sumRAR<RAR_amount) )//�qcheckRARamount[�аO][]array���H������n�^RAR���H�ç�"�аO"�]��1
									{
										sumRAR=0;
										int RAR_reply_user=0; //�x�sRANDOM�X�ӽ֭n�Q�^RAR��USER
										RAR_reply_user=(int)(lcgrand(7)*checkRAR_index); //random 0 ~ (checkRAR_index-1)
										checkRARamount[0][RAR_reply_user]=1; //RANDOM�襤��user �е��]��1
										for(int sumR_i=0; sumR_i<checkRAR_index; sumR_i++) //�֥[ �ثe�w�gRandom �X�h�֦� �n�Q�^RAR��user
										{
											sumRAR=sumRAR+checkRARamount[0][sumR_i];
										}
										
									}

									for(int identify_RAR=0; identify_RAR<checkRAR_index; identify_RAR++) //�N Random �^RAR�����G �i��B�z							
									{
										filePtr_CheckRARIndex<<checkRARamount[0][identify_RAR]<<","<<checkRARamount[1][identify_RAR]<<endl;
										if(checkRARamount[0][identify_RAR]==1)//checkRARamount[�аO][]==1 ;�]��Msg3�ǰe�ɶ�
										{
											int whois1=0;
											whois1=checkRARamount[1][identify_RAR]; //����user �bcheckRARamount[�аO][]==1
											RACH[x][whois1][5]=time+1; //����RAR��U�@��subframe�N�e�Xmsg3
											//RACH[x][whois1][4]++;	//msg3 tran round+1
										}
										else//checkRARamount[�аO][]==0 ;�i��Backoff
										{
											int whois0=0;
											whois0=checkRARamount[1][identify_RAR]; //����user �bcheckRARamount[�аO][]==0

											if(RACH[x][whois0][1]==upbound_preamble) // �Y���q�LPHY�]���A�B�Ypream tran round�P�ɤw�g�F�W�� �h���ݭnrandom backoff
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
												//preamble���q�L1-exp(-i)���]��
												//�qRAR listen window backoff
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

										}//checkRARamount[�аO][]==0 ;�i��Backoff

									}//�N Random �^RAR�����G �i��B�z
								} //end of �s���^RAR����

							
								
								for(int list2=0; list2<BOX[x]; list2++)//�P�_�O�_����RAR
								{
									
									/*	�ª��^RAR���� �̷Ӷ��� �ѤW�ӤU
									if(RACH[x][list2][5]!=0 && RACH[x][list2][9]>=1 && RACH[x][list2][7]==0 && RACH[x][list2][8]==0)//device�����\�����Ѥ��e pream�ǰe�ɶ��b msg3�Ĥ@���ǰe���� �ΥH�ɩwreal time�O�_�ݭn����p��
									{
										RACH[x][list2][10]=time-RACH[x][list2][9];	//�O���Y�ɪ�mac contention timer
									}


									if((RACH[x][list2][3]==time) && (RACH[x][list2][2]<RACH[x][list2][3]) && (RACH[x][list2][4]<=msg3_tran_bound))	
										//�P�_RAR listen window�O�_ > preamble send time
										//�ΥH�ϧOno preamble collision��devices
									{
										int r_2=0;
										r_2=RACH[x][list2][1];	//i means ��i���ǰepreamble
										if(( phy*(1-exp(-r_2))>lcgrand(4) )&&(RAR_amount_count<=RAR_amount))	//�L�P�H�I����preamble�A�åB�q�L1-exp(-i)���]��
										{
											RACH[x][list2][5]=time+1; //����RAR��U�@��subframe�N�e�Xmsg3
											//RACH[x][list2][4]++;	//msg3 tran round+1
											RAR_amount_count++;
										}
										else
										{
											if(RACH[x][list2][1]==upbound_preamble) // �Y���q�LPHY�]���A�B�Ypream tran round�P�ɤw�g�F�W�� �h���ݭnrandom backoff
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
												//preamble���q�L1-exp(-i)���]��
												//�qRAR listen window backoff

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
									} end of �ª��^RAR���� �̷Ӷ��� �ѤW�ӤU*/
									
									if(RACH[x][list2][5]==time)//�ɶ��I��FMsg3�ǰe�ɶ��N�������M3/M4���q
									{
										RACH[x][list2][14]=RACH[x][list2][14]+(time-1)-(RACH[x][list2][14]+RACH[x][list2][15]);
										//��(�{�b�ɶ�-1)�����e�����O�w�g��b( M3/M4���q+ �PPream���q)���ɶ��`�M �A�֥[�i P_time
									}

									if((RACH[x][list2][5]==time) && (RACH[x][list2][4]<msg3_tran_bound))	//�P�_�ɶ��O�_�Ө� ��"�e"msg3���ɭ� msg3���Ǧ��ƬO�_�W�L�W��5��
									{
										RACH[x][list2][6]=time+msg3_ACK_offset;	//�]��Msg3 ACK���ɶ�
										RACH[x][list2][9]=time;	//����Msg3�o�e���ɶ��I
										RACH[x][list2][4]++;	//msg3 tran round+1
										
									}

									if((time>=RACH[x][list2][5]) &&(RACH[x][list2][5]!=0))	//�bM3���e�X��~�}�l�O��Timer�Y�ɪ���
									{
										RACH[x][list2][10]=time-RACH[x][list2][9];	//��M3�e�X��Timer �Y�ɪ����x�s�_��
									}

									
									if(RACH[x][list2][10]<=mac_ContentionResolutionTimer)	
									{	//Timer�b�e�XMsg3��Ұ� �n�bexpierd���e�����H�U
										if(RACH[x][list2][6]==time)		//��FMsg3_ACK�������ɶ��I
										{
											if(msg3_percent<((int)((lcgrand(11)*10)+1)))	//msg3 ��10%�����v�|����"����"
											{
												if(RACH[x][list2][4]<msg3_tran_bound)		//�Y�S�W�LMsg3�i�H���Ǧ��ƪ��W��
												{
													RACH[x][list2][5]=time+msg3_BF_offset;	//�]��Msg3 ���s�ǰe���ɶ��I

												}
												else//�W�LMsg3�i�H���Ǧ��ƪ��W�� UE�|�@������Timer expired�~�|��Preamble retransmission
												{
													//�ݭn�`�NBackoff��preamble retransmission�� pream�i�H���Ǫ����ƬO�_�w�g�Χ� �Y�w�Χ��h
												}
											}
											else		
											{	//msg3�ǰe���\
												RACH[x][list2][11]=time+msg4_receive_offset;	//�]��Msg4 �|�����쪺���ɶ�
											}

										}


										
										if((RACH[x][list2][11]==time) && (RACH[x][list2][12]<msg4_tran_bound))	//�P�_�ɶ��O�_�Ө� ��"��"msg4���ɭ� �B���W�LMsg4���Ǧ��ƤW��
										{
											RACH[x][list2][13]=time+msg4_ACk_offset;	//�]��Msg4_ACK�������ɶ��I
											RACH[x][list2][12]++;	//Msg4�ǰe����+1
										}

										if(RACH[x][list2][13]==time)
										{
											RACH[x][list2][15]=RACH[x][list2][15]+time-(RACH[x][list2][14]+RACH[x][list2][15]);	
											//��FRA���\���q ���|�A�^��pream �ǰe���q �ҥH�qMsg3�e�X�h��� RA���\�o�@��몺�ɶ��ݭn�a�{�b�B�~�֥[�i M34_time
											if(msg4_percent<((int)((lcgrand(12)*10)+1)))	//msg4 ��10%�����v�|��������
											{
												if(RACH[x][list2][12]<msg4_tran_bound)	//�O�_���W�LMsg4�i�H���s�ǰe�����ƤW�� 
												{
													RACH[x][list2][11]=time+msg4_BF_offset;	//�Y���W�L���s�]��Msg4�ǰe�ɶ��I
												}
												else	//�W�LMsg4�i�H���s�ǰe�����ƤW���A�ҥH�n����Timer expired�~���s�ǰePreamble
												{
													//�ݭn�`�NBackoff��preamble retransmission�� pream�i�H���Ǫ����ƬO�_�w�g�Χ� �Y�w�Χ��h

												}
											}
											else
											{
												RACH[x][list2][7]=1;	//�Y���T������msg4 �]��stage=1
												//HT_success[HT_S_x]=time;	//��RA���\���ɶ��I�O���U��
												//cout<<"HT_success["<<HT_S_x<<"]="<<HT_success[HT_S_x]<<endl;
												sum_RA_success_time=sum_RA_success_time+time;


												success_table_RAD[time]=success_table_RAD[time]+1;	//delay=33�� success_table_RAD[33]++
												int PT_time=0;
												PT_time=RACH[x][list2][1];	//���RA ���\��deive ��pream tran round ��X��
												success_table_PT[PT_time]=success_table_PT[PT_time]+1;	//pream round=8��success_table_PT[8]++

												
												HT_S_x++;

												RACH[x][list2][0]=0;	//20110429�j�m����RA
												RACH[x][list2][1]=0;	//20110429�j�m����RA
												RACH[x][list2][2]=0;	//20110429�j�m����RA
												RACH[x][list2][3]=0;	//20110429�j�m����RA



												RACH[x][list2][4]=0;	//���\���Msg3 Msg4�ǰe�����������ɶ��Ireset
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
									{	//mac_ContentionResolutionTimer expired �^��preamble���s�ǰe���q
										
									
										RACH[x][list2][4]=0;	//��Msg3 Msg4�ǰe�����������ɶ��Ireset
										RACH[x][list2][5]=0;
										RACH[x][list2][6]=0;
										RACH[x][list2][9]=0;	//��Timer ����
										RACH[x][list2][10]=0;
										RACH[x][list2][11]=0;
										RACH[x][list2][12]=0;
										RACH[x][list2][13]=0;

										//�ݭn�`�NBackoff��preamble retransmission�� pream�i�H���Ǫ����ƬO�_�w�g�Χ� �Y�w�Χ��h�A�Y�w�F�W�� �����P�wRA���� �Ӥ� Random BF
										if (RACH[x][list2][1]==upbound_preamble)
										{
											RACH[x][list2][8]=1;
										}
										else //���F�W���ҥHRandom BF
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
									
									RACH[x][cell_diveces_number][0]=RACH[x][cell_diveces_number][0]+RACH[x][list2][7]+RACH[x][list2][8]; //�C�g�L�@��subframe �Y�ɲέp���h��device(���\+����)

									
								}

							//	Finish_Device_Number=Finish_Device_Number+RACH[x][cell_diveces_number][0];


								////
								if(RACH[x][cell_diveces_number][0]==BOX[x] && flag[x]==0)//�L�X�Ҧ�DEVICES�������ɶ�
									{
										//cout<<"CELL["<<x<<"����Devices�����bTime="<<time<<endl;
										flag[x]=1;
										int RACH_round=0;
										RACH_round=(time/inter_random_access_cycle);
										RACH_round_pream=RACH_round_pream+(RACH_round*preambles);//��ӧ���RA�� ����g�L�F�h�֦���RACH
					
									}
								

								//HT_success[time]=HT_success[time]+HT_S_x;	//��C�@�Ӯɶ��I ��ɦ��h��devices���\���ƾ� �O���i�Ӯɶ��I���}�C
		
							} //END//�C��cell���O run simulation 
							//HT_success[time]=HT_success[time]+HT_S_x;	//��C�@�Ӯɶ��I ��ɦ��h��devices���\���ƾ� �O���i�Ӯɶ��I���}�C
		
							//cout<<"Finish devices at Time"<<time<<"="<<Finish_Device_Number<<endl;

										
			/*				for(int xd=0; xd<cell; xd++) //debug show�X�Ҧ������G��Excel
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
							if(Finish_Device_Number==M && flag[x]==0)//�L�X�Ҧ�DEVICES�������ɶ�
							{
								//cout<<"����Devices�����bTime="<<time<<endl;
								flag[x]=1;
								int RACH_round=0;
								RACH_round=(time/5);
								RACH_round_pream=RACH_round_pream+(RACH_round*preambles);//��ӧ���RA�� ����g�L�F�h�֦���RACH
					
							}
		*/
				

					/*		
							int sum_s=0; //�έp����RA���ɶ�


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
							
						}//END//�@�����h�֮ɶ�
						

					}//END//�C��cell devices�ƥت��զX�n�]������
					
					
					//cout<<"===================="<<endl;
						
				}//END//�C��cell devices�ƥ� �nrandom�X��
			

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
				//Pc�έp�覡 �����令 ���R��preambles*(Imax/ T_RA_REP)
				average_pream_collision_prob=sum_preamble_collision_time/((float)sim_device_random_round*sim_round*cell*preambles*(1+(upbound_preamble-1)*OverlapRegion));
				random_access_success=(float)HT_S_x/(M*sim_device_random_round*sim_round);//�������\�H�����
				filePtrUs<<(float)HT_S_x/(sim_device_random_round*sim_round*cell*preambles*(1+(upbound_preamble-1)*6))<<","<<random_access_success<<","; //�������\���H��/(Ni*Imax)

				

				cout<<"Ni="<<preambles<<" M="<<M<<endl;
				cout<<"average_delay="<<average_delay<<endl;
				cout<<"HT_S_x="<<HT_S_x<<endl;

				cout<<sum_preamble_collision_time<<"/"<<(RACH_round_pream)<<endl;

				cout<<"average_pream_collision_prob="<<average_pream_collision_prob<<endl;

				cout<<"random_access_success="<<random_access_success<<endl;
				filePtrX<<M<<","<<average_delay<<","<<average_pream_collision_prob<<","<<random_access_success<<endl;


				float CDF_RA=0;
				for(int finish_RA_time=1; finish_RA_time<=300; finish_RA_time++)//�έp RAD��CDF
				{
					CDF_RA=CDF_RA+(float)success_table_RAD[finish_RA_time]/(float)HT_S_x;
					filePtrY<<CDF_RA<<",";
				}
				filePtrY<<endl;

		
				float CDF_PTR=0;
				for(int pre_tran_tound=1; pre_tran_tound<=10; pre_tran_tound++)//�έp PTR��CDF
				{
					CDF_PTR=CDF_PTR+(float)success_table_PT[pre_tran_tound]/(float)HT_S_x;
					filePtrZ<<CDF_PTR<<",";
				}
				filePtrZ<<endl;

				float average_PTR=0;
				for(int pre_tran_tound_average=1; pre_tran_tound_average<=10; pre_tran_tound_average++)// ����preamble�ǰe����
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
