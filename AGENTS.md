# Contexto do projeto
    Este é um projeto que estou usando para aprender a utilizar agentes
    Para isso, irei refatorar e aprimorar um projeto de sistema de arquivos em C

# Especificação do sistema de arquivos
    - O sistema de arquivos utiliza alocação contígua para armazenar blocos no disco 
    - O gerenciamento de blocos livres é feito por lista ligada, o sitema mantém uma lista ligada de blocos livres, cada bloco livre contém um ponteiro para o próximo bloco disponível, o último bloco da lista aponta para 0xFF (-1), essa lista ligada fica guardada no boot record
    - A estrutura hierárquica é de diretórios em nível único, não há subdiretórios além dos armazenados no diretório raiz, arquivos podem ser armazenados no diretório raiz
    - Cada entrada na tabela de entradas é uma struct com as seguintes informações :
        Nome do arquivo: Nome identificador do arquivo armazenado no diretório de nível 1.
        Tipo: Indica se a entrada corresponde a um arquivo.
        Ponteiro para o primeiro bloco: Para arquivos, aponta para o primeiro bloco onde o conteúdo do arquivo está armazenado.
        Tamanho: Representa o tamanho do arquivo em bytes.
        Número de blocos alocados: Indica a quantidade de blocos ocupados pelo arquivo no sistema de arquivos.

## Regras de Arquitetura do Sistema de Arquivos (Baseado na Especificação)
- **Alocação**: Arquivos devem usar alocação contígua (blocos consecutivos).
- **Gerenciamento de Espaço Livre**: Utilizar uma lista ligada de blocos livres, onde o último bloco aponta para `0xFFFFFFFF`. Um bloco livre possui 4 bytes para o ponteiro do próximo bloco e 508 bytes reservados.
- **Diretórios**: Estrutura hierárquica de nível único (apenas diretório raiz, sem subdiretórios).
- **Setores Principais**:
  1. **Boot Record (Setor 0)**: Contém tamanho do bloco (offset 0x00), blocos reservados (0x02), blocos da lista de livres (0x04), blocos da tabela de entradas (0x08), blocos de dados (0x0C), total de blocos (0x10), blocos do dir. raiz (0x14) e qtd. de entradas (0x18).
  2. **Tabela de Entradas**: Cada entrada possui exatos 32 bytes. Status em 0x00 (0xE5 para removido), Nome do arquivo em 0x01 (12 bytes), Extensão em 0x0D (4 bytes), Tipo em 0x11 (0x01 arquivo, 0x02 diretório), Primeiro bloco em 0x12 (4 bytes), Tamanho em 0x16 (4 bytes) e Número de blocos usados em 0x1A (4 bytes).
  3. **Lista de Blocos Livres**: Gerencia dinamicamente os blocos disponíveis.
  4. **Seção de Dados**: Armazena os dados dos arquivos consecutivamente.
- **Operações Principais Suportadas**: Formatação da partição, Cópia (Disco para FS e FS para Disco), Listagem de arquivos e Remoção.

## Regras de Código e Estilo (C99)
- A linguagem principal é C puro (padrão C99).
- Não utilize bibliotecas externas de terceiros; limite-se à biblioteca padrão C (`stdio.h`, `stdlib.h`, `string.h`, `stdint.h`).
- O código deve ser modular: separe a definição das estruturas de dados (`structs` de Boot Record, Entradas) em arquivos de cabeçalho (`.h`) e a lógica principal em `.c`.
- **Gerenciamento de Memória**: Todo `malloc`, `calloc` ou `realloc` DEVE ser validado contra retorno `NULL`. Toda memória alocada deve ser liberada com `free()` adequadamente para evitar vazamentos.
- **Precisão de Dados**: É OBRIGATÓRIO usar tipos de dados de tamanho fixo da biblioteca `<stdint.h>` (como `uint32_t`, `uint16_t`, `uint8_t`) para as estruturas, garantindo que os bytes e offsets em memória batam exatamente com a especificação do disco binário.

## Comportamento do Agente (IA)
- Ao ler, escrever ou manipular bytes e offsets, siga rigorosamente a tabela de documentação técnica deste projeto.
- Ao analisar código, caso identifique vazamentos de memória (memory leaks) ou dangling pointers (ponteiros soltos), interrompa e aponte-os imediatamente.
- Se for solicitado criar compilações, forneça instruções ou crie um `Makefile` usando `gcc` com as flags de rigor técnico `-Wall -Wextra -Werror -g`.
- Sempre comente a lógica complexa ao lidar com a travessia de ponteiros da lista ligada no disco.