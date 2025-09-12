# Documentación Técnica Proyecto 1

**Autor:** Jorge Gutiérrez Vindas  
**Carnet:** 2023086403  

---

## Estructura del proyecto

```
.
├── Dockerfile
├── run.sh
├── tea_asm_c/           # Implementación C + Ensamblador
│   ├── build.sh
|   ├── degub_test.gdb
|   ├── linker.ld
|   ├── main.c
|   ├── run-qemu.sh
|   ├── startup.s
|   └── tea.s
└── README.md 
└── DOC.md               # Documentación técnica detallada
```
- `Dockerfile` define la imagen que incluye el emulador QEMU y el toolchain RISC-V
- `run.sh` automatiza la construcción de la imagen y la ejecución del contenedor

---

## 1. Introducción

El propósito de este proyecto es implementar el algoritmo **TEA (Tiny Encryption Algorithm)** en un entorno **RISC-V**, utilizando una combinación de código en **C** y **ensamblador**.  

El programa permite:
- Tomar un mensaje de entrada.
- Cifrarlo bloque por bloque usando TEA (64 bits de bloque, 128 bits de clave).
- Imprimir el resultado en formato hexadecimal.
- Descifrar el mensaje y mostrar el texto original.

A diferencia de otras implementaciones que usan **PKCS#7**, en este caso se utilizó **padding con ceros** para rellenar bloques incompletos.

## 2. Descripción de la arquitectura del software

El sistema se organiza en **dos capas principales**:

### 2.1. Capa de aplicación (C)
- Gestiona la lógica de alto nivel: lectura del mensaje, cifrado, descifrado y salida por UART.  
- Implementa:
  - `paddig`: divide el mensaje en bloques de 64 bits y rellena con `0x00` cuando es necesario.
  - Funciones auxiliares: `print_char`, `print_string`, `print_hex`, `print_block_as_text`, `my_strlen`.
- Actúa como **interfaz** entre el usuario y el cifrado de bajo nivel.

### 2.2. Capa de cifrado en bajo nivel (ASM)
- Incluye `tea_encrypt_asm` y `tea_decrypt_asm`, implementadas en ensamblador RISC-V.
- Cada rutina:
  - Recibe un bloque de 64 bits y una clave de 128 bits.
  - Ejecuta 32 rondas del algoritmo TEA.
  - Devuelve el resultado cifrado o descifrado al programa en C.
- Usa registros temporales y pila para preservar el contexto.

### 2.3. Interacción entre capas
- El `main` invoca las funciones de ensamblador, pasando punteros a bloques y clave.
- El cifrado y descifrado recorren los bloques del mensaje.
- Los resultados se imprimen por UART, garantizando comunicación con el usuario.

---


## 3. Diagramas de diseño

### 3.1. Diagrama de flujo de la función `paddig` (C)
Este diagrama muestra cómo se construye un bloque de 64 bits (8 bytes) a partir del mensaje original.  
El proceso incluye:

1. Recibir el string de entrada, el `offset` y el puntero al bloque donde se va a copiar.  
2. Crear un buffer temporal de 8 bytes inicializado en cero.  
3. Calcular la longitud de datos desde el `offset`.  
4. Si la longitud es mayor a 8, se trunca para ajustarse al tamaño fijo del bloque.  
5. Copiar los datos al buffer temporal.  
6. Formar los enteros `block[0]` y `block[1]` con los 8 bytes.  

Esto garantiza que **cada bloque tenga siempre 64 bits listos para el cifrado**.

👉 *(Diagrama 1 aquí)*  

---

### 3.2. Diagrama de flujo de la función `print_block_as_text` (C)
Este diagrama representa el proceso de reconstrucción del mensaje legible:

1. Se recibe un bloque cifrado/descifrado de 64 bits y la cantidad de bytes a mostrar.  
2. Se extraen los 8 bytes del bloque en un buffer.  
3. Si se pidió imprimir más de 8 bytes, se ajusta a 8.  
4. Se agrega un terminador nulo (`\0`) para convertirlo en string en C.  
5. Se imprime el buffer como texto ASCII.  

De esta manera, se puede reconstruir el mensaje original después de descifrar.

👉 *(Diagrama 2 aquí)*  

---

### 3.3. Diagrama de flujo de `tea_encrypt_asm` (ASM)
Este diagrama describe el **cifrado de un bloque en ensamblador RISC-V**:

1. Se guardan los registros necesarios en la pila.  
2. Se cargan los valores iniciales de `v0`, `v1` y la clave de 128 bits (`k0..k3`).  
3. Se inicializan las constantes:  
   - `sum = 0`  
   - `delta = 0x9E3779B9`  
   - `rondas = 32`  
4. En cada ronda:  
   - Se incrementa `sum` en `delta`.  
   - Se actualiza `v0` aplicando la fórmula de TEA con `v1`, `k0`, `k1` y `sum`.  
   - Se actualiza `v1` aplicando la fórmula con `v0`, `k2`, `k3` y `sum`.  
   - Se decrementa el contador de rondas.  
5. Al finalizar, `v0` y `v1` (ya cifrados) se guardan en memoria.  
6. Se restauran los registros y se retorna al programa en C.  

👉 *(Diagrama 3 aquí)*  

---

### 3.4. Diagrama de flujo de `tea_decrypt_asm` (ASM)
El descifrado en ensamblador es el proceso inverso:  

1. Se guardan los registros en la pila.  
2. Se cargan los valores iniciales de `v0`, `v1` y la clave `k0..k3`.  
3. Se inicializan:  
   - `sum = delta * 32`  
   - `delta = 0x9E3779B9`  
   - `rondas = 32`  
4. En cada ronda:  
   - Se actualiza `v1` restando la parte correspondiente con `v0`, `k2`, `k3` y `sum`.  
   - Se actualiza `v0` restando la parte correspondiente con `v1`, `k0`, `k1` y `sum`.  
   - Se reduce `sum` restándole `delta`.  
   - Se decrementa el contador de rondas.  
5. Al terminar, los valores originales de `v0` y `v1` se guardan en memoria.  
6. Se restauran los registros y se retorna al programa en C.  

👉 *(Diagrama 4 aquí)*  

---

## 4. Implementación

### 4.1. Padding con ceros
Se implementó una función en C que rellena los bloques de 8 bytes (64 bits).  
Si el bloque es menor a 8 bytes, los espacios faltantes se completan con `0x00`.

Ejemplo:  
Mensaje = `"ABC"`  
Bloque (hex) = `41 42 43 00 00 00 00 00`

### 4.2. Programa en C
Responsabilidades:
- Funciones auxiliares: `print_char`, `print_string`, `print_hex`.
- Conversión de mensajes en bloques de 64 bits (`paddig`).
- Impresión de bloques en formato texto y hexadecimal.
- Ejecución de cifrado y descifrado bloque por bloque.

### 4.3. Programa en ASM
Funciones principales:
- **tea_encrypt_asm**:  
  - Itera 32 rondas.
  - Usa operaciones `slli`, `srli`, `xor`, `add`, `sub` para implementar la fórmula del algoritmo TEA.
- **tea_decrypt_asm**:  
  - Inicializa la variable `sum` en `delta * 32`.
  - Realiza 32 rondas de resta aplicando las mismas operaciones en orden inverso.

## 5. Validación y Pruebas
👉 *(Aquí podrás documentar los casos de prueba con ejemplos de entrada y salida, tanto cifrado como descifrado, capturas de pantalla o logs de ejecución)*

---

## 6. Instrucciones para compilar, ejecutar y utilizar el sistema
👉 *(En este apartado debes incluir los comandos necesarios para compilar con `riscv64-unknown-elf-gcc`, enlazar, cargar en el simulador/emulador, y correr el programa. Además, explica brevemente cómo se utiliza el sistema)*

---

## 7. Conclusión

La implementación del algoritmo TEA en el entorno RISC-V utilizando una combinación de C y ensamblador permitió aplicar de manera práctica los conceptos estudiados en el curso de Arquitectura de Computadores I.  

El proyecto evidenció la importancia de separar responsabilidades entre la lógica de alto nivel en C y las operaciones críticas en bajo nivel, asegurando así una correcta interacción entre capas y un control explícito sobre los registros y la pila. Además, se logró comprender de forma más profunda cómo un algoritmo criptográfico puede adaptarse a un entorno con recursos limitados, manteniendo la eficiencia computacional y la claridad estructural.  

El uso de QEMU y Docker como herramientas de apoyo no solo facilitó la portabilidad y depuración del sistema, sino que también brindó un entorno controlado que simuló las condiciones reales de ejecución en hardware RISC-V. Con ello, se reforzó la comprensión tanto de la arquitectura como de las técnicas de programación en ensamblador.  

En conclusión, el desarrollo de este proyecto permitió consolidar conocimientos de criptografía, programación en bajo nivel y arquitectura de computadores, mostrando cómo la teoría puede integrarse efectivamente con la práctica en un caso de estudio real como lo es el algoritmo TEA.  

---

## 8. Referencias
👉 *(Espacio reservado para referencias en formato APA, IEEE u otro requerido)*