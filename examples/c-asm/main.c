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

// === Programa principal ===
void main() {
    // Mensaje de 64 bits = "HOLA1234"
    uint32_t block[2] = {0x484F4C41, 0x31323334}; // ASCII en hex
    // Clave de 128 bits
    uint32_t key[4] = {0x12345678, 0x9ABCDEF0, 0xFEDCBA98, 0x76543210};

    print_string("=== Test TEA en RISC-V ===\n");

    // Mostrar bloque original
    print_string("Texto original:\n");
    print_hex(block[0]); print_char(' ');
    print_hex(block[1]); print_char('\n');

    // Cifrado
    tea_encrypt_asm(block, key);

    print_string("Texto cifrado:\n");
    print_hex(block[0]); print_char(' ');
    print_hex(block[1]); print_char('\n');

    // Descifrado
    tea_decrypt_asm(block, key);

    print_string("Texto descifrado:\n");
    print_hex(block[0]); print_char(' ');
    print_hex(block[1]); print_char('\n');

    print_string("=== Fin del test ===\n");

    // Loop infinito
    while (1) {
        __asm__ volatile("nop");
    }
}
