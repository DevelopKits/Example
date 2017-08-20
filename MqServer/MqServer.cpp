// MqServer.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include "AMQPcpp.h"

using namespace std;

int i = 0;

int onCancel(AMQPMessage * message) 
{
	cout << "cancel tag=" << message->getDeliveryTag() << endl;
	return 0;
}

int  onMessage(AMQPMessage * message)
{
	cout << "onMessage" << endl;
	uint32_t j = 0;
	char * data = message->getMessage(&j);
	if (data)
		cout << data << endl;

	i++;

	cout << "#" << i << " tag=" << message->getDeliveryTag() << " content-type:" << message->getHeader("Content-type");
	cout << " encoding:" << message->getHeader("Content-encoding") << " mode=" << message->getHeader("Delivery-mode") << endl;

	if (i > 10) 
	{
		AMQPQueue * q = message->getQueue();
		q->Cancel(message->getConsumerTag());
	}
	return 0;
};


int main() {

	int reconnects = 10;
	/*while (reconnects++)*/
	{
		try {
			//		AMQP amqp("123123:akalend@localhost/private");
			reconnects++;
			AMQP amqp("guest:guest@127.0.0.1");

			AMQPExchange * ex = amqp.createExchange("hello-exchange");
			ex->Declare("hello-exchange", "direct");

			AMQPQueue * qu2 = amqp.createQueue("hello-queue");
			qu2->Declare();
			qu2->Bind("hello-exchange", "hola");

			std::cout << "Connected." << std::endl;

			//qu2->setConsumerTag("hello-consumer");
			qu2->addEvent(AMQP_MESSAGE, onMessage);
			qu2->addEvent(AMQP_CANCEL, onCancel);

			qu2->Consume(AMQP_NOACK);//


		}
		catch (AMQPException e)
		{
			std::cout << e.getMessage() << std::endl;
		}
	}
	
	return 0;

}


