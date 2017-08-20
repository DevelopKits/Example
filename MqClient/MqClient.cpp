// MqClient.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include "AMQPcpp.h"
#include <windows.h>
using namespace std;

int main(int argc, char** argv) 
{



	try {
		//		AMQP amqp;
		//		AMQP amqp(AMQPDEBUG);

		AMQP amqp("guest:guest@127.0.0.1");	// all connect string
		//AMQP amqp;
		AMQPExchange * ex = amqp.createExchange("hello-exchange");
		ex->Declare("hello-exchange", "direct");

		AMQPQueue * qu2 = amqp.createQueue("hello-queue");
		qu2->Declare();
		qu2->Bind("hello-exchange", "hola");

		std::cout << "Connected." << std::endl;

		std::string routing_key("hola");
		string ss = "message 1 ";
		/* test very long message
		ss = ss+ss+ss+ss+ss+ss+ss;
		ss += ss+ss+ss+ss+ss+ss+ss;
		ss += ss+ss+ss+ss+ss+ss+ss;
		ss += ss+ss+ss+ss+ss+ss+ss;
		ss += ss+ss+ss+ss+ss+ss+ss;
		*/

		ex->setHeader("Delivery-mode", 2);
		ex->setHeader("Content-type", "text/text");
		ex->setHeader("Content-encoding", "UTF-8");

		ex->Publish(ss, routing_key); // publish very long message

		while (true)
		{
			ex->Publish("message 2 ", routing_key);
			ex->Publish("message 3 ", routing_key);
			Sleep(1000);
		}
		


		if (argc == 2) {
			AMQPQueue * qu = amqp.createQueue();
			qu->Cancel(amqp_cstring_bytes(argv[1]));
		}

	}
	catch (AMQPException e) {
		std::cout << e.getMessage() << std::endl;
	}

	return 0;

}

