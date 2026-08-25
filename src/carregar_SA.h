// Funções envolvidas no processo de pegar um sistema de arquivos armazenado no HD, e carregá-lo em memória

#include "structs.h"

void carregar_boot(){
    //  Abre o arquivo escolhido antes pelo usuário
    //  Lê o arquivo, armazena o boot record numa struct
    //  Printa os dados armazenadosna struct
    memset(&br_sistema, 0, sizeof(boot_record));                    // Preenchemos o espaço da memória de zeros
    FILE *file;                                                     // Abrimos o arquivo a ser lido
    file = fopen(nome_arquivo, "rb");
    fread(&br_sistema, sizeof(boot_record), 1, file);               // Escrevemos o boot record do arquivo na memória
    fclose(file);

    printf("Dados lidos: \nbytes_por_bloco: %hu", br_sistema.bytes_por_bloco);
    printf("\nblocos reservados: %hu", br_sistema.blocos_reservados);
    printf("\nnum blocos livres: %i", br_sistema.num_blocos_livres);
    printf("\nnum blocos tabela de entradas: %i", br_sistema.num_blocos_tabela_entradas);
    printf("\nnum blocos secao de dados: %i", br_sistema.num_blocos_secao_dados);
    printf("\nnum blocos totais: %i", br_sistema.num_blocos_totais);
    printf("\nquant entradas sistema: %i", br_sistema.quant_entradas_sistema);
    printf("\ncabeca da lista: %i\n", br_sistema.cabeca_lista);
}


int carregar_lista_blocos_livres(){
    //  Abre o arquivo escolhido pelo usuário
    //  Lê o arquivo, armazena a lista de blocos livros na memória
    //  Printa os dados armazenados na lista

    // Ok. Possivel ordem de fazer as coisas:
    // Abre o arquivo
    // fseek pro começo da seção da lista de blocos livres
    // pega o valor do número de blocos da tabela de entradas
    // aloca espaço em memória blocos_entrada * BLOCK_SIZE
        // usando o ponteiro global entradas_sistema
        // podse: entradas_sistema = (entrada)malloc(blocos_entrada * sizeof(bloco)))
        // Daí vai fazer o mesmo esquema feito anteriormente, de ir adicionando indices de array nesse espaço da memória

    // Quando esses valores estiverem carregador, tudo ok pode retornar.
    // Talvez ate printar eles na tela enquanto pega cada um

    // Dai depois disso tem que fazer a função de colocar eles da memória pro arquivo.

    FILE *file;
    file = fopen(nome_arquivo, "rb");                               // Abrimos o arquivo, para pegar a tabela de entradas dele
    uint32_t blocos_tabela = br_sistema.num_blocos_tabela_entradas; // Armazenamos o tamanho em blocos da tabela de entradas
    size_t tamanho_tabela_entradas = blocos_tabela * BLOCK_SIZE;    // Armazenamos o tamanho em bytes da tabela de entradas

    free(entrada_sistema);                                          // Liberamos a alocação anterior da tabela de entradas do sistema

    entrada_sistema = (entrada *)malloc(tamanho_tabela_entradas);   // Alocamos espaço em memória para a tabela
    if(entrada_sistema == NULL){
        printf("Erro ao alocar memoria para a tabela de entradas!");
        return 1;
    }

    memset(entrada_sistema, 0, tamanho_tabela_entradas);            // Enchemos o espaço da memória de zeros

    // Agora, fseek no arquivo para o começo da tabela de entradas
    // Depois for loop que vai lendo o coiso

    fseek(file, 1 * BLOCK_SIZE, SEEK_SET);                          // Movemos o ponteiro de leitura para o começo da tabela de entradas

    fread(entrada_sistema, tamanho_tabela_entradas, 1, file);       // Lê todos os arquivos da tabela de entrada, armazena eles na lista

    fclose(file);

    // Agora é pra tudo estar dentro dessa lista! Printamos ela por conveniencia:
    // Primeiro calculamos o tamanho do array:
    int tamanho = blocos_tabela * 16;   // 512 / 32 = 16 entradas por bloco

    //printf("\n\n ----------- TABELA DE ENTRADAS -----------");
    //for(int i = 0; i < tamanho; i++){
    //    printf("\n\n ------- Entrada %i: -------", i);
    //    //printf("Tamanho da struct: %llu bytes\n", sizeof(entrada_sistema[i]));
    //    printf("\nStatus: 0x%X", entrada_sistema[i].status);
    //    printf("\nNome: %.12s", entrada_sistema[i].nome);
    //    printf("\nExtensao: %.4s", entrada_sistema[i].ext);
    //    printf("\nTipo: 0x%X", entrada_sistema[i].tipo);
    //    printf("\nPrimeiro bloco: %u", entrada_sistema[i].primeiro_bloco);
    //    printf("\nTamanho (bytes): %u", entrada_sistema[i].tamanho);
    //    printf("\nBlocos usados: %u", entrada_sistema[i].numero_blocos_usados);
    //    printf("\nPadding: %u", entrada_sistema[i].padding);
    //}

    return 0;
}


int carregar_secao_dados(){
    //  Abre o arquivo escolhido pelo usuário
    //  Lê o arquivo, armazena a seção de dados na memória
    //  Printa os blocos armazenados

    // Ok. Possivel ordem de fazer as coisas:
    // Abre o arquivo
    // fseek pro começo do bloco da seção de dados
    // pega o valor do número de blocos da seção de dados

    // aloca espaço em memória blocos_dados * BLOCK_SIZE
        // usando o ponteiro global blocos_sistema
        // podse: blocos_sistema = (bloco *)malloc(blocos_dados * sizeof(bloco)))
        // Daí vai fazer o mesmo esquema feito anteriormente, de ir adicionando indices de array nesse espaço da memória

    // Quando esses valores estiverem carregados, tudo ok pode retornar.
    // Talvez ate printar eles na tela enquanto pega cada um

    // Dai depois disso tem que fazer a função de colocar eles da memória pro arquivo.

    FILE *file;
    file = fopen(nome_arquivo, "rb");                           // Abrimos o arquivo, para pegar a tabela de entradas dele
    uint32_t blocos_dados = br_sistema.num_blocos_secao_dados;  // Armazenamos o tamanho em blocos da seção de dados
    size_t tamanho_secao_dados = blocos_dados * BLOCK_SIZE;     // Armazenamos o tamanho em bytes da seção de dados
    uint32_t inicio_dados = br_sistema.blocos_reservados;   // Endereço do começo da seção de dados

    free(dados_sistema);                                         // Liberamos a alocação anterior da seção de dados do sistema

    dados_sistema = (bloco *)malloc(tamanho_secao_dados);       // Alocamos espaço em memória para a tabela

    if(dados_sistema == NULL){
        printf("Erro ao alocar memoria para a seção de dados!");
        return 1;
    }

    memset(dados_sistema, 0, tamanho_secao_dados);              // Enchemos o espaço da memória de zeros

    // Agora, fseek no arquivo para o começo da seção de dados
    // Depois a gente lê tudo e armazena na memória alocada anteriormente

    fseek(file, inicio_dados * BLOCK_SIZE, SEEK_SET);           // Movemos o ponteiro de leitura para o começo da seção de dados

    fread(dados_sistema, tamanho_secao_dados, 1, file);         // Lê todos os blocos da seção de dados, armazena eles na lista

    fclose(file);

    //// Agora é pra tudo estar dentro dessa lista! Printamos ela por conveniencia:
    //printf("\n\n ----------- SECAO DE DADOS -----------");
    //
    //for(uint32_t i = 0; i < blocos_dados; i++){                  // Iteramos por cada bloco da seção de dados
    //    printf("\nBloco %i:\n", i + inicio_dados);
    //    for (int j = 0; j < BLOCK_SIZE; j++) {                  // Iteramos por cada byte do bloco
    //        printf("%02X ", (unsigned char)dados_sistema[i].conteudo[j]);
    //        if ((j + 1) % 16 == 0) {
    //            printf("\n");                                   // Quebra de linha a cada 16 bytes, mesmo estilo do HexEd.it
    //        }
    //    }
    //    printf("\n\n");
    //}

    return 0;
}

void carregar_sistema_arquivos(){
    // Chama as funções responsáveis por carregar o sistema de arquivos do disco para a memória
    carregar_boot();
    carregar_lista_blocos_livres();
    carregar_secao_dados();
}