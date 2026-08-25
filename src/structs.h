// Structs, variáveis globais, e bibliotecas compartilhadas entre todos os headers

#ifndef HEADER_FILE
#define HEADER_FILE
#define BLOCK_SIZE 512

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <math.h>


char nome_arquivo[256];             // Arquivo a ser usado para simular o sistema de arquivos


struct Boot_record{
    unsigned short bytes_por_bloco;             // 2 bytes
    unsigned short blocos_reservados;           // 2 bytes
    unsigned int num_blocos_livres;             // 4 bytes 
    unsigned int num_blocos_tabela_entradas;    // 4 bytes 
    unsigned int num_blocos_secao_dados;        // 4 bytes
    unsigned int num_blocos_totais;             // 4 bytes
    unsigned int foo;                           // 4 bytes, espaço vazio
    uint32_t cabeca_lista;                      // 4 bytes      
    unsigned int quant_entradas_sistema;        // 4 bytes,     
} __attribute__((packed));


struct Entrada{
    unsigned char status;                       // 1 byte para status
    char nome[12];                              // 12 bytes, nome do arquivo
    char ext[4];                                // 4 bytes, extensão
    char tipo;
    unsigned int primeiro_bloco;
    unsigned int tamanho;
    unsigned int numero_blocos_usados;
    unsigned short padding;
} __attribute__((packed));


struct Bloco{
    char conteudo[BLOCK_SIZE];
} __attribute__((packed));

typedef struct Boot_record boot_record;
typedef struct Entrada entrada;
typedef struct Bloco bloco;


boot_record br_sistema;             // boot record do sistema de arquivos em memória
entrada *entrada_sistema = NULL;    // Lista que armazena a tabela de entradas do sistema de arquivos em memória (alocado com malloc)
bloco   *dados_sistema = NULL;      // Lista que armazena os blocos da seção de dados do sistema de arquivos em memória

#endif