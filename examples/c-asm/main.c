
#include <stdint.h>

// Declaración de funciones en ensamblador
extern void tea_encrypt_asm(uint32_t v[2], const uint32_t key[4]);
extern void tea_decrypt_asm(uint32_t v[2], const uint32_t key[4]);

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

int my_strlen(const char *str) {
    int len = 0;
    while (*str++) len++;
    return len;
}

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

// cero paddig(pading de cero)
void paddig(const char* str, int offset, uint32_t block[2]) {
    uint8_t temp[8] = {0};
    int len = my_strlen(str + offset);
    if (len > 8) len = 8;

    for (int i = 0; i < len; i++) {
        temp[i] = (uint8_t)str[offset + i];
    }

    block[0] = (temp[0]<<24) | (temp[1]<<16) | (temp[2]<<8) | temp[3];
    block[1] = (temp[4]<<24) | (temp[5]<<16) | (temp[6]<<8) | temp[7];
}

void main() {
    // PRUEBA 1
    // ======================
    char mensaje1[] = "HOLA1234";
    int len1 = my_strlen(mensaje1);

    // keys
    uint32_t key1[4] = {0x12345678, 0x9ABCDEF0, 0xFEDCBA98, 0x76543210};

    print_string("Prueba 1 \n");
    print_string("====================== \n");

 
    print_string("Texto original:\n");
    print_string(mensaje1);
    print_char('\n');

    // Cifrado 
    print_string("Texto cifrado (hex):\n");
    for (int i = 0; i < len1; i += 8) {
        uint32_t block[2];
        paddig(mensaje1, i, block);

        tea_encrypt_asm(block, key1);

        print_hex(block[0]); print_char(' ');
        print_hex(block[1]); print_char('\n');
    }

    // Descifrado
    print_string("Texto descifrado:\n");
    for (int i = 0; i < len1; i += 8) {
        uint32_t block[2];
        paddig(mensaje1, i, block);

        tea_encrypt_asm(block, key1);   // cifrar
        tea_decrypt_asm(block, key1);   // descifrar

        print_block_as_text(block);
    }
    print_char('\n');
    print_string("=== Fin Prueba 1 ===\n\n");


    // PRUEBA 2
    // ======================
    char mensaje2[] = "HOLA1234";
    int len2 = my_strlen(mensaje2);

    // keys
    uint32_t key2[4] = {0xA56BABCD, 0x0000FFFF, 0xABCDEF01, 0x12345678};

    print_string("Prueba 2\n");
    print_string("======================\n");

 
    print_string("Texto original:\n");
    print_string(mensaje2);
    print_char('\n');

    // Cifrado 
    print_string("Texto cifrado (hex):\n");
    for (int i = 0; i < len2; i += 8) {
        uint32_t block[2];
        paddig(mensaje2, i, block);

        tea_encrypt_asm(block, key2);

        print_hex(block[0]); print_char(' ');
        print_hex(block[1]); print_char('\n');
    }

    // Descifrado
    print_string("Texto descifrado:\n");
    for (int i = 0; i < len2; i += 8) {
        uint32_t block[2];
        paddig(mensaje2, i, block);

        tea_encrypt_asm(block, key2);   // cifrar
        tea_decrypt_asm(block, key2);   // descifrar

        print_block_as_text(block);
    }
    print_char('\n');
    print_string("=== Fin Prueba 2 ===\n");


    // Loop infinito
    while (1) {
        __asm__ volatile("nop");
    }
}
