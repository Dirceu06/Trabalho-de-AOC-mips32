#include <stdio.h>
#include <stdlib.h> // Para strtol
#include "simulador.h"

//variável externa para a instrução atual em formato Assembly
extern char instrucao_assembly_atual[100];

int ciclo_de_execucao()
{
    //BUSCA
    unsigned int instrucao = memoria_instrucoes[pc];

    //atualiza string para mostrar no menu
    sprintf(instrucao_assembly_atual, "%s", traduzir_para_assembly(instrucao));

    //DECODIFICACAO
    SinaisControle sinais = decodificar(instrucao);

    if (sinais.Syscall)
    {
        // Se for syscall, executa a chamada e retorna o status dela
        return executar_syscall();
    }
    else
    {
        int rs = (instrucao >> 21) & 0x1F;
        int rt = (instrucao >> 16) & 0x1F;
        int rd = (instrucao >> 11) & 0x1F;
        unsigned int imediato = instrucao & 0xFFFF;
        int funct = instrucao & 0x3F;

        // ULA
        unsigned int operando1 = registradores[rs];
        unsigned int imediato_estendido = estender_sinal(imediato);
        unsigned int operando2 = (sinais.ALUSrc == 1) ? imediato_estendido : registradores[rt];

        int zero_flag = 0;
        unsigned int resultado_ula = executar_ula(instrucao, operando1, operando2, sinais.ALUOp, funct, &zero_flag);

        // MEM
        unsigned int dado_da_memoria = acessar_memoria(resultado_ula, registradores[rt], sinais.MemRead, sinais.MemWrite);

        // WB
        escrever_no_registrador(rt, rd, dado_da_memoria, resultado_ula, sinais);
    }

    pc++;
    return 1;
}

//controle
SinaisControle decodificar(unsigned int instrucao)
{
    SinaisControle sc = {0, 0, 0, 0, 0, 0, 0, 0}; //zera todos os sinais
    int opcode = (instrucao >> 26) & 0x3F;
    int funct = instrucao & 0x3F;

    if (opcode == 0x00)
    { // Tipo-R (ADD, SUB, AND, OR, SLL, SLT, MULT)
        sc.RegDst = 1;
        sc.ALUOp = 2;
        sc.RegWrite = 1;
    }
    else if (funct == 0x0C)
    { // SYSCALL
        sc.Syscall = 1;
    }
    else if (opcode == 0x08)
    { // ADDI
        sc.ALUSrc = 1;
        sc.RegWrite = 1;
        sc.ALUOp = 0;
    }
    else if (opcode == 0x23)
    { // LW
        sc.ALUSrc = 1;
        sc.MemtoReg = 1;
        sc.RegWrite = 1;
        sc.MemRead = 1;
    }
    else if (opcode == 0x2B)
    { // SW
        sc.ALUSrc = 1;
        sc.MemWrite = 1;
    }
    else if (opcode == 0x0A)
    { // SLTI
        sc.ALUSrc = 1;
        sc.RegWrite = 1;
        sc.ALUOp = 3;
    }
    else if (opcode == 0x0F)
    { // LUI
        sc.ALUSrc = 1;
        sc.RegWrite = 1;
        sc.ALUOp = 4;
    }
    return sc;
}

// ULA
unsigned int executar_ula(unsigned int instrucao, unsigned int op1, unsigned int op2, int alu_op, int funct, int *zero_flag)
{
    unsigned int resultado = 0;
    if (alu_op == 2)
    { //r
        if (funct == 0x20)
        { // ADD
            resultado = op1 + op2;
        }
        else if (funct == 0x22)
        { // SUB
            resultado = op1 - op2;
        }
        else if (funct == 0x24)
        { // AND
            resultado = op1 & op2;
        }
        else if (funct == 0x25)
        { // OR
            resultado = op1 | op2;
        }
        else if (funct == 0x2A)
        { // SLT
            resultado = ((int)op1 < (int)op2) ? 1 : 0;
        }
        else if (funct == 0x00)
        { // SLL
            int shamt = (instrucao >> 6) & 0x1F;
            resultado = op2 << shamt; // Desloca o valor de rt (op2)
        }
        else if (funct == 0x18)
        { // MULT
            // long long para evitar overflow
            long long produto = (long long)op1 * (long long)op2;
            resultado = (unsigned int)produto;
        }
    }
    else if (alu_op == 0)
    { // ADDI, LW, SW
        resultado = op1 + op2;
    }
    else if (alu_op == 3)
    { // SLTI
        resultado = ((int)op1 < (int)op2) ? 1 : 0;
    }
    else if (alu_op == 4)
    { // LUI
        resultado = op2 << 16;
    }

    *zero_flag = (resultado == 0) ? 1 : 0;
    return resultado;
}

// MEM
unsigned int acessar_memoria(unsigned int endereco, unsigned int dado_escrita, int mem_read, int mem_write)
{
    unsigned int indice_memoria = endereco / 4;
    if (indice_memoria >= TAMANHO_MEMORIA)
    {
        printf("ERRO: Acesso invalido a memoria no indice %u!\n", indice_memoria);
        return 0;
    }
    if (mem_read == 1)
        return memoria_dados[indice_memoria];
    if (mem_write == 1)
        memoria_dados[indice_memoria] = dado_escrita;
    return 0;
}

// WB
void escrever_no_registrador(int rt, int rd, unsigned int dado_mem, unsigned int resultado_ula, SinaisControle sinais)
{
    if (sinais.RegWrite == 1)
    {
        unsigned int dado_a_escrever = (sinais.MemtoReg == 1) ? dado_mem : resultado_ula;
        int reg_destino = (sinais.RegDst == 1) ? rd : rt;

        if (reg_destino != 0)
        {
            registradores[reg_destino] = dado_a_escrever;
        }
    }
}

// Retorna 0 se for para sair do programa, 1 caso contrário.
int executar_syscall()
{
    int codigo_servico = registradores[2]; //$v0 é o registrador 2

    switch (codigo_servico)
    {
    case 1:                                  
        printf("%d", (int)registradores[4]); //$a0 (registrador 4)
        break;

    case 4: // Imprimir String
    {
        unsigned int endereco = registradores[4]; //string em $a0
        unsigned int indice_memoria = endereco / 4;
        int byte_offset = endereco % 4;
        char c = -1;

        while (c != '\0')
        {
            // Pega a palavra de 32 bits da memória
            unsigned int palavra = memoria_dados[indice_memoria];
            // Pega o byte correto de dentro da palavra
            c = (char)(palavra >> (8 * byte_offset));

            if (c != '\0')
            {
                printf("%c", c);
            }

            endereco++; // Vai para o próximo byte
            indice_memoria = endereco / 4;
            byte_offset = endereco % 4;
        }
        break;
    }

    case 10: // Sair (Exit)
        printf("\n--- Syscall Sair: Fim da execucao. ---\n");
        return 0; // Sinaliza para o loop principal parar

    default:
        printf("\nERRO: Syscall com codigo %d nao reconhecido.\n", codigo_servico);
        break;
    }
    return 1;
}

//16 bits para 32, preservando o sinal
unsigned int estender_sinal(unsigned int imediato)
{
    if ((imediato & 0x8000) != 0)
    {
        return imediato | 0xFFFF0000; //estende com 1
    }
    return imediato; 
}

//binário para assembly
char* traduzir_para_assembly(unsigned int instrucao) {
    static char buffer[100];
    int opcode = (instrucao >> 26) & 0x3F;
    int rs = (instrucao >> 21) & 0x1F;
    int rt = (instrucao >> 16) & 0x1F;
    int rd = (instrucao >> 11) & 0x1F;
    int shamt = (instrucao >> 6) & 0x1F;
    int funct = instrucao & 0x3F;
    short int imediato = instrucao & 0xFFFF;

    if (opcode == 0x00) { // Tipo R
        switch (funct) {
            case 0x20: sprintf(buffer, "add $%d, $%d, $%d", rd, rs, rt); break;
            case 0x22: sprintf(buffer, "sub $%d, $%d, $%d", rd, rs, rt); break;
            case 0x24: sprintf(buffer, "and $%d, $%d, $%d", rd, rs, rt); break;
            case 0x25: sprintf(buffer, "or $%d, $%d, $%d", rd, rs, rt); break;
            case 0x00: sprintf(buffer, "sll $%d, $%d, %d", rd, rt, shamt); break;
            case 0x2A: sprintf(buffer, "slt $%d, $%d, $%d", rd, rs, rt); break;
            case 0x18: sprintf(buffer, "mult $%d, $%d", rs, rt); break;
            case 0x0C: sprintf(buffer, "syscall"); break;
            default: sprintf(buffer, "Tipo R (funct: 0x%X)", funct); break;
        }
    } else { // Tipo I e J
        switch (opcode) {
            case 0x08: sprintf(buffer, "addi $%d, $%d, %d", rt, rs, imediato); break;
            case 0x23: sprintf(buffer, "lw $%d, %d($%d)", rt, imediato, rs); break;
            case 0x2B: sprintf(buffer, "sw $%d, %d($%d)", rt, imediato, rs); break;
            case 0x0F: sprintf(buffer, "lui $%d, 0x%X", rt, imediato); break;
            case 0x0A: sprintf(buffer, "slti $%d, $%d, %d", rt, rs, imediato); break;
            default: sprintf(buffer, "Tipo I/J (opcode: 0x%X)", opcode); break;
        }
    }
    return buffer;
}