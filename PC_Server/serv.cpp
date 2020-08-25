#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<stdio.h>
#include<unistd.h>
#include<stdio.h>
#include<errno.h>
#include<string.h>
#include<iostream>
#include<fstream>
#include<ctime>
#include"flex_can.h"
#include "serv.h"
#include"serverConfig.h"


#define MAX_NUM 10
#define PORT 8080

typedef struct
		{
	 flexcan_msgbuff_t* can_msg;
	 uint8_t channel;
		} tcpip_can_msg;
tcpip_can_msg can_msg_buff[128];
int msg_head;
int msg_rear;

typedef struct
{
	uint8_t channel;
	uint8_t msgid_h;
	uint8_t msgid_l;
	uint8_t datalength;
	uint8_t time_h;
	uint8_t time_l;
	uint8_t data[8];
}transferdata_t;

void msg_parser(transferdata_t *mb_message,int head, uint8_t* data) {
	data[0] = (mb_message[head]).channel;
	data[1] = (mb_message[head]).msgid_h;
	data[2] = (mb_message[head]).msgid_l;
	data[3] = (mb_message[head]).datalength;
	data[4] = (mb_message[head]).time_h;
	data[5] = (mb_message[head]).time_l;
	data[6] = (mb_message[head]).data[0];
	data[7] = (mb_message[head]).data[1] ;
	data[8] = (mb_message[head]).data[2];
	data[9] = (mb_message[head]).data[3];
	data[10] = (mb_message[head]).data[4];
	data[11] = (mb_message[head]).data[5];
	data[12] = (mb_message[head]).data[6];
  data[13] = (mb_message[head]).data[7];

}

uint8_t tmp_buf[14]={0};
char recv_buf[14]={0};
uint8_t testdata[60][14]={0};
char state_array[5]={};

int main()
{
int listenfd;
struct sockaddr_in serv_addr;
int remotefd;
struct sockaddr_in remote_addr;
char buff[3]={'0'};
std::fstream writefile;
writefile.open("/home/xywu/Desktop/time.txt",std::ios_base::in|std::ios::app);
socklen_t len;
int n,i;
int array_count = 0;
int val = 0;
int can_channel;
struct timeval timeout;
timeout.tv_sec = 1;
timeout.tv_usec = 0;
flexcan_msgbuff_t msg_buff = { 0,0,{0},0 };
flexcan_msgbuff_t *msg_buff_temp = &msg_buff;
flexcan_msgbuff_t msg_buff1 = { 0,0,{0},0 };
flexcan_msgbuff_t *msg_buff_temp1 = &msg_buff1;

NetDataHeader_t send_buff_header = {0,0,0};
NetDataBase_t send_buff = {{0,0,0},{0}};
NetDataBase_t *pSend_buff =nullptr;
uint8_t type=0;
uint16_t udatasize =0;
uint8_t reser=0;
uint8_t channel;
	uint8_t msgid_h;
	uint8_t msgid_l;
	uint8_t datalength;
	uint8_t time_h;
	uint8_t time_l;
	uint8_t data[8];
// transferdata_t test_time_buff={2,0xF,0xFF,8,0,0,{1,2,3,4,5,6,7,0},};
transferdata_t test_time_buff={16,0,0,0,0x0,0,{0b11111111,0b11111111,0,4,5,6,7,0},};
transferdata_t test_time_6b2;
test_time_6b2.channel=1;
test_time_6b2.msgid_h=0xF;
test_time_6b2.msgid_l=0xF1;
test_time_6b2.datalength=8;
test_time_6b2.time_h=0;
test_time_6b2.time_l=0;
for (i = 0; i < 8; i++)
{
  test_time_6b2.data[i] = 0;
}
transferdata_t transferdata[128] = {0};
//transferdata[0]=test_time_buff;
uint8_t temp_time1=0;
uint8_t temp_time2=0;

listenfd=socket(AF_INET,SOCK_STREAM,0);
if(listenfd==-1)
{perror("create socket error!");return 0;}
else {printf("Success to create socket %d\n",listenfd);}
bzero(&serv_addr,sizeof(serv_addr));
serv_addr.sin_family=AF_INET;
serv_addr.sin_port=htons(PORT);

//serv_addr.sin_addr.s_addr=inet_addr("172.26.60.205");
serv_addr.sin_addr.s_addr=htonl(INADDR_ANY); 
//serv_addr.sin_addr.s_addr=inet_addr("192.168.8.100");
bzero(&(serv_addr.sin_zero),8);
msg_buff_temp1->cs = 0;

for (i = 0; i < 8; i++)
{
  msg_buff_temp1->data[i] = i;
}
msg_buff_temp1->dataLen = 8;
msg_buff_temp1->msgId = 0x123;


if(bind(listenfd,(struct sockaddr*)&serv_addr,sizeof(serv_addr))!=0)
{
printf("bind address failed !%d\n",errno);
close(listenfd);
return 0;
}
else{printf("Success to bind !\n"); }

if(listen(listenfd,MAX_NUM)!=0)
{
printf("listen socket error!\n");
close(listenfd);
return 0;
}
else {printf("Success to listen !\n");}


len= sizeof(remote_addr);
msg_head=0;
msg_rear=0;
struct tm *vehicle_time_set;
time_t  global_times = time(NULL);
vehicle_time_set = localtime(&global_times);
struct timespec time_now={0, 0} ;
clock_gettime(CLOCK_REALTIME, &time_now);
//

uint16_t mpc_time_s,mpc_time_ms;
uint16_t print_time;
uint16_t last_ms;
uint16_t size ;
uint8_t msg_id_count=0;
int count=0;
last_ms=0;
while(1)
{
  remotefd = accept(listenfd, (struct sockaddr *)&remote_addr,&len);
   
  if(remotefd != -1)//接收到了信号
    {
        send(remotefd,"VER10",5,0);
        count =0;
      while(1)
    {

      count++;
      msg_id_count++;
      test_time_6b2.msgid_h=0xF;
      test_time_6b2.msgid_l=msg_id_count;
      
      clock_gettime(CLOCK_REALTIME, &time_now);
      // global_times = time(NULL);
      vehicle_time_set = localtime(&time_now.tv_sec);
      mpc_time_s=vehicle_time_set->tm_sec;
      mpc_time_ms=time_now.tv_nsec/1000000;
     
        test_time_buff.time_h=(uint8_t)((mpc_time_s<<2)|((mpc_time_ms>>8)&0x03));
        test_time_buff.time_l=(uint8_t)((mpc_time_ms)&0xFF);
        if(count%3==0)
        { 
          // std::cout<<"send 6b2";
          test_time_6b2.time_h=(uint8_t)((mpc_time_s<<2)|((mpc_time_ms>>8)&0x03));
          test_time_6b2.time_l=(uint8_t)((mpc_time_ms)&0xFF);
          test_time_6b2.data[5]=(uint8_t)(((vehicle_time_set->tm_hour-0x07)&0x0F)<<4);
          test_time_6b2.data[6]=(uint8_t)(((vehicle_time_set->tm_hour-0x07)&0x10)>>4)|((vehicle_time_set->tm_min&0x03F)<<1)|
                     ((vehicle_time_set->tm_sec&0x01)<<7);

          test_time_6b2.data[7]=(uint8_t)((vehicle_time_set->tm_sec&0x3f)>>1);
          transferdata[msg_rear]=test_time_6b2;
          // std::cout<<"6b2time:"<<
        }
        else
        {
          transferdata[msg_rear]=test_time_buff;
        }
        // if(temp_time1<255)
        // {
        //   temp_time1++;
        //   transferdata[msg_rear].time_l=temp_time1;
        //   transferdata[msg_rear].time_h=temp_time2;
        // }
        // else 
        // {
        //   if(temp_time2<=3)
        //   {
        //     temp_time2++;
        //     transferdata[msg_rear].time_h=temp_time2;
        //   }
        //   else 
        //   {
            
        //     transferdata[msg_rear].time_h=temp_time2;
        //     temp_time2=0;
        //   }
        //   temp_time1=0;
        //   transferdata[msg_rear].time_l=temp_time1;
          
          // }
        // std::cout<<"time:"<<(int)mpc_time_s<<"."<<(int)mpc_time_ms<<std::endl;
        print_time=(test_time_buff.time_h>> 2);
        // std::cout<<"time:"<<(int)print_time<<" s";
        // writefile<<"time:"<<(int)print_time<<" s";
         print_time=(((test_time_buff.time_h&0x03) << 8) +test_time_buff.time_l);
         if(last_ms>print_time)
         {
           std::cerr<<"error frame"<<std::endl;
         }
      // std::cout<<(int)print_time<<" ms"<<std::endl;
      // writefile<<(int)print_time<<" ms"<<std::endl;
      if(msg_rear==127)
      {
        msg_rear=0;
      }
      else
        msg_rear++;
      if(array_count < 50)
      {
        if (msg_head==msg_rear)
        {
            usleep(20000);
        }
        else
          {
            msg_parser(transferdata, msg_head,tmp_buf);
          if(msg_head==127)
          {
            msg_head=0;
          }
          else
            msg_head++;
          for (i = 0; i < 14; i++)
          {
              testdata[array_count][i] = tmp_buf[i];
          }
          array_count++;
            }
      }
      else
      {
        if (count <100000){
          size =0 ;
          send_buff.dataHeader.nDataType = 0x1;
          send_buff.dataHeader.nDataSize[0] = (uint8_t)((array_count * MAX_DATA_LENGTH * sizeof(char))&(0x00FF));
          send_buff.dataHeader.nDataSize[1] = (uint8_t)(((array_count * MAX_DATA_LENGTH * sizeof(char))>>8)&(0x00FF));
          send_buff.dataHeader.nReserved = 0x0;

          memset(send_buff.szData,0,sizeof(send_buff.szData));
          memmove(send_buff.szData,testdata,array_count * sizeof(tmp_buf));
          size=sizeof(send_buff.dataHeader) + array_count * MAX_DATA_LENGTH * sizeof(char);
          // pSend_buff-=size;
          // send_buff.dataHeader.nDataType = 0x1;
          // send_buff.dataHeader.nDataSize = array_count * sizeof(tmp_buf);
          // send_buff.dataHeader.nReserved = 0x0;

          // memset(pSend_buff->szData,0,sizeof(send_buff.szData));
          // memmove(pSend_buff->szData,testdata,array_count * sizeof(tmp_buf));

        }
        else {
          size =0 ;
          send_buff.dataHeader.nDataType = 0xFF;
          send_buff.dataHeader.nDataSize[0] =0;
          send_buff.dataHeader.nDataSize[1] = 0;
          send_buff.dataHeader.nReserved = 0x0;
          size=sizeof(send_buff.dataHeader);

        }
        std::cout << "count : "<<count<<std::endl;

        int send_status = send(remotefd,(char * )&send_buff,size,0);
        if(send_status < 0) //发送状态错误 退出连接
        {
          break;
          std::cout<<"failed to send"<<std::endl;
        }
        std::cout << "send: "<<size<<" Bytes"<<std::endl;
        // 记录上一次数据发送的时间
        // mul_ix_time_send = xTaskGetTickCount();

        array_count=0;
        memset(testdata,0,sizeof(testdata));
        usleep(20000);
         // send(remotefd,"begin",5,0);
        //   if((n=recv(remotefd,recv_buf,10,0))> 0)
        // {
        //   // int status;
        //   //printf("weather send data : \n");
        //   //scanf("%d",&status);
        //  // if(status ==0)
        //   //{send(remotefd,testdata,sizeof(testdata),0);}
        //   send(remotefd,testdata,sizeof(testdata),0);
        //   if(strncmp(recv_buf,"quit",4)==0)
        //   break;
        //   memset(recv_buf,0,14);
        //   array_count=0;
        //   memset(testdata,0,sizeof(testdata));

      }
    }
    // }
    }
      else
        {
            close(remotefd);
            close(listenfd);
            remotefd = -1;
            return 0 ;
        }

}
return 1;
}
