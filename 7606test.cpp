// 7606test.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"


/*
* File:   main.cpp
* By Dennis.
* TEST AD7606 sync adc ADC 16bits 8 channel converter
*/

#include <cstdlib>
#include <stdio.h>
#include <iostream>
#include <iomanip>
#include <string.h>

#include "7606test.h"
#include "ftd2xx.h"

using namespace std;
FT_HANDLE h;

/**
* init device
* @return
*/
int initftdi() {
	FT_STATUS st;
	DWORD ret; // rx, tx, ev;
	DWORD rte[3];
	char data[3];
	int result;
	cout << "init device" << endl;
	st = FT_OpenEx((PVOID) "ADC", (DWORD)1 << 1, &h);
	if (!FT_SUCCESS(st)) {
		cerr << "error FTOpen" << endl;
		Sleep(3000);
		return -1;
	}
	// reset device
	if (!FT_SUCCESS(st = FT_ResetDevice((FT_HANDLE)h))) {
		cerr << "error reset device" << endl;
		return -1;
	}
	// queue status
	DWORD r;
	if (!FT_SUCCESS(st = FT_GetQueueStatus((FT_HANDLE)h, &r))) {
		cerr << "error get queue status" << endl;
		return -1;
	}
	// reset device
	if (!FT_SUCCESS(st = FT_ResetDevice((FT_HANDLE)h))) {
		cerr << "error reset device" << endl;
		return -1;
	}
	// set timeout to 3000msec , rx = 3000msec, tx = none (0)
	if (!FT_SUCCESS(st = FT_SetTimeouts((FT_HANDLE)h, (DWORD)3000, (DWORD)3000))) {
		cerr << "error set timeout" << endl;
		return -1;
	}
	// set usb latency timer, to 1msec
	if (!FT_SUCCESS(st = FT_SetLatencyTimer((FT_HANDLE)h, (UCHAR)16))) {
		cerr << "error set latency timer" << endl;
		return -1;
	}
	if (!FT_SUCCESS(st = FT_SetUSBParameters((FT_HANDLE)h, (ULONG)512, (ULONG)512))) {
		cerr << "error set usb parameters" << endl;
		return -1;
	}
	// MPSSE reset bit mode 0x00, all input and reset
	if (!FT_SUCCESS(st = FT_SetBitMode((FT_HANDLE)h, (UCHAR)0x00, (UCHAR)0x00))) {
		cerr << "error set bit mode" << endl;
		return -1;
	}
	// MPSSE set input=0 output=1, bit 00000000// bit 2 input (TDO/DI) ,, all AD7-0 inputs
	if (!FT_SUCCESS(st = FT_SetBitMode((FT_HANDLE)h, (UCHAR)0x00, (UCHAR)0x02))) {
		cerr << "error set I/O mode" << endl;
		return -1;
	}
	unsigned char registers[] = { 0x8B, 0x97, 0x8d, 0x86, 0x00, 0x00, 0x80, 0x00, 0x00, 0x82, 0x15, 0x9f };
	int nreg = sizeof(registers) / sizeof(unsigned char);
	//cout << "nreg = " << std::dec << nreg << endl;
	if (!FT_SUCCESS(st = FT_Write((FT_HANDLE)h, (LPVOID)registers, nreg, (LPDWORD)& ret))) {
		cerr << "error writing register" << endl;
		return -1;
	}
	if (!FT_SUCCESS(st = FT_Purge((FT_HANDLE)h, (DWORD)(FT_PURGE_RX | FT_PURGE_TX)))) {
		cerr << "error purge buffer" << endl;
		return -1;
	}
	Sleep(50);
	return 0;
}

long delay = 1;

/**
* close device
* @return
*/
int closeftdi() {
	cout << "close device" << endl;
	FT_STATUS st;
	st = FT_Close((FT_HANDLE)h);
	if (!FT_SUCCESS(st)) {
		cerr << "error close device" << endl;
		return -1;
	}
	return 0;
}

int reset7606adc() {
	FT_STATUS st;
	DWORD ret; // rx, tx, ev;
	DWORD rte[3];
	int nreg = 0;
	unsigned char registers0[] = { 0x82, 0x15, 0x9f, 0x82, 0x15, 0x9f, 0x82, 0x17, 0x9f, 0x82, 0x15, 0x9f }; // reset 83.3*2 = 166.6ns
	nreg = sizeof(registers0) / sizeof(unsigned char);
	if (!FT_SUCCESS(st = FT_Write((FT_HANDLE)h, (LPVOID)registers0, nreg, (LPDWORD)& ret))) {
		cerr << "error writing register" << endl;
		return -1;
	}
	return 0;
}

int waitbusy = 8 * 8; // number of BUSY check cmd

int read7606adc() {
	FT_STATUS st;
	DWORD ret; // rx, tx, ev;
	DWORD result;
	DWORD rte[3];
	char data[128];
	int c = 0;
	int nreg = 0;
	cout << "read ADC" << endl;
	memset(data, 0, sizeof(data) / sizeof(unsigned char));

	unsigned char registers1[] = {
		0x82, 0x15, 0x9f, //default state
		0x82, 0x14, 0x9f, //CVA+CVB go low then high   
		0x82, 0x14, 0x9f, //CVA+CVB go low then high.
		0x82, 0x15, 0x9f, //default state
		0x83, 0x83, 0x83, 0x83, 0x83, 0x83, 0x83, 0x83, // busy flag check, 133us* 8 X 10 = 10.6us wait
		0x83, 0x83, 0x83, 0x83, 0x83, 0x83, 0x83, 0x83,
		0x83, 0x83, 0x83, 0x83, 0x83, 0x83, 0x83, 0x83,
		0x83, 0x83, 0x83, 0x83, 0x83, 0x83, 0x83, 0x83,
		0x83, 0x83, 0x83, 0x83, 0x83, 0x83, 0x83, 0x83,
		0x83, 0x83, 0x83, 0x83, 0x83, 0x83, 0x83, 0x83,
		0x83, 0x83, 0x83, 0x83, 0x83, 0x83, 0x83, 0x83,
		0x83, 0x83, 0x83, 0x83, 0x83, 0x83, 0x83, 0x83, // busy flag check.
		0x82, 0x15, 0x9f, // default
		0x82, 0x11, 0x9f, 0x82, 0x01, 0x9f, 0x83, 0x80, 0x00, 0x00, 0x81, // read channel 1 MSB, #cs=0, #rd=1 -> #cs=0, #rd=0 -> check FRST -> read Data
		0x82, 0x11, 0x9f, 0x82, 0x01, 0x9f, 0x83, 0x80, 0x00, 0x00, 0x81, // read channel 1 LSB, #cs=0, #rd=1 -> #cs=0, #rd=0 -> check FRST -> read Data
		0x82, 0x11, 0x9f, 0x82, 0x01, 0x9f, 0x83, 0x80, 0x00, 0x00, 0x81, // read channel 2 MSB, #cs=0, #rd=1 -> #cs=0, #rd=0 -> check FRST -> read Data
		0x82, 0x11, 0x9f, 0x82, 0x01, 0x9f, 0x83, 0x80, 0x00, 0x00, 0x81, // read channel 2 LSB, #cs=0, #rd=1 -> #cs=0, #rd=0 -> check FRST -> read Data
		0x82, 0x11, 0x9f, 0x82, 0x01, 0x9f, 0x83, 0x80, 0x00, 0x00, 0x81, // read channel 3 MSB, #cs=0, #rd=1 -> #cs=0, #rd=0 -> check FRST -> read Data
		0x82, 0x11, 0x9f, 0x82, 0x01, 0x9f, 0x83, 0x80, 0x00, 0x00, 0x81, // read channel 3 LSB, #cs=0, #rd=1 -> #cs=0, #rd=0 -> check FRST -> read Data
		0x82, 0x11, 0x9f, 0x82, 0x01, 0x9f, 0x83, 0x80, 0x00, 0x00, 0x81, // read channel 4 MSB, #cs=0, #rd=1 -> #cs=0, #rd=0 -> check FRST -> read Data
		0x82, 0x11, 0x9f, 0x82, 0x01, 0x9f, 0x83, 0x80, 0x00, 0x00, 0x81, // read channel 4 LSB, #cs=0, #rd=1 -> #cs=0, #rd=0 -> check FRST -> read Data
		0x82, 0x11, 0x9f, 0x82, 0x01, 0x9f, 0x83, 0x80, 0x00, 0x00, 0x81, // read channel 5 MSB, #cs=0, #rd=1 -> #cs=0, #rd=0 -> check FRST -> read Data
		0x82, 0x11, 0x9f, 0x82, 0x01, 0x9f, 0x83, 0x80, 0x00, 0x00, 0x81, // read channel 5 LSB, #cs=0, #rd=1 -> #cs=0, #rd=0 -> check FRST -> read Data
		0x82, 0x11, 0x9f, 0x82, 0x01, 0x9f, 0x83, 0x80, 0x00, 0x00, 0x81, // read channel 6 MSB, #cs=0, #rd=1 -> #cs=0, #rd=0 -> check FRST -> read Data
		0x82, 0x11, 0x9f, 0x82, 0x01, 0x9f, 0x83, 0x80, 0x00, 0x00, 0x81, // read channel 6 LSB, #cs=0, #rd=1 -> #cs=0, #rd=0 -> check FRST -> read Data
		0x82, 0x11, 0x9f, 0x82, 0x01, 0x9f, 0x83, 0x80, 0x00, 0x00, 0x81, // read channel 7 MSB, #cs=0, #rd=1 -> #cs=0, #rd=0 -> check FRST -> read Data
		0x82, 0x11, 0x9f, 0x82, 0x01, 0x9f, 0x83, 0x80, 0x00, 0x00, 0x81, // read channel 7 LSB, #cs=0, #rd=1 -> #cs=0, #rd=0 -> check FRST -> read Data
		0x82, 0x11, 0x9f, 0x82, 0x01, 0x9f, 0x83, 0x80, 0x00, 0x00, 0x81, // read channel 8 MSB, #cs=0, #rd=1 -> #cs=0, #rd=0 -> check FRST -> read Data
		0x82, 0x11, 0x9f, 0x82, 0x01, 0x9f, 0x83, 0x80, 0x00, 0x00, 0x81, // read channel 8 LSB, #cs=0, #rd=1 -> #cs=0, #rd=0 -> check FRST -> read Data
		0x82, 0x15, 0x9f //default state
	};
	nreg = sizeof(registers1) / sizeof(unsigned char);
	if (!FT_SUCCESS(st = FT_Write((FT_HANDLE)h, (LPVOID)registers1, nreg, (LPDWORD)& ret))) {
		cerr << "error writing register" << endl;
		return -1;
	}
	if (!FT_SUCCESS(st = FT_Read((FT_HANDLE)h, (LPVOID)data, waitbusy + 32, (LPDWORD)& result))) {
		cerr << "error writing register" << endl;
		return -1;
	}
	//data[0] = data[0] & 0x20; // busy flag mask
	hexdump(data, waitbusy + 32);

	return 0;
}

void testwrite() {
	FT_STATUS st;
	DWORD ret; // rx, tx, ev;
	DWORD result;
	DWORD rte[3];
	int c = 0;
	int nreg = 0;
	cout << "test I/O AD7606" << endl;
	unsigned char registers0[] = { 0x80, 0xff, 0xff, 0x82, 0xff, 0xff };
	unsigned char registers1[] = { 0x80, 0x00, 0xff, 0x82, 0x00, 0xff };
	int nreg0 = sizeof(registers0) / sizeof(unsigned char);
	int nreg1 = sizeof(registers1) / sizeof(unsigned char);

	if (!FT_SUCCESS(st = FT_Write((FT_HANDLE)h, (LPVOID)registers0, nreg0, (LPDWORD)& ret))) {
		cerr << "error writing register" << endl;
		exit(-1);
	}
	Sleep(1000);
	if (!FT_SUCCESS(st = FT_Write((FT_HANDLE)h, (LPVOID)registers1, nreg1, (LPDWORD)& ret))) {
		cerr << "error writing register" << endl;
		exit(-1);
	}
	Sleep(1000);

}

void testread() {
	FT_STATUS st;
	DWORD ret; // rx, tx, ev;
	DWORD result;
	DWORD rte[3];
	char data[2];
	int c = 0;
	int nreg = 0;
	cout << "test I/O AD7606" << endl;
	memset(data, 0, sizeof(data) / sizeof(unsigned char));
	unsigned char registers0[] = { 0x81, 0x83 };
	int nreg0 = sizeof(registers0) / sizeof(unsigned char);
	if (!FT_SUCCESS(st = FT_Write((FT_HANDLE)h, (LPVOID)registers0, nreg0, (LPDWORD)& ret))) {
		cerr << "error writing register" << endl;
		exit(-1);
	}
	if (!FT_SUCCESS(st = FT_Read((FT_HANDLE)h, (LPVOID)data, 2, (LPDWORD)& result))) {
		cerr << "error writing register" << endl;
		exit(-1);
	}
	hexdump(data, 2);
}

void hex(char data) {
	cout << "0x" << std::hex << std::setw(2) << std::setfill('0') << (data & 0xff) << ":";
}

void hex16(int data) {
	cout << "0x" << std::hex << std::setw(4) << std::setfill('0') << (data & 0xffff) << ":";
}
double vref = 2.5000;

void hexdump(char *data, int size) {
	int z;
	for (z = 0; z < 8; z++) {

		// hex(*(data + waitbusy + z * 2) >> 5 & 0x03); // FRST and BUSY
		cout << "value=";
		double volt = 0;
		int msbvolt = *(data + waitbusy + z * 4 + 1);
		if ((msbvolt & 0x80) == 0) {
			int reading = (msbvolt << 8) & 0x7f00 | (*(data + waitbusy + z * 4 + 3)) & 0x00ff;
			volt = (double)reading * 5 / 32768 * 2.5 / vref;
		}
		else {
			int reading = 0x7fff - ((msbvolt << 8) & 0x7f00 | (*(data + waitbusy + z * 4 + 3)) & 0x00ff);
			volt = -(double)reading * 5 / 32768 * 2.5 / vref;
		}
		cout << "CH=" << z << ":";
		cout << std::fixed;
		cout << setprecision(4) << volt;
		cout << endl;
	}

}

int main(int argc, char** argv) {
	if (initftdi() < 0) {
		cerr << "init() device, error init device" << endl;
		Sleep(3000);
		exit(-1);
	}
	if (reset7606adc() < 0) {
		cerr << "reset adc error" << endl;
		Sleep(3000);
		exit(-1);
	}
	Sleep(1);
	while (1) {
		if (read7606adc() < 0) {
			cout << "read adc error!!!" << endl;
			Sleep(3000);
			exit(-1);
		};
	}
	Sleep(5);
	if (closeftdi() < 0) {
		cerr << "close() device, error close device" << endl;
		Sleep(3000);
		exit(-1);
	}
	return 0;
}



