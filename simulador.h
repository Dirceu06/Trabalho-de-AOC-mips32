#ifndef SIMULADOR_H
#define SIMULADOR_H

// --- NOTA DE PROJETO ---
// Para simplificar, usamos 'unsigned int' para representar valores de 32 bits,
// como registradores e instruções. Isso assume que o compilador usa 32 bits
// para 'unsigned int', o que é padrão em sistemas modernos.

// --- Constantes ---
#define TAMANHO_MEMORIA 1024
#define NUM_REGISTRADORES 32

// --- Estruturas de Dados ---

// Sinais de controle, usando 'int' como booleano (1=true, 0=false)
// Em simulador.h
typedef struct {
    int RegDst;
    int ALUSrc;
    int MemtoReg;
    int RegWrite;
    int MemRead;
    int MemWrite;
    int Branch;
    int ALUOp;
    int Syscall;
} SinaisControle;

// --- Variáveis Globais (Hardware Simulado) ---
extern unsigned int pc;
extern unsigned int registradores[NUM_REGISTRADORES];
extern unsigned int memoria_instrucoes[TAMANHO_MEMORIA];
extern unsigned int memoria_dados[TAMANHO_MEMORIA];
extern int total_instrucoes;

// --- Protótipos das Funções ---

// Funções de 'main.c'
void exibir_menu();
void carregar_programa();
void mostrar_registradores();
void gerar_relatorio_final();

// Funções de 'simulador.c'
int ciclo_de_execucao();
unsigned int buscar_instrucao();
SinaisControle decodificar(unsigned int instrucao);
unsigned int executar_ula(unsigned int instrucao, unsigned int op1, unsigned int op2, int alu_op, int funct, int* zero_flag);
unsigned int acessar_memoria(unsigned int endereco, unsigned int dado_escrita, int mem_read, int mem_write);
void escrever_no_registrador(int rt, int rd, unsigned int dado_mem, unsigned int resultado_ula, SinaisControle sinais);
int executar_syscall();
char* traduzir_para_assembly(unsigned int instrucao);
unsigned int estender_sinal(unsigned int imediato);
long binario_para_inteiro(const char* bin);

#endif // SIMULADOR