//  pass2.c


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

#include "pass2.h"


//Creates an object file from a header record, set of text records, and end record.
void create_object_file(header_record header_r, queue* text_records, end_record end_r)
{
	//FILE* output;
	
	//The SIC object file.
	//output = fopen(strcat(input_file_name, ".o"), "w+");
}

//Pass 2 of the SIC assembler input is a SIC program output will be a SIC object file.
int pass2(hash_table* dir_tab, hash_table* instruct_tab, hash_table* sym_tab, FILE* input, char* input_file_name, int first_instruction, int program_size)
{
	//Record data.
	header_record header_r;
	queue* text_records = queue_init(128);
	end_record end_r;
	queue* mod_records = queue_init(128);
	
	int source_line = 1;
	int location_counter = 0;
	int header_record_created = 0;
	int end_record_created = 0;
	
	//Variables specific to creation of text record(s).
	int text_record_in_prog = 0;
	int text_record_end = 0;
	int column_counter = 1;
	int record_continuity_broken = 0;
	
	char* token1;
	char* token2;
	char* token3;
	char line[1024];
	
	
	while(fgets(line, 1024, input))
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
		
		//If the header record has not be created it is, the first line pass1 confirms START directive is the first valid line of code.
		if(!header_record_created)
		{
			symbol* start_sym = hash_table_get(sym_tab, token1);
		
			header_r.identifier = 'H';
			strcpy(header_r.name,start_sym->name);
			header_r.start_address = start_sym->address;
			header_r.program_size = program_size;
			
			//Initializes location_counter to program start.
			location_counter = start_sym->address;
			
			header_record_created = 1;
			
			source_line += 1;
			continue;
		}
		
		//Line contains a symbol ignore it.
		if(line[0] > 32 && line[0] < 127 )
		{
			token1 = NULL;
		}
		
		//Lines tokens into correct columns.
		else
		{
			token3 = token2;
			token2 = token1;
			token1 = NULL;
		}
		
		//Checks for end directive begins to end pass2 when encontred.
		if(!strcmp(token2, "END"))
		{
			end_r.identifier = 'E';
			end_r.first_instruction = first_instruction;
			
			end_record_created = 1;
			
			break;
		}
		
		//This section will create the text records.
		
		//Starts a text record if one is not in progress, pushes it to the text record queue for storage and modification.
		if(!text_record_in_prog)
		{
			text_record_in_prog = 1;
			
			text_record* text_r = malloc(sizeof(text_record));
			
			text_r->identifier = 'T';
			text_r->start_address = location_counter;
			text_r->object_data = queue_init(64);
			
			queue_element* el = malloc(sizeof(queue_element));
			
			el->item = text_r;
			
			queue_enqueue(text_records, el);
			
			column_counter = 10;
		}
		
		//Checks if the line contains a directive if so convert the operand (if it exists) to object code.
		if(is_directive(dir_tab, token2))
		{
			
		}
		
		//Checks if the line is an instruction converts to object code and checks for format errors.
		else if(is_instruction(instruct_tab, token2))
		{
			
			//Checks if an argument was supplied to the instruction is not NULL, also not RSUB(Special case no operand), if so generate object code.
			if(token3 != NULL)
			{
				//Checks for RSUB case.
				if(!strcmp(token2, "RSUB"))
				{
					printf("ERROR! Instruction RSUB does not use an operand, given, %s Line: %d\n", token3, source_line);
					return 1;
				}
				
				int indexing_mode = 0;
				
				//Splits the operand on a , to check for indexed addressing. The operand is also separated from the ",X"
				token = strtok(token3, ",");
				
				//The index mode character.
				token = strtok(NULL, "\n");
				
				if(token != NULL && *token == 'X')
				{
					indexing_mode = 1;
				}
				
				//Retrieve instruction data, and add it to the record.
				instruction* cur_instruct = hash_table_get(instruct_tab, token2);
			   
				queue_element* object_code_mnemonic = malloc(sizeof(queue_element));
			   
				object_code_mnemonic->item = &cur_instruct->opcode;
			   
				//Get symbol data.
				symbol* cur_operand = hash_table_get(sym_tab, token3);
			   
				//Checks if the operand is a valid symbol (exists in symbol table). If not error.
				if(cur_operand == NULL)
				{
					printf("ERORR! Symbol %s not defined Line: %d\n", token3, source_line);
				}
			   	
				queue_element* object_code_address = malloc(sizeof(queue_element));
			   
				object_code_address->item = &cur_operand->address;
				
				//Sets the 16th bit to 1.
				if(indexing_mode)
				{
					object_code_address->item += 32768;
				}
				
				//Gets current record data to be appended too.
				text_record* cur_t_record = queue_peek(text_records);
				
				//Adds data to current text records object code queue.
				queue_enqueue(cur_t_record->object_data, object_code_mnemonic);
				queue_enqueue(cur_t_record->object_data, object_code_address);
				
				location_counter += cur_instruct->size;
				
				column_counter += 6;
			}
			
			//Creates object code for RSUB (special case instruction no operand)
			else if(!(strcmp(token2, "RSUB")))
			{
				
				instruction* cur_instruct = hash_table_get(instruct_tab, token2);
				
				queue_element* object_code_mnemonic = malloc(sizeof(queue_element));
				
				object_code_mnemonic->item = &cur_instruct->opcode;
				
				text_record* cur_t_r = queue_peek(text_records);
				
				queue_enqueue(cur_t_r->object_data, object_code_mnemonic);
			}
			
			//If the instruction has no operand and is not RSUB requires operand error.
			else
			{
				printf("ERROR! No operand provided for %s requires 1 Line: %d\n", token2, source_line);
				return 1;
			}
			
			
			
		}
		
		source_line += 1;
		
	}
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	queue_print(text_records);
	
	
	create_object_file(header_r, text_records, end_r);
	
	return 0;
}
