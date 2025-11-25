#include "Uart.h"
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include "Log.h"
#include <thread>
// #include <sys/ioctl.h>
// #include <sys/socket.h>
#include <stdio.h>

#define DEBUG_ENABLE 0

static void HandleInMessage(Uart *uart);

Uart::Uart(char *port, int baudrate, int timeout) : port(port), baudrate(baudrate), timeout(timeout)
{
}

Uart::~Uart()
{
	Close();
	if (uartThread)
	{
		// uartThread->stop();
		delete uartThread;
		uartThread = NULL;
	}
}

void Uart::init()
{
	if (Open(baudrate) < 0)
	{
		LOGE("Open uart error");
		_exit(1);
	}
}

int Uart::Open(int baudrate)
{
	LOGI("Open port %s baudrate: %d", port, baudrate);
	// fd = open(port, O_RDWR);
	// // Create new termios struc, we call it 'tty' for convention
	// struct termios tty;
	// // Read in existing settings, and handle any error
	// if (tcgetattr(fd, &tty) != 0)
	// {
	// 	LOGE("Error %i from tcgetattr: %s", errno, strerror(errno));
	// 	return CODE_ERROR;
	// }
	// tty.c_cflag &= ~PARENB;				 // Clear parity bit, disabling parity (most common)
	// tty.c_cflag &= ~CSTOPB;				 // Clear stop field, only one stop bit used in communication (most common)
	// tty.c_cflag &= ~CSIZE;				 // Clear all bits that set the data size
	// tty.c_cflag |= CS8;						 // 8 bits per byte (most common)
	// tty.c_cflag &= ~CRTSCTS;			 // Disable RTS/CTS hardware flow control (most common)
	// tty.c_cflag |= CREAD | CLOCAL; // Turn on READ & ignore ctrl lines (CLOCAL = 1)

	// tty.c_lflag &= ~ICANON;
	// tty.c_lflag &= ~ECHO;																												 // Disable echo
	// tty.c_lflag &= ~ECHOE;																											 // Disable erasure
	// tty.c_lflag &= ~ECHONL;																											 // Disable new-line echo
	// tty.c_lflag &= ~ISIG;																												 // Disable interpretation of INTR, QUIT and SUSP
	// tty.c_iflag &= ~(IXON | IXOFF | IXANY);																			 // Turn off s/w flow ctrl
	// tty.c_iflag &= ~(IGNBRK | BRKINT | PARMRK | ISTRIP | INLCR | IGNCR | ICRNL); // Disable any special handling of received bytes

	// tty.c_oflag &= ~OPOST; // Prevent special interpretation of output bytes (e.g. newline chars)
	// tty.c_oflag &= ~ONLCR; // Prevent conversion of newline to carriage return/line feed
	// // tty.c_oflag &= ~OXTABS; // Prevent conversion of tabs to spaces (NOT PRESENT ON LINUX)
	// // tty.c_oflag &= ~ONOEOT; // Prevent removal of C-d chars (0x004) in output (NOT PRESENT ON LINUX)

	// tty.c_cc[VTIME] = 0; // Wait for up to 1s (10 deciseconds), returning as soon as any data is received.
	// tty.c_cc[VMIN] = 0;

	// // Set in/out baud rate to be 115200
	// cfsetispeed(&tty, baudrate);
	// cfsetospeed(&tty, baudrate);
	// //	printf("Baudrate: %d\n",arrayBaudRate[baudrate]);

	// // Save tty settings, also checking for error
	// if (tcsetattr(fd, TCSANOW, &tty) != 0)
	// {
	// 	LOGE("Error %i from tcsetattr: %s", errno, strerror(errno));
	// 	return CODE_ERROR;
	// }
	// uartThread = new thread(HandleInMessage, this);
	// uartThread->detach();
	// return fd;
	return -1;
}

int Uart::Close()
{
	// int rs = close(fd);
	// fd = 0;
	// if (uartThread)
	// {
	// 	// TODO: Stop uartThread
	// 	// uartThread->stop();
	// 	delete uartThread;
	// 	uartThread = NULL;
	// }
	// return rs;
	return -1;
}

int Uart::ChangeBaudrate(int baudrate)
{
	// struct termios tty;
	// int rc1, rc2;

	// if (fd <= 0)
	// 	return CODE_ERROR;
	// if (tcgetattr(fd, &tty) < 0)
	// {
	// 	printf("Error from tcgetattr: %s\n", strerror(errno));
	// 	return CODE_ERROR;
	// }
	// rc1 = cfsetospeed(&tty, baudrate);
	// rc2 = cfsetispeed(&tty, baudrate);
	// if ((rc1 | rc2) != 0)
	// {
	// 	printf("Error from cfsetxspeed: %s\n", strerror(errno));
	// 	return CODE_ERROR;
	// }
	// if (tcsetattr(fd, TCSANOW, &tty) != 0)
	// {
	// 	printf("Error from tcsetattr: %s\n", strerror(errno));
	// 	return CODE_ERROR;
	// }
	// tcflush(fd, TCIOFLUSH); /* discard buffers */

	return CODE_OK;
}

ssize_t Uart::Read(void *buf, size_t count)
{
	return read(fd, buf, count);
}

ssize_t Uart::Write(const void *buf, size_t count)
{
	// LOGD("Write buf: %s", buf);
#if DEBUG_ENABLE
	printf("tx: ");
	for (size_t i = 0; i < count; i++)
		printf("%02X ", ((uint8_t *)buf)[i]);
	printf("\r\n");
#endif
	mtx.lock();
	ssize_t s = write(fd, buf, count);
	mtx.unlock();
	return s;
}

static void HandleInMessage(Uart *uart)
{
// 	int len_one_read;
// 	int len = 0;
// 	while (1)
// 	{
// 		ioctl(uart->fd, FIONREAD, &len_one_read);
// 		while (len_one_read > 0 && len < BUFFER_SIZE - 128)
// 		{
// 			usleep(uart->timeout);
// 			len_one_read = uart->Read(uart->rx_buf + len, BUFFER_SIZE - len);
// 			if (len_one_read > 0)
// 				len += len_one_read;
// 		}
// 		if (len > 0)
// 		{
// #if DEBUG_ENABLE
// 			printf("rx: ");
// 			for (int i = 0; i < len; i++)
// 				printf("%02X ", (uart->rx_buf)[i]);
// 			printf("\r\n");
// #endif
// 			uart->OnMessage(uart->rx_buf, len);
// 			len = 0;
// 			memset(uart->rx_buf, 0, BUFFER_SIZE);
// 		}
// 		else
// 			usleep(5000);
// 	}
}

int Uart::OnMessage(unsigned char *data, int len)
{
	LOGE("OnMessage len: %d, data: %s", len, data);
	return CODE_ERROR;
}
