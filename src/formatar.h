// Funções envolvidas no processo de pegar um arquivo, e formatá-lo para nele simular um sistema de arquivos

#include "structs.h"


void inicializar_secao_dados() {
    //  Recebe o nome de um arquivo, abre ele
    //  Obtém o começo, e o tamanho da seção de dados
    //  A partir do primeiro endereço, percorre a seção de dados, criando a lista de blocos livres
        // Adiciona um ponteiro de 4 bytes para o próximo bloco, no começo de cada bloco
    //  Escreve no arquivo, e libera os espaços alocados

    FILE *file;
    uint32_t inicio_secao = br_sistema.blocos_reservados;   // Obtem o endereço do primeiro bloco da seção de dados
    uint32_t num_blocos = br_sistema.num_blocos_secao_dados;    // Obtem o tamnho total da seção de dados

    file = fopen(nome_arquivo, "rb+");
    if (!file) {                                                // Verifica se foi aberto corretamente
        printf("\n\nArquivo %s", nome_arquivo);
        perror("Erro ao abrir arquivo");
        exit(EXIT_FAILURE);
    }

    for (uint32_t i = 0; i < num_blocos; i++) {
        uint32_t endereco_atual = inicio_secao + i; // Vai caminhando por cada bloco da seção de dados
        uint32_t proximo_endereco;                  // Variavel para calcular o proximo endereço

        if (i == num_blocos - 1) {
            proximo_endereco = 0xFFFFFFFF;          // Se chegamos ao ultimo bloco, proximo = -1
        } else {
            proximo_endereco = endereco_atual + 1; //proximo endereço é apenas o atual + 1
        }
        //printf("\nProximo endereco: %i", proximo_endereco);

        unsigned char bytes[4]; //fazemos a conversão de um inteiro para 4 bytes
        bytes[0] = (proximo_endereco >> 0) & 0xFF;
        bytes[1] = (proximo_endereco >> 8) & 0xFF;
        bytes[2] = (proximo_endereco >> 16) & 0xFF;
        bytes[3] = (proximo_endereco >> 24) & 0xFF;

        char *conteudo_bloco = (char*)malloc(br_sistema.bytes_por_bloco);   // Alocamos o conteudo para o bloco
        if (!conteudo_bloco) {
            perror("Erro ao alocar memória para o bloco");
            exit(EXIT_FAILURE);
        }

        memcpy(conteudo_bloco, bytes, 4);                                   // Os primeiros 4 bytes guardam o proximo endereço
        memset(conteudo_bloco + 4, 0x00, br_sistema.bytes_por_bloco - 4);   // O restante enchemos de 0

        fseek(file, endereco_atual * br_sistema.bytes_por_bloco, SEEK_SET); // Movemos o cursor, deslocamento atual * tamanho do bloco
        fwrite(conteudo_bloco, br_sistema.bytes_por_bloco, 1, file);        // Escrevemos o conteudo do bloco

        free(conteudo_bloco);   // Liberamos o ponteiro
    }
    fclose(file);               // Fecha o arquivo após a execução
}


int formatar(){
    //  Recebe o nome do arquivo aonde será feita a simulação do sistema de arquivos
    //  Pede para o usuário o tamanho do disco (simulado), e o número de entradas da tabela de entradas
    //  Valida os dados inseridos
    //  Gera a stuct do boot record, calcula e insere os dados na struct
    //  Salva a struct no arquivo
    //  Retorna 0 caso tudo der certo

    int total_blocos_reservados, tamanho_disco, n_entradas, tamanho_total_entradas;
    int continuar = 1;          // Usado na validação dos inputs


    while (continuar){                      // Recebemos e validamos os inputs do usuário
        printf("\n=--=- Informacoes de Formatacao\n");
        printf("=-- Insira o tamanho do disco, em bytes\nR: ");
        scanf("%i",&tamanho_disco);

        printf("=-- Insira o numero de entradas na tabela de entradas\nR: ");
        scanf("%i",&n_entradas);

        // TODO - É 4 se considerarmos o root directory (1 bloco)
        if (tamanho_disco >= (3 * BLOCK_SIZE)                           // O disco armazena no mínimo 3 blocos (boot_r, tabela, dados)?
            && n_entradas <= ((int)(tamanho_disco / BLOCK_SIZE)) - 2 ){ // Não pode ter mais entradas do que blocos de dados
            // Não é necessário verificar se o tamanho da partição é suficiente pra tabela de entradas.

            continuar = 0;  // Se os inputs forem válidos, continua a execução

        } else {            // Se não, pede novamente os inputs
            printf("\n\n=-- DADOS INVALIDOS!!!\n");
        }
    }

    br_sistema.bytes_por_bloco = BLOCK_SIZE;     // Quantidade fixa de blocos por byte.
    br_sistema.blocos_reservados = 1;            // Quantidade de blocos reservados: 1 (boot record) (adicionar mais 1? root dir?)
    br_sistema.quant_entradas_sistema = 0;       // inicia em 0 porque o sistema está vazio
    br_sistema.foo = 1;                          // Numero de blocos reservados para o root directory
    br_sistema.num_blocos_totais = tamanho_disco/br_sistema.bytes_por_bloco;  // separamos o disco em blocos

    tamanho_total_entradas = n_entradas * sizeof(entrada);          // tamanho total da tabela de entradas em bytes

    if(tamanho_total_entradas < br_sistema.bytes_por_bloco){        // alocamos apenas um bloco se o tamanho da tabela é menor que um bloco
        br_sistema.num_blocos_tabela_entradas = 1;
    }else{                                                          // separamos em blocos e arredondamos para cima
        br_sistema.num_blocos_tabela_entradas = (tamanho_total_entradas + br_sistema.bytes_por_bloco - 1) / br_sistema.bytes_por_bloco;
    }

    total_blocos_reservados = br_sistema.blocos_reservados + br_sistema.num_blocos_tabela_entradas;   // para calcular o inicio da seção de dados
    br_sistema.blocos_reservados = total_blocos_reservados;
    br_sistema.num_blocos_secao_dados = br_sistema.num_blocos_totais - total_blocos_reservados;
                                            // numero de blocos totais do sistema - (reservados + tabela de entradas)
    br_sistema.num_blocos_livres = br_sistema.num_blocos_secao_dados;

    br_sistema.cabeca_lista = total_blocos_reservados;       // Obtemos o endereço do primeiro bloco livre

    FILE *file = fopen(nome_arquivo, "wb");             // Abrimos o arquivo

    if (!file) {
        printf("\n\nErro ao abrir o arquivo %s!\n", nome_arquivo);
        return 1;
    }

    fwrite(&br_sistema, sizeof(boot_record), 1, file);
    fclose(file);

    return 0;
}
