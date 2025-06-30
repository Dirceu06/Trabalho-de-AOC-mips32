// Dirceu Morais da Costa Junior
// Gabriel Mancuso Bonfim


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "simulador.h"

// --- Códigos de Cor ANSI ---
#define ANSI_COR_VERMELHO  "\x1b[31m"
#define ANSI_COR_VERDE     "\x1b[32m"
#define ANSI_COR_AMARELO   "\x1b[33m"
#define ANSI_COR_AZUL      "\x1b[34m"
#define ANSI_COR_MAGENTA   "\x1b[35m"
#define ANSI_COR_CIANO     "\x1b[36m"
#define ANSI_COR_RESET     "\x1b[0m"

// --- Variáveis Globais ---
unsigned int pc = 0;
unsigned int registradores[NUM_REGISTRADORES];
unsigned int memoria_instrucoes[TAMANHO_MEMORIA];
unsigned int memoria_dados[TAMANHO_MEMORIA];
unsigned int total_instrucoes = 0;
char instrucao_assembly_atual[100] = "N/A";
char syscall_output_buffer[256] = "";

// --- Protótipos Locais ---
void exibir_menu_inicial();
void exibir_menu_principal();
void exibir_menu_pos_execucao();

int main() {
    int escolha = 0;
    int programa_carregado = 0;

    for (int i = 0; i < NUM_REGISTRADORES; i++) registradores[i] = 0;
    for (int i = 0; i < TAMANHO_MEMORIA; i++) memoria_instrucoes[i] = 0;
    for (int i = 0; i < TAMANHO_MEMORIA; i++) memoria_dados[i] = 0;

    while (1) {
        if (!programa_carregado) {
            exibir_menu_inicial();
            scanf("%d", &escolha);
            if (escolha == 1) {
                carregar_programa();
                if (total_instrucoes > 0) {
                    programa_carregado = 1;
                }
            } else if (escolha == 5) {
                exit(0);
            } else {
                 printf(ANSI_COR_VERMELHO "\nOpcao invalida.\n" ANSI_COR_RESET);
            }
        } else {
            exibir_menu_principal();
            scanf("%d", &escolha);
            switch (escolha) {
                case 1:
                    carregar_programa();
                    break;
                case 2:
                    if (pc >= total_instrucoes) {
                        exibir_menu_pos_execucao();
                    } else {
                        ciclo_de_execucao();
                        if (pc >= total_instrucoes) {
                            exibir_menu_pos_execucao();
                        }
                    }
                    break;
                case 3:
                    while (pc < total_instrucoes) {
                        ciclo_de_execucao();
                    }
                    exibir_menu_pos_execucao();
                    break;
                case 4:
                    mostrar_registradores();
                    break;
                case 5:
                    printf(ANSI_COR_AMARELO "\nGerando relatorio final e saindo...\n" ANSI_COR_RESET);
                    gerar_relatorio_final();
                    exit(0);
                default:
                    printf(ANSI_COR_VERMELHO "\nOpcao invalida. Tente novamente.\n" ANSI_COR_RESET);
            }
        }
    }
    return 0;
}

void exibir_menu_inicial() {
    printf(ANSI_COR_AZUL "\n\n--- Mipsinho ---\n" ANSI_COR_RESET); // NOME ALTERADO
    printf("\nNenhum programa carregado.\n");
    printf("\n[1] Carregar Programa Binario (.txt)");
    printf("\n[5] Sair\n");
    printf(ANSI_COR_VERDE "\nEscolha uma opcao: " ANSI_COR_RESET);
}

void exibir_menu_principal() {
    if (strlen(syscall_output_buffer) > 0) {
        printf("\n\n" ANSI_COR_CIANO ">>> SAIDA DA SYSCALL <<<" ANSI_COR_RESET);
        printf(ANSI_COR_AMARELO "\n%s" ANSI_COR_RESET, syscall_output_buffer);
        printf(ANSI_COR_CIANO "\n========================\n" ANSI_COR_RESET);
        strcpy(syscall_output_buffer, "");
    }
   
    printf(ANSI_COR_AZUL "\n\n--- Mipsinho ---\n" ANSI_COR_RESET); // NOME ALTERADO
    printf(ANSI_COR_VERDE "\nPC: " ANSI_COR_RESET "0x%08X" ANSI_COR_VERDE " | Instrucao: " ANSI_COR_RESET "%s\n", 0x00400000 + (pc * 4), instrucao_assembly_atual);
    
    printf("\n[1] Carregar Outro Programa Binario (.txt)");
    printf("\n[2] Executar Proximo Passo");
    printf("\n[3] Executar Tudo");
    printf("\n[4] Mostrar Registradores");
    printf("\n[5] Sair e Gerar Relatorio\n");
    
    printf(ANSI_COR_VERDE "\nEscolha uma opcao: " ANSI_COR_RESET);
}

void exibir_menu_pos_execucao() {
    int escolha_final = 0;

    if (strlen(syscall_output_buffer) > 0) {
        printf("\n\n" ANSI_COR_CIANO ">>> SAIDA DA SYSCALL <<<" ANSI_COR_RESET);
        printf(ANSI_COR_AMARELO "\n%s" ANSI_COR_RESET, syscall_output_buffer);
        printf(ANSI_COR_CIANO "\n========================\n" ANSI_COR_RESET);
        strcpy(syscall_output_buffer, "");
    }

    while (1) {
        printf(ANSI_COR_AZUL "\n--- Execucao Concluida ---" ANSI_COR_RESET);
        printf("\n[1] Mostrar Registradores Finais");
        printf("\n[2] Carregar novo programa binario");
        printf("\n[3] Sair e Gerar Relatorio");
        printf("\n[4] Sair (sem relatorio)");
        printf(ANSI_COR_VERDE "\n\nEscolha uma opcao: " ANSI_COR_RESET);
        
        scanf("%d", &escolha_final);

        switch (escolha_final) {
            case 1:
                mostrar_registradores();
                break;
            case 2:
                carregar_programa();
                return;
            case 3:
                printf(ANSI_COR_AMARELO "\nGerando relatorio final e saindo...\n" ANSI_COR_RESET);
                gerar_relatorio_final();
                exit(0);
            case 4:
                printf(ANSI_COR_AMARELO "\nSaindo...\n" ANSI_COR_RESET);
                exit(0);
            default:
                printf(ANSI_COR_VERMELHO "\nOpcao invalida. Tente novamente.\n" ANSI_COR_RESET);
        }
    }
}

void carregar_programa() {
    char nome_arquivo[100];
    char linha[33];
    printf(ANSI_COR_VERDE "Digite o caminho do arquivo (ex: hello_final.txt): " ANSI_COR_RESET);
    scanf("%s", nome_arquivo);

    FILE* arquivo = fopen(nome_arquivo, "r");
    if (arquivo == NULL) {
        printf(ANSI_COR_VERMELHO "Erro ao abrir o arquivo %s!\n" ANSI_COR_RESET, nome_arquivo);
        total_instrucoes = 0; // Garante que, se o ficheiro falhar, o programa não fica num estado inconsistente
        return;
    }

    pc = 0;
    total_instrucoes = 0;
    for (int i = 0; i < NUM_REGISTRADORES; i++) registradores[i] = 0;
    for (int i = 0; i < TAMANHO_MEMORIA; i++) memoria_instrucoes[i] = 0;
    for (int i = 0; i < TAMANHO_MEMORIA; i++) memoria_dados[i] = 0;
    sprintf(instrucao_assembly_atual, "N/A");

    while (fscanf(arquivo, "%s", linha) != EOF && total_instrucoes < TAMANHO_MEMORIA) {
        memoria_instrucoes[total_instrucoes] = (unsigned int)strtoul(linha, NULL, 2);
        total_instrucoes++;
    }
    
    fclose(arquivo);
    printf(ANSI_COR_VERDE "Programa carregado com %u instrucoes.\n" ANSI_COR_RESET, total_instrucoes);
}

void mostrar_registradores() {
    printf(ANSI_COR_AZUL "\n--- Conteudo dos Registradores ---" ANSI_COR_RESET);
    for (int i = 0; i < NUM_REGISTRADORES; i++) {
        if ((i % 4) == 0) printf("\n");
        printf(ANSI_COR_VERDE "$%02d: " ANSI_COR_RESET, i);
        printf(ANSI_COR_MAGENTA "0x%08X " ANSI_COR_RESET, registradores[i]);
        printf("(%d)\t", (int)registradores[i]);
    }
    printf("\n");
}

void gerar_relatorio_final() {
    FILE* relatorio = fopen("relatorio_final.txt", "w");
    if (relatorio == NULL) {
        printf(ANSI_COR_VERMELHO "Erro ao criar arquivo de relatorio.\n" ANSI_COR_RESET);
        return;
    }
    
    fprintf(relatorio, "--- RELATORIO FINAL DE EXECUCAO ---\n\n");
    fprintf(relatorio, "Total de instrucoes carregadas: %u\n", total_instrucoes);
    fprintf(relatorio, "Total de instrucoes executadas: %u\n\n", pc);
    
    fprintf(relatorio, "--- Conteudo Final dos Registradores ---\n");
    for (int i = 0; i < NUM_REGISTRADORES; i++) {
        fprintf(relatorio, "$%02d: 0x%08X (%d)\n", i, registradores[i], (int)registradores[i]);
    }
    
    fprintf(relatorio, "\n\n--- Listagem do Programa em Assembly ---\n");
    if (total_instrucoes > 0) {
        for (unsigned int i = 0; i < total_instrucoes; i++) {
            unsigned int instrucao_binaria = memoria_instrucoes[i];
            char* instrucao_assembly = traduzir_para_assembly(instrucao_binaria);
            fprintf(relatorio, "%02u.\t%s\n", i + 1, instrucao_assembly);
        }
    } else {
        fprintf(relatorio, "Nenhum programa foi carregado.\n");
    }

    fclose(relatorio);
    printf(ANSI_COR_VERDE "Relatorio 'relatorio_final.txt' gerado com sucesso.\n" ANSI_COR_RESET);
}