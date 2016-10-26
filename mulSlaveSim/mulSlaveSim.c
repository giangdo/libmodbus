/*
 * Author: Giang Do
 *
 * This program is used to simulate multiple modbus slaves ID by using one serial port only.
 * It can be controled by ctrlSlave program by open UDP port 61234
 * It simulate modbus slave ID from 1 to 254
 *
 * Usage:
 *    + To bind this multiple modbus slaves into serial device /dev/ttyUSB0 at baudrate 1200
 *       ./mulSlaveSim -s /dev/ttyUSB0 -b 1200
 *
 *    + To enable modbus debug:
 *       ./mulSlaveSim -s /dev/ttyUSB0 -b 1200 -d 1
 */

#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <ctype.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <pthread.h>
#include <modbus.h>
#include "mulSlaveSim.h"

static int gBaudRate = 1200;
static const char* gSerialDevice = "/dev/ttyUSB0";
static uint16_t gPort = 61234;
static uint8_t gDebug;


static modbus_mapping_t* ModbusMapArr[MAX_NUM_MODBUS_SLAVE];
static pthread_mutex_t MtxModbusMapArr = PTHREAD_MUTEX_INITIALIZER;
static modbus_t *pModbusCtx = NULL;

static void* handle_modbus_map_data(void * pVoid)
{
	if ((pVoid != NULL) && (strlen((char*)pVoid) < 1000))
	{
			printf("thread name: %s\n", (char*)pVoid);
	}
	else
	{
		fprintf(stderr, "pass wrong thread name\n");
		exit(EXIT_FAILURE);
	}

	int sockFd;
	struct sockaddr_in serverAddr;
	struct sockaddr_in clientAddr;
	char buf[BUFSIZE];

	sockFd = socket(AF_INET, SOCK_DGRAM, 0);
	if (sockFd < 0)
	{
		fprintf(stderr, "err: socket()\n");
		exit(EXIT_FAILURE);
	}
	bzero((char*) &serverAddr, sizeof(serverAddr));
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);
	serverAddr.sin_port = htons((unsigned short)gPort);

	if (bind(sockFd, (struct sockaddr *) &serverAddr, sizeof(serverAddr)) < 0)
	{
		fprintf(stderr, "err: bind()\n");
		exit(EXIT_FAILURE);
	}

	int clientLen = sizeof(clientAddr);
	while (1)
	{
		bzero(buf, sizeof(buf));
		int n = recvfrom(sockFd, buf, BUFSIZE, 0,
		                 (struct sockaddr *) &clientAddr, (socklen_t*)&clientLen);
		if (n < 0)
		{
			fprintf(stderr, "err: recvfrom()\n");
			continue;
		}

		char *clientAddrStr = inet_ntoa(clientAddr.sin_addr);
		printf("Server received datagram from %s, port %d\n", clientAddrStr, (int)ntohs(clientAddr.sin_port));
		printf("Server received %d/%d bytes: %s\n", (int)strlen(buf), n, buf);

		char * pch;
		pch = strtok (buf," ");
		unsigned char index = 0;
		modbus_mapping_t* pModbusMap = NULL;
		uint16_t setRegBuf[INPUT_REGISTERS_NB];
		uint16_t *p16 = setRegBuf;
		bzero(setRegBuf, sizeof(setRegBuf));
		int isChange = 0;
		int isAll = 0;
		int slaveId = 0;
		while (pch != NULL)
		{
			if (index == 0)
			{
				if (strcmp(pch, "a") == 0)
				{
					isAll = 1;
				}
				else
				{
					slaveId = atoi(pch);
					if ((slaveId < 0) || (slaveId > 250))
					{
						fprintf(stderr, "err: wrong slave id\n");
						break;
					}
					else
					{
						slaveId = atoi(pch);
						pModbusMap = ModbusMapArr[slaveId];
						for (uint32_t ii = 0; ii < INPUT_REGISTERS_NB; ++ii)
						{
							setRegBuf[ii] = pModbusMap->tab_input_registers[ii];
						}
					}
				}
			}
			else if (index == 1)
			{
				if (strcmp(pch, "r") == 0)
				{
					isChange = 0;
				}
				else if (strcmp(pch, "s") == 0)
				{
					isChange = 1;
				}
				else
				{
					fprintf(stderr, "wrong command from control\n");
					continue;
				}
			}
			else
			{
				*p16++  = HexStringToUInt(pch);
			}
			index++;
			pch = strtok (NULL, " ");
		}

		char resBufStr[1000];
		bzero(resBufStr, sizeof(resBufStr));
		if (isAll == 1)
		{
			if (isChange == 1)
			{
				uint16_t mm;
				for (mm = 0; mm < MAX_NUM_MODBUS_SLAVE; ++mm)
				{
					if (ModbusMapArr[mm] != NULL)
					{
						fprintf(stderr, "set modbus map of slave %d\n", mm);
						for (uint8_t nn = 0; nn < INPUT_REGISTERS_NB; ++nn)
						{
							pthread_mutex_lock(&MtxModbusMapArr);
							ModbusMapArr[mm]->tab_input_registers[nn] = setRegBuf[nn] ;
							pthread_mutex_unlock(&MtxModbusMapArr);
						}
					}
					else
					{
						fprintf(stderr, "Failed to get modbus map of slave %d\n", mm);
						break;
					}
				}
				if (mm == MAX_NUM_MODBUS_SLAVE)
				{
					strcat(resBufStr, "Modbus register of all slaves have been set\n");
				}
				else
				{
					strcat(resBufStr, "Modbus register of all slaves have not been set\n");
				}
			}
			else
			{
				strcat(resBufStr, "Do not support all read all slaves, please specify slave Id to read\n");
			}
		}
		else
		{
			if (pModbusMap != NULL)
			{
				if (isChange == 1)
				{
					strcat(resBufStr, "Previous value: ");
				}
				else
				{
					strcat(resBufStr, "Current value : ");
				}

				for (uint32_t j = 0; j < INPUT_REGISTERS_NB; ++j)
				{
					char numStr[10];
					sprintf(numStr, "%x", pModbusMap->tab_input_registers[j]);
					strcat(resBufStr, numStr);
					strcat(resBufStr, " ");
				}
				strcat(resBufStr, "\n");

				if (isChange == 1)
				{
					for (uint8_t jj = 0; jj < INPUT_REGISTERS_NB; ++jj)
					{
						pthread_mutex_lock(&MtxModbusMapArr);
						pModbusMap->tab_input_registers[jj] = setRegBuf[jj] ;
						pthread_mutex_unlock(&MtxModbusMapArr);
					}
				}
			}
			else
			{
				continue;
			}
		}

		n = sendto(sockFd, resBufStr , strlen(resBufStr), 0,
		           (struct sockaddr *) &clientAddr, clientLen);
		if (n < 0)
		{
			fprintf(stderr, "err: sendto()\n");
			continue;
		}
	}
	return NULL;
}

unsigned int HexStringToUInt(char const* hexstring)
{
	const int  ASCII_0_VALU = 48;
	const int  ASCII_9_VALU = 57;
	const int  ASCII_A_VALU = 65;
	const int  ASCII_F_VALU = 70;
	unsigned int result = 0;
	char const *c = hexstring;
	char thisC;

	while( (thisC = *c) != NULL )
	{
		unsigned int add;
		thisC = toupper(thisC);

		result <<= 4;

		if( thisC >= ASCII_0_VALU &&  thisC <= ASCII_9_VALU )
			add = thisC - ASCII_0_VALU;
		else if( thisC >= ASCII_A_VALU && thisC <= ASCII_F_VALU)
			add = thisC - ASCII_A_VALU + 10;
		else
		{
			printf("Unrecognised hex character \"%c\"\n", thisC);
			exit(-1);
		}

		result += add;
		++c;
	}

	return result;
}

int InitModbusMap(void)
{
	/* Create modbus map */
	pthread_mutex_lock(&MtxModbusMapArr);
	for (uint8_t i = 0; i < MAX_NUM_MODBUS_SLAVE; ++i)
	{
		modbus_mapping_t *pModbusMap =
			modbus_mapping_new_start_address(BITS_ADDRESS, BITS_NB,
			                                 INPUT_BITS_ADDRESS, INPUT_BITS_NB,
			                                 REGISTERS_ADDRESS, REGISTERS_NB,
			                                 INPUT_REGISTERS_ADDRESS, INPUT_REGISTERS_NB);
		if (pModbusMap == NULL)
		{
			fprintf(stderr, "Failed to allocate the mapping: %s\n",
			        modbus_strerror(errno));
			return -1;
		}

		/* Initialize value for first 16 Input Registers in Modbus map */
		for (uint32_t j = 0; j < INPUT_REGISTERS_NB; ++j)
		{
			pModbusMap->tab_input_registers[j] = j;
		}
		pModbusMap->tab_input_registers[0] = i;

		/* Add to array */
		ModbusMapArr[i] = pModbusMap;
	}
	pthread_mutex_unlock(&MtxModbusMapArr);

	return 0;
}

int InitModbusContext(void)
{
	int ret = 0;
	/* Create modbus context */
	pModbusCtx = modbus_new_rtu(gSerialDevice, gBaudRate, 'N', 8, 1);
	if (pModbusCtx == NULL)
	{
		return -1;
	}

	/* Hacked here, we only use one modbus context for all modbus slaves from 1 to 254 */
	ret = modbus_set_slave(pModbusCtx, COMMON_SLAVE_ID);
	if (ret == -1)
	{
		return -1;
	}

	if (gDebug > 0)
	{
		ret = modbus_set_debug(pModbusCtx, TRUE);
		if (ret == -1)
		{
			return -1;
		}
	}

	/* Sets up a serial port for RTU communications */
	ret = modbus_connect(pModbusCtx);
	if (ret == -1)
	{
		fprintf(stderr, "Unable to connect %s\n", modbus_strerror(errno));
		modbus_free(pModbusCtx);
		return -1;
	}
	return ret;
}

int main(int argc, char*argv[])
{
	int ret;
	/* Parse command line option */
	if (argc < 3)
	{
		fprintf(stderr, "Usage: %s [-s SerialDevice] [-b baudrate]\n", argv[0]);
		exit(EXIT_FAILURE);
	}

	int Opt;
	while ((Opt = getopt(argc, argv, "s:b:p:d:")) != -1)
	{
		switch (Opt)
		{
			case 's':
				gSerialDevice = optarg;
				break;
			case 'b':
				gBaudRate = atoi(optarg);
				break;
			case 'p':
				gPort = atoi(optarg);
				break;
			case 'd':
				gDebug = atoi(optarg);
				break;
			default:
				fprintf(stderr, "Usage: %s [-s SerialDevice] [-b baudrate]\n", argv[0]);
				exit(EXIT_FAILURE);
		}
	}
	if (optind < argc)
	{
		fprintf(stderr, "Wrong argument after options\n");
		fprintf(stderr, "Usage: %s [-s SerialDevice] [-b baudrate]\n", argv[0]);
		exit(EXIT_FAILURE);
	}

	printf("Program: %s\nSerialDevice: %s, Baudrate %d, UDP port %d\n", argv[0],
	       gSerialDevice, gBaudRate, gPort);

	/* Init modbusmap */
	ret = InitModbusMap();
	if (ret != 0)
	{
		fprintf(stderr, "can not InitModbusMap");
		exit(EXIT_FAILURE);
	}

	/* Init handle modbus map data thread */
	pthread_t threadId;
	const char *threadName = "Handle_ModBus";
	ret = pthread_create(&threadId, NULL, &handle_modbus_map_data, (void*)threadName);
	if (ret != 0)
	{
		fprintf(stderr, "pthread_create");
		exit(EXIT_FAILURE);
	}

	/* Init modbus context */
	ret = InitModbusContext();
	if (ret != 0)
	{
		fprintf(stderr, "can not InitModbusContext ");
		exit(EXIT_FAILURE);
	}

	int header_length = modbus_get_header_length(pModbusCtx);
	uint8_t pQuery[MODBUS_RTU_MAX_ADU_LENGTH];

	/* Biggest loop to handle query from DAU */
	int RC;
	while (1)
	{
		do
		{
			RC = modbus_receive(pModbusCtx, pQuery);
			/* Filtered queries return 0 */
		} while (RC == 0);

		uint8_t slaveId = pQuery[header_length - 1];
		fprintf(stdout, "Request to slave ID: %d\n", slaveId );

		/* Reply message for DAU base on DAU's request */
		pthread_mutex_lock(&MtxModbusMapArr);
		RC = modbus_reply(pModbusCtx , pQuery, RC, ModbusMapArr[slaveId]);
		pthread_mutex_unlock(&MtxModbusMapArr);
		if (RC == -1)
		{
			break;
		}
	}

	/* Clean up */
	for (uint8_t i = 0; i < MAX_NUM_MODBUS_SLAVE; ++i)
	{
		modbus_mapping_free(ModbusMapArr[i]);
	}
	modbus_close(pModbusCtx);
	modbus_free(pModbusCtx);
	exit(EXIT_FAILURE);
}
