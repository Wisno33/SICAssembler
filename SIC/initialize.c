//  initialize.c

/*
 * Functions to initialize the assembler for processing of a file. Allocates memory for a directive, instruction, and symbol
 * tables. Directives and instruction data is loaded from files into the associated tables.
 */

//STD C libraries.
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

//Data Structures
#include "Hash_Table.h"

//SIC_ASM libraries.
#include "assembler_components.h"

//Header for function prototypes.
#include "initialize.h"



//Creates the structures needed for the processing of assembly code. A predefined file of directives and instructions are needed.
void initialize_assembler(hash_table* dir_tab, hash_table* instruct_tab)
{
    char line[256];
    
    //Opens the directives file.
    FILE* directive_data;
    directive_data = fopen(SIC_DIRECTIVES, "r");
    
    if(directive_data == NULL)
    {
        printf("ERROR! No Directives\n");
        return;
    }
    
    //Pulls all directives from the directives file locates memory, and inserts them into a hash-table.
    while ( fgets( line, 256, directive_data))
    {
        hash_table_element* el = malloc(sizeof(hash_table_element));
        
        directive* dir = malloc(sizeof(directive));
        
        char* name = malloc(sizeof(char) * strlen(line));
        strcpy(name, line);
        name = strtok(name, "\n");
        
        dir->name = name;
        
        el->key = dir->name;
        el->value = dir;
        
        hash_table_set(dir_tab, el->key, el);
    }
    
    fclose(directive_data);
    
    //Opens the instructions file.
    FILE*  instruction_data;
    instruction_data = fopen(SIC_INSTRUCTIONS, "r");
    
    if(instruction_data == NULL)
    {
        printf("ERROR! No Instructions\n");
        return;
    }
    
    //Pulls all instructions from the instructions file, allocates memory and inserts them and associated data into a hash-table.
    while ( fgets( line, 256, instruction_data))
    {
        hash_table_element* el = malloc(sizeof(hash_table_element));
        
        instruction* instruct = malloc(sizeof(instruction));
        
        char* instruction_attribute = strtok(line, "\t");
        
        char* mnemonic = malloc(sizeof(char) * strlen(instruction_attribute));
        strcpy(mnemonic, instruction_attribute);
        instruct->mnemonic = mnemonic;
        
        instruction_attribute = strtok(NULL, "\t");
        instruct->size = atoi(instruction_attribute);
        
        instruction_attribute = strtok(NULL, "\t");
        instruct->opcode = strtol(instruction_attribute, NULL, 16);
        
        el->key = instruct->mnemonic;
        el->value = instruct;
        
        hash_table_set(instruct_tab, el->key, el);
    }
    
    fclose(instruction_data);
}
