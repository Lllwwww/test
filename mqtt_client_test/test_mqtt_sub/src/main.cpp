#include "MQTTClient_ACU.h"
#define NUM_THREADS         2
char tmp_buf[50][11];//定义变量
int main(int argc, char* argv[])
{
    tmp_buf[0][0]='H';
    // m_mqtt_init();
    m_mqtt_main();
    return 1;
}

