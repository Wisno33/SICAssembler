//  pass2.h


#ifndef pass2_h
#define pass2_h

//Public functions.

//Pass 2 of the SIC assembler input is a SIC program output will be a SIC object file.
int pass2(hash_table* dir_tab, hash_table* instruct_tab, hash_table* sym_tab, FILE* input, char* input_file_name, int first_instruction, int program_size);

#endif /* pass2_h */
