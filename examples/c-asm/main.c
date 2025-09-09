// main.c - Programa en C que usa TEA en ensamblador RISC-V

#include <stdint.h>

// Declaración de funciones en ensamblador
extern void tea_encrypt_asm(uint32_t v[2], const uint32_t key[4]);
extern void tea_decrypt_asm(uint32_t v[2], const uint32_t key[4]);

// === Funciones simples de impresión (bare-metal UART en QEMU) ===
void print_char(char c) {
    volatile char *uart = (volatile char*)0x10000000;
    *uart = c;
}

void print_string(const char* str) {
    while (*str) {
        print_char(*str++);
    }
}

void print_hex(uint32_t num) {
    char hex_chars[] = "0123456789ABCDEF";
    for (int i = 7; i >= 0; i--) {
        uint8_t nibble = (num >> (i * 4)) & 0xF;
        print_char(hex_chars[nibble]);
    }
}

// === Implementación propia de strlen (no usar <string.h>) ===
int my_strlen(const char *str) {
    int len = 0;
    while (*str++) len++;
    return len;
}

// Imprimir 8 bytes de un bloque como caracteres ASCII
void print_block_as_text(uint32_t block[2]) {
    char buf[9];
    buf[0] = (block[0] >> 24) & 0xFF;
    buf[1] = (block[0] >> 16) & 0xFF;
    buf[2] = (block[0] >> 8) & 0xFF;
    buf[3] = block[0] & 0xFF;
    buf[4] = (block[1] >> 24) & 0xFF;
    buf[5] = (block[1] >> 16) & 0xFF;
    buf[6] = (block[1] >> 8) & 0xFF;
    buf[7] = block[1] & 0xFF;
    buf[8] = '\0';
    print_string(buf);
}

// Preparar bloque de 8 bytes con padding en ceros
void prepare_block(const char* str, int offset, uint32_t block[2]) {
    uint8_t temp[8] = {0};
    int len = my_strlen(str + offset);
    if (len > 8) len = 8;

    for (int i = 0; i < len; i++) {
        temp[i] = (uint8_t)str[offset + i];
    }

    block[0] = (temp[0]<<24) | (temp[1]<<16) | (temp[2]<<8) | temp[3];
    block[1] = (temp[4]<<24) | (temp[5]<<16) | (temp[6]<<8) | temp[7];
}

// === Programa principal ===
void main() {
    char mensaje[] = "HOLA1234";  // Cadena de ejemplo
    int len = my_strlen(mensaje);

    // Clave de 128 bits
    uint32_t key[4] = {0x12345678, 0x9ABCDEF0, 0xFEDCBA98, 0x76543210};

    print_string("=== Test TEA en RISC-V ===\n");

    // Mostrar texto original
    print_string("Texto original:\n");
    print_string(mensaje);
    print_char('\n');

    // Cifrado bloque por bloque
    print_string("Texto cifrado (hex):\n");
    for (int i = 0; i < len; i += 8) {
        uint32_t block[2];
        prepare_block(mensaje, i, block);

        tea_encrypt_asm(block, key);

        print_hex(block[0]); print_char(' ');
        print_hex(block[1]); print_char('\n');
    }

    // Descifrado de nuevo bloque por bloque
    print_string("Texto descifrado:\n");
    for (int i = 0; i < len; i += 8) {
        uint32_t block[2];
        prepare_block(mensaje, i, block);

        tea_encrypt_asm(block, key);   // cifrar
        tea_decrypt_asm(block, key);   // descifrar

        print_block_as_text(block);    // mostrar como texto
    }
    print_char('\n');

    print_string("=== Fin del test ===\n");

    while (1) {
        __asm__ volatile("nop");
    }
}
