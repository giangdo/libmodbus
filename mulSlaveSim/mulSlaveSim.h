/*
 * Copyright © 2016 Halliburton
 * Author: Giang Do
 */

#ifndef _GAUGE_SIMULATION_H_
#define _GAUGE_SIMULATION_H_

static const int COMMON_SLAVE_ID = 1;

#define MAX_NUM_MODBUS_SLAVE 254
static const int BUFSIZE = 1024;
/* Modbus Map Constants */
static const unsigned int BITS_ADDRESS = 0;
static const unsigned int BITS_NB = 0;
static const unsigned int INPUT_BITS_ADDRESS= 0;
static const unsigned int INPUT_BITS_NB = 0;
static const unsigned int REGISTERS_ADDRESS = 0;
static const unsigned int REGISTERS_NB = 0;
static const unsigned int INPUT_REGISTERS_ADDRESS = 0;
static const unsigned int INPUT_REGISTERS_NB = 16;

static void* handle_modbus_map_data(void * pVoid);
int InitModbusMap(void);
int InitModbusContext(void);
unsigned int HexStringToUInt(char const* hexstring);
#endif /* _GAUGE_SIMULATION_H_ */
