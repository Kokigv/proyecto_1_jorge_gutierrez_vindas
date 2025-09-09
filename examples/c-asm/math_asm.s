# Tiny Encryption Algorithm (TEA) in RISC-V
# Funciones: tea_encrypt_asm(v0,v1,key) y tea_decrypt_asm(v0,v1,key)

    .section .text
    .globl tea_encrypt_asm
    .globl tea_decrypt_asm

# ======================================
# tea_encrypt_asm(int* v, int* k)
# v -> a0 = ptr [v0,v1]
# k -> a1 = ptr [k0,k1,k2,k3]
# ======================================
tea_encrypt_asm:
    addi sp, sp, -32
    sw ra, 28(sp)
    sw s0, 24(sp)
    sw s1, 20(sp)
    sw s2, 16(sp)
    sw s3, 12(sp)

    lw t0, 0(a0)    # v0
    lw t1, 4(a0)    # v1
    lw t2, 0(a1)    # k0
    lw t3, 4(a1)    # k1
    lw t4, 8(a1)    # k2
    lw t5, 12(a1)   # k3

    li s0, 32       # contador de rondas
    li s1, 0        # sum = 0
    li s2, 0x9e3779b9  # delta

enc_loop:
    add s1, s1, s2       # sum += delta

    # v0 += ((v1<<4)+k0) ^ (v1+sum) ^ ((v1>>5)+k1)
    slli s3, t1, 4
    add  s3, s3, t2
    add  t6, t1, s1
    xor  s3, s3, t6
    srli t6, t1, 5
    add  t6, t6, t3
    xor  s3, s3, t6
    add  t0, t0, s3

    # v1 += ((v0<<4)+k2) ^ (v0+sum) ^ ((v0>>5)+k3)
    slli s3, t0, 4
    add  s3, s3, t4
    add  t6, t0, s1
    xor  s3, s3, t6
    srli t6, t0, 5
    add  t6, t6, t5
    xor  s3, s3, t6
    add  t1, t1, s3

    addi s0, s0, -1
    bnez s0, enc_loop

    sw t0, 0(a0)
    sw t1, 4(a0)

    lw ra, 28(sp)
    lw s0, 24(sp)
    lw s1, 20(sp)
    lw s2, 16(sp)
    lw s3, 12(sp)
    addi sp, sp, 32
    ret

# ======================================
# tea_decrypt_asm(int* v, int* k)
# ======================================
tea_decrypt_asm:
    addi sp, sp, -32
    sw ra, 28(sp)
    sw s0, 24(sp)
    sw s1, 20(sp)
    sw s2, 16(sp)
    sw s3, 12(sp)

    lw t0, 0(a0)    # v0
    lw t1, 4(a0)    # v1
    lw t2, 0(a1)    # k0
    lw t3, 4(a1)    # k1
    lw t4, 8(a1)    # k2
    lw t5, 12(a1)   # k3

    li s0, 32           # contador de rondas
    li s2, 0x9e3779b9   # delta
    li s1, 0xC6EF3720   # sum = delta * 32

dec_loop:
    # v1 -= ((v0<<4)+k2) ^ (v0+sum) ^ ((v0>>5)+k3)
    slli s3, t0, 4
    add  s3, s3, t4
    add  t6, t0, s1
    xor  s3, s3, t6
    srli t6, t0, 5
    add  t6, t6, t5
    xor  s3, s3, t6
    sub  t1, t1, s3

    # v0 -= ((v1<<4)+k0) ^ (v1+sum) ^ ((v1>>5)+k1)
    slli s3, t1, 4
    add  s3, s3, t2
    add  t6, t1, s1
    xor  s3, s3, t6
    srli t6, t1, 5
    add  t6, t6, t3
    xor  s3, s3, t6
    sub  t0, t0, s3

    sub  s1, s1, s2
    addi s0, s0, -1
    bnez s0, dec_loop

    sw t0, 0(a0)
    sw t1, 4(a0)

    lw ra, 28(sp)
    lw s0, 24(sp)
    lw s1, 20(sp)
    lw s2, 16(sp)
    lw s3, 12(sp)
    addi sp, sp, 32
    ret
