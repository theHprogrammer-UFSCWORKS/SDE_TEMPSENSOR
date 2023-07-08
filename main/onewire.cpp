// Inclusão de bibliotecas que o código depende
#include "onewire.h"
#include "digital.h"
#include "delay.h"

// O construtor da classe ONEWIRE. Ele recebe um número de pino como argumento e armazena esse número.
ONEWIRE::ONEWIRE(gpio_num_t p)
{
    DEBUG("ONEWIRE:construtor\n"); // Gera uma mensagem de depuração.
    PIN_DADOS = p;                 // Atribui o valor do pino à variável PIN_DADOS.
}

// Este método configura o pino como saída e define seu valor para LOW (0).
void ONEWIRE::low(void)
{
    digital.pinMode(PIN_DADOS, OUTPUT);   // Configura o pino como OUTPUT.
    digital.digitalWrite(PIN_DADOS, LOW); // Escreve o valor LOW no pino.
}

// Este método configura o pino como entrada, tornando-o um high-impedance input.
void ONEWIRE::high(void)
{
    digital.pinMode(PIN_DADOS, INPUT); // Configura o pino como INPUT.
}

// Método de reset do barramento 1-Wire.
uint8_t ONEWIRE::reset(void)
{
    uint8_t resposta;
    low();
    delay_us(500);
    high();
    delay_us(60); // Aumente este delay
    resposta = digital.digitalRead(PIN_DADOS);
    delay_us(600);    // Ajuste este delay se necessário
    return !resposta; // Inverta a resposta se necessário
}

// Este método lê um bit do barramento 1-Wire.
uint8_t ONEWIRE::readBit(void)
{
    uint8_t c;                          // Declaração de uma variável para armazenar o bit.
    low();                              // Chama a função low().
    delay_us(5);                        // Delay de 5 microssegundos.
    high();                             // Chama a função high().
    c = digital.digitalRead(PIN_DADOS); // Lê o valor do pino.
    delay_us(70);                       // Delay de 70 microssegundos.
    return c;                           // Retorna o bit lido.
}

// Este método lê um byte (8 bits) do barramento 1-Wire.
uint8_t ONEWIRE::readByte(void)
{
    uint8_t valor, x, c; // Declaração das variáveis.

    valor = 0;              // Inicia a variável valor com 0.
    for (x = 0; x < 8; x++) // Loop para ler 8 bits.
    {
        low();                              // Chama a função low().
        delay_us(5);                        // Delay de 5 microssegundos.
        high();                             // Chama a função high().
        c = digital.digitalRead(PIN_DADOS); // Lê o valor do pino.
        valor = (valor) | (c << x);         // Realiza uma operação OR bit a bit entre o valor existente e o bit lido.
        delay_us(70);                       // Delay de 70 microssegundos.
    }
    return valor; // Retorna o byte lido.
}

// Este método escreve um bit no barramento 1-Wire.
void ONEWIRE::escreve_bit(uint8_t meu_bit)
{
    if (meu_bit) // Se o bit a ser escrito for 1.
    {
        low();        // Chama a função low().
        delay_us(5);  // Delay de 5 microssegundos.
        high();       // Chama a função high().
        delay_us(60); // Delay de 60 microssegundos.
    }
    else // Se o bit a ser escrito for 0.
    {
        low();        // Chama a função low().
        delay_us(80); // Delay de 80 microssegundos.
        high();       // Chama a função high().
    }
    delay_us(5); // Delay de 5 microssegundos.
}

// Este método escreve um byte (8 bits) no barramento 1-Wire.
void ONEWIRE::writeByte(uint8_t v)
{
    DEBUG("ONEWIRE:writeByte\n"); // Gera uma mensagem de depuração.

    uint8_t x;              // Declaração da variável.
    for (x = 0; x < 8; x++) // Loop para escrever 8 bits.
    {
        escreve_bit((v >> x) & 1); // Chama a função escreve_bit com cada bit do byte.
    }
}

static unsigned char crc_table[] = {
    0, 94, 188, 226, 97, 63, 221, 131, 194, 156, 126, 32, 163, 253, 31, 65,
    157, 195, 33, 127, 252, 162, 64, 30, 95, 1, 227, 189, 62, 96, 130, 220,
    35, 125, 159, 193, 66, 28, 254, 160, 225, 191, 93, 3, 128, 222, 60, 98,
    190, 224, 2, 92, 223, 129, 99, 61, 124, 34, 192, 158, 29, 67, 161, 255,
    70, 24, 250, 164, 39, 121, 155, 197, 132, 218, 56, 102, 229, 187, 89, 7,
    219, 133, 103, 57, 186, 228, 6, 88, 25, 71, 165, 251, 120, 38, 196, 154,
    101, 59, 217, 135, 4, 90, 184, 230, 167, 249, 27, 69, 198, 152, 122, 36,
    248, 166, 68, 26, 153, 199, 37, 123, 58, 100, 134, 216, 91, 5, 231, 185,
    140, 210, 48, 110, 237, 179, 81, 15, 78, 16, 242, 172, 47, 113, 147, 205,
    17, 79, 173, 243, 112, 46, 204, 146, 211, 141, 111, 49, 178, 236, 14, 80,
    175, 241, 19, 77, 206, 144, 114, 44, 109, 51, 209, 143, 12, 82, 176, 238,
    50, 108, 142, 208, 83, 13, 239, 177, 240, 174, 76, 18, 145, 207, 45, 115,
    202, 148, 118, 40, 171, 245, 23, 73, 8, 86, 180, 234, 105, 55, 213, 139,
    87, 9, 235, 181, 54, 104, 138, 212, 149, 203, 41, 119, 244, 170, 72, 22,
    233, 183, 85, 11, 136, 214, 52, 106, 43, 117, 151, 201, 74, 20, 246, 168,
    116, 42, 200, 150, 21, 75, 169, 247, 182, 232, 10, 84, 215, 137, 107, 53};

unsigned char ONEWIRE::crc_verify(unsigned char crc, unsigned char value)
{
    crc = crc_table[crc ^ value];
    return crc;
}