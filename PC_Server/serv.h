#include <iostream>

#define MAX_DATA_NUM 50
#define MAX_DATA_LENGTH 14
/* 数据包头类型 */
typedef struct
{
	uint8_t nDataType;											//数据包类型，标识对应的对象类型
	uint8_t nReserved;											//数据包中保留位
	uint8_t nDataSize[2];											//数据包中szData真实数据的长度

}NetDataHeader_t;


/*  数据包类型 */
typedef struct
{
	NetDataHeader_t  dataHeader;			//数据包头
	char	 szData[MAX_DATA_NUM][MAX_DATA_LENGTH];				//真实数据
}NetDataBase_t;


/*  退出数据类型 */
typedef struct
{
	NetDataHeader_t  dataHeader;			//数据包头
}NetDataExit_t;

