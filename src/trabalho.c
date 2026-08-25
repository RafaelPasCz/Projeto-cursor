#include "leitura_arquivos.h"
#include "escrita_arquivo.h"
#include "deletar_arquivo.h"
#include "armazenar_SA.h"
#include "carregar_SA.h"
#include "formatar.h"


void listar_arquivos() {
    // Função responsável por listar todos os arquivos presentes no sistema de arquivos.
    // Percorre a tabela de entradas e exibe as informações de cada arquivo válido.

    printf("\n\n==--- Listagem de Arquivos do Sistema de Arquivos ---==\n");

    int total_entradas = br_sistema.num_blocos_tabela_entradas * 16;            // Calcula o número total de entradas na tabela
    int count = 0;                                                              // Contagem de arquivos no sistema

    for (int i = 0; i < total_entradas; i++) {
        // Verifica se a entrada está em uso (status != 0x00 (inexistente) e != 0xE5 (removido))
        if (entrada_sistema[i].status != 0x00 && entrada_sistema[i].status != 0xE5) {
            count++;

            // Exibe as informações do arquivo
            printf("\n=-== Arquivo de Indice: %d ==-=\n", i);
            printf("Nome: %.12s\n", entrada_sistema[i].nome);
            printf("Extensao: %.4s\n", entrada_sistema[i].ext);
            printf("Status: 0x%X\n", entrada_sistema[i].status);
            printf("Tipo: 0x%X\n", entrada_sistema[i].tipo);
            printf("Primeiro bloco: %u\n", entrada_sistema[i].primeiro_bloco);
            printf("Tamanho (bytes): %u\n", entrada_sistema[i].tamanho);
            printf("Blocos usados: %u\n", entrada_sistema[i].numero_blocos_usados);
        }
    }

    if (count == 0) {
        printf("\n\n+====-- Nenhum arquivo encontrado no sistema de arquivos!\n");
    } else {
        printf("\n==---= Total de arquivos encontrados: %d\n =---==", count);
    }
}


int main(){
    // unsigned int* lista_livres;
    // unsigned int* lista_ordenado;
    // unsigned int espaco_livre; // contem o deslocamento inicial e final do espaço livre necessário
    // unsigned int i = 0;
    
    int operacao = 0, continuar = 1;
    uint32_t index;
    char arquivo[256];
	printf("hello world\n");

    while(continuar){
        printf("\n\n==----------- Sistema de Arquivos -----------==\n");
        printf("=--=- O que deseja fazer?\n");
        printf("=--[1] Formatar uma particao a ser simulada\n");
        printf("=--[2] Carregar uma particao simulada salva (HD -> RAM)\n");
        printf("=--[3] Salvar uma particao simulada (RAM -> HD)\n");
        printf("=--[4] Listar os arquivos no Sistema de Arquivos\n");
        printf("=--[5] Carregar arquivo do disco para o sistema simulado (HD -> RAM)\n");
        printf("=--[6] Carregar arquivo do sistema simulado para o disco(RAM -> HD)\n");
        printf("=--[7] Remover um arquivo do Sistema de Arquivos\n");
        printf("=--[0] Sair\nR: ");
        scanf("%i", &operacao);

        switch(operacao){
            case 1:
                get_nomeArquivo();

                formatar();                 // Cria o boot record
                inicializar_secao_dados();  // Inicializa todos os blocos da seção de dados

                memset(&br_sistema, 0, sizeof(br_sistema));   // Limpa os dados armazenados nas structs

                break;

            case 2:
                // Pega um sistema de arquivos salvo no disco, carrega ele na memória
                get_nomeArquivo();
                carregar_sistema_arquivos();
                break;

            case 3:
                // Pega o sistema de arquivos carregado na memória, salva ele no disco
                get_nomeArquivo();
                salvar_sistema_arquivos();
                break;

            case 4:
                // Lista os arquivos armazenados no sistema de arquivos
                listar_arquivos();
                break;

            case 5:
                // Funções para copiar arquivo do disco para o sistema
                printf("=-- Insira o nome do arquivo a ser copiado\nR: ");

                while (getchar() != '\n');      // Limpa o buffer

                if (fgets(arquivo, sizeof(arquivo), stdin)) {
                    arquivo[strcspn(arquivo, "\n")] = '\0';   // Remove o '\n' do final
                }   // Lemos o nome do arquivo, junto com alguns tratamentos de strings. Cortesia do StackOverflow

                printf("Arquivo escolhido: %s\n", arquivo);

                escrever_entrada(arquivo);

                break;

            case 6:
                // Funções para copiar um arquivo 
                escrever_arquivo();
                
                break;

            case 7:
                printf("=-- Insira o indice do arquivo a ser deletado (conforme listagem)\nR: ");
                scanf("%i", &index);
                deletar_arquivo(index);
                break;

            case 0:
                printf("Tchau!\n");
                continuar = 0;
                break;

            default:
                printf("Opcao invalida!\n");
        }
    }
}
