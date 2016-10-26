A commandline multi slaves modbus simulator
=======================

Overview
--------

This modbus simulator is free software library to simulate multiple modbus slaves
on the same serial port.

Practically, there is only one modbus slave on each serial device.

But in development process, sometimes we need multiple modbus slaves in only one
serial port for testing purpose.

This program support that feature for you.

This program is writen base libmodbus of Stephane [www.libmodbus.org](http://www.libmodbus.org)

Thank him for his work hard.

For Modbus Protocol, it is industrial protocol which is used in various area to connect between
computers and sensors.

[www.schneiderautomation.com](http://www.schneiderautomation.com).

The license of modbusMSim is *LGPL v2.1 or later*.

Compile and installation
------------

We don't need to install libmodbus before compiling modbusMSim because that library is built-in.

To native compiling:

	cd modbusMSim/
	mkdir build/
	cd build/
	cmake -DCMAKE_BUILD_TYPE=Release ../
	make -j4

To cross compiling:

	You need to specify appropriate path for cross_compiler_tool and target_sysroot.

	cd modbusMSim/
	mkdir crossBuild/
	cd crossBuild/
	cmake -DCMAKE_TOOLCHAIN_FILE=../Toolchain.cmake ../cmake -DCMAKE_BUILD_TYPE=Release ../
	make -j4

After build finish we will have two executable binary in build folder

	build/ctrlSlave/ctrlSlave
	build/mulSlaveSim/mulSlaveSim

Usage
-------
	* Run mulSlaveSim first:
		mulSlaveSim [-s SerialDevice] [-b baudrate]
		EX: 
		./mulSlaveSim -s /dev/ttyUSB0 -b 1200

	* Run ctrlSlave to change or read modbus resister value in mulSlaveSim.
		ctrlSlave [slaveID] [r/s] [modbus value]
		[slaveID] is decimal format
		[modbus value] is in hexadecimal format, if you want to set modbus value, you need to specify all 16 registers

		EX:
    	+ To read register value of slave 28: (return register will be in hexadecimal format)
    	   ./ctrlSlave 28 r
    	+ To read register value of slave 230: (return register will be in hexadecimal format)
    	   ./ctrlSlave 230 r
    	+ To read register value of all slave from 1 to 250: (return register will be in hexadecimal format)
    	   ./ctrlSlave a r

    	+ To set register value of slave 28: (value of register need to be in hexadecimal format)
    	   ./ctrlSlave 28 s ab0 ffd0 ae09 1 1 1 1 1 9 a a b c d e f
    	+ To set register value of slave 230: (value of register need to be in hexadecimal format)
    	   ./ctrlSlave 230 s ff ffd0 ae09 1 1 1 1 1 9 a a b c d e f
    	+ To set register value of all slave from 1 to 250: (value of register need to be in hexadecimal format)
    	   ./ctrlSlave a s ab0 ffd0 ae09 1 1 1 1 1 9 a a b c d e f
