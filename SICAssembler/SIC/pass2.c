//  pass2.c


//STD C libraries.
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

//Helper functions
#include "Functions.h"

//Data Structures
#include "Hash_Table.h"
#include "Queue.h"

//SIC libraries.
#include "assembler_components.h"

#include "pass2.h"


//Creates an object file from a header record, set of text records, and end record.
void create_object_file(char* input_file_name, header_record* header_r, queue* text_rs, end_record* end_r)
{
	//Removes the file extension to name the object file.
	remove_file_extension(input_file_name);
	
	FILE* output;
	
	//The SIC object file.
	output = fopen(strcat(input_file_name, ".o"), "w+");
	
	//Writes the header record to the object file.
	fprintf(output, "%c%-6s%06X%06X\n", header_r->identifier, header_r->name, header_r->start_address, header_r->program_size);
	
	//Writes text records to the object file.
	while(text_rs->size > 0)
	{
		text_record* text_r = queue_dequeue(text_rs);
		
		//Skip over empty text records.
		if(text_r->record_len == 0)
		{
			continue;
		}
		
		fprintf(output, "%c%06X%02X%s\n", text_r->identifier, text_r->start_address, text_r->record_len, text_r->data);
	}

	//Writes the end record
	fprintf(output, "%c%06X", end_r->identifier, end_r->first_instruction);
	
	fclose(output);
}

//Pass 2 of the SIC assembler input is a SIC program output will be a SIC object file.
int pass2(hash_table* dir_tab, hash_table* instruct_tab, hash_table* sym_tab, FILE* input, char* input_file_name, int first_instruction, int program_size)
{
	//Record data.
	header_record* header_r = NULL;
	end_record* end_r = NULL;
	
	//Pointer to current text record and the queue of all text records.
	text_record* text_r_cur = NULL;
	queue* text_rs = queue_init(128);
	
	//Individual object code to be written to the text record, never can exceed 60 hex chars.
	char* text_object_code_string = malloc(sizeof(char) * 60);
	memset(text_object_code_string, '\0', 60);
	
	//modification_record* mod_r_cur;
	//queue* mod_rs = queue_init(128);
	
	int source_line = 1;
	int location_counter = 0;
	int header_record_created = 0;
	
	//Variables specific to creation of text record(s).
	int text_record_in_prog = 0;
	int column_counter = 0;
	
	//Modification Record tracker.
	//int record_continuity_broken = 0;
	
	char* token1 = NULL;
	char* token2 = NULL;
	char* token3 = NULL;
	char line[1024];
	
	
	while(fgets(line, 1024, input))
	{
		//Reset the string formatter.
		memset(text_object_code_string, '\0', 60);
		
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
			
			header_r = malloc(sizeof(header_record));
		
			header_r->identifier = 'H';
			strcpy(header_r->name,start_sym->name);
			header_r->start_address = start_sym->address;
			header_r->program_size = program_size;
			
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
		
		//Removes extra whitespace on the tokens.
		token2 = remove_begin_whitespace(token2);
		token3 = remove_begin_whitespace(token3);
		
		//Checks for end directive begins to end pass2 when encontred.
		if(!strcmp(token2, "END"))
		{
			end_r = malloc(sizeof(end_record));
			
			end_r->identifier = 'E';
			end_r->first_instruction = first_instruction;
			
			source_line += 1;
			continue;
		}
		
		//Start a new text record if one is not in progress.
		if(!text_record_in_prog)
		{
			text_record_in_prog = 1;
			
			//Creates a new text record and places it in the queue of text records for writing to a file.
			text_r_cur = new_text_record(text_rs, location_counter, &column_counter);
		}
		
		//Checks if the line contains a directive if so convert the operand (if it exists) to object code.
		if(is_directive(dir_tab, token2))
		{
			//Add the character or hex constant to the text record string.
			if(!strcmp(token2, "BYTE"))
			{
				//If the operand is a character constant location counter is incremented by len(operand) bytes.
				if(token3[0] == 'C')
				{
					//Moves pointer past the C.
					token3 += 1;
					char* char_constant = strtok(token3, "'");
					int char_constant_len  = (int) strlen(char_constant);
					
					//If the new object code would overfill the text record a new one is created. (Each char is 2 hex chars)
					if(!is_room_left_text_record(char_constant_len * 2, &column_counter))
					{
						//Place as much as possible into current record.
						int columns_left = 70 - column_counter;
						int char_constant_len_remaining = char_constant_len;
						
						int index = 0;
						for(; index < columns_left / 2; index++)
						{
							//Convert the string of chars to ascii and then hex, and add it to the text record.
							sprintf(text_object_code_string, "%X", char_constant[index]);
							strcat(text_r_cur->data, text_object_code_string);
							
							--char_constant_len_remaining;
							
							column_counter += 2;
							location_counter += 1;
						}
						
						
						//BYTE constants can exceed multiple records so multiple may have to be created.
						//Get the number of records required for the constant.
						int text_records_needed = (int) ceil((char_constant_len_remaining * 2.0) / 60.0);
						
						int i = 0;
						
						for(; i < text_records_needed; i++)
						{
							//Finish the completed text record.
							text_r_cur->record_len = location_counter - text_r_cur->start_address;
							//text_r_cur->data[strlen(text_r_cur->data)] = '\0';
							
							//Create new text record.
							text_r_cur = new_text_record(text_rs, location_counter, &column_counter);
							
							int j = 0;
							for(; j < 30; j++)
							{
								//End if the constant is done before the record is full.
								if(index == char_constant_len)
								{
									break;
								}
								
								//Convert the string of chars to ascii and then hex, and add it to the text record.
								sprintf(text_object_code_string, "%X", char_constant[index++]);
								strcat(text_r_cur->data, text_object_code_string);
								
								column_counter += 2;
								location_counter += 1;
							}
						}
					}
					
					else
					{
						//Convert the string of chars to ascii and then hex, and add it to the text record.
						int i = 0;
						for(; i < char_constant_len; i++)
						{
							sprintf(text_object_code_string, "%X", char_constant[i]);
							strcat(text_r_cur->data, text_object_code_string);
						}
										
						location_counter += char_constant_len;
					}
				}
				
				//If the operand is a hex constant location counter is incremented by len(operand)/2 bytes.
				else if(token3[0] == 'X')
				{
					//Moves pointer past the X.
					token3 += 1;
					char* hex_constant = strtok(token3, "'");
					int hex_constant_len  = (int) strlen(hex_constant);
					
					//If the new object code would overfill the text record a new one is created. (Each char is 2 hex chars)
					if(!is_room_left_text_record(hex_constant_len, &column_counter))
					{
						//Place as much as possible into current record.
						int columns_left = 70 - column_counter;
						int hex_constant_len_remaining = hex_constant_len;
						
						int index = 0;
						for(; index < columns_left; index++)
						{
							//Convert the string of chars to ascii and then hex, and add it to the text record.
							sprintf(text_object_code_string, "%c", hex_constant[index]);
							strcat(text_r_cur->data, text_object_code_string);
							
							--hex_constant_len_remaining;
							
							column_counter += 1;
							
							//Only increment location counter every other hex char. (Each hex char is a half byte)
							if((index + 1) % 2 == 0)
							{
								location_counter += 1;
							}
						}
						
						
						//BYTE constants can exceed multiple records so multiple may have to be created.
						//Get the number of records required for the constant.
						int text_records_needed = (int) ceil(hex_constant_len_remaining / 60.0);
						
						int i = 0;
						
						for(; i < text_records_needed; i++)
						{
							//Finish the completed text record.
							text_r_cur->record_len = location_counter - text_r_cur->start_address;
							//text_r_cur->data[strlen(text_r_cur->data)] = '\0';
							
							//Create new text record.
							text_r_cur = new_text_record(text_rs, location_counter, &column_counter);
							
							int j = 0;
							for(; j < 60; j++)
							{
								//End if the constant is done before the record is full.
								if(index == hex_constant_len)
								{
									break;
								}
								
								sprintf(text_object_code_string, "%c", hex_constant[index++]);
								strcat(text_r_cur->data, text_object_code_string);
								
								column_counter += 1;
								
								//Only increment location counter every other hex char. (Each hex char is a half byte)
								if((index + 1) % 2 == 0)
								{
									location_counter += 1;
								}
							}
						}
					}
					
					else
					{
						//Convert the string of chars to ascii and then hex, and add it to the text record.
						int i = 0;
						for(; i < hex_constant_len; i++)
						{
							sprintf(text_object_code_string, "%c", hex_constant[i]);
							strcat(text_r_cur->data, text_object_code_string);
						}
										
						location_counter += hex_constant_len / 2;
					}
				}
			}
			
			//Add an integer literal to the text record string.
			else if(!strcmp(token2, "WORD"))
			{
				//Removes extra whitespace.
				remove_end_whitespace(token3);
				
				//If the new object code would overfill the text record a new one is created.
				if(!is_room_left_text_record(6, &column_counter))
				{
					//Finish the completed text record.
					text_r_cur->record_len = location_counter - text_r_cur->start_address;
					//text_r_cur->data[strlen(text_r_cur->data)] = '\0';
					
					//Create new text record.
					text_r_cur = new_text_record(text_rs, location_counter, &column_counter);
					
					column_counter += 6;
				}
				
				//Converts string to an integer.
				int integer_constant = atoi(token3);
				
				//Convert to hex and add constant to text record.
				sprintf(text_object_code_string, "%06X", integer_constant);
				strcat(text_r_cur->data, text_object_code_string);
				
				location_counter += 3;
			}
			
			//RESB directive generates no object code, but must be scanned for proper location counter tracking.
			else if(!strcmp(token2, "RESB"))
			{
				//Removes extra whitespace.
				remove_end_whitespace(token3);
				
				//End the text record, reserving memory is not documented in object code.
				text_r_cur->record_len = location_counter - text_r_cur->start_address;
				//text_r_cur->data[strlen(text_r_cur->data)] = '\0';
			
				text_record_in_prog = 0;
				
				int bytes_reserved = (int) strtol(token3, NULL, 10);
				
				location_counter += bytes_reserved;
			}
			
			//RESW directive generates no object code, but must be scanned for proper location counter tracking.
			else if(!strcmp(token2, "RESW"))
			{
				//Removes extra whitespace.
				remove_end_whitespace(token3);
				
				//End the text record, reserving memory is not documented in object code.
				text_r_cur->record_len = location_counter - text_r_cur->start_address;
				//text_r_cur->data[strlen(text_r_cur->data)] = '\0';
			
				text_record_in_prog = 0;
				
				int words_reserved = (int) strtol(token3, NULL, 10);
				
				location_counter += words_reserved * 3;
			}
			
			//RESR directive generates no object code, but must be scanned for proper location counter tracking.
			else if(!strcmp(token2, "RESR"))
			{
				//Removes extra whitespace.
				remove_end_whitespace(token3);
				
				//End the text record, reserving memory is not documented in object code.
				text_r_cur->record_len = location_counter - text_r_cur->start_address;
				//text_r_cur->data[strlen(text_r_cur->data)] = '\0';
			
				text_record_in_prog = 0;
				
				location_counter += 3;
			}
			
			//EXPORTS
			else if(!strcmp(token2, "EXPORTS"))
			{
			}
			
		}
		
		//Checks if the line is an instruction converts to object code and checks for format errors.
		else if(is_instruction(instruct_tab, token2))
		{
			
			//Checks if an argument was supplied to the instruction is not NULL, also not RSUB(Special case no operand), if so generate object code.
			if(token3 != NULL)
			{
				//Removes extra whitespace.
				remove_end_whitespace(token3);
				
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
				
				//If an X char is found then set indexing mode, value of indexing mode is the value of the 16th bit = 1.
				if(token != NULL && *token == 'X')
				{
					indexing_mode = 32768;
				}
				
				//Retrieve instruction data, and add it to the record.
				instruction* cur_instruct = hash_table_get(instruct_tab, token2);
			   
				//Get symbol data.
				symbol* cur_operand = hash_table_get(sym_tab, token3);
			   
				//Checks if the operand is a valid symbol (exists in symbol table). If not error.
				if(cur_operand == NULL)
				{
					printf("ERORR! Symbol %s not defined Line: %d\n", token3, source_line);
					return 1;
				}
				
				//If the new object code would overfill the text record a new one is created. (Size of instruction is in bytes * 2 = hex chars)
				if(!is_room_left_text_record(cur_instruct->size * 2, &column_counter))
				{
					//Finish the completed text record.
					text_r_cur->record_len = location_counter - text_r_cur->start_address;
					//text_r_cur->data[strlen(text_r_cur->data)] = '\0';
					
					//Create new text record.
					text_r_cur = new_text_record(text_rs, location_counter, &column_counter);
					
					column_counter += cur_instruct->size * 2;
				}
				
				//Convert to hex and add constant to text record.
				sprintf(text_object_code_string, "%02X%04X", cur_instruct->opcode, cur_operand->address + indexing_mode);
				strcat(text_r_cur->data, text_object_code_string);
				
				location_counter += cur_instruct->size;
				
			}
			
			//Creates object code for RSUB (special case instruction no operand)
			else if(!(strcmp(token2, "RSUB")))
			{
				
				instruction* cur_instruct = hash_table_get(instruct_tab, token2);
				
				//If the new object code would overfill the text record a new one is created. (Size of instruction is in bytes * 2 = hex chars)
				if(!is_room_left_text_record(cur_instruct->size * 2, &column_counter))
				{
					//Finish the completed text record.
					text_r_cur->record_len = location_counter - text_r_cur->start_address;
					//text_r_cur->data[strlen(text_r_cur->data)] = '\0';
					
					//Create new text record.
					text_r_cur = new_text_record(text_rs, location_counter, &column_counter);
					
					column_counter += cur_instruct->size * 2;
				}
				
				//Convert to hex and add constant to text record.
				sprintf(text_object_code_string, "%02X0000", cur_instruct->opcode);
				strcat(text_r_cur->data, text_object_code_string);
				
				location_counter += cur_instruct->size;
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
	
	//Finish the completed final text record.
	text_r_cur->record_len = location_counter - text_r_cur->start_address;
	//text_r_cur->data[strlen(text_r_cur->data)] = '\0';
	
	create_object_file(input_file_name, header_r, text_rs, end_r);
	
	return 0;
}
