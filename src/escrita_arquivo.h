// Funções envolvidas no processo de cópia de arquivos do Sistema de Arquivos para o Disco

#include "structs.h"
#include "strings.h"

void escrever_arquivo(){
    // Função responsável por pegar um arquivo armazenado no Sistema de Arquivos, e escrever ele na memória.
    // Pode funcionar assim:
    //  Pede ao usuário o indice da tabela de entradas a ser escrito no disco
    //  Vai na lista da tabela de arquivos, pega o índice espeificado
    //  Printa infos básicas dele na tela
    //  Cria o arquivo, escreve ele.
    
    
    uint32_t index;
    char arquivo[12];

    printf("=-- Insira o indice da tabela de entradas a ser escrido no disco\nR: ");    
    scanf("%i", &index);                                                    // Índice pode ser obtido pela listagem de arquivos

    if(index <= (br_sistema.num_blocos_tabela_entradas * 16)){              // Primeiro verifica se o indice pode existir
        if(entrada_sistema[index].status == 0x00 || entrada_sistema[index].status == 0xE5){
            printf("\nIndice Vazio ou Deletado!\n");                        // Verificamos se o indice nao foi deletado

        } else {

            printf("\n\n=- Nome: %.12s", entrada_sistema[index].nome);
            printf("\n\n ------- Entrada %i: -------", index);
            printf("\nStatus: 0x%X", entrada_sistema[index].status);
            printf("\nNome: %.12s", entrada_sistema[index].nome);
            printf("\nExtensao: %.4s", entrada_sistema[index].ext);
            printf("\nTipo: 0x%X", entrada_sistema[index].tipo);
            printf("\nPrimeiro bloco: %u", entrada_sistema[index].primeiro_bloco);
            printf("\nTamanho (bytes): %u", entrada_sistema[index].tamanho);
            printf("\nBlocos usados: %u", entrada_sistema[index].numero_blocos_usados);
            
            strncpy(arquivo, entrada_sistema[index].nome, 12);
                        
            // Remove os espaços finais do nome do arquivo
            int len = strlen(arquivo);
            while (len > 0 && arquivo[len - 1] == ' ') {
                arquivo[len - 1] = '\0';
                len--;
            }
            
            // Substitui todos os espaços no meio do nome do arquivo por pontos
            char *ptr = strchr(arquivo, ' ');
            while(ptr){
                if (ptr) {
                    *ptr = '.';
                }
                ptr = strchr(arquivo, ' ');
            }
            
            // Vai ficar um monte de ponto no final, então substitui eles por '\0'
            len = strlen(arquivo);
            while (len > 0 && arquivo[len - 1] == '.') {
                arquivo[len - 1] = '\0';
                len--;
            }
            
            printf("\n\nNome do arquivo a ser criado: '%s'", arquivo);
            
            // Agora, podemos abrir o arquivo:
            FILE *file = fopen(arquivo, "wb");
            if(!file){
                printf("\n\nERRO AO ABRI O ARQUIVO '%s'", arquivo);
            }
            
            // Escreve o conteúdo do arquivo no disco
            uint32_t bloco = entrada_sistema[index].primeiro_bloco;
            uint32_t offset = 1 + br_sistema.num_blocos_tabela_entradas;        // Offset para acessar dados_sistema
            uint32_t blocos_lidos = entrada_sistema[index].numero_blocos_usados;
            
            while (blocos_lidos > 1) {                                          // Escreve todos os blocos, ignorando o último
                fwrite(dados_sistema[bloco - offset].conteudo, BLOCK_SIZE, 1, file);
                bloco++;
                blocos_lidos--;
            }

            // Agora, escrevemos o último bloco, mas ignorando o monte de 0 que tem no final dele, escreve só os bytes que falta
            fwrite(dados_sistema[bloco - offset].conteudo, (entrada_sistema[index].tamanho % BLOCK_SIZE), 1, file);
            
            fclose(file);                                                       // Fechamos o arquivo
            printf("\nArquivo %s escrito no disco com sucesso!\n", arquivo);    // Só o sucesso
        }
    } else {
        printf("\nIndice invalido!\n");
    }
}
    