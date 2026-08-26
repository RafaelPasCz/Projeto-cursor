/* Funcoes para persistir o sistema de arquivos que esta em memoria. */
#ifndef ARMAZENAR_SA_H
#define ARMAZENAR_SA_H

#include <stdio.h>
#include <string.h>

#include "structs.h"

static int estado_sistema_valido_para_salvar(void) {
    if (br_sistema.bytes_por_bloco != BLOCK_SIZE ||
        br_sistema.num_blocos_totais == 0 ||
        br_sistema.num_blocos_tabela_entradas == 0 ||
        br_sistema.num_blocos_secao_dados == 0 ||
        br_sistema.blocos_reservados > br_sistema.num_blocos_totais ||
        br_sistema.num_blocos_secao_dados !=
            br_sistema.num_blocos_totais - br_sistema.blocos_reservados ||
        entrada_sistema == NULL || dados_sistema == NULL) {
        return 0;
    }

    return 1;
}

static int escrever_tudo(FILE *arquivo, const void *origem,
                         size_t tamanho_item, size_t quantidade) {
    return fwrite(origem, tamanho_item, quantidade, arquivo) == quantidade;
}

int salvar_sistema_arquivos(void) {
    FILE *arquivo;
    uint8_t bloco_boot[BLOCK_SIZE] = {0};

    if (!estado_sistema_valido_para_salvar()) {
        fprintf(stderr, "O sistema em memoria nao esta pronto para ser salvo.\n");
        return 1;
    }

    arquivo = fopen(nome_arquivo, "wb");
    if (arquivo == NULL) {
        perror("Erro ao abrir a particao simulada para escrita");
        return 1;
    }

    /* O boot record ocupa o setor 0; os bytes restantes devem ser zero. */
    memcpy(bloco_boot, &br_sistema, sizeof(br_sistema));

    if (!escrever_tudo(arquivo, bloco_boot, sizeof(bloco_boot), 1) ||
        !escrever_tudo(arquivo, entrada_sistema, BLOCK_SIZE,
                       br_sistema.num_blocos_tabela_entradas) ||
        !escrever_tudo(arquivo, dados_sistema, sizeof(bloco),
                       br_sistema.num_blocos_secao_dados)) {
        perror("Erro ao escrever a particao simulada");
        fclose(arquivo);
        return 1;
    }

    if (fclose(arquivo) != 0) {
        perror("Erro ao finalizar a gravacao da particao simulada");
        return 1;
    }

    printf("Sistema de arquivos salvo em: '%s'\n", nome_arquivo);
    return 0;
}

#endif /* ARMAZENAR_SA_H */
