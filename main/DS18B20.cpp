#include "DS18B20.h"
#include "delay.h"
#include <vector>
#include <string>
#include <cstring>

// Usa o pino GPIO16 para fazer a comunicacao
DS18B20::DS18B20(gpio_num_t pino)
{
	DEBUG("DS18B20:Construtor\n");
	onewire = new ONEWIRE(pino);
}

int DS18B20::fazScanPrimeiro()
{
	// Reinicializando o estado da busca
	ultima_discrepancia = 0;
	ultimo_dispositivo = FALSE;
	ultima_discrepancia_familia = 0;

	// Chamando o método fazScan e retornando o seu resultado
	return fazScan();
}

int DS18B20::fazScanProximo()
{
	// Chamando o método fazScan e retornando o seu resultado
	return fazScan();
}

int DS18B20::fazScan()
{
	// Inicializando variáveis para a busca
	int posicao;
	int ultimo_zero, num_byte_endereco, resultado;
	int normal, complemento;
	unsigned char mascara_byte_endereco, procura_direcao;

	// Inicializando as variáveis para a busca
	posicao = 1;
	ultimo_zero = 0;
	num_byte_endereco = 0;
	mascara_byte_endereco = 1;
	resultado = 0;

	// Se a última chamada não foi a última
	if (!ultimo_dispositivo)
	{
		// Reset no 1-Wire
		if (!onewire->reset())
		{
			// Reset na busca
			ultima_discrepancia = 0;
			ultimo_dispositivo = FALSE;
			ultima_discrepancia_familia = 0;

			return FALSE;
		}

		// Emitindo o comando de busca
		onewire->writeByte(SEARCH_ROM);

		// Iniciando loop para realizar a busca
		do
		{
			// Lendo um bit e seu complemento
			normal = onewire->readBit();
			complemento = onewire->readBit();

			// Verificando se não há dispositivos no 1-wire
			if ((normal == 1) && (complemento == 1))
				break;
			else
			{
				// Todos os dispositivos acoplados têm 0 ou 1
				if (normal != complemento)
					procura_direcao = normal; // valor de bit de gravação para busca
				else
				{
					// Se esta discrepância for antes da última discrepância
					// numa próxima anterior, então escolhe o mesmo da última vez
					if (posicao < ultima_discrepancia)
						procura_direcao = ((endereco[num_byte_endereco] & mascara_byte_endereco) > 0);
					else
						// Se igual ao último, escolhe 1, se não, escolhe 0
						procura_direcao = (posicao == ultima_discrepancia);

					// Se 0 foi escolhido, então grava sua posição em ultimo_zero
					if (procura_direcao == 0)
					{
						ultimo_zero = posicao;

						// Checando por última discrepância na família
						if (ultimo_zero < 9)
							ultima_discrepancia_familia = ultimo_zero;
					}
				}

				// Define ou limpa o bit no byte ROM num_byte_endereco
				// com a máscara mascara_byte_endereco
				if (procura_direcao == 1)
					endereco[num_byte_endereco] |= mascara_byte_endereco;
				else
					endereco[num_byte_endereco] &= ~mascara_byte_endereco;

				// Escreve o bit de direção de busca do número de série
				onewire->escreve_bit(procura_direcao);

				// Incrementa o contador de byte posicao
				// e desloca a máscara mascara_byte_endereco
				posicao++;
				mascara_byte_endereco <<= 1;

				// Se a máscara for 0, então vai para o novo byte SerialNum num_byte_endereco e reinicia a máscara
				if (mascara_byte_endereco == 0)
				{
					onewire->crc_verify(crc_number, endereco[num_byte_endereco]); // acumula o CRC
					num_byte_endereco++;
					mascara_byte_endereco = 1;
				}
			}
		} while (num_byte_endereco < 8); // Loop até passar por todos os bytes ROM 0-7

		// Se a busca foi bem-sucedida, então
		if (!((posicao < 65) || (crc_number != 0)))
		{
			// A busca foi bem-sucedida, então define ultima_discrepancia,ultimo_dispositivo,resultado
			ultima_discrepancia = ultimo_zero;

			// Verifica se é o último dispositivo
			if (ultima_discrepancia == 0)
				ultimo_dispositivo = TRUE;

			resultado = TRUE;
		}
	}

	// Se nenhum dispositivo foi encontrado, então reseta os contadores para que a próxima 'busca' seja como a primeira
	if (!resultado || !endereco[0])
	{
		ultima_discrepancia = 0;
		ultimo_dispositivo = FALSE;
		ultima_discrepancia_familia = 0;
		resultado = FALSE;
	}

	return resultado;
}

// void DS18B20::capturaBit(int posicao, char v[], int valor)
// {
// 	unsigned char pbyte = posicao / 8;
// 	unsigned char pbit = posicao % 8;

// 	if (valor == 1)
// 		v[pbyte] |= (1 << pbit);
// 	else
// 		v[pbyte] &= ~(1 << pbit);
// }

// void DS18B20::init(void)
// {
// 	uint8_t serial[6], crc;
// 	delay_ms(500);

// 	printf("INIT\n");
// 	if (onewire->reset() == 0)
// 		printf("Detectou escravo na linha\n");
// 	else
// 		printf("Nao detectou escravo\n");

// 	onewire->writeByte(READ_ROM);
// 	printf("Codigo da Familia: %d\n", onewire->readByte());
// 	for (uint8_t x = 0; x < 6; x++)
// 		serial[x] = onewire->readByte();

// 	printf("Numero de Serie  : %d %d %d %d %d %d\n", serial[0], serial[1], serial[2], serial[3], serial[4], serial[5]);

// 	crc = onewire->readByte();
// 	printf("CRC=             : %d\n", crc);

// 	delay_ms(1000);
// }

// void DS18B20::init(char v[])
// {

// 	delay_ms(500);

// 	printf("INIT\n");
// 	if (onewire->reset() == 0)
// 		printf("Detectou escravo na linha\n");
// 	else
// 		printf("Nao detectou escravo\n");

// 	onewire->writeByte(READ_ROM);
// 	v[7] = onewire->readByte(); // family number
// 	printf("Codigo da Familia: %d\n", v[7]);

// 	for (uint8_t x = 6; x >= 1; x--)
// 		v[x] = onewire->readByte(); // serial

// 	printf("Numero de Serie  : %d %d %d %d %d %d\n", v[1], v[2], v[3], v[4], v[5], v[6]);

// 	v[0] = onewire->readByte();
// 	printf("CRC=             : %d\n", v[0]);

// 	delay_ms(1000);
// }

// char DS18B20::CRC(char end[])
// {
// 	// calcula o CRC e retorna o resultado
// 	return 0;
// }

// float DS18B20::readTargetTemp(char vetor_64bits[])
// {
// 	float temperatura;

// 	uint8_t a, b, inteira;
// 	float frac;

// 	onewire->reset();
// 	onewire->writeByte(MATCH_ROM);
// 	for (int x = 7; x >= 0; x--)
// 	{
// 		onewire->writeByte(vetor_64bits[x]);
// 	}
// 	onewire->writeByte(INICIA_CONVERSAO_TEMP);
// 	delay_ms(1000);
// 	onewire->reset();
// 	onewire->writeByte(MATCH_ROM);
// 	for (int x = 7; x >= 0; x--)
// 	{
// 		onewire->writeByte(vetor_64bits[x]);
// 	}
// 	onewire->writeByte(READ_TEMP_MEMORY);
// 	a = onewire->readByte();
// 	b = onewire->readByte();
// 	inteira = (b << 4) | (a >> 4);

// 	frac = ((a & 1) * (0.0625)) + (((a >> 1) & 1) * (0.125)) + (((a >> 2) & 1) * (0.25)) + (((a >> 3) & 1) * (0.5));

// 	temperatura = inteira + frac;
// 	return temperatura;
// }

// float DS18B20::readTemp(void)
// {
// 	float temperatura;

// 	uint8_t a, b, inteira;
// 	float frac;

// 	onewire->reset();
// 	onewire->writeByte(SKIP_ROM);
// 	onewire->writeByte(INICIA_CONVERSAO_TEMP);
// 	delay_ms(1000);
// 	onewire->reset();
// 	onewire->writeByte(SKIP_ROM);
// 	onewire->writeByte(READ_TEMP_MEMORY);
// 	a = onewire->readByte();
// 	b = onewire->readByte();
// 	inteira = (b << 4) | (a >> 4);

// 	frac = ((a & 1) * (0.0625)) + (((a >> 1) & 1) * (0.125)) + (((a >> 2) & 1) * (0.25)) + (((a >> 3) & 1) * (0.5));

// 	temperatura = inteira + frac;
// 	return temperatura;
// }
