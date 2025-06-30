// Dirceu Morais da Costa Junior
// Gabriel Mancuso Bonfim


#include <stdio.h>
#include <stdlib.h> // Para strtol      
#include <string.h> 
#include "simulador.h"

// Variável externa para a instrução atual em formato Assembly
extern char instrucao_assembly_atual[100];

void ciclo_de_execucao() {
    //BUSCA
    unsigned int instrucao = memoria_instrucoes[pc];
    sprintf(instrucao_assembly_atual, "%s", traduzir_para_assembly(instrucao));

    //DECODIFICACAO
    SinaisControle sinais = decodificar(instrucao);
    
    if (sinais.Syscall) { // Se for uma SYSCALL, executa-a e sai do ciclo.
        if (executar_syscall()) {
            pc = total_instrucoes; // Força o fim da execução no modo "Executar Tudo"
        } else {
            pc++; // Avança para a próxima instrução
        }
        return; // Termina o ciclo de execução
    }

    int rs = (instrucao >> 21) & 0x1F;
    int rt = (instrucao >> 16) & 0x1F;
    int rd = (instrucao >> 11) & 0x1F;
    unsigned int imediato = instrucao & 0xFFFF;
    int funct = instrucao & 0x3F;

    //ULA
    unsigned int operando1 = registradores[rs];
    unsigned int imediato_estendido = estender_sinal(imediato);
    unsigned int operando2 = (sinais.ALUSrc == 1) ? imediato_estendido : registradores[rt];
    int zero_flag = 0;
    unsigned int resultado_ula = executar_ula(instrucao, operando1, operando2, sinais.ALUOp, funct, &zero_flag);

    //MEM
    unsigned int dado_da_memoria = acessar_memoria(resultado_ula, registradores[rt], sinais.MemRead, sinais.MemWrite);

    //WB
    escrever_no_registrador(rt, rd, dado_da_memoria, resultado_ula, sinais);

    pc++;
}

// Simula a Unidade de Controle
SinaisControle decodificar(unsigned int instrucao) {
    SinaisControle sc = {0,0,0,0,0,0,0,0}; // Zera todos os sinais, incluindo o novo Syscall
    int opcode = (instrucao >> 26) & 0x3F;
    int funct = instrucao & 0x3F; // Pega o funct code para o tipo-R

    if (opcode == 0x00) { // Tipo-R
        if (funct == 0x0C) { // syscall
            sc.Syscall = 1;
        } else { 
            // Outras instruções Tipo-R (ADD, SUB, etc.)
            sc.RegDst = 1;
            sc.ALUOp = 2;
            sc.RegWrite = 1;
        }
    } else if (opcode == 0x08) { // ADDI
        sc.ALUSrc = 1;
        sc.RegWrite = 1;
        sc.ALUOp = 0;
    } else if (opcode == 0x23) { // LW
        sc.ALUSrc = 1;
        sc.MemtoReg = 1;
        sc.RegWrite = 1;
        sc.MemRead = 1;
    } else if (opcode == 0x2B) { // SW
        sc.ALUSrc = 1;
        sc.MemWrite = 1;
    } else if (opcode == 0x0A) { // SLTI
        sc.ALUSrc = 1;
        sc.RegWrite = 1;
        sc.ALUOp = 3; 
    } else if (opcode == 0x0F) { // LUI
        sc.ALUSrc = 1;
        sc.RegWrite = 1;
        sc.ALUOp = 4;
    }
    return sc;
}

//ULA
unsigned int executar_ula(unsigned int instrucao, unsigned int op1, unsigned int op2, int alu_op, int funct, int* zero_flag) {
    unsigned int resultado = 0;
    if (alu_op == 2) { // Tipo-R
        if (funct == 0x20) { // ADD
            resultado = op1 + op2;
        } else if (funct == 0x22) { // SUB
            resultado = op1 - op2;
        } else if (funct == 0x24) { // AND
            resultado = op1 & op2;
        } else if (funct == 0x25) { // OR
            resultado = op1 | op2;
        } else if (funct == 0x2A) { // SLT
            resultado = ((int)op1 < (int)op2) ? 1 : 0;
        } else if (funct == 0x00) { // SLL
            int shamt = (instrucao >> 6) & 0x1F;
            resultado = op2 << shamt; // Desloca o valor de rt (op2)
        } else if (funct == 0x18) { // MULT 
            //long long para evitar overflow
            long long produto = (long long)op1 * (long long)op2;
            resultado = (unsigned int)produto;
        }
    } 
    else if (alu_op == 0) { // ADDI, LW, SW
        resultado = op1 + op2;
    } else if (alu_op == 3) { // SLTI
        resultado = ((int)op1 < (int)op2) ? 1 : 0;
    } else if (alu_op == 4) { // LUI
        resultado = op2 << 16;
    }
    
    *zero_flag = (resultado == 0) ? 1 : 0;
    return resultado;
}

//MEM
unsigned int acessar_memoria(unsigned int endereco, unsigned int dado_escrita, int mem_read, int mem_write) {
    if (mem_read == 1 || mem_write == 1) {
        // Endereços MIPS são em bytes, nosso array é em palavras. Dividimos por 4.
        unsigned int indice_memoria = endereco / 4;

        // Agora a verificação de segurança está no sítio certo.
        if (indice_memoria >= TAMANHO_MEMORIA) {
            printf("ERRO: Acesso invalido a memoria no indice %u!\n", indice_memoria);
            return 0; // Retorna 0 em caso de erro.
        }

        // Executa a leitura ou escrita.
        if (mem_read == 1) {
            return memoria_dados[indice_memoria];
        }
        if (mem_write == 1) {
            memoria_dados[indice_memoria] = dado_escrita;
        }
    } 
    return 0;
}

//WB
void escrever_no_registrador(int rt, int rd, unsigned int dado_mem, unsigned int resultado_ula, SinaisControle sinais) {
    if (sinais.RegWrite == 1) {
        unsigned int dado_a_escrever = (sinais.MemtoReg == 1) ? dado_mem : resultado_ula;
        int reg_destino = (sinais.RegDst == 1) ? rd : rt;

        if (reg_destino != 0) { // Não se pode escrever no registrador $zero
            registradores[reg_destino] = dado_a_escrever;
        }
    }
}

// Estende um valor de 16 bits para 32, preservando o sinal
unsigned int estender_sinal(unsigned int imediato) {
    // Verifica o bit 15 (0x8000). Se for 1, o número é negativo.
    if ((imediato & 0x8000) != 0) {
        return imediato | 0xFFFF0000; // Estende com 1s
    }
    return imediato; // Senão, já está estendido com 0s
}

// Traduz o binário para Assembly (função auxiliar para exibição)
char* traduzir_para_assembly(unsigned int instrucao) {

    static char buffer[100]; // 'static' para o buffer persistir após o retorno da função
    int opcode = (instrucao >> 26) & 0x3F;
    int rs = (instrucao >> 21) & 0x1F;
    int rt = (instrucao >> 16) & 0x1F;
    int rd = (instrucao >> 11) & 0x1F;
    int shamt = (instrucao >> 6) & 0x1F;
    int funct = instrucao & 0x3F;
    short int imediato = instrucao & 0xFFFF; // 'short int' para tratar o sinal corretamente na impressão

    if (opcode == 0x00) { // Tipo R
        switch (funct) {
            case 0x20: sprintf(buffer, "add $%d, $%d, $%d", rd, rs, rt); break;
            case 0x22: sprintf(buffer, "sub $%d, $%d, $%d", rd, rs, rt); break;
            case 0x24: sprintf(buffer, "and $%d, $%d, $%d", rd, rs, rt); break;
            case 0x25: sprintf(buffer, "or $%d, $%d, $%d", rd, rs, rt); break;
            case 0x00: sprintf(buffer, "sll $%d, $%d, %d", rd, rt, shamt); break;
            case 0x2A: sprintf(buffer, "slt $%d, $%d, $%d", rd, rs, rt); break;
            case 0x18: sprintf(buffer, "mult $%d, $%d", rs, rt); break; 
                        case 0x0C: sprintf(buffer, "syscall"); break; // <<< LINHA ADICIONADA

            default: sprintf(buffer, "Tipo R (funct: %d)", funct); break;
        }
    } else { // Tipo I e J
        switch (opcode) {
            case 0x08: sprintf(buffer, "addi $%d, $%d, %d", rt, rs, imediato); break;
            case 0x23: sprintf(buffer, "lw $%d, %d($%d)", rt, imediato, rs); break;
            case 0x2B: sprintf(buffer, "sw $%d, %d($%d)", rt, imediato, rs); break;
            case 0x0F: sprintf(buffer, "lui $%d, 0x%X", rt, imediato); break;
            case 0x0A: sprintf(buffer, "slti $%d, $%d, %d", rt, rs, imediato); break;
            default: sprintf(buffer, "Tipo I/J (opcode: %d)", opcode); break;
        }
    }
    return buffer;
}


// Retorna 1 se a syscall for 'exit', 0 caso contrário.
int executar_syscall() {
    // O código do serviço está no registrador $v0 (índice 2)
    unsigned int servico = registradores[2];
    char buffer_temporario[256]; // Usamos um buffer local para formatar a nova saída

    switch (servico) {
        case 1: // Imprimir Inteiro
            // Formata a nova saída no buffer temporário
            sprintf(buffer_temporario, "%d", (int)registradores[4]);
            // Concatena a nova saída ao buffer principal
            strcat(syscall_output_buffer, buffer_temporario);
            break;

        case 4: // Imprimir String
        {
            unsigned int addr = registradores[4];
            char* memoria_base = (char*)memoria_dados;
            if (addr < TAMANHO_MEMORIA * 4) {
                // Concatena a string da memória ao buffer principal
                strcat(syscall_output_buffer, &memoria_base[addr]);
            } else {
                sprintf(buffer_temporario, "\nERRO: Syscall de print_string com endereco invalido (0x%X)\n", addr);
                strcat(syscall_output_buffer, buffer_temporario);
            }
        }
        break;
    
        case 10: // Sair (Exit)
            sprintf(buffer_temporario, "\n--- FIM DA EXECUCAO (syscall exit) ---");
            strcat(syscall_output_buffer, buffer_temporario);
            return 1; // Sinaliza para o ciclo principal parar
    
        default:
            sprintf(buffer_temporario, "\nERRO: Syscall com codigo de servico %u nao reconhecido.\n", servico);
            strcat(syscall_output_buffer, buffer_temporario);
            break;
    }
    return 0; // Não é para sair
}