// Dirceu Morais da Costa Junior
// Gabriel Mancuso Bonfim



#ifndef SIMULADOR_H
#define SIMULADOR_H

// --- NOTA DE PROJETO ---
// Para simplificar, usamos 'unsigned int' para representar valores de 32 bits,
// como registradores e instruções. Isso assume que o compilador usa 32 bits
// para 'unsigned int', o que é padrão em sistemas modernos.

// --- Constantes ---
#define TAMANHO_MEMORIA 1024U
#define NUM_REGISTRADORES 32

// --- Estruturas de Dados ---

// Sinais de controle, usando 'int' como booleano (1=true, 0=false)
typedef struct {
    int RegDst;         //registrador de destino é rd (para instruções tipo-R) ou rt (para instruções tipo-I)
    int ALUSrc;         //Decide se o segundo operando da ULA vem de um registrador (rt) ou do valor imediato da instrução
    int MemtoReg;       //Decide se o dado a ser escrito no registrador vem da memória ou da ULA
    int RegWrite;       //Habilita a escrita num registrador
    int MemRead;        //Habilita a leitura na memória de dados
    int MemWrite;       //Habilita a escrita na memória de dados
    int ALUOp;          //Envia uma operação base para a ULA (ex: somar, comparar, etc.)
    int Syscall;        //syscall
} SinaisControle;

// --- Variáveis Globais (Hardware Simulado) ---
extern unsigned int pc;
extern unsigned int registradores[NUM_REGISTRADORES];
extern unsigned int memoria_instrucoes[TAMANHO_MEMORIA];
extern unsigned int memoria_dados[TAMANHO_MEMORIA];
extern unsigned int total_instrucoes;
extern unsigned int total_instrucoes;
extern char syscall_output_buffer[256];

// --- Protótipos das Funções ---

// Funções de 'main.c'
void exibir_menu();
void carregar_programa();
void mostrar_registradores();
void gerar_relatorio_final();
void exibir_menu_pos_execucao();

// Funções de 'simulador.c'
void ciclo_de_execucao();
unsigned int buscar_instrucao();
SinaisControle decodificar(unsigned int instrucao);
unsigned int executar_ula(unsigned int instrucao, unsigned int op1, unsigned int op2, int alu_op, int funct, int* zero_flag);
unsigned int acessar_memoria(unsigned int endereco, unsigned int dado_escrita, int mem_read, int mem_write);
void escrever_no_registrador(int rt, int rd, unsigned int dado_mem, unsigned int resultado_ula, SinaisControle sinais);
char* traduzir_para_assembly(unsigned int instrucao);
unsigned int estender_sinal(unsigned int imediato);
long binario_para_inteiro(const char* bin);
int executar_syscall();

#endif // SIMULADOR_H