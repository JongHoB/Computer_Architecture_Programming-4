/***************************************************************/
/*                                                             */
/*   MIPS-32 Instruction Level Simulator                       */
/*                                                             */
/*   SCE212 Ajou University                                    */
/*   util.h                                                    */
/*   Adapted from CS311@KAIST                                  */
/*                                                             */
/***************************************************************/

/* !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! */
/*          DO NOT MODIFY THIS FILE!                            */
/*          You should only the parse.c and run.c files!        */
/* !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! */

#ifndef _UTIL_H_
#define _UTIL_H_

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#define FALSE 0
#define TRUE  1

/* Basic Information */
#define MEM_TEXT_START	0x00400000
#define MEM_TEXT_SIZE	0x00100000
#define MEM_DATA_START	0x10000000
#define MEM_DATA_SIZE	0x00100000
#define MIPS_REGS	32
#define BYTES_PER_WORD	4
#define PIPE_STAGE	5

#define IF_STAGE 	0
#define ID_STAGE	1
#define EX_STAGE	2
#define MEM_STAGE	3
#define WB_STAGE	4

/* You should decode your instructions from the
 * ASCII-binary format to this structured format */
typedef struct inst_s {
    short opcode;

    /*R-type*/
    short func_code;

    union {
        /* R-type or I-type: */
        struct {
	    unsigned char rs;
	    unsigned char rt;

	    union {
	        short imm;

	        struct {
		    unsigned char rd;
		    unsigned char shamt;
		} r;
	    } r_i;
	} r_i;
        /* J-type: */
        uint32_t target;
    } r_t;

    uint32_t value;
} instruction;

typedef struct{
    uint32_t Instr;
    uint32_t NPC;
}IFID;

typedef struct {
    uint32_t NPC;
    uint32_t REG1;
    uint32_t REG2;
    short IMM;
    short SIGN_EX_IMM;
    int DEST;
}IDEX;

typedef struct {
    uint32_t NPC;
    uint32_t BR_TARGET;
    uint32_t ALU_OUT;
    uint32_t REG2;
    uint32_t W_VALUE;
    uint32_t BR_TAKE;
    int DEST;
}EXMEM;

typedef struct {
    uint32_t NPC;
    uint32_t ALU_OUT;
    uint32_t MEM_OUT;
    uint32_t BR_TAKE;
    int DEST;
}MEMWB;


typedef struct CPU_State_Struct {
    uint32_t PC;		/* program counter */
    uint32_t REGS[MIPS_REGS];	/* register file. */

    uint32_t REGS_LOCK[MIPS_REGS];    /* register lock to support stalls 
                                        Lock registers when data is not ready*/

    uint32_t PIPE[PIPE_STAGE];	/* PC being executed at each stage*/
    uint32_t PIPE_STALL[PIPE_STAGE];

    IFID IF_ID;//0
    IDEX ID_EX;//1
    EXMEM EX_MEM;//2
    MEMWB MEM_WB;//3
    
    uint32_t JUMP_PC;

} CPU_State;

/* All simulated memory will be managed by this structure
 * use the mem_write_32() and mem_read_32() functions to
 * access/modify the simulated memory */
typedef struct {
    uint32_t start, size;
    uint8_t *mem;
} mem_region_t;

/* For PC * Registers */
extern CPU_State CURRENT_STATE;

/* For Instructions */
extern instruction *INST_INFO;
extern int NUM_INST;

/* For Memory Regions */
extern mem_region_t MEM_REGIONS[2];

/* For Execution */
extern int RUN_BIT;	/* run bit */
extern int FETCH_BIT;	/* instruction fetch bit */
extern int INSTRUCTION_COUNT;

extern int BR_BIT;	/* Branch predictor enabled */
extern int FORWARDING_BIT;
extern uint64_t MAX_INSTRUCTION_NUM;
extern uint64_t CYCLE_COUNT;

/* Functions */
char**		str_split(char *a_str, const char a_delim);
int		    fromBinary(const char *s);
uint32_t	mem_read_32(uint32_t address);
void		mem_write_32(uint32_t address, uint32_t value);
void		cycle();
void		run();
void		go();
void		mdump(int start, int stop);
void		rdump();
void		pdump();
void		init_memory();
void		init_inst_info();

/* YOU IMPLEMENT THIS FUNCTION */
void	process_instruction();

#endif
