/***************************************************************/
/*                                                             */
/*   MIPS-32 Instruction Level Simulator                       */
/*                                                             */
/*   SCE212 Ajou University                                    */
/*   parse.c                                                   */
/*   Adapted from CS311@KAIST                                  */
/*                                                             */
/***************************************************************/

#include <stdio.h>

#include "util.h"
#include "parse.h"

int text_size;
int data_size;

instruction parsing_instr(const char *buffer, const int index)
{
    instruction instr;
	/** Implement this function */
    uint32_t address=MEM_TEXT_START+index;
    mem_write_32(address,fromBinary((char*)buffer));
    char temp[33];
    strncpy(temp,(char*)buffer,6);
    temp[6]='\0';
    instr.opcode=fromBinary(temp);
switch(instr.opcode)
        {
            //Type I
            case 0x9:		//(0x001001)ADDIU
            case 0xc:		//(0x001100)ANDI
            case 0xf:		//(0x001111)LUI	
            case 0xd:		//(0x001101)ORI
            case 0xb:		//(0x001011)SLTIU
            case 0x23:		//(0x100011)LW
            case 0x2b:		//(0x101011)SW
            case 0x4:		//(0x000100)BEQ
            case 0x5:		//(0x000101)BNE
            strncpy(temp,(char*)buffer+6,5);
            temp[5]='\0';
            instr.r_t.r_i.rs=fromBinary(temp);//rs
            strncpy(temp,(char*)buffer+11,5);
            temp[5]='\0';
            instr.r_t.r_i.rt=fromBinary(temp);//rt
            strncpy(temp,(char*)buffer+16,16);
            temp[16]='\0';
            instr.r_t.r_i.r_i.imm=fromBinary(temp);//imm
            break;
 

            //TYPE R
            case 0x0:		//(0x000000)ADDU, AND, NOR, OR, SLTU, SLL, SRL, SUBU  if JR
            strncpy(temp,(char*)buffer+6,5);
            temp[5]='\0';
            instr.r_t.r_i.rs=fromBinary(temp);//rs
            strncpy(temp,(char*)buffer+11,5);
            temp[5]='\0';
            instr.r_t.r_i.rt=fromBinary(temp);//rt
            strncpy(temp,(char*)buffer+16,5);
            temp[5]='\0';
            instr.r_t.r_i.r_i.r.rd=fromBinary(temp);//rd
            strncpy(temp,(char*)buffer+21,5);
            temp[5]='\0';
            instr.r_t.r_i.r_i.r.shamt=fromBinary(temp);//shamt
            strncpy(temp,(char*)buffer+26,6);
            temp[6]='\0';
            instr.func_code=fromBinary(temp);//funct
            break;


            //TYPE J
            case 0x2:		//(0x000010)J
            case 0x3:		//(0x000011)JAL
            strncpy(temp,(char*)buffer+6,26);
            temp[26]='\0';
            instr.r_t.target=fromBinary(temp);//add
            break;

            default:
            break;

        }
         instr.value=address;
    return instr;
}

void parsing_data(const char *buffer, const int index)
{
	/** Implement this function */
    uint32_t address=MEM_DATA_START+index;
    mem_write_32(address,fromBinary((char*)buffer));
    return;
}

void print_parse_result()
{
    int i;
    printf("Instruction Information\n");

    for(i = 0; i < text_size/4; i++)
    {
        printf("INST_INFO[%d].value : %x\n",i, INST_INFO[i].value);
        printf("INST_INFO[%d].opcode : %d\n",i, INST_INFO[i].opcode);

	    switch(INST_INFO[i].opcode)
        {
            //Type I
            case 0x9:		//(0x001001)ADDIU
            case 0xc:		//(0x001100)ANDI
            case 0xf:		//(0x001111)LUI	
            case 0xd:		//(0x001101)ORI
            case 0xb:		//(0x001011)SLTIU
            case 0x23:		//(0x100011)LW
            case 0x2b:		//(0x101011)SW
            case 0x4:		//(0x000100)BEQ
            case 0x5:		//(0x000101)BNE
                printf("INST_INFO[%d].rs : %d\n",i, INST_INFO[i].r_t.r_i.rs);
                printf("INST_INFO[%d].rt : %d\n",i, INST_INFO[i].r_t.r_i.rt);
                printf("INST_INFO[%d].imm : %d\n",i, INST_INFO[i].r_t.r_i.r_i.imm);
                break;

            //TYPE R
            case 0x0:		//(0x000000)ADDU, AND, NOR, OR, SLTU, SLL, SRL, SUBU  if JR
                printf("INST_INFO[%d].func_code : %d\n",i, INST_INFO[i].func_code);
                printf("INST_INFO[%d].rs : %d\n",i, INST_INFO[i].r_t.r_i.rs);
                printf("INST_INFO[%d].rt : %d\n",i, INST_INFO[i].r_t.r_i.rt);
                printf("INST_INFO[%d].rd : %d\n",i, INST_INFO[i].r_t.r_i.r_i.r.rd);
                printf("INST_INFO[%d].shamt : %d\n",i, INST_INFO[i].r_t.r_i.r_i.r.shamt);
                break;

            //TYPE J
            case 0x2:		//(0x000010)J
            case 0x3:		//(0x000011)JAL
                printf("INST_INFO[%d].target : %d\n",i, INST_INFO[i].r_t.target);
                break;

            default:
                printf("Not available instruction\n");
                assert(0);
        }
    }

    printf("Memory Dump - Text Segment\n");
    for(i = 0; i < text_size; i+=4)
        printf("text_seg[%d] : %x\n", i, mem_read_32(MEM_TEXT_START + i));
    for(i = 0; i < data_size; i+=4)
        printf("data_seg[%d] : %x\n", i, mem_read_32(MEM_DATA_START + i));
    printf("Current PC: %x\n", CURRENT_STATE.PC);
}
