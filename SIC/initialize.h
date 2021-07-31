//  initialize.h

#ifndef initialize_h
#define initialize_h

//Constants
#define SIC_DIRECTIVES "SIC/directives.txt"
#define SIC_INSTRUCTIONS "SIC/instructions.txt"

//Public Functions.

//Creates the structures needed for the processing of assembly code. A predefined file of directives and instructions are needed.
void initialize_assembler(hash_table* dir_tab, hash_table* instruct_tab);

#endif /* initialize_h */
