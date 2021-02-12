//  pass1.h

#ifndef pass1_h
#define pass1_h

//Public functions.

//Pass 1 of the SIC assembler input is a SIC program output will be a symbol table.
int pass1(hash_table* dir_tab, hash_table* instruct_tab, hash_table* sym_tab, FILE* input, int* first_instruction, int* program_size);

#endif /* pass1_h */
