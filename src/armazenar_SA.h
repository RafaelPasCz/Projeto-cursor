// Funções envolvidas no processo de pegar um sistema de arquivos armazenado na memória, e escrevê-lo no HD

#include "structs.h"

int salvar_sistema_arquivos(){
    // Função que pega as estruturas de dados do sistema de arquivos, e armazena elas no disco

    FILE* file = fopen(nome_arquivo, "wb");
    if(!file){
        printf("\n++--- Erro na abertura do arquivo! Impossível armazenar o sistema.");
        return 1;
    }

    // Primeiro, escrevemos o boot record no arquivo
    char bloco[BLOCK_SIZE] = {0};                       // Inicializa um bloco com zeros
    memcpy(bloco, &br_sistema, sizeof(boot_record));    // Copia o boot record para o bloco
    fwrite(bloco, BLOCK_SIZE, 1, file);                 // Escrevemos o bloco na memória


    // Segundo, escrevemos a tabela de entradas no arquivo, bloco por bloco
    //fwrite(&entrada_sistema[0], BLOCK_SIZE * br_sistema.num_blocos_tabela_entradas, 1, file);
    for (size_t i = 0; i < br_sistema.num_blocos_tabela_entradas; i++) {
        fwrite(&entrada_sistema[i], BLOCK_SIZE, 1, file);
    }


    // Por fim, escrevemos a seção de dados no arquivo, bloco por bloco
    // fwrite(&dados_sistema, BLOCK_SIZE * br_sistema.num_blocos_secao_dados, 1, file);
    for (size_t i = 0; i < br_sistema.num_blocos_secao_dados; i++) {
        fwrite(&dados_sistema[i], BLOCK_SIZE, 1, file);
    }



    fclose(file);

    printf("\n Sistema de arquivos salvo em: '%s'", nome_arquivo);

    return 0;
}