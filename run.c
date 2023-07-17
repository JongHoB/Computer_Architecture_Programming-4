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

    //Flushed instruction state
     if(!CURRENT_STATE.PIPE[IF_STAGE]&&CURRENT_STATE.PIPE_STALL[IF_STAGE]){
        CURRENT_STATE.PIPE_STALL[IF_STAGE]=0;
        return;
    }

    //IF instruction DELAY after ID stage stall
    if(CURRENT_STATE.PIPE_STALL[IF_STAGE]==1){
        CURRENT_STATE.PIPE_STALL[IF_STAGE]=0;
        return;
    }

    //ID stage stalled
    //IF stage will read same instruction in ID stage(DELAY)
    if(CURRENT_STATE.PIPE_STALL[IF_STAGE]==CURRENT_STATE.PC){
        CURRENT_STATE.PC = CURRENT_STATE.PIPE_STALL[ID_STAGE];
        CURRENT_STATE.PIPE_STALL[ID_STAGE]=1;
        CURRENT_STATE.PIPE_STALL[IF_STAGE]=1;
    }

    if(FETCH_BIT==FALSE&&CURRENT_STATE.PIPE[MEM_STAGE]==0){
        RUN_BIT=FALSE;
        return;
    }

    if ((CURRENT_STATE.PC - MEM_TEXT_START) / 4>= MAX_INSTRUCTION_NUM)
    {
        CURRENT_STATE.PIPE[IF_STAGE]=0;
        FETCH_BIT=FALSE;
        return;
    }
    FETCH_BIT=TRUE;

    CURRENT_STATE.PIPE[IF_STAGE]=CURRENT_STATE.PC;

    //LOAD instruction IF stage-1
    //STORE instruction
    CURRENT_STATE.IF_ID.Instr=CURRENT_STATE.PIPE[IF_STAGE];

    //LOAD instruction IF stage-2
    CURRENT_STATE.PC+=4;

    //Unconditional jump(J,JAL,JR)
    if(CURRENT_STATE.JUMP_PC){
        CURRENT_STATE.PC=CURRENT_STATE.JUMP_PC;
    }

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

    //Flushed instruction state
    if(!CURRENT_STATE.PIPE[ID_STAGE]&&CURRENT_STATE.PIPE_STALL[ID_STAGE]){
        CURRENT_STATE.PIPE_STALL[ID_STAGE]=0;
        return;
    }

    //Unconditional jump(J,JAL,JR)
    if(CURRENT_STATE.JUMP_PC){
        CURRENT_STATE.JUMP_PC=0;
        CURRENT_STATE.PIPE[ID_STAGE]=0;
        return;
    }

    //IF->ID->EX stage DELAY 
    if(CURRENT_STATE.PIPE_STALL[ID_STAGE]==1){
        CURRENT_STATE.PIPE_STALL[ID_STAGE]=0;
        return;
    }

    CURRENT_STATE.PIPE[ID_STAGE]=CURRENT_STATE.PIPE[IF_STAGE];

    if(!CURRENT_STATE.PIPE[ID_STAGE]){
        return;
    }

    if(CURRENT_STATE.PIPE_STALL[ID_STAGE]==CURRENT_STATE.PIPE[ID_STAGE]){
        CURRENT_STATE.PIPE_STALL[ID_STAGE]=0;
    }

    //HAZARD DETECTION UNIT
    //It can insert the stall between load
    //IF ID/EX is lw format, and ID/EX.REG2 is equal to IF/ID.REG1 or ID/EX.REG2 is equal to IF/ID.REG2
    //Stall the pipeline(DELAY ID AND IF stage instruction)
    if(CURRENT_STATE.PIPE[EX_STAGE]){
        if(OPCODE(get_inst_info(CURRENT_STATE.PIPE[EX_STAGE]))==0x23&&(((int)RT(get_inst_info(CURRENT_STATE.PIPE[EX_STAGE]))==(int)RS(get_inst_info(CURRENT_STATE.PIPE[ID_STAGE])))||((int)RT(get_inst_info(CURRENT_STATE.PIPE[EX_STAGE]))==(int)RT(get_inst_info(CURRENT_STATE.PIPE[ID_STAGE]))))){

            //IF ID stage is stalled, IF stage must also be stalled
            //PC register and IF/ID register must be preserved
            CURRENT_STATE.PIPE_STALL[ID_STAGE]=CURRENT_STATE.PIPE[ID_STAGE];
            CURRENT_STATE.PIPE_STALL[IF_STAGE]=CURRENT_STATE.IF_ID.NPC;

            //AFTER ID stage, each stage must excute NOP instruction
            CURRENT_STATE.PIPE_STALL[EX_STAGE]=CURRENT_STATE.PIPE[ID_STAGE];
            CURRENT_STATE.PIPE_STALL[MEM_STAGE]=CURRENT_STATE.PIPE[ID_STAGE];
            CURRENT_STATE.PIPE_STALL[WB_STAGE]=CURRENT_STATE.PIPE[ID_STAGE];

            //CURRENT_STATE.IF_PC=CURRENT_STATE.PIPE[ID_STAGE];// why border...?
            return;
        }
    }

    //For unconditional jump(J,JAL,JR)
    //Always add a one-cycle stall to the pipeline
    //Flush the pipeline of IF stage
    //JAL instuction, save the PC+8 value into R31 register
    //After every JAL instruction, NOP instruction(add $0,$0,$0) is inserted
    uint32_t jump;
    if(OPCODE(get_inst_info(CURRENT_STATE.PIPE[ID_STAGE]))==0x2||OPCODE(get_inst_info(CURRENT_STATE.PIPE[ID_STAGE]))==0x3||(OPCODE(get_inst_info(CURRENT_STATE.PIPE[ID_STAGE]))==0x0&&FUNC(get_inst_info(CURRENT_STATE.PIPE[ID_STAGE]))==0x08)){
        
        switch(OPCODE(get_inst_info(CURRENT_STATE.PIPE[ID_STAGE]))){
            //TYPE J
            case 0x2:		//(0x000010)J
            jump=((CURRENT_STATE.PIPE[ID_STAGE])&0xf0000000)+(TARGET(get_inst_info(CURRENT_STATE.PIPE[ID_STAGE]))<<2);
            CURRENT_STATE.JUMP_PC=jump;
            break;
            case 0x3:		//(0x000011)JAL
            // CURRENT_STATE.REGS[31] = CURRENT_STATE.PC + 4; --> ㅇㅒ가  문제야!!!!!!!!!!!
            CURRENT_STATE.ID_EX.DEST=31;
            jump=((CURRENT_STATE.PIPE[ID_STAGE]+4)&0xf0000000)+(TARGET(get_inst_info(CURRENT_STATE.PIPE[ID_STAGE]))<<2);
            CURRENT_STATE.JUMP_PC=jump;
            break;
            case 0x0:        //(0x000000)JR
            CURRENT_STATE.JUMP_PC=CURRENT_STATE.REGS[(int)RS(get_inst_info(CURRENT_STATE.PIPE[ID_STAGE]))];
            break;
        }
        
        return;
    }

    //LOAD instruction ID stage-1
    CURRENT_STATE.ID_EX.NPC=CURRENT_STATE.IF_ID.NPC;

    //LOAD instruction ID stage-2
    //STORE instruction
    CURRENT_STATE.ID_EX.REG1=CURRENT_STATE.REGS[(int)RS(get_inst_info(CURRENT_STATE.PIPE[ID_STAGE]))];
    CURRENT_STATE.ID_EX.REG2=CURRENT_STATE.REGS[(int)RT(get_inst_info(CURRENT_STATE.PIPE[ID_STAGE]))];
   
    CURRENT_STATE.ID_EX.IMM=IMM(get_inst_info(CURRENT_STATE.PIPE[ID_STAGE])); 
    CURRENT_STATE.ID_EX.SIGN_EX_IMM=SIGN_EX(CURRENT_STATE.ID_EX.IMM);
    printf("RS: %llu , RT: %llu ,REG1: %llu , REG2 : %llu , IMM: %llu\n",RS(get_inst_info(CURRENT_STATE.PIPE[ID_STAGE])),RT(get_inst_info(CURRENT_STATE.PIPE[ID_STAGE])),CURRENT_STATE.ID_EX.REG1,CURRENT_STATE.ID_EX.REG2,CURRENT_STATE.ID_EX.IMM); 
    //Modified pipeline datapath for LOAD instruction
    //lw,sw,beq instruction Write Register comes from RT field
    CURRENT_STATE.ID_EX.DEST=(int)RT(get_inst_info(CURRENT_STATE.PIPE[ID_STAGE]));
    
    //R-Type
    //Write Register comes from RD field
    if(OPCODE(get_inst_info(CURRENT_STATE.PIPE[ID_STAGE]))==0x0)
    {
        CURRENT_STATE.ID_EX.DEST=(int)RD(get_inst_info(CURRENT_STATE.PIPE[ID_STAGE]));
    }
    //EX HAZARD
    //IF EX/MEM is lw or R format, and EX/MEM.DEST is equal to ID/EX.REG1 or ID/EX.REG2
    //NEED TO FORWARD EX/MEM.ALU_OUT to ALU operand
    //ALSO LA instruction!
    if(CURRENT_STATE.PIPE[EX_STAGE]){
        switch(OPCODE(get_inst_info(CURRENT_STATE.PIPE[EX_STAGE]))){
            case 0x9://addiu
            case 0xc://andi
            case 0xf://lui
            case 0xd://ori
            case 0xb://sltiu
            case 0x23://lw
            case 0x0://R-Type

            //FORWARDING_BIT=FALSE; --> WHY BORDER...?

            if(CURRENT_STATE.EX_MEM.DEST!=0&&CURRENT_STATE.EX_MEM.DEST==(int)RS(get_inst_info(CURRENT_STATE.PIPE[ID_STAGE])))
            {
                CURRENT_STATE.ID_EX.REG1=CURRENT_STATE.EX_MEM.ALU_OUT;
            }
            if(CURRENT_STATE.EX_MEM.DEST!=0&&CURRENT_STATE.EX_MEM.DEST==(int)RT(get_inst_info(CURRENT_STATE.PIPE[ID_STAGE])))
            {
                CURRENT_STATE.ID_EX.REG2=CURRENT_STATE.EX_MEM.ALU_OUT;
            }
            break;
            default:
            break;
        }
    }
    

    //MEM HAZARD
    //IF MEM/WB is lw or R format, and MEM/WB.DEST is equal to ID/EX.REG1 or ID/EX.REG2
    //NEED TO FORWARD MEM/WB.MEM_OUT to ALU operand
    if(CURRENT_STATE.PIPE[MEM_STAGE]){
        int inst=0;
        switch(OPCODE(get_inst_info(CURRENT_STATE.PIPE[MEM_STAGE]))){

            case 0x23://lw
            inst=1;
            case 0x9://addiu
            case 0xc://andi
            case 0xf://lui
            case 0xd://ori
            case 0xb://sltiu
            case 0x0://R-Type

            //FORWARDING_BIT=FALSE; --> WHY BORDER...?

            if(CURRENT_STATE.MEM_WB.DEST!=0&&CURRENT_STATE.MEM_WB.DEST==(int)RS(get_inst_info(CURRENT_STATE.PIPE[ID_STAGE]))&&!(OPCODE(get_inst_info(CURRENT_STATE.PIPE[EX_STAGE]))==(0x23||0x0)&&CURRENT_STATE.EX_MEM.DEST!=0&&CURRENT_STATE.EX_MEM.DEST!=(int)RS(get_inst_info(CURRENT_STATE.PIPE[ID_STAGE]))))
            {
                /***************************************************************/
                /*  if (MEM/WB.RegWrite and (MEM/WB.RegisterRd ≠  0)           */ 
                /*  and not(EX/MEM.RegWrite and (EX/MEM.RegisterRd ≠  0)       */ 
                /*          and (EX/MEM.RegisterRd ≠  ID/EX.RegisterRs))       */ 
                /*  and  (MEM/WB.RegisterRd = ID/EX.RegisterRs)) ForwardA = 01 */
                /***************************************************************/ 
                CURRENT_STATE.ID_EX.REG1=(inst==0)?CURRENT_STATE.MEM_WB.ALU_OUT:CURRENT_STATE.MEM_WB.MEM_OUT;
               
            }
            if(CURRENT_STATE.MEM_WB.DEST!=0&&CURRENT_STATE.MEM_WB.DEST==(int)RT(get_inst_info(CURRENT_STATE.PIPE[ID_STAGE]))&&!(OPCODE(get_inst_info(CURRENT_STATE.PIPE[EX_STAGE]))==(0x23||0x0)&&CURRENT_STATE.EX_MEM.DEST!=0&&CURRENT_STATE.EX_MEM.DEST!=(int)RT(get_inst_info(CURRENT_STATE.PIPE[ID_STAGE]))))
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

    //Flushed instruction state
    if(!CURRENT_STATE.PIPE[EX_STAGE]&&CURRENT_STATE.PIPE_STALL[EX_STAGE]){
        CURRENT_STATE.PIPE_STALL[EX_STAGE]=0;
        return;
    }

    CURRENT_STATE.PIPE[EX_STAGE]=CURRENT_STATE.PIPE[ID_STAGE];
    
    if(!CURRENT_STATE.PIPE[EX_STAGE]){
        return;
    }

    //EX stage NOP instruction because of ID stage stall
    if(CURRENT_STATE.PIPE_STALL[EX_STAGE]==CURRENT_STATE.PIPE[EX_STAGE]){
        CURRENT_STATE.PIPE[EX_STAGE]=0;
        CURRENT_STATE.PIPE_STALL[EX_STAGE]=0;
        //CURRENT_STATE.PIPE[EX_STAGE]=NULL;
        return;
    }

    CURRENT_STATE.EX_MEM.NPC=CURRENT_STATE.ID_EX.NPC;


    //In the book, it assumes that REG2 will be stored in EX/MEM only in STORE instruction.
    //But in the code, i will store REG2 in EX/MEM in LOAD and STORE inst universally.
    CURRENT_STATE.EX_MEM.REG2=CURRENT_STATE.ID_EX.REG2;

    //Modified pipeline datapath for LOAD instruction
    //R-Type Dest is RD field
    CURRENT_STATE.EX_MEM.DEST=CURRENT_STATE.ID_EX.DEST;

    //lw,sw instruction uses SIGN_EX_IMM as ALU operand


    //ALU operation
    //R-Type and beq instruction uses Read Data2(REG2) as ALU operand
    switch(OPCODE(get_inst_info(CURRENT_STATE.PIPE[EX_STAGE]))){

        //Type I
        case 0x9:		//(0x001001)ADDIU
        CURRENT_STATE.EX_MEM.ALU_OUT=CURRENT_STATE.ID_EX.REG1+CURRENT_STATE.ID_EX.SIGN_EX_IMM; 
        break;
        case 0xc:		//(0x001100)ANDI
        CURRENT_STATE.EX_MEM.ALU_OUT=CURRENT_STATE.ID_EX.REG1&CURRENT_STATE.ID_EX.IMM;
        break;
        case 0xf:		//(0x001111)LUI	
        CURRENT_STATE.EX_MEM.ALU_OUT=CURRENT_STATE.ID_EX.IMM<<16; 
        printf("LUI:0x%x\n",CURRENT_STATE.EX_MEM.ALU_OUT);
        break;
        case 0xd:		//(0x001101)ORI
        printf("CURRENT REG1: 0x%x, IMM: 0x%x\n",CURRENT_STATE.ID_EX.REG1,CURRENT_STATE.ID_EX.IMM);
        CURRENT_STATE.EX_MEM.ALU_OUT=CURRENT_STATE.ID_EX.REG1|CURRENT_STATE.ID_EX.IMM;
        printf("ORI:0x%x\n",CURRENT_STATE.EX_MEM.ALU_OUT);
        break;
        case 0xb:		//(0x001011)SLTIU
        CURRENT_STATE.EX_MEM.ALU_OUT=(CURRENT_STATE.ID_EX.REG1 < CURRENT_STATE.ID_EX.SIGN_EX_IMM) ? 1 : 0;
        break;

        //LOAD instruction EX stage
        //READ REG1 and SIGN_EX_IMM
        //ALU operationss
        case 0x23:		//(0x100011)LW
        case 0x2b:		//(0x101011)SW
        CURRENT_STATE.EX_MEM.ALU_OUT=CURRENT_STATE.ID_EX.REG1+CURRENT_STATE.ID_EX.SIGN_EX_IMM; 
        break;

        case 0x4:		//(0x000100)BEQ
        if(CURRENT_STATE.ID_EX.REG1 == CURRENT_STATE.ID_EX.REG2){
            CURRENT_STATE.EX_MEM.BR_TAKE=CURRENT_STATE.PIPE[EX_STAGE]+4+IDISP(get_inst_info(CURRENT_STATE.PIPE[EX_STAGE]));
        };
        break;
        case 0x5:		//(0x000101)BNE
        // printf("!!!!!!!cur PC: 0x%x, IDISP:0x%x\n",CURRENT_STATE.PIPE[EX_STAGE],IDISP(get_inst_info(CURRENT_STATE.PIPE[EX_STAGE])));
        if(CURRENT_STATE.ID_EX.REG1 != CURRENT_STATE.ID_EX.REG2){
            CURRENT_STATE.EX_MEM.BR_TAKE=CURRENT_STATE.PIPE[EX_STAGE]+4+IDISP(get_inst_info(CURRENT_STATE.PIPE[EX_STAGE]));
        };
        break;

        //TYPE R
        case 0x0:		//(0x000000)ADDU, AND, NOR, OR, SLTU, SLL, SRL, SUBU  if JR
            switch(FUNC(get_inst_info(CURRENT_STATE.PIPE[EX_STAGE])))
            {
                case 0x21:
                CURRENT_STATE.EX_MEM.ALU_OUT = CURRENT_STATE.ID_EX.REG1 + CURRENT_STATE.ID_EX.REG2;
                break;
                case 0x24:
                CURRENT_STATE.EX_MEM.ALU_OUT = CURRENT_STATE.ID_EX.REG1 & CURRENT_STATE.ID_EX.REG2;
                break;
                case 0x27:
                CURRENT_STATE.EX_MEM.ALU_OUT = ~(CURRENT_STATE.ID_EX.REG1 | CURRENT_STATE.ID_EX.REG2);
                break;
                case 0x25:
                CURRENT_STATE.EX_MEM.ALU_OUT = CURRENT_STATE.ID_EX.REG1 | CURRENT_STATE.ID_EX.REG2;
                break;
                case 0x2B:
                CURRENT_STATE.EX_MEM.ALU_OUT = (CURRENT_STATE.ID_EX.REG1 < CURRENT_STATE.ID_EX.REG2) ? 1 : 0;
                break;
                case 0x00:
                CURRENT_STATE.EX_MEM.ALU_OUT = CURRENT_STATE.ID_EX.REG2 << SHAMT(get_inst_info(CURRENT_STATE.PIPE[EX_STAGE]));
                break;
                case 0x02:
                CURRENT_STATE.EX_MEM.ALU_OUT = CURRENT_STATE.ID_EX.REG2 >> SHAMT(get_inst_info(CURRENT_STATE.PIPE[EX_STAGE]));
                break;
                case 0x23:
                CURRENT_STATE.EX_MEM.ALU_OUT = CURRENT_STATE.ID_EX.REG1 - CURRENT_STATE.ID_EX.REG2;
                break;
            }
            break;


        //TYPE J
        case 0x3:		//(0x000011)JAL
        CURRENT_STATE.EX_MEM.ALU_OUT = CURRENT_STATE.PIPE[EX_STAGE] + 8;
        break; 

        default:
        break;

    }
    printf("EX_MEM.ALU_OUT : 0x%x\n",CURRENT_STATE.EX_MEM.ALU_OUT);
    // printf("func: %llu\n",FUNC(get_inst_info(CURRENT_STATE.PIPE[EX_STAGE])));
    // printf("opcode: %llu\n",OPCODE(get_inst_info(CURRENT_STATE.PIPE[EX_STAGE])));
    // printf("BR_TAKE: 0x%x\n",CURRENT_STATE.EX_MEM.BR_TAKE);

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

    if(!CURRENT_STATE.PIPE[MEM_STAGE]){
        return;
    }

    //MEM stage NOP instruction because of ID stage stall
    if(CURRENT_STATE.PIPE_STALL[MEM_STAGE]==CURRENT_STATE.PIPE[MEM_STAGE]){
        CURRENT_STATE.PIPE_STALL[MEM_STAGE]=0;
        return;
    }

    CURRENT_STATE.MEM_WB.NPC=CURRENT_STATE.EX_MEM.NPC;

    //FOR MEM HAZARD
    CURRENT_STATE.MEM_WB.ALU_OUT=CURRENT_STATE.EX_MEM.ALU_OUT;

    //Modified pipeline datapath for LOAD instruction
    CURRENT_STATE.MEM_WB.DEST=CURRENT_STATE.EX_MEM.DEST;

    //beq instruction and ALU_ZERO then branch address

    //lw and sw sequence instruction DATA HAZARD
    if(CURRENT_STATE.PIPE[WB_STAGE]){
        if(OPCODE(get_inst_info(CURRENT_STATE.PIPE[WB_STAGE]))==0x23&&OPCODE(get_inst_info(CURRENT_STATE.PIPE[MEM_STAGE]))==0x2b){
            mem_write_32(CURRENT_STATE.EX_MEM.ALU_OUT,CURRENT_STATE.MEM_WB.MEM_OUT);
        }
    }


   switch(OPCODE(get_inst_info(CURRENT_STATE.PIPE[MEM_STAGE])))
        {
            //Type I
            case 0x23:		//(0x100011)LW
            //LOAD instruction MEM stage
            CURRENT_STATE.MEM_WB.MEM_OUT=mem_read_32(CURRENT_STATE.EX_MEM.ALU_OUT);
            break;
            case 0x2b:		//(0x101011)SW
            //STORE instruction MEM stage
            mem_write_32(CURRENT_STATE.EX_MEM.ALU_OUT,CURRENT_STATE.EX_MEM.REG2);
            break;
            case 0x4:		//(0x000100)BEQ
            case 0x5:		//(0x000101)BNE
            //conditional branches(BEQ,BNE)
            //branch taken
            if(CURRENT_STATE.EX_MEM.BR_TAKE){
       
                //Flush the pipeline in IF,ID,EX stage
                CURRENT_STATE.PIPE_STALL[IF_STAGE]=CURRENT_STATE.PC;
                CURRENT_STATE.PIPE_STALL[ID_STAGE]=CURRENT_STATE.PIPE[IF_STAGE];
                CURRENT_STATE.PIPE_STALL[EX_STAGE]=CURRENT_STATE.PIPE[ID_STAGE];
                CURRENT_STATE.PIPE[IF_STAGE]=0;
                CURRENT_STATE.PIPE[ID_STAGE]=0;
                CURRENT_STATE.PIPE[EX_STAGE]=0;
                BRANCH_INST(1,CURRENT_STATE.EX_MEM.BR_TAKE,);
                CURRENT_STATE.EX_MEM.BR_TAKE=0;
            }
            break;

            default:
            break;

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

    if(!CURRENT_STATE.PIPE[WB_STAGE]){
        return;
    }

    //WB stage NOP instruction because of ID stage stall
    if(CURRENT_STATE.PIPE_STALL[WB_STAGE]==CURRENT_STATE.PIPE[WB_STAGE]){
        CURRENT_STATE.PIPE_STALL[WB_STAGE]=0;
        //CURRENT_STATE.PIPE[WB_STAGE]=NULL;
        return;
    }

    //LOAD instruction WB stage
    // Will be modified because of Design Bugs
    // CURRENT_STATE.ID_EX.REG2=CURRENT_STATE.MEM_WB.MEM_OUT;

    //Modified pipeline datapath for LOAD instruction
    //CURRENT_STATE.REGS[CURRENT_STATE.MEM_WB.DEST]=CURRENT_STATE.MEM_WB.MEM_OUT;

    //STORE instruction nothing to do

    //R-Type writes Register but not MemoryToRegister
    // printf("opcode: %llu dest: %llu memout:%llu\n",OPCODE(get_inst_info(CURRENT_STATE.PIPE[WB_STAGE])),CURRENT_STATE.MEM_WB.DEST,CURRENT_STATE.MEM_WB.MEM_OUT);

   switch(OPCODE(get_inst_info(CURRENT_STATE.PIPE[WB_STAGE])))
        {
            //Type I
            case 0x9:		//(0x001001)ADDIU
            case 0xc:		//(0x001100)ANDI
            case 0xf:		//(0x001111)LUI	
            case 0xd:		//(0x001101)ORI
            case 0xb:		//(0x001011)SLTIU 
            CURRENT_STATE.REGS[CURRENT_STATE.MEM_WB.DEST]=CURRENT_STATE.MEM_WB.ALU_OUT; 
            break;
            case 0x23:		//(0x100011)LW
            CURRENT_STATE.REGS[CURRENT_STATE.MEM_WB.DEST]=CURRENT_STATE.MEM_WB.MEM_OUT;
            break;

            //TYPE R
            case 0x0:		//(0x000000)ADDU, AND, NOR, OR, SLTU, SLL, SRL, SUBU 
            switch(FUNC(get_inst_info(CURRENT_STATE.PIPE[WB_STAGE])))
            {
                case 0x21: 
                case 0x24:
                case 0x27:
                case 0x25:
                case 0x2B:
                case 0x00:
                case 0x02:
                case 0x23:
                CURRENT_STATE.REGS[CURRENT_STATE.MEM_WB.DEST]=CURRENT_STATE.MEM_WB.ALU_OUT;
                break;            
            }
            break;

            //TYPE J
            case 0x3:		//(0x000011)JAL
            CURRENT_STATE.REGS[CURRENT_STATE.MEM_WB.DEST] = CURRENT_STATE.MEM_WB.ALU_OUT;
            break;

            default:
            break;

        }
        printf("opcode: %llu, func: %llu REGS7:0x%x\n", OPCODE(get_inst_info(CURRENT_STATE.PIPE[WB_STAGE])), FUNC(get_inst_info(CURRENT_STATE.PIPE[WB_STAGE])),CURRENT_STATE.REGS[7]);
 


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
