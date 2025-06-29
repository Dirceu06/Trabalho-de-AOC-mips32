#include <stdio.h>
#include <string.h>
#include <stdlib.h> // Para strtol e exit()
#include "simulador.h"

// --- Variáveis Globais ---
unsigned int pc = 0;
unsigned int registradores[NUM_REGISTRADORES];
unsigned int memoria_instrucoes[TAMANHO_MEMORIA];
unsigned int memoria_dados[TAMANHO_MEMORIA];
int total_instrucoes = 0;
char instrucao_assembly_atual[100] = "N/A";

// --- Protótipos das funções neste arquivo ---
void exibir_menu();
void carregar_programa();
void mostrar_registradores();
void gerar_relatorio_final();

int main() {
    int escolha = 0;
    int programa_carregado = 0;
    int status_execucao = 1; // 1 para continuar, 0 para parar (controlado pela syscall de exit)
    int i;

    // Inicializa os arrays de hardware uma única vez
    for (i = 0; i < NUM_REGISTRADORES; i++) registradores[i] = 0;
    for (i = 0; i < TAMANHO_MEMORIA; i++) memoria_instrucoes[i] = 0;
    for (i = 0; i < TAMANHO_MEMORIA; i++) memoria_dados[i] = 0;

    // O loop principal agora depende da escolha do usuário E do status da execução do MIPS
    while (escolha != 5 && status_execucao != 0) {
        exibir_menu();
        scanf("%d", &escolha);

        switch (escolha) {
            case 1:
                carregar_programa();
                if (total_instrucoes > 0) {
                    programa_carregado = 1;
                    status_execucao = 1; // Permite que um novo programa seja executado após um 'exit'
                }
                break;

            case 2: // Executar Passo a Passo
                if (!programa_carregado) {
                    printf("\nERRO: Nenhum programa carregado!\n");
                } else if (pc >= total_instrucoes) {
                    printf("\nFim do programa. O relatorio final sera gerado ao sair.\n");
                } else {
                    // Captura o status retornado para saber se uma syscall de 'exit' ocorreu
                    status_execucao = ciclo_de_execucao();
                }
                break;

            case 3: // Executar Tudo
                if (!programa_carregado) {
                    printf("\nERRO: Nenhum programa carregado!\n");
                } else {
                    // O loop de execução automática também precisa verificar o status
                    while (pc < total_instrucoes && status_execucao != 0) {
                        status_execucao = ciclo_de_execucao();
                    }
                    if (status_execucao != 0) { // Mostra esta mensagem apenas se não parou por um 'exit'
                        printf("\nExecucao finalizada.\n");
                    }
                }
                break;

            case 4:
                if (!programa_carregado) {
                    printf("\nERRO: Nenhum programa carregado!\n");
                } else {
                    mostrar_registradores();
                }
                break;
                
            case 5:
                // Apenas quebra o loop para a lógica de saída ser executada
                break;

            default:
                printf("\nOpcao invalida. Tente novamente.\n");
                break;
        }
    }

    // Lógica de encerramento do simulador
    printf("\nSimulador encerrado.\n");
    if (programa_carregado) {
        gerar_relatorio_final();
    }

    return 0;
}

void exibir_menu() {
    printf("\n\n--- Mini Simulador MIPS ---\n");
    printf("PC: 0x%08X | Instrucao: %s\n", 0x00400000 + (pc * 4), instrucao_assembly_atual);
    printf("\n[1] Carregar Programa Binario (.txt)\n");
    printf("[2] Executar Proximo Passo\n");
    printf("[3] Executar Tudo\n");
    printf("[4] Mostrar Registradores\n");
    printf("[5] Sair e Gerar Relatorio\n");
    printf("\nEscolha uma opcao: ");
}

void carregar_programa() {
    char nome_arquivo[100];
    char linha[33];
    printf("Digite o caminho do arquivo (ex: programas_teste/prog1.txt): ");
    scanf("%s", nome_arquivo);

    FILE* arquivo = fopen(nome_arquivo, "r");
    if (arquivo == NULL) {
        printf("Erro ao abrir o arquivo %s!\n", nome_arquivo);
        return;
    }

    // Reinicia o simulador para um novo programa
    pc = 0;
    total_instrucoes = 0;
    for (int i = 0; i < NUM_REGISTRADORES; i++) registradores[i] = 0;
    for (int i = 0; i < TAMANHO_MEMORIA; i++) memoria_instrucoes[i] = 0;
    sprintf(instrucao_assembly_atual, "N/A");

    while (fscanf(arquivo, "%s", linha) != EOF && total_instrucoes < TAMANHO_MEMORIA) {
        if(strlen(linha) == 32) {
            memoria_instrucoes[total_instrucoes] = (unsigned int)strtol(linha, NULL, 2);
            total_instrucoes++;
        }
    }
    
    fclose(arquivo);

    if (total_instrucoes > 0) {
        printf("Programa carregado com %d instrucoes.\n", total_instrucoes);
    } else {
        printf("Arquivo vazio ou em formato invalido. Nenhuma instrucao carregada.\n");
    }
}

void mostrar_registradores() {
    printf("\n--- Conteudo dos Registradores ---\n");
    for (int i = 0; i < NUM_REGISTRADORES; i++) {
        printf("$%02d: 0x%08X (%d)\t", i, registradores[i], (int)registradores[i]);
        if ((i + 1) % 4 == 0) {
            printf("\n");
        }
    }
}

void gerar_relatorio_final() {
    FILE* relatorio = fopen("relatorio_final.txt", "w");
    if (relatorio == NULL) {
        printf("Erro ao criar arquivo de relatorio.\n");
        return;
    }
    fprintf(relatorio, "--- RELATORIO FINAL DE EXECUCAO ---\n\n");
    fprintf(relatorio, "Total de instrucoes executadas: %d\n\n", pc);
    fprintf(relatorio, "--- Conteudo Final dos Registradores ---\n");
    for (int i = 0; i < NUM_REGISTRADORES; i++) {
        fprintf(relatorio, "$%02d: 0x%08X (%d)\n", i, registradores[i], (int)registradores[i]);
    }
    fclose(relatorio);
    printf("Relatorio 'relatorio_final.txt' gerado com sucesso.\n");
}