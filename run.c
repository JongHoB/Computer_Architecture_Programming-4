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

    CURRENT_STATE.PIPE[IF_STAGE]=CURRENT_STATE.PC;

    //LOAD instruction IF stage-1
    //STORE instruction
    CURRENT_STATE.IF_ID.Instr=get_inst_info(CURRENT_STATE.PC);

    //LOAD instruction IF stage-2
    CURRENT_STATE.PC+=4;
    CURRENT_STATE.IF_ID.NPC=CURRENT_STATE.PC;

    
    

return;
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

    CURRENT_STATE.PIPE[ID_STAGE]=CURRENT_STATE.PIPE[IF_STAGE];

    CURRENT_STATE.ID_EX.NPC=CURRENT_STATE.IF_ID.NPC;

    //LOAD instruction ID stage-1
    CURRENT_STATE.ID_EX.NPC=CURRENT_STATE.IF_ID.NPC;

    //LOAD instruction ID stage-2
    //STORE instruction
    CURRENT_STATE.ID_EX.REG1=CURRENT_STATE.REGS[RS(CURRENT_STATE.IF_ID.Instr)];
    CURRENT_STATE.ID_EX.REG2=CURRENT_STATE.REGS[RT(CURRENT_STATE.IF_ID.Instr)];
    CURRENT_STATE.ID_EX.IMM=SIGN_EX(IMM(CURRENT_STATE.IF_ID.Instr)); 

    //Modified pipeline datapath for LOAD instruction
    //lw,sw,beq instruction Write Register comes from RT field
    CURRENT_STATE.ID_EX.DEST=RT(CURRENT_STATE.IF_ID.Instr);
    
    //R-Type
    //Write Register comes from RD field
    if(OPCODE(CURRENT_STATE.IF_ID.Instr)==0x0)
    {
        CURRENT_STATE.ID_EX.DEST=RD(CURRENT_STATE.IF_ID.Instr);
    }

    //EX HAZARD
    //IF EX/MEM is lw or R format, and EX/MEM.DEST is equal to ID/EX.REG1 or ID/EX.REG2
    //NEED TO FORWARD EX/MEM.ALU_OUT to ALU operand
    switch(OPCODE(get_inst_info(CURRENT_STATE.PIPE[EX_STAGE]))){
        case 0x23://lw
        case 0x0://R-Type

        //FORWARDING_BIT=FALSE; --> WHY BORDER...?

        if(CURRENT_STATE.EX_MEM.DEST!=0&&CURRENT_STATE.EX_MEM.DEST==RS(get_inst_info(CURRENT_STATE.PIPE[ID_STAGE])))
        {
            CURRENT_STATE.ID_EX.REG1=CURRENT_STATE.EX_MEM.ALU_OUT;
        }
        if(CURRENT_STATE.EX_MEM.DEST!=0&&CURRENT_STATE.EX_MEM.DEST==RT(get_inst_info(CURRENT_STATE.PIPE[ID_STAGE])))
        {
            CURRENT_STATE.ID_EX.REG2=CURRENT_STATE.EX_MEM.ALU_OUT;
        }
        break;
        default:
        break;
    }

    //MEM HAZARD
    //IF MEM/WB is lw or R format, and MEM/WB.DEST is equal to ID/EX.REG1 or ID/EX.REG2
    //NEED TO FORWARD MEM/WB.MEM_OUT to ALU operand
    int inst=0;
    switch(OPCODE(get_inst_info(CURRENT_STATE.PIPE[MEM_STAGE]))){
        case 0x23://lw
        inst=1;
        case 0x0://R-Type

        //FORWARDING_BIT=FALSE; --> WHY BORDER...?

        if(CURRENT_STATE.MEM_WB.DEST!=0&&CURRENT_STATE.MEM_WB.DEST==RS(get_inst_info(CURRENT_STATE.PIPE[ID_STAGE]))&&!(OPCODE(get_inst_info(CURRENT_STATE.PIPE[EX_STAGE]))==(0x23||0x0)&&CURRENT_STATE.EX_MEM.DEST!=0&&CURRENT_STATE.EX_MEM.DEST!=RS(get_inst_info(CURRENT_STATE.PIPE[ID_STAGE]))))
        {
            /***************************************************************/
            /*  if (MEM/WB.RegWrite and (MEM/WB.RegisterRd ≠  0)           */ 
            /*  and not(EX/MEM.RegWrite and (EX/MEM.RegisterRd ≠  0)       */ 
            /*          and (EX/MEM.RegisterRd ≠  ID/EX.RegisterRs))       */ 
            /*  and  (MEM/WB.RegisterRd = ID/EX.RegisterRs)) ForwardA = 01 */
            /***************************************************************/ 
            CURRENT_STATE.ID_EX.REG1=(inst==0)?CURRENT_STATE.MEM_WB.ALU_OUT:CURRENT_STATE.MEM_WB.MEM_OUT;
               
        }
        if(CURRENT_STATE.MEM_WB.DEST!=0&&CURRENT_STATE.MEM_WB.DEST==RT(get_inst_info(CURRENT_STATE.PIPE[ID_STAGE]))&&!(OPCODE(get_inst_info(CURRENT_STATE.PIPE[EX_STAGE]))==(0x23||0x0)&&CURRENT_STATE.EX_MEM.DEST!=0&&CURRENT_STATE.EX_MEM.DEST!=RT(get_inst_info(CURRENT_STATE.PIPE[ID_STAGE]))))
        {
            /***************************************************************/
            /*  if (MEM/WB.RegWrite and (MEM/WB.RegisterRd ≠  0)           */
            /*  and not(EX/MEM.RegWrite and (EX/MEM.RegisterRd ≠  0)       */ 
            /*          and (EX/MEM.RegisterRd ≠  ID/EX.RegisterRt))       */ 
            /*  and  (MEM/WB.RegisterRd = ID/EX.RegisterRt)) ForwardB = 01 */
            /***************************************************************/
            CURRENT_STATE.ID_EX.REG2=(inst==0)?CURRENT_STATE.MEM_WB.ALU_OUT:CURRENT_STATE.MEM_WB.MEM_OUT;    
        }
        break;
        default:
        break;
    }

return;
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

    CURRENT_STATE.PIPE[EX_STAGE]=CURRENT_STATE.PIPE[ID_STAGE];

    CURRENT_STATE.EX_MEM.NPC=CURRENT_STATE.ID_EX.NPC;

    //LOAD instruction EX stage
    //READ REG1 and SIGN_EX_IMM
    //ALU operation
    CURRENT_STATE.EX_MEM.ALU_OUT=CURRENT_STATE.ID_EX.REG1+CURRENT_STATE.ID_EX.IMM;
    //In the book, it assumes that REG2 will be stored in EX/MEM only in STORE instruction.
    //But in the code, i will store REG2 in EX/MEM in LOAD and STORE inst universally.
    CURRENT_STATE.EX_MEM.REG2=CURRENT_STATE.ID_EX.REG2;

    //Modified pipeline datapath for LOAD instruction
    //R-Type Dest is RD field
    CURRENT_STATE.EX_MEM.DEST=CURRENT_STATE.ID_EX.DEST;

    //lw,sw instruction uses SIGN_EX_IMM as ALU operand

    //R-Type and beq instruction uses Read Data2(REG2) as ALU operand

return;
}

/***************************************************************/
/*                                                             */
/* Procedure: MEM                                              */
/*                                                             */
/* Purpose: access memory for load and store operationsn       */
/*                                                             */
/***************************************************************/
void MEM(){

    CURRENT_STATE.PIPE[MEM_STAGE]=CURRENT_STATE.PIPE[EX_STAGE];

    CURRENT_STATE.MEM_WB.NPC=CURRENT_STATE.EX_MEM.NPC;

    //FOR MEM HAZARD
    CURRENT_STATE.MEM_WB.ALU_OUT=CURRENT_STATE.EX_MEM.ALU_OUT;

    //LOAD instruction MEM stage
    CURRENT_STATE.MEM_WB.MEM_OUT=mem_read_32(CURRENT_STATE.EX_MEM.ALU_OUT);

    //STORE instruction MEM stage
    mem_write_32(CURRENT_STATE.EX_MEM.ALU_OUT,CURRENT_STATE.EX_MEM.REG2);

    //Modified pipeline datapath for LOAD instruction
    CURRENT_STATE.MEM_WB.DEST=CURRENT_STATE.EX_MEM.DEST;

    //beq instruction and ALU_ZERO then branch address

    //lw and sw sequence instruction DATA HAZARD
    if(OPCODE(get_inst_info(CURRENT_STATE.PIPE[WB_STAGE]))==0x23&&OPCODE(get_inst_info(CURRENT_STATE.PIPE[MEM_STAGE]))==0x2b){
        mem_write_32(CURRENT_STATE.EX_MEM.ALU_OUT,CURRENT_STATE.MEM_WB.MEM_OUT);
    }

return;
}

/***************************************************************/
/*                                                             */
/* Procedure: WB                                               */
/*                                                             */
/* Purpose: write back the result to the register              */
/*                                                             */
/***************************************************************/
void WB(){

    CURRENT_STATE.PIPE[WB_STAGE]=CURRENT_STATE.PIPE[MEM_STAGE];

    //LOAD instruction WB stage
    // Will be modified because of Design Bugs
    // CURRENT_STATE.ID_EX.REG2=CURRENT_STATE.MEM_WB.MEM_OUT;

    //Modified pipeline datapath for LOAD instruction
    CURRENT_STATE.REGS[CURRENT_STATE.MEM_WB.DEST]=CURRENT_STATE.MEM_WB.MEM_OUT;

    //STORE instruction nothing to do

    //R-Type writes Register but not MemoryToRegister

    


return;
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
    WB();
    MEM();
    EX();
    ID();
    IF();

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
