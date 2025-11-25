#pragma once

#define BUFFER_SIZE 2048

#include <unistd.h>
#include <thread>
#include <functional>
// #include <termios.h>
#include <mutex>
#include "ErrorCode.h"

using namespace std;

class Uart
{
private:
	char *port;
	int baudrate;
	thread *uartThread;
	mutex mtx;

public:
	volatile int fd;
	int timeout;
	unsigned char rx_buf[BUFFER_SIZE];

	Uart(char *port, int baudrate, int timeout);
	virtual ~Uart();
	
	void init();

	int Open(int baudrate);
	int Close();
	int ChangeBaudrate(int baudrate);
	ssize_t Read(void *buf, size_t count);
	ssize_t Write(const void *buf, size_t count);

	virtual int OnMessage(unsigned char *data, int len);
};
