// Funções envolvidas no processo de cópia de arquivos do Disco para o Sistema de Arquivos

#include "structs.h"
#include "strings.h"


int procurar_espaco_livre(uint32_t tamanho_blocos){
    //  Recebe o tamanho (em blocos) da sequência contígua de blocos a ser armazenado na seção de dados
    //  Ontem o ponteiro do começo da seção de dados.
    //  Caminha pela lista de blocos livres de seção de dados
        //  Em busca de um conjunto contíguo de blocos de tamanho suficiente para armazenar o arquivo requisitado 
    //  Se encontrar um conjunto contíguo de blocos suficiente:
        //  Reconecta os ponteiros da lista
        //  Faz alguns tratamentos de dados
    
    // Retorna o ponteiro para o primeiro bloco da nossa sequência
    // Retorna -1 se não houver espaço livre contíguo o suficiente

    //int blocos_dados = br_sistema.num_blocos_secao_dados;   // Tamanho em blocos da seção de dados

    printf("\naq1");

    uint32_t cabeca_lista = br_sistema.cabeca_lista;        // Primeiro bloco livre
    uint32_t sequencia = 1;                                 // Contador que armazena a sequência atual de blocos livres contíguos
    printf("\naq2");
    int offset = 1 + br_sistema.num_blocos_tabela_entradas; // Offset a ser utilizado quando formos acessar o vetor dados_sistema[]
    printf("\naq3");
    uint32_t ptr_bloco = cabeca_lista;                      // Armazena o bloco sendo analizado atualmente
    uint32_t ptr_inicio_atual = cabeca_lista;               // Armazena o começo do conjunto contíguo de blocos atual 
    uint32_t ptr_fim_anterior = cabeca_lista;               // Armazena o final do conjunto contíguo de blocos anterior
    uint32_t ptr = 0;                                       // Usado em comparações
    printf("\naq4");
    // dados_sistema[]

    // Primeiro, checamos se o sistema tem blocos livres suficientes para armazenar o arquivo
    if (br_sistema.num_blocos_livres < tamanho_blocos){ // Se a qtd de blocos requisitados for maior que a qtd de blocos livres, erro
        printf("\naq5");
        printf("\nERRO: Sistema não há blocos livres o suficiente! Nem se o sistema for desfragmentado.");
        return -1;
    }
    printf("\naq6");
    printf("\nCabecalista eh %i", ptr_bloco);
    printf("\nOffset eh %i", offset);

    printf("\n1 eh %i", dados_sistema[ptr_bloco - offset].conteudo[1]);
    printf("\n2 eh %i", dados_sistema[ptr_bloco - offset].conteudo[2]);
    printf("\n3 eh %i", dados_sistema[ptr_bloco - offset].conteudo[3]);
    printf("\n4 eh %i", dados_sistema[ptr_bloco - offset].conteudo[4]);

    for (int i = 0; i < 4; i++) {           // Pegamos o ponteiro armazenado pela cabeça da lista. Isso estava aqui antes?
        ptr |= ((uint32_t)(uint8_t)dados_sistema[ptr_bloco - offset].conteudo[i]) << (i * 8);
    }

    //memcpy(&ptr, dados_sistema[ptr_bloco - offset].conteudo, sizeof(uint32_t));

    printf("\n ptr eh %i", ptr);

    printf("\n1");

    // Agora, caminhamos pela lista de blocos livres
    while(ptr_bloco != 0xFFFFFFFF){         // Percorremos a lista até chegar no último bloco livre.
        printf("\n2");
        // Primeiro vemos se nós já encontramos uma sequência de blocos contíguos grande o suficiente.
        if(sequencia == tamanho_blocos){
            printf("\n3");
            break;                          // Encontramos nossa sequência de blocos contíguos! Saímos do loop
        }
        // Pegamos os 4 primeiros bytes do bloco atual, para obter o ponteiro para o próximo bloco
        ptr = 0;
        for (int i = 0; i < 4; i++) {
            printf("\n4");
            ptr |= ((uint32_t)(uint8_t)dados_sistema[ptr_bloco - offset].conteudo[i]) << (i * 8);
        }
        printf("\n5");

        // Seguimos para o próximo bloco
        if(ptr == (ptr_bloco + 1)){         // Se o próximo bloco livre está grudado no bloco atual...
            printf("\n6");
            ptr_bloco = ptr;                // Avançamos para o próximo bloco
            sequencia = sequencia + 1;      // Aumentamos em 1 a sequência
        
        }else{                              // Se não...
            printf("\n7");
            ptr_fim_anterior = ptr_bloco;   // Armazenamos o final do conjunto contíguo de blocos anterior
            ptr_inicio_atual = ptr;         // Armazenamos o começo da próxima sequência de blocos contígua
            ptr_bloco  = ptr;               // Avançaos para o próximo bloco
            sequencia  = 1;                 // Reiniciamos o contador de sequência
        }
        printf("\n8");
        printf("\nCabeca: %i, sequencia: %i, ptr: %i, ptr_bloco: %i", cabeca_lista, sequencia, ptr, ptr_bloco);
    }

    printf("\n9");
    if(sequencia != tamanho_blocos){        // Se não tivermos encontrado uma sequência contígua de blocos livres...
        printf("\n\n+++_---------- Não foi encontrada uma sequência contígua de blocos livres! Mas você pode tentar desfragmentar o disco!");
        return -1;
    }


    // Agora, temos:
        //  ptr_inicio_atual: o primeiro bloco da nossa sequência contígua de blocos
        //  ptr_bloco       : o último bloco da nossa sequência contígua de blocos
        //  ptr_fim_anterior: o último bloco da sequência anterior de blocos
        //  ptr             : bloco livre que vem depois da nossa sequência contígua de blocos


    // Então, agora vamos religar os ponteiros
    if(ptr_fim_anterior == cabeca_lista){   // Se o arquivo vai ser alocado no lugar que era a cabeça da lista de blocos livres...
        printf("\n10");
        printf("\nArquivo vai ser alocado na cabeca da lista! cabeca eh: %i, ptr eh %i", cabeca_lista, ptr);
        br_sistema.cabeca_lista = ptr;      // A cabeça passa a apontar para o mesmo local que o final da nossa sequência apontava
    
    }else{                                  // Se não...
        printf("\n11");
        memcpy(dados_sistema[ptr_fim_anterior - offset].conteudo, &ptr, sizeof(uint32_t));
        // Fazemos o final da sequênca anterior apontar para o mesmo local que o final da nossa sequência apontava
    }

    uint32_t zero = 0;  // Também apagamos o ponteiro armazenado no último bloco, para caso ele não ser totalmente sobrescrito depois
    memcpy(dados_sistema[ptr_bloco - offset].conteudo, &zero, sizeof(uint32_t));    
    printf("\n12");
    br_sistema.num_blocos_livres = br_sistema.num_blocos_livres - tamanho_blocos;   // Reduzimos o tamanho de blocos livres no sistema

    return ptr_inicio_atual;                // E por fim retornamos o começo da nossa sequência contígua de blocos livres!
}


int escrever_conteudo(uint32_t tamanho_blocos, char* arquivo) {
    //  Recebe o tamanho de blocos a serem reservados, e o nome de um arquivo de origem.
    //  Chama a função de procurar por um espaço contíguo livre na lista de blocos livres
        //  Essa função reordena a lista de blocos livres, e retorna o endereço do primeiro bloco livre
    //  Escrevemos o arquivo diretamente na lista dados_sistema 
    //  Retorna o endereço do primeiro bloco escrito

    
    FILE *file_src = fopen(arquivo, "rb");
    if (!file_src) {
        perror("Erro ao abrir arquivo de origem");
        return -1;
    }
    
    int offset = 1 + br_sistema.num_blocos_tabela_entradas; // Offset a ser utilizado quando formos acessar o vetor dados_sistema[]
    printf("\nVai busca espaco livre");
    int inicio = procurar_espaco_livre(tamanho_blocos);
    printf("\nVolta busca espaco livre");
    if (inicio == -1){
        printf("\nEspaco insuficiente, Abortando operacao...\n");
        fclose(file_src);
        return -1;
    }
    
    printf("\nEspaco livre suficiente encontrado, na posicao %i", inicio);
    

    // Agora, é só ler diretamente o conteúdo do arquivo a partir da posição equivalente do array
        // (malloc alocou o array de forma contígua, então isso funciona)
    int retorno = fread(&dados_sistema[inicio - offset].conteudo, (tamanho_blocos * BLOCK_SIZE), 1, file_src);
    
    printf("\n\n O Retorno de fread foi: %i", retorno);


    // É para o arquivo agora estar armazenado na seção de dados do Sistema de Arquivos (em memória)
    // Só falta atualizar a tabela de entradas
    return inicio;
}


int escrever_entrada(char* arquivo) {
    // Recebe o nome de um arquivo, abre ele
    // Caminha pela tabela de entradas, procura por uma entrada livre
    // Caminha pela lista de blocos livres, procura por um espaço contíguo grande o suficiente para armazenar o arquivo
    // Armazena o arquivo, adiciona ele na tabela de entradas
    // Atualiza o boot record
    // Retorna

    // Primeiro, procuramos o primeiro espaço livre na tabela de entradas
    int entrada_livre = -1;
    int blocos_tabela = br_sistema.num_blocos_tabela_entradas;      // Obtém o número de blocos da tabela de enradas
    int tamanho_t = blocos_tabela * 16;                             // Calcula o número de entradas possíveis na tabela
    
    for (int i = 0; i < tamanho_t; i++) {                           // Caminhamos pela tabela de entradas, em busca de uma entrada disponível
        if (entrada_sistema[i].status == 0x00 || entrada_sistema[i].status == 0xE5) {
            entrada_livre = i;
            printf("Entrada livre encontrada");
            break;
        }
    }
    
    if (entrada_livre == -1) {                                      // Se a tabela estiver cheia...
        printf("Nenhuma entrada livre.\n");
        return 1;
    }

    // Abre o arquivo a ser copiado
    FILE *file_src = fopen(arquivo, "rb");                          // Abrimos o arquivo a ser copiado
    if (!file_src) {
        perror("Erro ao abrir arquivo de origem");
        return 1;
    }

    printf("\n1");
    
    fseek(file_src, 0, SEEK_END);                                   // Calcula tamanho do arquivo a ser copiado                  
    long tamanho = ftell(file_src);                                 // Tamanho em bytes
    fclose(file_src);

    uint32_t tamanho_blocos = (tamanho + BLOCK_SIZE - 1) / BLOCK_SIZE;  // Usamos o mesmo esquema do boot record para ter sobra

    printf("\nVai pra");

    // Escreve o conteúdo no arquivo, e obtém o primeiro bloco
    int primeiro_bloco = escrever_conteudo(tamanho_blocos, arquivo); // Escreve o conteúdo, retorna o primeiro bloco aonde foi escrito
    if (primeiro_bloco == -1) {
        return 1;                                                    // -1: erro na escrita
    }

    printf("\nvoltou de escrever_conteudo");

    char nome[100] = {0}, extensao[100] = {0}, resultado[100] = {0};// Tratamos o nome do arquivo
    separar_nome_extensao(arquivo, nome, extensao);                 // Separa o nome do arquivo da extensão
    concatenar_nome_extensao(nome, extensao, resultado);            // Concatena os dois

    memset(&entrada_sistema[entrada_livre], 0, sizeof(entrada));    // Enchemos o espaço da memória de zeros, para o remoto caso de ter lixo
    
    // Dai por fim, preenchemos a tabela de entradas
    entrada_sistema[entrada_livre].status = 0x01;                   // Preenchemos o status da nova entrada
    memcpy(entrada_sistema[entrada_livre].nome, resultado, 12);     // Nome do arquivo
    memcpy(entrada_sistema[entrada_livre].ext,  extensao,  4);      // Extensão (até 4 caracteres)

    entrada_sistema[entrada_livre].tipo = 0x01;                     // Tipo (0x01 é arquivo)
    entrada_sistema[entrada_livre].primeiro_bloco = primeiro_bloco; // Ponteiro para o primeiro bloco alocado
    entrada_sistema[entrada_livre].tamanho = tamanho;               // Tamanho do arquivo em bytes
    entrada_sistema[entrada_livre].numero_blocos_usados = tamanho_blocos;   // Tamanho do arquivo em blocos
    entrada_sistema[entrada_livre].padding = 0;                     // Padding, coiso extra pra completar os 32 bytes

    br_sistema.quant_entradas_sistema++;                            // Aumentamos o contador de arquivos salvados no bot record                 

    //strncpy(entrada_sistema[entrada_livre].nome, resultado, 12);    
    //strncpy(entrada_sistema[entrada_livre].ext, extensao, 4);

    return 0;
}

