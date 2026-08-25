//Funções utilizadas para tratamento de strings
#ifndef HEADER_STRINGS
#define HEADER_STRINGS

#include <stdio.h>
#include <string.h>
#include "structs.h"


void get_nomeArquivo(){
    // Função que pede o nome do arquivo ao usuário, e armazena ele numa var global

    printf("=-- Insira o nome do arquivo a ser usado\nR: ");

    while (getchar() != '\n');                              // Limpa o buffer

    if (fgets(nome_arquivo, sizeof(nome_arquivo), stdin)) {
        nome_arquivo[strcspn(nome_arquivo, "\n")] = '\0';   // Remove o '\n' do final
    }   // Lemos o nome do arquivo, junto com alguns tratamentos de strings. Cortesia do StackOverflow

    printf("Arquivo escolhido: %s\n", nome_arquivo);
}


void separar_nome_extensao(const char *entrada, char *nome, char *extensao) {
    // Recebe um nome de arquivo
    // Separa o nome da extensão (até 4 chars)
    // Peguei do stackoverflow

    const char *ultima_barra = strrchr(entrada, '/');       // Encontra a última barra
    if(!ultima_barra){
        ultima_barra = strrchr(entrada, '\\');
    }

    const char *ponto = strrchr(entrada, '.');              // Encontra o último ponto

    if (!ponto || (ultima_barra && ponto < ultima_barra)) {
        // Se não houver ponto ou se ele estiver antes da última barra, não há extensão
        strncpy(nome, ultima_barra ? ultima_barra + 1 : entrada, 7);
        nome[7] = '\0';                                     // Limita a 7 caracteres
        extensao[0] = '\0';                                 // Não há extensão

    } else {
        // Copia o nome do arquivo sem a extensão (limita a 7 caracteres)
        strncpy(nome, ultima_barra ? ultima_barra + 1 : entrada, ponto - (ultima_barra ? ultima_barra + 1 : entrada));
        nome[ponto - (ultima_barra ? ultima_barra + 1 : entrada)] = '\0';  // Garante que o nome não ultrapasse o limite
        if (strlen(nome) > 7) nome[7] = '\0';               // Limita o nome a 7 caracteres
        
        strncpy(extensao, ponto + 1, 4);                    // Copia a extensão sem o ponto (limita a 4 caracteres)
        extensao[4] = '\0';                                 // Garante que a extensão tenha no máximo 4 caracteres
    }
}


void concatenar_nome_extensao(const char *nome, const char *extensao, char *resultado) {
    // Recebe um nome de arquivo (até 7 chars), e uma extensão (até 4 chars)
    // Concatena les conforme a especificação

    int i = 0, j = 0;
    
    while (nome[i] != '\0' && i < 7) {                      // Copiar nome
        resultado[i] = nome[i];
        i++;
    }

    resultado[i] = ' ';                                     // Adiciona espaço no lugar do '.'
    i++;

    while (extensao[j] != '\0' && j < 4) {                  // Concatena a extensão
        resultado[i] = extensao[j];
        i++;
        j++;
    }
}


#endif