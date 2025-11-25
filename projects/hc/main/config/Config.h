#ifndef __CONFIG_H__
#define __CONFIG_H__

#include <stdbool.h>
// #include "esp_err.h"
// #include "driver/uart.h"

#define MAX_SERIAL_NUM 2

#ifdef __cplusplus
extern "C"
{
#endif

	typedef struct
	{
		char device_id[64];
		char host[128];
		int port;
		int tls;
		char client_id[64];
		char username[64];
		char password[64];
		int keep_alive;
	} config_server_t;

	typedef struct
	{
		char protocol[32];
		char ip[32];
		char mask[32];
		char gw[32];
		char dns1[32];
		char dns2[32];
	} config_eth_t;

	typedef struct
	{
		bool ap_enable;
		char ap_ssid[32];
		char ap_password[32];
		int ap_channel;
		int ap_ssid_hidden;
		int ap_max_connection;

		bool sta_enable;
		char sta_ssid[32];
		char sta_password[32];

		char protocol[32];
		char ip[32];
		char mask[32];
		char gw[32];
		char dns1[32];
		char dns2[32];
	} config_wifi_t;

	/**
	 * @brief UART word length constants
	 */
	// typedef enum {
	// 	UART_DATA_5_BITS   = 0x0,    /*!< word length: 5bits*/
	// 	UART_DATA_6_BITS   = 0x1,    /*!< word length: 6bits*/
	// 	UART_DATA_7_BITS   = 0x2,    /*!< word length: 7bits*/
	// 	UART_DATA_8_BITS   = 0x3,    /*!< word length: 8bits*/
	// 	UART_DATA_BITS_MAX = 0x4,
	// } uart_word_length_t;

	/**
	 * @brief UART stop bits number
	 */
	// typedef enum {
	// 	UART_STOP_BITS_1   = 0x1,  /*!< stop bit: 1bit*/
	// 	UART_STOP_BITS_1_5 = 0x2,  /*!< stop bit: 1.5bits*/
	// 	UART_STOP_BITS_2   = 0x3,  /*!< stop bit: 2bits*/
	// 	UART_STOP_BITS_MAX = 0x4,
	// } uart_stop_bits_t;

	/**
	 * @brief UART parity constants
	 */
	// typedef enum
	// {
	// 	UART_PARITY_DISABLE = 0x0, /*!< Disable UART parity*/
	// 	UART_PARITY_EVEN = 0x2,		 /*!< Enable UART even parity*/
	// 	UART_PARITY_ODD = 0x3			 /*!< Enable UART odd parity*/
	// } uart_parity_t;

	/**
	 * @brief UART hardware flow control modes
	 */
	// 	typedef enum {
	//     UART_HW_FLOWCTRL_DISABLE = 0x0,   /*!< disable hardware flow control*/
	//     UART_HW_FLOWCTRL_RTS     = 0x1,   /*!< enable RX hardware flow control (rts)*/
	//     UART_HW_FLOWCTRL_CTS     = 0x2,   /*!< enable TX hardware flow control (cts)*/
	//     UART_HW_FLOWCTRL_CTS_RTS = 0x3,   /*!< enable hardware flow control*/
	//     UART_HW_FLOWCTRL_MAX     = 0x4,
	// } uart_hw_flowcontrol_t;

	typedef struct
	{
		int baudrate;
		int data_size;
		int parity;
		int stop_bit;
	} config_serial_t;

	typedef struct
	{
		int sensitive;
	} config_sensor_t;

	typedef struct
	{
		config_server_t server;
		config_eth_t eth;
		config_wifi_t wifi;
		config_serial_t serial[MAX_SERIAL_NUM];
		config_sensor_t sensor;
	} config_t;

	extern config_t config_data;

	int config_read();
	int config_write();
	void config_print();

#ifdef __cplusplus
}
#endif

#endif
