#include <iostream>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <string.h>


#define MAX_NUM 10
#define PORT 8080
#define MAX_DATA_SIZE		4086
#define MAX_NETPACK_SIZE	10000
/* 数据包头类型 */
struct NetDataHeader_t
{
	int nDataType;											//数据包类型，标识对应的对象类型
	int nDataSize;											//数据包中szData真实数据的长度
};
 
/*  数据包类型 */
struct NetDataBase_t
{
	NetDataHeader_t  dataHeader;			//数据包头
	char	 szData[MAX_DATA_SIZE];				//真实数据
};
/**
    处理整理好的对象。
*/
bool HandleNetPack(NetDataHeader_t* pDataHeader);
 
 
bool TcpDataSplit(const char* szRecNetData, int nRecSize)
{
	/**
	    对于szLastSaveData, nRemainSize,为了简单，本例子只
		作为静态变量使用，因此只限于一个socket的数据接收，
		假如要同时处理多个socket数据，请放在对应容器里保存
	*/
	static char szLastSaveData[MAX_NETPACK_SIZE];
	static int nRemainSize = 0;
	static bool bFirst = true;
 
	if (bFirst)
	{
		memset(szLastSaveData, 0, sizeof(szLastSaveData));
		bFirst = false;
	}
 
	/* 本次接收到的数据拼接到上次数据 */
	memcpy( (char*)(szLastSaveData+nRemainSize), szRecNetData, nRecSize );
	nRemainSize = nRecSize + nRemainSize;
 
	/* 强制转换成NetDataPack指针 */
	NetDataHeader_t* pDataHead = (NetDataHeader_t*)szLastSaveData;
 
	/**
	   核心算法 
	*/
	while ( nRemainSize >sizeof(NetDataHeader_t) &&
				nRemainSize >= pDataHead->nDataSize +sizeof(NetDataHeader_t) )
	{
			HandleNetPack(pDataHead);
			int  nRecObjectSize = sizeof(NetDataHeader_t) + pDataHead->nDataSize;		//本次收到对象的大小
			nRemainSize -= nRecObjectSize ;				
			pDataHead = (NetDataHeader_t*)( (char*)pDataHead + nRecObjectSize );		//移动下一个对象头
	}
	
	/* 余下数据未能组成一个对象，先保存起来 */
	if (szLastSaveData != (char*)pDataHead)
	{
		memmove(szLastSaveData, (char*)pDataHead, nRemainSize);
		memset( (char*)( szLastSaveData+nRemainSize), 0, sizeof(szLastSaveData)-nRemainSize );
	}
	
	return true;
}
/**
    处理整理好的对象。
*/
bool HandleNetPack(NetDataHeader_t* pDataHeader)
{
	//处理数据包
	if  (pDataHeader->nDataType == 0)
	{
		NetDataBase_t* pPeople = (NetDataBase_t*)pDataHeader;
        std::cout<<"RECV : "<<pDataHeader->nDataSize<<" byte "<<std::endl;
        for(int i =0 ; i< (pDataHeader->nDataSize-1); i++)
        {
            std::cout<<pPeople->szData[i]<<" ";
        }
        std::cout<<std::endl;
	}
}
int main ()
{
    NetDataBase_t  people;
    int client_fd = socket(AF_INET,SOCK_STREAM,0);
    struct sockaddr_in clie_addr;
    bzero(&clie_addr,sizeof(clie_addr));
    clie_addr.sin_family = AF_INET;
    clie_addr.sin_port = htons (PORT);
    clie_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    bzero(&(clie_addr.sin_zero),8);
    if(connect(client_fd,(struct sockaddr *)&clie_addr,sizeof(clie_addr))!=0)
    {
        std::cout<< "connect failed "<<std::endl;
        close(client_fd);
        return 0;
    }
    char buf[MAX_DATA_SIZE];
    int count=0;
    while(count<4084)
    {
        ssize_t recv_len = recv(client_fd,buf,MAX_DATA_SIZE,0);
        TcpDataSplit(buf,  recv_len);
        memset(buf,0,sizeof(buf));
        // std::cout<<"recv : "<<buf<<std::endl;
        count++;

    }
    close(client_fd);
    return 1;
}