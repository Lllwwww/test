#include <thread>
#include <iostream> 
#include <string> 
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <nlohmann/json.hpp>
// #include "MQTTClient.h"
#include "MQTTClient_ACU.h"
#include "mqtt/client.h"
// #include "mqtt/async_client.h"
#include "mqtt/topic.h"
#include <unistd.h>
//#include "queue_can.h"
#include <time.h>

using namespace std;
using json = nlohmann::json;

#define PROTOBUF_USE_DLLS
#define LOCAL_ADDRESS       "tcp://localhost:1883" //更改此处地址
#define PUBLIC_ADDRESS      "broker.hivemq.com:1883"
#define ADDRESS             "52.82.96.6:1883"

#define PUB_CLIENTID        "aaabbbccc_pub_local" //更改此处客户端ID
#define SUB_CLIENTID        "aaabbbccc_sub_local" //更改此处客户端ID
#define TOPIC_PUB           "err1234"  //更改发送的话题
#define TOPIC_SUB           "r/car/status"  //更改发送的话题
#define PAYLOAD             "Hello Man, Can you see me ?!" //
#define QOS_default         0
#define TIMEOUT             10000L
#define USERNAME            "test_user"
#define PASSWORD            "12345678"
#define DISCONNECT          "out"
#define NUM_THREADS         2
//extern char tmp_buf[50][11];
unsigned len =4096;
int CONNECT = 1;

const std::string SERVER_ADDRESS(PUBLIC_ADDRESS);
const std::string CLIENT_ID("aaabbbccc_sub_local");
const std::string TOPIC("r/car/status");

const int	QOS = 1;
const int	N_RETRY_ATTEMPTS = 5;

/////////////////////////////////////////////////////////////////////////////

// Callbacks for the success or failures of requested actions.
// This could be used to initiate further action, but here we just log the
// results to the console.

class action_listener : public virtual mqtt::iaction_listener
{
	std::string name_;

	void on_failure(const mqtt::token& tok) override {
		std::cout << name_ << " failure";
		if (tok.get_message_id() != 0)
			std::cout << " for token: [" << tok.get_message_id() << "]" << std::endl;
		std::cout << std::endl;
	}

	void on_success(const mqtt::token& tok) override {
		std::cout << name_ << " success";
		if (tok.get_message_id() != 0)
			std::cout << " for token: [" << tok.get_message_id() << "]" << std::endl;
		auto top = tok.get_topics();
		if (top && !top->empty())
			std::cout << "\ttoken topic: '" << (*top)[0] << "', ..." << std::endl;
		std::cout << std::endl;
	}

public:
	action_listener(const std::string& name) : name_(name) {}
};

/////////////////////////////////////////////////////////////////////////////

/**
 * Local callback & listener class for use with the client connection.
 * This is primarily intended to receive messages, but it will also monitor
 * the connection to the broker. If the connection is lost, it will attempt
 * to restore the connection and re-subscribe to the topic.
 */
class callback : public virtual mqtt::callback,
					public virtual mqtt::iaction_listener

{
	// Counter for the number of connection retries
	int nretry_;
	// The MQTT client
	mqtt::async_client& cli_;
	// Options to use if we need to reconnect
	mqtt::connect_options& connOpts_;
	// An action listener to display the result of actions.
	action_listener subListener_;

	// This deomonstrates manually reconnecting to the broker by calling
	// connect() again. This is a possibility for an application that keeps
	// a copy of it's original connect_options, or if the app wants to
	// reconnect with different options.
	// Another way this can be done manually, if using the same options, is
	// to just call the async_client::reconnect() method.
	void reconnect() {
		std::this_thread::sleep_for(std::chrono::milliseconds(2500));
		try {
			cli_.connect(connOpts_, nullptr, *this);
		}
		catch (const mqtt::exception& exc) {
			std::cerr << "Error: " << exc.what() << std::endl;
			exit(1);
		}
	}

	// Re-connection failure
	void on_failure(const mqtt::token& tok) override {
		std::cout << "Connection attempt failed" << std::endl;
		if (++nretry_ > N_RETRY_ATTEMPTS)
			exit(1);
		reconnect();
	}

	// (Re)connection success
	// Either this or connected() can be used for callbacks.
	void on_success(const mqtt::token& tok) override {}

	// (Re)connection success
	void connected(const std::string& cause) override {
		std::cout << "\nConnection success" << std::endl;
		std::cout << "\nSubscribing to topic '" << TOPIC << "'\n"
			<< "\tfor client " << CLIENT_ID
			<< " using QoS" << QOS << "\n"
			<< "\nPress Q<Enter> to quit\n" << std::endl;

		cli_.subscribe(TOPIC, QOS, nullptr, subListener_);
	}

	// Callback for when the connection is lost.
	// This will initiate the attempt to manually reconnect.
	void connection_lost(const std::string& cause) override {
		std::cout << "\nConnection lost" << std::endl;
		if (!cause.empty())
			std::cout << "\tcause: " << cause << std::endl;

		std::cout << "Reconnecting..." << std::endl;
		nretry_ = 0;
		reconnect();
	}

	// Callback for when a message arrives.
	void message_arrived(mqtt::const_message_ptr msg) override {
		std::cout << "Message arrived" << std::endl;
		std::cout << "\ttopic: '" << msg->get_topic() << "'" << std::endl;
		std::cout << "\tpayload: '" << msg->to_string() << "'\n" << std::endl;
        std::string recv_msg = msg->to_string();
        auto recv_data = json::parse(recv_msg);

        std::cout<<"recv:  mileage : "<<recv_data["mileage"]<<std::endl;
        std::cout<<"recv:  action  : "<<recv_data["action"]<<std::endl;
	}

	void delivery_complete(mqtt::delivery_token_ptr token) override {}

public:
	callback(mqtt::async_client& cli, mqtt::connect_options& connOpts)
				: nretry_(0), cli_(cli), connOpts_(connOpts), subListener_("Subscription") {}
};

// void m_mqtt_init()
// {
//     pubClentInit();
// }


void subClient()
{
    
	mqtt::connect_options connOpts;
	connOpts.set_keep_alive_interval(20);
	connOpts.set_clean_session(true);

	mqtt::async_client client(SERVER_ADDRESS, CLIENT_ID);

	callback cb(client, connOpts);
	client.set_callback(cb);

	// Start the connection.
	// When completed, the callback will subscribe to topic.

	try {
		std::cout << "Connecting to the MQTT server..." << std::flush;
		client.connect(connOpts, nullptr, cb);
	}
	catch (const mqtt::exception&) {
		std::cerr << "\nERROR: Unable to connect to MQTT server: '"
			<< SERVER_ADDRESS << "'" << std::endl;
		return ;
	}

	// Just block till user tells us to quit.

	while (std::tolower(std::cin.get()) != 'q')
		;

	// Disconnect

	try {
		std::cout << "\nDisconnecting from the MQTT server..." << std::flush;
		client.disconnect()->wait();
		std::cout << "OK" << std::endl;
	}
	catch (const mqtt::exception& exc) {
		std::cerr << exc.what() << std::endl;
		return ;
	}

 	return ;

}
void sub_disconnect()
{
// // Disconnect

// 	try {
// 		std::cout << "Disconnecting from the chat server..." << std::flush;
// 		cli.disconnect()->wait();
// 		std::cout << "OK" << std::endl;
// 	}
// 	catch (const mqtt::exception& exc) {
// 		std::cerr << exc.what() << std::endl;
// 		return 1;
// 	}
}

string getTime()
{
    time_t timep;
    time (&timep);
    char tmp[64];
    strftime(tmp, sizeof(tmp), "%Y-%m-%d %H:%M:%S",localtime(&timep) );
    return tmp;
 }




void m_mqtt_main( )
{  
    // GOOGLE_PROTOBUF_VERIFY_VERSION;
    // printf("*MQTT*: Hello,It's mqtt, thread #%ld!\n");
    // // pthread_create(&threads[0], NULL, pubClient, (void *)0);
    // pthread_create(&threads[1], NULL, subClient, (void *)1);
    // // pthread_join(threads[0],NULL);
    // pthread_join(threads[1],NULL);    
    std::thread mqtt_thread_post(subClient);
    mqtt_thread_post.join();
    // google::protobuf::ShutdownProtobufLibrary();

}

