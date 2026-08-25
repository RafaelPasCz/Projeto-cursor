/* Estruturas que representam o formato binario persistido no disco. */
#ifndef STRUCTS_H
#define STRUCTS_H

#include <stdint.h>

#define BLOCK_SIZE 512u
#define ENTRADA_TAMANHO 32u
#define ENTRADAS_POR_BLOCO (BLOCK_SIZE / ENTRADA_TAMANHO)

#define BLOCO_INVALIDO UINT32_C(0xFFFFFFFF)

#define STATUS_ENTRADA_LIVRE UINT8_C(0x00)
#define STATUS_ENTRADA_REMOVIDA UINT8_C(0xE5)
#define TIPO_ENTRADA_ARQUIVO UINT8_C(0x01)
#define TIPO_ENTRADA_DIRETORIO UINT8_C(0x02)

#define NOME_TAMANHO 12u
#define EXTENSAO_TAMANHO 4u

#if defined(__GNUC__) || defined(__clang__)
#define FS_PACKED __attribute__((packed))
#else
#define FS_PACKED
#endif

/*
 * Boot record (setor 0). Os campos seguem os offsets definidos no AGENTS.md.
 * A cabeca da lista de livres fica em 0x1C, apos os campos documentados.
 */
typedef struct FS_PACKED {
    uint16_t bytes_por_bloco;              /* 0x00 */
    uint16_t blocos_reservados;             /* 0x02 */
    uint32_t num_blocos_livres;             /* 0x04 */
    uint32_t num_blocos_tabela_entradas;    /* 0x08 */
    uint32_t num_blocos_secao_dados;        /* 0x0C */
    uint32_t num_blocos_totais;             /* 0x10 */
    uint32_t num_blocos_diretorio_raiz;     /* 0x14 */
    uint32_t quant_entradas_sistema;        /* 0x18 */
    uint32_t cabeca_lista;                  /* 0x1C */
} boot_record;

/* Entrada da tabela: exatamente 32 bytes. */
typedef struct FS_PACKED {
    uint8_t status;                         /* 0x00 */
    char nome[NOME_TAMANHO];                /* 0x01 */
    char ext[EXTENSAO_TAMANHO];             /* 0x0D */
    uint8_t tipo;                           /* 0x11 */
    uint32_t primeiro_bloco;                /* 0x12 */
    uint32_t tamanho;                       /* 0x16 */
    uint32_t numero_blocos_usados;          /* 0x1A */
    uint16_t padding;                       /* 0x1E */
} entrada;

/* Bloco em memoria da secao de dados. */
typedef struct {
    uint8_t conteudo[BLOCK_SIZE];
} bloco;

/* Formato de um bloco enquanto pertence a lista de blocos livres. */
typedef struct FS_PACKED {
    uint32_t proximo_bloco;
    uint8_t reservado[BLOCK_SIZE - sizeof(uint32_t)];
} bloco_livre;

/* Impede que uma mudanca quebre silenciosamente o formato em disco. */
typedef char verificacao_tamanho_boot_record[(sizeof(boot_record) == 32u) ? 1 : -1];
typedef char verificacao_tamanho_entrada[(sizeof(entrada) == ENTRADA_TAMANHO) ? 1 : -1];
typedef char verificacao_tamanho_bloco_livre[(sizeof(bloco_livre) == BLOCK_SIZE) ? 1 : -1];

/* Estado em memoria; as definicoes ficam em trabalho.c. */
extern char nome_arquivo[256];
extern boot_record br_sistema;
extern entrada *entrada_sistema;
extern bloco *dados_sistema;

#endif /* STRUCTS_H */
