// I have not used C language code obtained from other students, the
// Internet, and any other unauthorized sources, either modified or
// unmodified. If any code in my program was obtained from an authorized
// source, such as textbook or course notes, that has been clearly noted
// as a citation in the comments of the program.
// Adam Elias
// eliasadam@Knights.ucf.edu

#include "spimcore.h"

/* ALU */
/* 10 Points */
void ALU(unsigned A, unsigned B, char ALUControl, unsigned *ALUresult, char *Zero)
{
    switch (ALUControl) 
    {
        case 0: // Addition
            *ALUresult = A + B;
            break;

        case 1: // Subtraction
            *ALUresult = A - B;
            break;

        case 2: // Set on less than (signed)
            *ALUresult = ((int)A < (int)B) ? 1 : 0;
            break;

        case 3: // Set on less than (unsigned)
            *ALUresult = (A < B) ? 1 : 0;
            break;

        case 4: // Bitwise AND
            *ALUresult = A & B;
            break;

        case 5: // Bitwise OR
            *ALUresult = A | B;
            break;

        case 6: // Shift left B by 16 bits
            *ALUresult = B << 16;
            break;

        case 7: // Bitwise NOT
            *ALUresult = ~A;
            break;
    }

    *Zero = (*ALUresult == 0) ? 1 : 0;
}

/* Instruction Fetch */
/* 10 Points */
int instruction_fetch(unsigned PC, unsigned *Mem, unsigned *instruction)
{
    // Check if PC is beyond the range of 0x0000 to 0xFFFF or is not word-aligned
    if (PC < 0x0000 || PC > 0xFFFC || PC % 4 != 0) {
        return 1; // Halt condition
    }

    *instruction = Mem[PC >> 2]; // Fetch instruction from memory
    return 0; // No halt condition
}

/* Instruction Partition */
/* 10 Points */
void instruction_partition(unsigned instruction, unsigned *op, unsigned *r1, unsigned *r2, unsigned *r3, unsigned *funct, unsigned *offset, unsigned *jsec)
{
    *op = (instruction & 0xFC000000) >> 26; // Get bits 31-26
    *r1 = (instruction & 0x03E00000) >> 21; // Get bits 25-21
    *r2 = (instruction & 0x001F0000) >> 16; // Get bits 20-16
    *r3 = (instruction & 0x0000F800) >> 11; // Get bits 15-11
    *funct = instruction & 0x0000003F; // Get bits 5-0
    *offset = instruction & 0x0000FFFF; // Get bits 15-0
    *jsec = instruction & 0x03FFFFFF; // Get bits 25-0
}

/* Instruction Decode */
/* 15 Points */
int instruction_decode(unsigned op,struct_controls *controls)
{
    // Set default control signals to "don't care"
    controls->RegDst = 2;
    controls->Jump = 0;
    controls->Branch = 0;
    controls->MemRead = 0;
    controls->MemtoReg = 2;
    controls->ALUOp = 0;
    controls->MemWrite = 0;
    controls->ALUSrc = 0;
    controls->RegWrite = 0;

    // Determine the instruction type based on the opcode
    switch (op) 
    {
        case 0: // R-type instruction
            controls->RegDst = 1;
            controls->ALUOp = 7;
            controls->RegWrite = 1;
            break;

        case 35: // Lw instruction
            controls->MemRead = 1;
            controls->MemtoReg = 1;
            controls->ALUOp = 0;
            controls->ALUSrc = 1;
            controls->RegWrite = 1;
            break;

        case 43: // Sw instruction
            controls->MemWrite = 1;
            controls->ALUOp = 0;
            controls->ALUSrc = 1;
            break;

        case 4: // Beq instruction
            controls->Branch = 1;
            controls->ALUOp = 1;
            controls->ALUSrc = 0;
            break;

        case 10: // Slti instruction
            controls->ALUOp = 2;
            controls->ALUSrc = 1;
            controls->RegWrite = 1;
            break;

        case 11: // Sltiu instruction
            controls->ALUOp = 3;
            controls->ALUSrc = 1;
            controls->RegWrite = 1;
            break;

        case 8: // Addi instruction
            controls->ALUOp = 0;
            controls->ALUSrc = 1;
            controls->RegWrite = 1;
            break;

        case 15: // Lui instruction
            controls->ALUOp = 6;
            controls->ALUSrc = 1;
            controls->RegWrite = 1;
            break;

        default:
            return 1; // Halt condition
    }

    return 0; // No halt condition
}

/* Read Register */
/* 5 Points */
void read_register(unsigned r1,unsigned r2,unsigned *Reg,unsigned *data1,unsigned *data2)
{
    *data1 = Reg[r1]; // Read data from register r1
    *data2 = Reg[r2]; // Read data from register r2
}

/* Sign Extend */
/* 10 Points */
void sign_extend(unsigned offset,unsigned *extended_value)
{
    if (offset >> 15) { // check if sign bit is set
        *extended_value = 0xFFFF0000 | offset; // sign extend with 1's
    } 
    
    else {
        *extended_value = offset; // sign extend with 0's
    }
}

/* ALU Operations */
/* 10 Points */
int ALU_operations(unsigned data1,unsigned data2,unsigned extended_value,unsigned funct,char ALUOp,char ALUSrc,unsigned *ALUresult,char *Zero)
{
    if (ALUOp == 7) // if this is a R-type instruction
    { 
        // Determine ALU operation based on funct field
        if (funct == 0b100000) // Add instruction
            ALUOp = 0;
        else if (funct == 0b100010) // Subtract instruction
            ALUOp = 1;
        else if (funct == 0b100100) // And instruction
            ALUOp = 2;
        else if (funct == 0b100101) // Or instruction
            ALUOp = 3;
        else if (funct == 0b101010) // Set Less Than (signed) instruction
            ALUOp = 4;
        else if (funct == 0b101011) // Set Less Than (unsigned) instruction
            ALUOp = 5;

        // Determine second ALU operand based on ALUSrc field
        if (ALUSrc == 0)
            ALU(data1, data2, ALUOp, ALUresult, Zero); // Use data2 as second operand
        else
            ALU(data1, extended_value, ALUOp, ALUresult, Zero); // Use extended_value as second operand
    }

    // Handle other instruction types here

    return (*Zero == '1'); // Return result of zero flag comparison
}

/* Read / Write Memory */
/* 10 Points */
int rw_memory(unsigned ALUresult,unsigned data2,char MemWrite,char MemRead,unsigned *memdata,unsigned *Mem)
{
    // If MemRead != 0:
    if (MemRead) 
    {
        // If ALUresult is word-aligned:
        if (ALUresult % 4 == 0) 
        {
            // Read data from memory at memory address ALUresult
            *memdata = Mem[ALUresult/4];
        } 
        else {
            return 1; // Return error code 1 for misaligned memory access
        }
    }

    // If MemWrite != 0:
    if (MemWrite) 
    {
        // If ALUresult is word-aligned:
        if (ALUresult % 4 == 0) 
        {
            // Write data2 to memory at memory address ALUresult
            Mem[ALUresult/4] = data2;
        } 
        else {
            return 1; // Return error code 1 for misaligned memory access
        }
    }

    return 0; // Return success code 0
}

/* Write Register */
/* 10 Points */
void write_register(unsigned r2,unsigned r3,unsigned memdata,unsigned ALUresult,char RegWrite,char RegDst,char MemtoReg,unsigned *Reg)
{
    if (RegWrite == 0) 
    {
        // Do nothing if RegWrite is 0
        return;
    }

    unsigned value;
    if (MemtoReg) 
    {
        // Load value from memory if MemtoReg is set
        value = memdata;
    } 
    
    else 
    {
        // Use ALUresult if MemtoReg is not set
        value = ALUresult;
    }

    // Determine which register to write to based on RegDst
    unsigned reg_address;
    if (RegDst) {
        // If RegDst is set, write to register r3
        reg_address = r3;
    } 
    
    else {
        // If RegDst is not set, write to register r2
        reg_address = r2;
    }

    // Write the value to the register
    Reg[reg_address] = value;
}

/* PC Update */
/* 10 Points */
void PC_update(unsigned jsec,unsigned extended_value,char Branch,char Jump,char Zero,unsigned *PC)
{
    // Update PC with PC+4
    *PC += 4;
    
    // Check for branch and zero flag
    if (Branch && Zero) 
    {
        // Update PC according to previous PC and extended_value
        *PC += (extended_value << 2);
    }
    
    // Check for jump
    if (Jump) 
    {
        // Update PC according to previous PC and jsec
        *PC = ((*PC) & 0xf0000000) | (jsec << 2);
    }
}