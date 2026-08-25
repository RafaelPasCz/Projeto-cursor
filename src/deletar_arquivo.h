#include "structs.h"


void deletar_arquivo(uint32_t index) {
    // Função responsável por deletar um arquivo do sistema de arquivos.
    // Recebe o índice do arquivo a ser deletado, e a função marca a entrada como excluída
    // e libera os blocos usados pelo arquivo.

    // Verifica se o índice é válido e se a entrada está em uso
    if (index >=  br_sistema.num_blocos_tabela_entradas * 16) {
        printf("\nÍndice inválido!\n");
        return;
    }

    if (entrada_sistema[index].status == 0x00 || entrada_sistema[index].status == 0xE5) {
        printf("\nNenhum arquivo encontrado no índice especificado.\n");
        return;
    }

    // Marcamos a entrada como excluída
    entrada_sistema[index].status = 0xE5;
    printf("\nArquivo no indice %d marcado como excluído.\n", index);


    // Libera os blocos usados pelo arquivo
    uint32_t bloco_inicio_deletado = entrada_sistema[index].primeiro_bloco;             // Primeiro bloco do arquivo deletado
    uint32_t numero_blocos_deletado = entrada_sistema[index].numero_blocos_usados;      // numero de blocos do arquivo deletado
    uint32_t bloco_fim_deletado = bloco_inicio_deletado + numero_blocos_deletado - 1;   // Último bloco do arquivo deletado

    // Agora, o que dá pra fazer:
    //  Percorrer a lista de blocos livres
    //  Ter em mãos o primeiro bloco do arquivo deletado
    //  Quando fizermos um "pulo" que for passar do primeiro bloco, saimos do loop
    //  Pegamos o bloco_livre_anterior, fazemos ele apontar pro primeiro bloco
    //  Primeiro bloco aponta pro segundo, terceiro, etc
    //  Último aponta para onde o bloco_livre_anterior apontava.
    

    uint32_t cabeca_lista = br_sistema.cabeca_lista;        // Primeiro bloco livre
    int offset = 1 + br_sistema.num_blocos_tabela_entradas; // Offset a ser utilizado quando formos acessar o vetor dados_sistema[]

    uint32_t ptr_bloco = cabeca_lista;                      // Armazena o bloco sendo analizado atualmente
    uint32_t ptr_inicio_atual = cabeca_lista;               // Armazena o começo do conjunto contíguo de blocos atual 
    uint32_t ptr_fim_anterior = cabeca_lista;               // Armazena o final do conjunto contíguo de blocos anterior
    uint32_t ptr = 0;                                       // Usado em comparações


    ptr = cabeca_lista;                                     // Pegamos o ponteiro armazenado pela cabeça da lista


    // Agora, caminhamos pela lista de blocos livres
    while(ptr_bloco != 0xFFFFFFFF){         // Percorremos a lista até chegar no último bloco livre.
        // Primeiro vemos se nós já passamos da localização dos blocos deletados
        if(ptr > bloco_inicio_deletado){
            printf("\nPassou! ptr %i, e bloco %i", ptr, bloco_inicio_deletado);
            break;                          // Nós já passamos dele! Saímos do loop
        }

        // Pegamos os 4 primeiros bytes do bloco atual, para obter o ponteiro para o próximo bloco
        ptr = 0;                            // Como na aritmetica abaixo usamos um OR, devemos primeiro zerar ptr
        for (int i = 0; i < 4; i++) {
            ptr |= ((uint32_t)(uint8_t)dados_sistema[ptr_bloco - offset].conteudo[i]) << (i * 8);
        }

        // Seguimos para o próximo bloco
        if(ptr == (ptr_bloco + 1)){         // Se o próximo bloco livre está grudado no bloco atual...
            ptr_bloco = ptr;                // Avançamos para o próximo bloco
        
        }else{                              // Se não...
            ptr_fim_anterior = ptr_bloco;   // Armazenamos o final do conjunto contíguo de blocos anterior
            ptr_inicio_atual = ptr;         // Armazenamos o começo da próxima sequência de blocos contígua
            ptr_bloco  = ptr;               // Avançaos para o próximo bloco
        }
        printf("\nCabeca: %i, ptr: %i, ptr_bloco: %i", cabeca_lista, ptr, ptr_bloco);
    }

    if(bloco_inicio_deletado < cabeca_lista){               // Se o primeiro bloco do arquivo deletado vier antes do primeiro da lista...
        br_sistema.cabeca_lista = bloco_inicio_deletado;
    }

    // Obtivemos os valores que precisávamos. Agora basta reconectar os ponteiros
    // Agora, temos:
        //  ptr_inicio_atual: 
        //  ptr_bloco       : o último bloco da sequência contígua de blocos que vem antes do arquivo deletado
        //  ptr             : o primeiro bloco da sequência contígua de blocos que vem depois do arquivo deletado
        //  ptr_fim_anterior: o último bloco da sequência 2 antes do arquivo

    // Então, agora vamos religar os ponteiros
    
    // Primeiro:
        //  O bloco representado por ptr_bloco
        //  O ponteiro dele deve ser alterado para bloco_inicio_deletado
    if(ptr != ptr_bloco){   // Não ativa no caso específico aonde o arquivo deletado se torna o novo header da lista 
        memcpy(dados_sistema[ptr_bloco - offset].conteudo, &bloco_inicio_deletado, sizeof(uint32_t));   
    }
        // Agora, o arquivo deletado faz parte da lista de blocos livres
        
    // Segundo:
    //  Dentro do arquivo deletado, conectamos os primeiros 4 bytes, fazendo um ponteiro que vai de um para o outro
    for (uint32_t i = bloco_inicio_deletado; i < bloco_fim_deletado; i++) {
        uint32_t proximo_bloco = i + 1;
        memcpy(dados_sistema[i - offset].conteudo, &proximo_bloco, sizeof(uint32_t));
    }

    // Terceiro:
        //  O último bloco do arquivo deletado vai apontar para ptr
    memcpy(dados_sistema[bloco_fim_deletado - offset].conteudo, &ptr, sizeof(uint32_t));

    // Atualiza o número de blocos livres no boot record
    br_sistema.num_blocos_livres += entrada_sistema[index].numero_blocos_usados;


    // Caso específico: se o ultimo bloco do arquivo deletado estiver no finaldo sistema de arquivos, ele não colocaria 0xFFFFFFFF
    if(bloco_fim_deletado == (br_sistema.num_blocos_totais - 1)){
        uint32_t fim_lista = 0xFFFFFFFF;
        memcpy(dados_sistema[bloco_fim_deletado - offset].conteudo, &fim_lista, sizeof(uint32_t));
    }

    // Limpa os dados da entrada na tabela de entradas
    // memset(&entrada_sistema[index], 0, sizeof(entrada));

    printf("\nBlocos liberados e lista de blocos livres atualizada.\n");
}