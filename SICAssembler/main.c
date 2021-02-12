//
//  main.c
//  SIC_Assembler

/*
 * Two pass SIC Assembler, three phases, input is a .sic SIC assembly program and output is a .obj SIC object file.
 * 1. Initialize: required data is loaded from files, the directives and instruction set data.
 * 2. Pass 1: generates a symbol table and checks for some syntax and declaration errors.
 * 3. Pass 2: NOT IMPLEMENTED YET!
 */

//STD C libraries.
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

// Data Structures
#include "Hash_Table.h"

//SIC Library
#include "initialize.h"
#include "pass1.h"

int main(int argc, const char * argv[]) {
    
    //Checks if the correct number of arguments is provided.
    if(argc != 2)
    {
        printf("ERROR! Invalid argument count! Expected 1. USAGE: %s filename \n", argv[0]);
        return 1;
    }
    
    FILE* input;
    input = fopen(argv[1], "r");
    
    //Checks if the file can be read.
    if(input == NULL)
    {
        printf("ERROR! %s cannot be opened\n", argv[1]);
        return 1;
    }
    
    //Creates a directive, instruction, and symbol tables.
    hash_table* dir_tab = hash_table_init();
    hash_table* instruct_tab = hash_table_init();
    hash_table* sym_tab = hash_table_init();
    
    //Stores the address of the first instruction.
    int first_instruction = 0;
    int program_size = 0;
    initialize_assembler(dir_tab, instruct_tab);
    
	//Runs pass 1 of the assembler to build a symbol table and look for some syntax errors. If pass 1 returns a 1 the assembler exits.
    if(pass1(dir_tab, instruct_tab, sym_tab, input, &first_instruction, &program_size))
	{
        return 1;
	}
	
	//rewind(input);
	
	//if(pass2())
	//	return 1;
    
    fclose(input);
    
    return 0;
}
