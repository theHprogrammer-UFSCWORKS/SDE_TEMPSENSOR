#ifndef __DS18B20__
#define __DS18B20__

#include <inttypes.h>
#include "onewire.h"

#include "digital.h"
#include "debug.h"
#include <string>

// definitions
#define FALSE 0
#define TRUE 1

class DS18B20
{

private:
	ONEWIRE *onewire;
	// char CRC(char end[]);
	// void capturaBit(int posicao, char v[], int valor);

public:
	DS18B20(gpio_num_t pino);
	// float readTemp(void);
	// float readTargetTemp(char end[]);

	// void init(void);
	// void init(char v[]);

	int fazScanPrimeiro();
	int fazScanProximo();
	int fazScan();

	unsigned char endereco[8];
	int ultima_discrepancia = 0;
	int ultima_discrepancia_familia= 0;
	int ultimo_dispositivo = 0;
	unsigned char crc_number = 0;
};

#endif
