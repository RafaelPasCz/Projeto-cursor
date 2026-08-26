/* Funcoes para carregar uma particao simulada do disco para a memoria. */
#ifndef CARREGAR_SA_H
#define CARREGAR_SA_H

#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "structs.h"

static int boot_record_valido(const boot_record *boot) {
    uint32_t primeiro_bloco_dados;

    if (boot->bytes_por_bloco != BLOCK_SIZE ||
        boot->num_blocos_totais == 0 ||
        boot->num_blocos_tabela_entradas == 0 ||
        boot->num_blocos_tabela_entradas > UINT32_MAX / ENTRADAS_POR_BLOCO ||
        boot->blocos_reservados < 1 ||
        boot->blocos_reservados > boot->num_blocos_totais ||
        boot->num_blocos_secao_dados > boot->num_blocos_totais ||
        boot->quant_entradas_sistema >
            boot->num_blocos_tabela_entradas * ENTRADAS_POR_BLOCO) {
        return 0;
    }

    primeiro_bloco_dados = boot->blocos_reservados;
    if (primeiro_bloco_dados + boot->num_blocos_secao_dados !=
        boot->num_blocos_totais ||
        boot->num_blocos_livres > boot->num_blocos_secao_dados) {
        return 0;
    }

    if (boot->num_blocos_livres > 0 &&
        (boot->cabeca_lista < primeiro_bloco_dados ||
         boot->cabeca_lista >= boot->num_blocos_totais)) {
        return 0;
    }

    return 1;
}

static int ler_regiao_do_arquivo(FILE *arquivo, long deslocamento,
                                 void *destino, size_t tamanho) {
    if (fseek(arquivo, deslocamento, SEEK_SET) != 0) {
        return 0;
    }

    return fread(destino, 1, tamanho, arquivo) == tamanho;
}

int carregar_boot(void) {
    FILE *arquivo;
    boot_record boot_lido;

    arquivo = fopen(nome_arquivo, "rb");
    if (arquivo == NULL) {
        perror("Erro ao abrir a particao simulada");
        return 1;
    }

    if (!ler_regiao_do_arquivo(arquivo, 0L, &boot_lido, sizeof(boot_lido))) {
        perror("Erro ao ler o boot record");
        fclose(arquivo);
        return 1;
    }
    fclose(arquivo);

    if (!boot_record_valido(&boot_lido)) {
        fprintf(stderr, "Boot record invalido ou incompativel.\n");
        return 1;
    }

    br_sistema = boot_lido;
    return 0;
}

int carregar_lista_blocos_livres(void) {
    FILE *arquivo;
    entrada *nova_tabela;
    size_t tamanho_tabela;

    if (!boot_record_valido(&br_sistema)) {
        fprintf(stderr, "Nao ha um boot record valido carregado.\n");
        return 1;
    }

    if (br_sistema.num_blocos_tabela_entradas > SIZE_MAX / BLOCK_SIZE) {
        fprintf(stderr, "Tabela de entradas grande demais.\n");
        return 1;
    }
    tamanho_tabela = (size_t)br_sistema.num_blocos_tabela_entradas * BLOCK_SIZE;

    nova_tabela = malloc(tamanho_tabela);
    if (nova_tabela == NULL) {
        fprintf(stderr, "Erro ao alocar a tabela de entradas.\n");
        return 1;
    }

    arquivo = fopen(nome_arquivo, "rb");
    if (arquivo == NULL) {
        perror("Erro ao abrir a particao simulada");
        free(nova_tabela);
        return 1;
    }

    if (!ler_regiao_do_arquivo(arquivo, (long)BLOCK_SIZE, nova_tabela,
                               tamanho_tabela)) {
        perror("Erro ao ler a tabela de entradas");
        fclose(arquivo);
        free(nova_tabela);
        return 1;
    }
    fclose(arquivo);

    free(entrada_sistema);
    entrada_sistema = nova_tabela;
    return 0;
}

int carregar_secao_dados(void) {
    FILE *arquivo;
    bloco *novos_dados;
    size_t tamanho_secao_dados;
    uint64_t deslocamento;

    if (!boot_record_valido(&br_sistema)) {
        fprintf(stderr, "Nao ha um boot record valido carregado.\n");
        return 1;
    }

    if (br_sistema.num_blocos_secao_dados > SIZE_MAX / BLOCK_SIZE) {
        fprintf(stderr, "Secao de dados grande demais.\n");
        return 1;
    }
    tamanho_secao_dados = (size_t)br_sistema.num_blocos_secao_dados * BLOCK_SIZE;
    deslocamento = (uint64_t)br_sistema.blocos_reservados * BLOCK_SIZE;

    if (deslocamento > LONG_MAX) {
        fprintf(stderr, "Deslocamento da secao de dados nao suportado.\n");
        return 1;
    }

    novos_dados = malloc(tamanho_secao_dados);
    if (novos_dados == NULL) {
        fprintf(stderr, "Erro ao alocar a secao de dados.\n");
        return 1;
    }

    arquivo = fopen(nome_arquivo, "rb");
    if (arquivo == NULL) {
        perror("Erro ao abrir a particao simulada");
        free(novos_dados);
        return 1;
    }

    if (!ler_regiao_do_arquivo(arquivo, (long)deslocamento, novos_dados,
                               tamanho_secao_dados)) {
        perror("Erro ao ler a secao de dados");
        fclose(arquivo);
        free(novos_dados);
        return 1;
    }
    fclose(arquivo);

    free(dados_sistema);
    dados_sistema = novos_dados;
    return 0;
}

int carregar_sistema_arquivos(void) {
    if (carregar_boot() != 0) {
        return 1;
    }
    if (carregar_lista_blocos_livres() != 0) {
        return 1;
    }
    if (carregar_secao_dados() != 0) {
        return 1;
    }

    printf("Particao carregada com sucesso: '%s'\n", nome_arquivo);
    return 0;
}

#endif /* CARREGAR_SA_H */
