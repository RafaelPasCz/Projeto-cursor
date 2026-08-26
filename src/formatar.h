/* Funcoes para criar uma nova particao no formato do sistema de arquivos. */
#ifndef FORMATAR_H
#define FORMATAR_H

#include <limits.h>
#include <stdio.h>
#include <string.h>

#include "structs.h"

static void descartar_linha_entrada(void) {
    int caractere;

    do {
        caractere = getchar();
    } while (caractere != '\n' && caractere != EOF);
}

static int ler_inteiro_sem_sinal(const char *mensagem, uint64_t *valor) {
    unsigned long long entrada;

    printf("%s", mensagem);
    if (scanf("%llu", &entrada) != 1) {
        descartar_linha_entrada();
        return 0;
    }
    descartar_linha_entrada();

    *valor = (uint64_t)entrada;
    return 1;
}

static void escrever_uint32_little_endian(uint8_t destino[4], uint32_t valor) {
    destino[0] = (uint8_t)(valor & UINT32_C(0xFF));
    destino[1] = (uint8_t)((valor >> 8) & UINT32_C(0xFF));
    destino[2] = (uint8_t)((valor >> 16) & UINT32_C(0xFF));
    destino[3] = (uint8_t)((valor >> 24) & UINT32_C(0xFF));
}

/*
 * Cada bloco livre armazena, nos quatro primeiros bytes, o endereco do proximo
 * bloco. O ultimo bloco recebe BLOCO_INVALIDO para encerrar a lista ligada.
 */
static int escrever_lista_blocos_livres(FILE *arquivo,
                                        uint32_t primeiro_bloco,
                                        uint32_t quantidade_blocos) {
    uint8_t bloco[BLOCK_SIZE];

    for (uint32_t indice = 0; indice < quantidade_blocos; indice++) {
        uint32_t bloco_atual = primeiro_bloco + indice;
        uint32_t proximo_bloco = (indice + 1u == quantidade_blocos)
            ? BLOCO_INVALIDO
            : bloco_atual + 1u;

        memset(bloco, 0, sizeof(bloco));
        escrever_uint32_little_endian(bloco, proximo_bloco);

        if (fwrite(bloco, sizeof(bloco), 1, arquivo) != 1) {
            return 0;
        }
    }

    return 1;
}

int formatar(void) {
    boot_record novo_boot = {0};
    uint8_t bloco_vazio[BLOCK_SIZE] = {0};
    uint64_t tamanho_disco_bytes;
    uint64_t quantidade_entradas;
    uint64_t total_blocos;
    uint64_t blocos_tabela;
    FILE *arquivo;

    for (;;) {
        printf("\n=--=- Informacoes de formatacao\n");

        if (!ler_inteiro_sem_sinal("=-- Tamanho do disco, em bytes: ",
                                   &tamanho_disco_bytes) ||
            !ler_inteiro_sem_sinal("=-- Numero de entradas: ",
                                   &quantidade_entradas)) {
            printf("Entrada invalida. Tente novamente.\n");
            continue;
        }

        if (quantidade_entradas > UINT64_MAX - (ENTRADAS_POR_BLOCO - 1u)) {
            printf("Numero de entradas grande demais.\n");
            continue;
        }

        total_blocos = tamanho_disco_bytes / BLOCK_SIZE;
        blocos_tabela = (quantidade_entradas + ENTRADAS_POR_BLOCO - 1u) /
                         ENTRADAS_POR_BLOCO;

        if (quantidade_entradas == 0 ||
            tamanho_disco_bytes % BLOCK_SIZE != 0 ||
            total_blocos > UINT32_MAX ||
            blocos_tabela == 0 ||
            blocos_tabela > UINT32_MAX ||
            1u + blocos_tabela > UINT16_MAX ||
            total_blocos < 3u ||
            1u + blocos_tabela >= total_blocos) {
            printf("Dados invalidos: e necessario haver boot, tabela e ao menos um bloco de dados.\n");
            continue;
        }

        break;
    }

    novo_boot.bytes_por_bloco = BLOCK_SIZE;
    novo_boot.num_blocos_totais = (uint32_t)total_blocos;
    novo_boot.num_blocos_tabela_entradas = (uint32_t)blocos_tabela;
    /* O diretorio raiz e representado pela propria tabela de entradas. */
    novo_boot.num_blocos_diretorio_raiz = (uint32_t)blocos_tabela;
    novo_boot.blocos_reservados =
        1u + novo_boot.num_blocos_tabela_entradas;
    novo_boot.num_blocos_secao_dados =
        novo_boot.num_blocos_totais - novo_boot.blocos_reservados;
    novo_boot.num_blocos_livres = novo_boot.num_blocos_secao_dados;
    novo_boot.quant_entradas_sistema = 0;
    novo_boot.cabeca_lista = novo_boot.blocos_reservados;

    arquivo = fopen(nome_arquivo, "wb");
    if (arquivo == NULL) {
        perror("Erro ao criar a particao simulada");
        return 1;
    }

    memcpy(bloco_vazio, &novo_boot, sizeof(novo_boot));
    if (fwrite(bloco_vazio, sizeof(bloco_vazio), 1, arquivo) != 1) {
        perror("Erro ao escrever o boot record");
        fclose(arquivo);
        return 1;
    }

    memset(bloco_vazio, 0, sizeof(bloco_vazio));
    for (uint32_t indice = 0;
         indice < novo_boot.num_blocos_tabela_entradas;
         indice++) {
        if (fwrite(bloco_vazio, sizeof(bloco_vazio), 1, arquivo) != 1) {
            perror("Erro ao inicializar a tabela de entradas");
            fclose(arquivo);
            return 1;
        }
    }

    if (!escrever_lista_blocos_livres(arquivo, novo_boot.cabeca_lista,
                                      novo_boot.num_blocos_livres)) {
        perror("Erro ao inicializar a lista de blocos livres");
        fclose(arquivo);
        return 1;
    }

    if (fclose(arquivo) != 0) {
        perror("Erro ao finalizar a formatacao");
        return 1;
    }

    printf("Particao formatada com sucesso: '%s'\n", nome_arquivo);
    return 0;
}

#endif /* FORMATAR_H */
