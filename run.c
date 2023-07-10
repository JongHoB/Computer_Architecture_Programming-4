/***************************************************************/
/*                                                             */
/*   MIPS-32 Instruction Level Simulator                       */
/*                                                             */
/*   SCE212 Ajou University                                    */
/*   run.c                                                     */
/*   Adapted from CS311@KAIST                                  */
/*                                                             */
/***************************************************************/

#include <stdio.h>

#include "util.h"
#include "run.h"

/***************************************************************/
/*                                                             */
/* Procedure: get_inst_info                                    */
/*                                                             */
/* Purpose: Read insturction information                       */
/*                                                             */
/***************************************************************/
instruction* get_inst_info(uint32_t pc)
{
    return &INST_INFO[(pc - MEM_TEXT_START) >> 2];
}

/***************************************************************/
/*                                                             */
/* Procedure: IF                                               */
/*                                                             */
/* Purpose:fetch a new instruction from memory                 */
/*                                                             */
/***************************************************************/
void IF(){


}

/***************************************************************/
/*                                                             */
/* Procedure: ID                                               */
/*                                                             */
/* Purpose: decode the fetched instruction                     */ 
/*           and read the register file                        */
/*                                                             */
/***************************************************************/
void ID(){

}

/***************************************************************/
/*                                                             */
/* Procedure: EX                                               */
/*                                                             */
/* Purpose: execute an ALU operation                           */ 
/*    a. Execute arithmetic and logical operations             */ 
/*    b. Calculate the addresses for loads and stores          */
/*                                                             */
/***************************************************************/
void EX(){

}

/***************************************************************/
/*                                                             */
/* Procedure: MEM                                              */
/*                                                             */
/* Purpose: access memory for load and store operationsn       */
/*                                                             */
/***************************************************************/
void MEM(){

}

/***************************************************************/
/*                                                             */
/* Procedure: WB                                               */
/*                                                             */
/* Purpose: write back the result to the register              */
/*                                                             */
/***************************************************************/
void WB(){

}

/***************************************************************/
/*                                                             */
/* Procedure: process_instruction                              */
/*                                                             */
/* Purpose: Process one instrction                             */
/*                                                             */
/***************************************************************/
void process_instruction()
{
	// /** Implement this function */
    // if ((CURRENT_STATE.PC - MEM_TEXT_START) / 4== NUM_INST)
    // {
    //     RUN_BIT = FALSE;
    //     return;
    // }

    // instruction *current=get_inst_info(CURRENT_STATE.PC);
    // CURRENT_STATE.PC+=4;

    // uint32_t jump;
    // switch(OPCODE(current))
    //     {
    //         //Type I
    //         case 0x9:		//(0x001001)ADDIU
    //         CURRENT_STATE.REGS[RT(current)]=CURRENT_STATE.REGS[RS(current)]+SIGN_EX(IMM(current));
    //         break;
    //         case 0xc:		//(0x001100)ANDI
    //         CURRENT_STATE.REGS[RT(current)]=CURRENT_STATE.REGS[RS(current)]&IMM(current);
    //         break;
    //         case 0xf:		//(0x001111)LUI	
    //         CURRENT_STATE.REGS[RT(current)]=IMM(current)<<16;
    //         break;
    //         case 0xd:		//(0x001101)ORI
    //         CURRENT_STATE.REGS[RT(current)]=CURRENT_STATE.REGS[RS(current)]|IMM(current);
    //         break;
    //         case 0xb:		//(0x001011)SLTIU
    //         CURRENT_STATE.REGS[RT(current)]=(CURRENT_STATE.REGS[RS(current)] < SIGN_EX(IMM(current))) ? 1 : 0;
    //         break;
    //         case 0x23:		//(0x100011)LW
    //         CURRENT_STATE.REGS[RT(current)] = mem_read_32(CURRENT_STATE.REGS[RS(current)] + SIGN_EX(IMM(current)));
    //         break;
    //         case 0x2b:		//(0x101011)SW
    //         mem_write_32(CURRENT_STATE.REGS[RS(current)] + SIGN_EX(IMM(current)), CURRENT_STATE.REGS[RT(current)]);
    //         break;
    //         case 0x4:		//(0x000100)BEQ
    //         BRANCH_INST(CURRENT_STATE.REGS[RS(current)] == CURRENT_STATE.REGS[RT(current)],CURRENT_STATE.PC+IDISP(current),);
    //         break;
    //         case 0x5:		//(0x000101)BNE
    //         BRANCH_INST(CURRENT_STATE.REGS[RS(current)] != CURRENT_STATE.REGS[RT(current)],CURRENT_STATE.PC+IDISP(current),);
    //         break;

    //         //TYPE R
    //         case 0x0:		//(0x000000)ADDU, AND, NOR, OR, SLTU, SLL, SRL, SUBU  if JR
    //         switch(FUNC(current))
    //         {
    //             case 0x21:
    //             CURRENT_STATE.REGS[RD(current)] = CURRENT_STATE.REGS[RS(current)] + CURRENT_STATE.REGS[RT(current)];
    //             break;
    //             case 0x24:
    //             CURRENT_STATE.REGS[RD(current)] = CURRENT_STATE.REGS[RS(current)] & CURRENT_STATE.REGS[RT(current)];
    //             break;
    //             case 0x08:
    //             CURRENT_STATE.PC=CURRENT_STATE.REGS[RS(current)];
    //             break;
    //             case 0x27:
    //             CURRENT_STATE.REGS[RD(current)] = ~(CURRENT_STATE.REGS[RS(current)] | CURRENT_STATE.REGS[RT(current)]);
    //             break;
    //             case 0x25:
    //             CURRENT_STATE.REGS[RD(current)] = CURRENT_STATE.REGS[RS(current)] | CURRENT_STATE.REGS[RT(current)];
    //             break;
    //             case 0x2B:
    //             CURRENT_STATE.REGS[RD(current)] = (CURRENT_STATE.REGS[RS(current)] < CURRENT_STATE.REGS[RT(current)]) ? 1 : 0;
    //             break;
    //             case 0x00:
    //             CURRENT_STATE.REGS[RD(current)] = CURRENT_STATE.REGS[RT(current)] << SHAMT(current);
    //             break;
    //             case 0x02:
    //             CURRENT_STATE.REGS[RD(current)] = CURRENT_STATE.REGS[RT(current)] >> SHAMT(current);
    //             break;
    //             case 0x23:
    //             CURRENT_STATE.REGS[RD(current)] = CURRENT_STATE.REGS[RS(current)] - CURRENT_STATE.REGS[RT(current)];
    //             break;
    //         }
    //         break;

    //         //TYPE J
    //         case 0x2:		//(0x000010)J
    //         jump=((CURRENT_STATE.PC-4)&0xf0000000)+(TARGET(current)<<2);
    //         JUMP_INST(jump);
    //         break;
    //         case 0x3:		//(0x000011)JAL
    //         CURRENT_STATE.REGS[31] = CURRENT_STATE.PC + 4;
    //         jump=(CURRENT_STATE.PC&0xf0000000)+(TARGET(current)<<2);
    //         JUMP_INST(jump);
    //         break;

    //         default:
    //         break;

    //     }

        return;
}
