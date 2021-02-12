//  pass1.c

//STD C libraries.
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

//Helper functions
#include "Functions.h"

//Data Structures
#include "Hash_Table.h"
#include "Queue.h"

//SIC libraries.
#include "assembler_components.h"

//Header for function prototypes.
#include "pass1.h"

//Prints the symbol table constructed from output.
void print_sym_tab(hash_table* sym_tab, queue* order)
{
	while(order->size > 0)
	{
		char* sym_name = queue_dequeue(order);
		
		symbol* sym =  hash_table_get(sym_tab, sym_name);
		
		printf("%6s\t%X\n", sym->name, sym->address);
	}
	
	return;
}

//Pass 1 of the SIC assembler input is a SIC program output will be a symbol table.
int pass1(hash_table* dir_tab, hash_table* instruct_tab, hash_table* sym_tab, FILE* input, int* first_instruction, int* program_size)
{
	int source_line = 1;
	int location_counter = 0;
	int start_symbol = 0;
	int start_dir_encountered = 0;
	int end_dir_encountered = 0;
	int start_address = 0;
	char* token1;
	char* token2;
	char* token3;
	char line[1024];
	
	//This stack is used to preserved the order of the symbols for printing to std out.
	queue* symbol_order = queue_init(128);
	
	while(fgets(line, 1024, input))
	{
		//Checks if the line is a comment if true the line is discarded. If the line is not a comment it must be processed.
		if(!is_comment(line))
		{
			//Get tokens, a line may have up to 3 tokens and a comment, comments are discarded.
			//Input lines take this general form: O denotes optional.
			/*
			 *  Column:     1           2           3           4
			 *           Symbol(o)   Dir/Ins    Operand(O)  Comment(O)
			 */
			
			//Finds the index of a comment if it is within a line.
			int comment_index = contains_comment(line);
			
			//If a comment is found the line is terminated at the index of the comment.
			if(comment_index != -1)
			{
				line[comment_index] = '\0';
			}
			
			//Hold a copy of the line so no information is lost on tokenization if the line contains a comment it is discarded.
			char line_copy[1024];
			memset(line_copy, '\0', 1024);
			strcpy(line_copy, line);
			
			//Get first token. (Possibly NULL)
			char* token = strtok(line_copy, " \t\n\r");
			token1 = token;
			
			//Get second token. (Possibly NULL)
			token = strtok(NULL, " \t\n\r");
			token2 = token;
			
			//Get third token. (Possibly NULL)
			token = strtok(NULL, "\n");
			token3 = token;
			
			//If the line is empty move to next line of source.
			if(token1 == NULL)
			{
				source_line += 1;
				continue;
			}
			
			//Line contains a possible symbol.
			if(line[0] > 32 && line[0] < 127 )
			{
				//Checks if possible symbol is valid if so the symbol is added to the symbol table.
				if( is_valid_symbol(sym_tab, token1, dir_tab, instruct_tab) )
				{
					symbol* sym = malloc(sizeof(symbol));
					
					sym->source_line = source_line;
					sym->address = location_counter;
					strcpy(sym->name, token1);
				
					hash_table_element* el = malloc(sizeof(hash_table_element));
					el->key = sym->name;
					el->value = sym;
					
					hash_table_set(sym_tab, el->key, el);
					
					queue_element* q_el = malloc(sizeof(queue_element));
					
					q_el->item = el->key;
					
					queue_enqueue(symbol_order, q_el);
				}
				//Prints line error message after the detailed error printed in assembler_components.
				else
				{
					printf("Line: %d, %s\n", source_line, token1);
					return 1;
				}
			}
			
			//Line did not contain a possible symbol, tokens moved to corresponding input column if a symbol was not in column 1.
			else
			{
				token3 = token2;
				token2 = token1;
				token1 = NULL;
			}
			
			
			
			//Removes extra whitespace on the tokens.
			token2 = remove_begin_whitespace(token2);
			token3 = remove_begin_whitespace(token3);
			
			//Verifies the value in the operand column is possibly valid.
			if(!test_operand_column(token3))
			{
				printf("Line: %d, %s", source_line, token3);
				return 1;
			}
			
			//Line has nothing left.
			if(token2 == NULL)
			{
				printf("ERROR! Unrecognized instruction, directive, or line format Line: %d\n", source_line);
				return 1;
			}
			
			//Checks if the line contains a directive, if so pass1 will take appropriate action.
			if(is_directive(dir_tab, token2))
			{
				//If the directive is START location counter is set to the operand provided.
				if(!strcmp(token2, "START"))
				{
					//If START is encountered already an error is displayed.
					if(start_dir_encountered)
					{
						printf("ERROR! Multiple START directives 2nd START Line %d, %s\n", source_line, token3);
						return 1;
					}
					
					//Checks if the START directive has a symbol.
					if(token1 != NULL)
					{
						start_symbol = 1;
					}
					
					start_dir_encountered = 1;
					
					if(token3 == NULL)
					{
						printf("ERROR! No operand for START directive Line: %d, %s\n", source_line, token3);
						return 1;
					}
					
					//Removes extra whitespace.
					remove_end_whitespace(token3);
					
					//Set the location counter and starting address as specified by the START directive.
					location_counter = (int) strtol(token3, NULL, 16);
					start_address = location_counter;
					
					//Add the location counter to the first symbol, if it exists.
					if(start_symbol)
					{
						symbol* start_symbol = hash_table_get(sym_tab, strtok(line, " \t"));
						start_symbol->address = location_counter;
					}
					
				}
				
				//If BYTE directive is encountered the location counter is increased by the size of the operand in bytes.
				else if(!strcmp(token2, "BYTE"))
				{
					if(token3 == NULL)
					{
						printf("ERROR! No operand for BYTE directive Line: %d, %s\n", source_line, token3);
						return 1;
					}
					
					
					
					//If the operand is a character constant location counter is incremented by len(operand) bytes.
					if(token3[0] == 'C')
					{
						//Moves pointer past the C.
						token3 += 1;
						char* char_constant = strtok(token3, "'");
						int char_constant_len  = (int) strlen(char_constant);
						
						location_counter += char_constant_len;
					}
					
					//If the operand is a hex constant location counter is incremented by len(operand)/2 bytes.
					else if(token3[0] == 'X')
					{
						//Moves pointer past the X.
						token3 += 1;
						char* hex_constant = strtok(token3, "'");
						int hex_constant_len  = (int) strlen(hex_constant);
						
						//If the operand is odd numbered an additional half byte is added.
						if(hex_constant_len % 2)
						{
							hex_constant_len += 1;
						}
						
						location_counter += hex_constant_len/2;
					}
					
					//If the type is nor C or X error.
					else
					{
						printf("ERROR! Invalid type for BYTE directive Line: %d, %c\n", source_line, token3[0]);
						return 1;
					}
				}
				
				//If WORD is encountered the location counter is incremented the size of a SIC word 3 bytes.
				else if(!strcmp(token2, "WORD"))
				{
					if(token3 == NULL)
					{
						printf("ERROR! No operand for WORD directive Line: %d, %s\n", source_line, token3);
						return 1;
					}
					
					//Removes extra whitespace.
					remove_end_whitespace(token3);
					
					//If the operand for WORD is not an integer error.
					if(!is_integer(token3))
					{
						printf("ERROR! Operand for WORD is not a integer Line: %d, %s\n", source_line, token3);
						return 1;
					}
					
					//If the integer constant is too large for the SIC architecture error.
					long int_literal = strtol(token3, NULL, 10);
					if(int_literal >= 16777216)
					{
						printf("ERROR! Int literal is too large Line: %d, %lx\n", source_line, int_literal);
						return 1;
					}
					
					location_counter += 3;
				}
				
				//Reserves the specified number of bytes, and increments the location counter by the number of bytes reserved.
				else if(!strcmp(token2, "RESB"))
				{
					if(token3 == NULL)
					{
						printf("ERROR! No operand for RESB directive Line: %d, %s\n", source_line, token3);
						return 1;
					}
					
					//Removes extra whitespace.
					remove_end_whitespace(token3);
					
					//If the operand for RESB is not an integer error.
					if(!is_integer(token3))
					{
						printf("ERROR! Operand for RESB is not a integer Line: %d, %s\n", source_line, token3);
						return 1;
					}
					
					int bytes_reserved = (int) strtol(token3, NULL, 10);
					
					location_counter += bytes_reserved;
				}
				
				//Reserves the specified number of words, and increments the location counter by number of words * 3.
				else if(!strcmp(token2, "RESW"))
				{
					if(token3 == NULL)
					{
						printf("ERROR! No operand for RESW directive Line: %d, %s\n", source_line, token3);
						return 1;
					}
					
					//Removes extra whitespace.
					remove_end_whitespace(token3);
					
					//If the operand for RESW is not an integer error.
					if(!is_integer(token3))
					{
						printf("ERROR! Operand for RESW is not a integer Line: %d, %s\n", source_line, token3);
						return 1;
					}
					
					int words_reserved = (int) strtol(token3, NULL, 10);
					
					location_counter += words_reserved * 3;
				}
				
				//Reserves space for an external reference 3 bytes.
				else if(!strcmp(token2, "RESR"))
				{
					if(token3 == NULL)
					{
						printf("ERROR! No operand for RESR directive Line: %d, %s\n", source_line, token3);
						return 1;
					}
					
					//Removes extra whitespace.
					remove_end_whitespace(token3);
					
					location_counter += 3;
				}
				
				//Export the symbol address pass 2.
				else if(!strcmp(token2, "EXPORTS"))
				{
					if(token3 == NULL)
					{
						printf("ERROR! No operand for EXPORTS directive Line: %d, %s\n", source_line, token3);
						return 1;
					}
					
					//Removes extra whitespace.
					remove_end_whitespace(token3);
				}
				
				//End directive.
				else
				{
					if(token3 != NULL)
					{
						//Removes extra whitespace.
						remove_end_whitespace(token3);
						
						symbol* temp = hash_table_get(sym_tab, token3);
						
						//If the symbol for the first instruction does not exist error.
						if(temp == NULL)
						{
							printf("ERROR! Not a valid first instruction for END directive Line: %d\n", source_line);
							return 1;
						}
						
						*first_instruction = temp->address;
					}
					
					end_dir_encountered = 1;
				}
			}
			
			//Checks if the line contains an instruction if so add instruction length to the location counter.
			else if(is_instruction(instruct_tab, token2))
			{
				instruction* temp = (instruction*) hash_table_get(instruct_tab, token2);
				location_counter += temp->size;
			}
			
			//Errors due to a non valid line format.
			else
			{
				printf("ERROR! Unrecognized instruction, directive, or line format Line: %d, %s\n", source_line, token2);
				return 1;
			}
		}
		source_line += 1;
	}
	
	//Errors and exits if no start directive is found in the input.
	if(!start_dir_encountered)
	{
		printf("ERROR! No START directive\n");
		return 1;
	}
	
	//Errors and exits if no end directive is found in the input.
	if(!end_dir_encountered)
	{
		printf("ERROR! No END directive\n");
		return 1;
	}
	
	//Calculate the size of the program.
	*program_size = location_counter - start_address;
	
	//If the program size is larger than the SIC memory error.
	if(*program_size > MAX_ADDRESS)
	{
		printf("ERROR! Program is larger than available memory %X is > %X\n", *program_size, MAX_ADDRESS);
		return 1;
	}
	
	//Checks if the program goes past the highest address.
	if(location_counter > MAX_ADDRESS)
	{
		printf("ERROR! Program exceeds highest memory address %X\n", MAX_ADDRESS);
		return 1;
	}
	
	//Prints the symbol table.
	print_sym_tab(sym_tab, symbol_order);
	
	return 0;
}
