/* Funcoes para copiar um arquivo do disco para a particao simulada. */
#ifndef LEITURA_ARQUIVOS_H
#define LEITURA_ARQUIVOS_H

#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "structs.h"

typedef struct {
    uint32_t primeiro_bloco;
    uint32_t bloco_anterior;
    uint32_t proximo_bloco;
    uint32_t quantidade_blocos;
} alocacao_contigua;

static int bloco_de_dados_valido(uint32_t numero_bloco) {
    return numero_bloco >= br_sistema.blocos_reservados &&
           numero_bloco < br_sistema.num_blocos_totais;
}

static uint32_t ler_proximo_bloco_livre(uint32_t numero_bloco) {
    const uint8_t *conteudo;

    conteudo = dados_sistema[numero_bloco - br_sistema.blocos_reservados].conteudo;
    return (uint32_t)conteudo[0] |
           ((uint32_t)conteudo[1] << 8) |
           ((uint32_t)conteudo[2] << 16) |
           ((uint32_t)conteudo[3] << 24);
}

static void escrever_proximo_bloco_livre(uint32_t numero_bloco,
                                          uint32_t proximo_bloco) {
    uint8_t *conteudo;

    conteudo = dados_sistema[numero_bloco - br_sistema.blocos_reservados].conteudo;
    conteudo[0] = (uint8_t)(proximo_bloco & UINT32_C(0xFF));
    conteudo[1] = (uint8_t)((proximo_bloco >> 8) & UINT32_C(0xFF));
    conteudo[2] = (uint8_t)((proximo_bloco >> 16) & UINT32_C(0xFF));
    conteudo[3] = (uint8_t)((proximo_bloco >> 24) & UINT32_C(0xFF));
}

/*
 * Percorre a lista livre no disco e procura uma sequencia de blocos com
 * enderecos consecutivos. O limite de iteracoes impede loops infinitos caso
 * a lista esteja corrompida. Esta funcao nao altera a lista.
 */
static int encontrar_espaco_contiguo(uint32_t quantidade_blocos,
                                     alocacao_contigua *resultado) {
    uint32_t atual;
    uint32_t anterior = BLOCO_INVALIDO;
    uint32_t anterior_na_sequencia = BLOCO_INVALIDO;
    uint32_t inicio_sequencia = BLOCO_INVALIDO;
    uint32_t ultimo_sequencia = BLOCO_INVALIDO;
    uint32_t tamanho_sequencia = 0;

    if (quantidade_blocos == 0 || dados_sistema == NULL ||
        br_sistema.num_blocos_livres < quantidade_blocos ||
        br_sistema.cabeca_lista == BLOCO_INVALIDO) {
        return 0;
    }

    atual = br_sistema.cabeca_lista;
    for (uint32_t visitados = 0;
         visitados < br_sistema.num_blocos_livres;
         visitados++) {
        uint32_t proximo;

        if (!bloco_de_dados_valido(atual)) {
            return 0;
        }

        proximo = ler_proximo_bloco_livre(atual);
        if (proximo != BLOCO_INVALIDO && !bloco_de_dados_valido(proximo)) {
            return 0;
        }

        if (tamanho_sequencia == 0 || atual != ultimo_sequencia + 1u) {
            inicio_sequencia = atual;
            anterior_na_sequencia = anterior;
            tamanho_sequencia = 1;
        } else {
            tamanho_sequencia++;
        }
        ultimo_sequencia = atual;

        if (tamanho_sequencia == quantidade_blocos) {
            resultado->primeiro_bloco = inicio_sequencia;
            resultado->bloco_anterior = anterior_na_sequencia;
            resultado->proximo_bloco = proximo;
            resultado->quantidade_blocos = quantidade_blocos;
            return 1;
        }

        if (proximo == BLOCO_INVALIDO) {
            break;
        }

        anterior = atual;
        atual = proximo;
    }

    return 0;
}

static void reservar_espaco_contiguo(const alocacao_contigua *alocacao) {
    if (alocacao->bloco_anterior == BLOCO_INVALIDO) {
        br_sistema.cabeca_lista = alocacao->proximo_bloco;
    } else {
        escrever_proximo_bloco_livre(alocacao->bloco_anterior,
                                     alocacao->proximo_bloco);
    }

    br_sistema.num_blocos_livres -= alocacao->quantidade_blocos;
}

static int obter_tamanho_arquivo(const char *caminho, uint32_t *tamanho) {
    FILE *arquivo;
    long tamanho_lido;

    arquivo = fopen(caminho, "rb");
    if (arquivo == NULL) {
        perror("Erro ao abrir o arquivo de origem");
        return 0;
    }

    if (fseek(arquivo, 0L, SEEK_END) != 0 ||
        (tamanho_lido = ftell(arquivo)) < 0 ||
        (uintmax_t)tamanho_lido > UINT32_MAX) {
        fprintf(stderr, "Nao foi possivel obter um tamanho de arquivo valido.\n");
        fclose(arquivo);
        return 0;
    }
    fclose(arquivo);

    *tamanho = (uint32_t)tamanho_lido;
    return 1;
}

static int copiar_conteudo_para_particao(const char *caminho,
                                         uint32_t tamanho_arquivo,
                                         uint32_t quantidade_blocos,
                                         uint32_t *primeiro_bloco) {
    FILE *arquivo;
    uint8_t *conteudo;
    alocacao_contigua alocacao;
    int leitura_completa;

    conteudo = malloc(tamanho_arquivo);
    if (conteudo == NULL) {
        fprintf(stderr, "Erro ao alocar memoria para o arquivo de origem.\n");
        return 0;
    }

    arquivo = fopen(caminho, "rb");
    if (arquivo == NULL) {
        perror("Erro ao abrir o arquivo de origem");
        free(conteudo);
        return 0;
    }

    leitura_completa = fread(conteudo, 1, tamanho_arquivo, arquivo) == tamanho_arquivo;
    if (fclose(arquivo) != 0 || !leitura_completa) {
        fprintf(stderr, "Erro ao ler o arquivo de origem.\n");
        free(conteudo);
        return 0;
    }

    if (!encontrar_espaco_contiguo(quantidade_blocos, &alocacao)) {
        fprintf(stderr, "Nao existe espaco livre contiguo suficiente.\n");
        free(conteudo);
        return 0;
    }

    /* O vetor de blocos e contiguo em memoria, assim como a alocacao encontrada. */
    {
        uint8_t *destino = (uint8_t *)dados_sistema +
            (size_t)(alocacao.primeiro_bloco - br_sistema.blocos_reservados) * BLOCK_SIZE;

        memset(destino, 0, (size_t)alocacao.quantidade_blocos * BLOCK_SIZE);
        memcpy(destino, conteudo, tamanho_arquivo);
    }
    free(conteudo);

    reservar_espaco_contiguo(&alocacao);
    *primeiro_bloco = alocacao.primeiro_bloco;
    return 1;
}

static void preencher_nome_entrada(entrada *destino, const char *caminho) {
    const char *nome_base = caminho;
    const char *ultima_barra = strrchr(caminho, '/');
    const char *ultima_barra_invertida = strrchr(caminho, '\\');
    const char *ponto;
    size_t tamanho_nome;

    if (ultima_barra != NULL && ultima_barra + 1 > nome_base) {
        nome_base = ultima_barra + 1;
    }
    if (ultima_barra_invertida != NULL && ultima_barra_invertida + 1 > nome_base) {
        nome_base = ultima_barra_invertida + 1;
    }

    ponto = strrchr(nome_base, '.');
    if (ponto == nome_base) {
        ponto = NULL;
    }

    tamanho_nome = ponto == NULL ? strlen(nome_base) : (size_t)(ponto - nome_base);
    if (tamanho_nome > NOME_TAMANHO) {
        tamanho_nome = NOME_TAMANHO;
    }
    memcpy(destino->nome, nome_base, tamanho_nome);

    if (ponto != NULL) {
        size_t tamanho_extensao = strlen(ponto + 1);
        if (tamanho_extensao > EXTENSAO_TAMANHO) {
            tamanho_extensao = EXTENSAO_TAMANHO;
        }
        memcpy(destino->ext, ponto + 1, tamanho_extensao);
    }
}

int escrever_entrada(char *caminho) {
    uint32_t tamanho_arquivo;
    uint32_t quantidade_blocos;
    uint32_t primeiro_bloco;
    uint32_t capacidade_tabela;
    uint32_t indice_entrada;

    if (caminho == NULL || entrada_sistema == NULL || dados_sistema == NULL ||
        br_sistema.bytes_por_bloco != BLOCK_SIZE) {
        fprintf(stderr, "Carregue uma particao valida antes de copiar arquivos.\n");
        return 1;
    }

    if (br_sistema.num_blocos_tabela_entradas >
        UINT32_MAX / ENTRADAS_POR_BLOCO) {
        fprintf(stderr, "Tabela de entradas invalida.\n");
        return 1;
    }
    capacidade_tabela = br_sistema.num_blocos_tabela_entradas * ENTRADAS_POR_BLOCO;
    for (indice_entrada = 0; indice_entrada < capacidade_tabela; indice_entrada++) {
        if (entrada_sistema[indice_entrada].status == STATUS_ENTRADA_LIVRE ||
            entrada_sistema[indice_entrada].status == STATUS_ENTRADA_REMOVIDA) {
            break;
        }
    }
    if (indice_entrada == capacidade_tabela) {
        fprintf(stderr, "Nenhuma entrada livre na tabela.\n");
        return 1;
    }

    if (!obter_tamanho_arquivo(caminho, &tamanho_arquivo)) {
        return 1;
    }
    if (tamanho_arquivo == 0) {
        fprintf(stderr, "Arquivos vazios ainda nao sao suportados.\n");
        return 1;
    }

    quantidade_blocos = tamanho_arquivo / BLOCK_SIZE;
    if (tamanho_arquivo % BLOCK_SIZE != 0) {
        quantidade_blocos++;
    }
    if (quantidade_blocos > SIZE_MAX / BLOCK_SIZE) {
        fprintf(stderr, "Arquivo grande demais para esta plataforma.\n");
        return 1;
    }
    if (!copiar_conteudo_para_particao(caminho, tamanho_arquivo,
                                       quantidade_blocos, &primeiro_bloco)) {
        return 1;
    }

    {
        uint8_t status_anterior = entrada_sistema[indice_entrada].status;
        entrada nova_entrada = {0};

        nova_entrada.status = TIPO_ENTRADA_ARQUIVO;
        nova_entrada.tipo = TIPO_ENTRADA_ARQUIVO;
        nova_entrada.primeiro_bloco = primeiro_bloco;
        nova_entrada.tamanho = tamanho_arquivo;
        nova_entrada.numero_blocos_usados = quantidade_blocos;
        preencher_nome_entrada(&nova_entrada, caminho);
        entrada_sistema[indice_entrada] = nova_entrada;

        if (status_anterior == STATUS_ENTRADA_LIVRE) {
            br_sistema.quant_entradas_sistema++;
        }
    }

    printf("Arquivo copiado para a entrada %u.\n", indice_entrada);
    return 0;
}

#endif /* LEITURA_ARQUIVOS_H */
