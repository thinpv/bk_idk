#pragma once

#ifndef MODEL
#ifdef __OPENWRT__
#define MODEL "RD_HC_FULL"
#elif defined(__ANDROID__)
#define MODEL "RD_HC_ANDROID"
#elif defined(ESP_PLATFORM)
#define MODEL "RD_HC_MINI"
#else
#define MODEL "RD_HC_V2"
#endif
#endif

#define STR_(x) #x
#define STR(x) STR_(x)

#ifndef VERSION
#ifdef ESP_PLATFORM
#define VERSION 2.0.4
#else
#define VERSION 2.1.13
#endif
#endif

#if __ANDROID__
#define CONFIG_FILE_NAME "/etc/smh/config.json"
#define TMP_FOLDER "/data/rd/"
#define CERT_FILE_NAME "/data/rd/server.pem"
#define LOG_FILE_PATH "/data/rd/"
#define LOG_FILE_NAME "smh-"
#elif defined(__OPENWRT__)
#define CONFIG_FILE_NAME "/root/smh/config.json"
#define TMP_FOLDER "/tmp/"
#define CERT_FILE_NAME "/root/smh/server.pem"
#define LOG_FILE_PATH "/root/smh/"
#define LOG_FILE_NAME "smh-"
#else
#define CONFIG_FILE_NAME "config.json"
#define TMP_FOLDER ""
#define CERT_FILE_NAME "./server.pem"
#define LOG_FILE_PATH ""
#define LOG_FILE_NAME "smh-"
#endif

#ifndef DB_NAME
#ifdef __OPENWRT__
#define DB_NAME "/root/smh/smh.sqlite"
#elif defined(__ANDROID__)
#define DB_NAME "/data/rd/smh.sqlite"
#elif defined(ESP_PLATFORM)
#define DB_NAME "/storage/smh.sqlite"
#else
#define DB_NAME "./smh.sqlite"
#endif
#endif

#ifndef BLE_UART_PORT
#ifdef __OPENWRT__
#define BLE_UART_PORT "/dev/ttyS1"
// #define BLE_UART_PORT "/dev/ttyUSB0"
#elif defined(__ANDROID__)
#define BLE_UART_PORT "/dev/ttyS5"
#elif defined(ESP_PLATFORM)
#else
#define BLE_UART_PORT "/dev/ttyUSB0"
#endif
#endif

#ifdef CONFIG_ENABLE_ZIGBEE
#include "ZigbeeProtocol.h"
#ifdef __OPENWRT__
#define ZIGBEE_UART_PORT "/dev/ttyS0"
#elif defined(__ANDROID__)
#define BLE_UART_PORT "/dev/ttyS4"
#else
#define ZIGBEE_UART_PORT "/dev/ttyUSB1"
#endif
#endif

#define URL_PRO "https://rallismartv2.rangdong.com.vn"
#define URL_STAGING "https://rallismartv2-staging.rangdong.com.vn"
#define URL_DEV "https://iot-dev.truesight.asia"

#define MAX_RULE_IN_QUEUE_TO_CHECK 10