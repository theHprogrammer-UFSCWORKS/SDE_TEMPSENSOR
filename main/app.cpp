/* Detectou escravo na linha
Codigo da Familia: 40
Numero de Serie  : 63 161 4 5 0 0
CRC=             : 18
*/
#include <stdio.h>
#include <string.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_spi_flash.h"
#include "serial.h"
#include "DS18B20.h"
#include "delay.h"
#include <inttypes.h>
#include "digital.h"
#include <vector>

extern "C" void app_main();

void app_main()
{
  delay_ms(2000);
  serial.begin(9600);

  // Contador de Dispositivos
  int numDispositivos = 0;

  DS18B20 meuSensor = DS18B20(PIN16);

  printf("\nBUSCA POR DISPOSITIVOS NO BARRAMENTO 1-WIRE\n");

  int rslt = meuSensor.fazScanPrimeiro();
  while (rslt)
  {
    printf("\n------ INFORMACOES DO SENSOR ------\n");
    printf("Codigo da Familia: %d\n", meuSensor.endereco[0]);
    printf("Numero de Serie: %d %d %d %d %d %d\n", meuSensor.endereco[1], meuSensor.endereco[2], meuSensor.endereco[3], meuSensor.endereco[4], meuSensor.endereco[5], meuSensor.endereco[6]);
    printf("CRC: %d\n", meuSensor.endereco[7]);
    printf("Endereco completo: %d %d %d %d %d %d %d %d\n", meuSensor.endereco[7], meuSensor.endereco[6], meuSensor.endereco[5], meuSensor.endereco[4], meuSensor.endereco[3], meuSensor.endereco[2], meuSensor.endereco[1], meuSensor.endereco[0]);

    printf("\n-----------------------------------\n");

    ++numDispositivos;

    rslt = meuSensor.fazScanProximo();
  }

  printf("\nNumero de Dispositivos = %d\n", numDispositivos);
}

// Função para print da temperatura do Fábio
// void printFloat(float f)
// {
//   char str[20];
//   sprintf(str, "%d.%04d\n", (int)f, (int)(f * 10000) % 10000);
//   printf(str);
// }
