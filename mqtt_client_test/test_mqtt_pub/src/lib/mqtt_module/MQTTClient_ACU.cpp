#include <pthread.h>
#include <iostream> 
#include <string> 
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <nlohmann/json.hpp>
#include "MQTTClient.h"
#include "MQTTClient_ACU.h"
// #include "cloud_msg.pb.h"
#include "cloud_control_msg.pb.h"
#include "main.h"
#include <unistd.h>


using namespace std;
#define PROTOBUF_USE_DLLS
#define LOCAL_ADDRESS       "tcp://localhost:1883" //更改此处地址
#define PUBLIC_ADDRESS      "broker.hivemq.com:1883"
#define ADDRESS             "52.82.96.6:1883"

#define PUB_CLIENTID        "test_mqtt_pub" //更改此处客户端ID
#define TOPIC               "r/car/setRoute"  //更改发送的话题
#define PAYLOAD             "Hello Man, Can you see me ?!" //
#define QOS                 0
#define TIMEOUT             10000L
#define USERNAME            "test"
#define PASSWORD            "12345678"
#define DISCONNECT          "out"
using json = nlohmann::json;


unsigned len =4096;
int CONNECT = 1;
MQTTClient c_pubClient;    //声明一个pubMQTTClient
MQTTClient_connectOptions c_pubConn_opts = MQTTClient_connectOptions_initializer; //初始化MQTT Client选项
MQTTClient_message c_pubmsg = MQTTClient_message_initializer;    //#define MQTTClient_message_initializer { {'M', 'Q', 'T', 'M'}, 0, 0, NULL, 0, 0, 0, 0 }
MQTTClient_deliveryToken c_pubToken;    //声明消息token
int v_pub_rc;


void pubClentInit()
{
    MQTTClient_create(&c_pubClient, PUBLIC_ADDRESS, PUB_CLIENTID,MQTTCLIENT_PERSISTENCE_NONE, NULL);    //使用参数创建一个client，并将其赋值给之前声明的client
    c_pubConn_opts.keepAliveInterval = 20;
    c_pubConn_opts.cleansession = 1;
    // c_pubConn_opts.username = USERNAME;
    // c_pubConn_opts.password = PASSWORD;

    if ((v_pub_rc = MQTTClient_connect(c_pubClient, &c_pubConn_opts)) != MQTTCLIENT_SUCCESS)     //使用MQTTClient_connect将client连接到服务器，使用指定的连接选项。成功则返回MQTTCLIENT_SUCCESS
    {
        printf("*pub*Failed to connect, return code %d\n", v_pub_rc);
        exit(EXIT_FAILURE);
    }
}

void unpubClient()
{
    MQTTClient_disconnect(c_pubClient, 10000);
    MQTTClient_destroy(&c_pubClient);
}

void pubClient()
{
    // cloudProtoPackage::cloudControl msg;   // EMESSAGE
    json pub_msg;
    json route;
    route["name"]="route2";
    string buf;
    char keyboardinput[2];
    // pub_msg["command"]="start";
    pub_msg["route"]=route;
    // pub_msg["REMOTE_CONTROL"]=true;
    // msg.set_remotestart(false); 

    while(CONNECT)
    {
        // cout<<"please input 't'or'f': "<<endl;
        // cin>>keyboardinput;
        // if(strcmp(keyboardinput,"t")==0)//相等为0
        // {
        // //    pub_msg["BUTTON_STRAT"]=true;
        // //    msg.set_remotestart(true);
        //    cout<<"**********************************************"<<endl;
        // //    cout<<" msg.remotestart：ttttt "<< msg.remotestart()<<endl;
        //    cout<<"**********************************************"<<endl;
        // }
        // else if(strcmp(keyboardinput,"f")==0)//相等为0
        // {
        //    pub_msg["BUTTON_STRAT"]=false;
        // //    msg.set_remotestart(false); 
        // } 
        // else
        // {
        //     continue;
        // }
        
        //msg.set_remotestart(true);

        // msg.SerializeToString(&buf);
        
        // char* p = new char[buf.length() + 1];
        // memset(p, 0x00, buf.length() + 1);
        // strcpy(p, buf.c_str());
        // cout<<p<<endl;
        // cout<<"ok"<<endl;
        std::string pubstr= pub_msg.dump();
        std::cout<<"publish : "<<pubstr <<std::endl;
        const char *p =new char[pubstr.length()];
        p=pubstr.data();
        c_pubmsg.payload = const_cast<char *>(p);//tmp_buf;
        c_pubmsg.payloadlen = pubstr.length();//sizeof(tmp_buf);
        c_pubmsg.qos = QOS;
        c_pubmsg.retained = 0;
        MQTTClient_publishMessage(c_pubClient, TOPIC, &c_pubmsg, &c_pubToken);
        printf("\n*********pub*******\nWaiting for up to %d seconds for publication \n"
                "on topic %s for client with ClientID: %s\n",
                (int)(TIMEOUT/1000), TOPIC, PUB_CLIENTID);
        v_pub_rc = MQTTClient_waitForCompletion(c_pubClient, c_pubToken, TIMEOUT);

        // delete []p;
        usleep(300000L);
    }     

    

    unpubClient();
}

void m_mqtt_init()
{
    pubClentInit();
}

void m_mqtt_main()
{  
    GOOGLE_PROTOBUF_VERIFY_VERSION;
    pubClient();
    google::protobuf::ShutdownProtobufLibrary();

}

