//  assembler_components.h

#ifndef assembler_components_h
#define assembler_components_h

//STD C libraries.
#include <stdio.h>
#include <string.h>

//Data Structures
#include "Hash_Table.h"
#include "Queue.h"

/*
 * Structures used in during the assembly process.
 */

//Constants.

#define MAX_ADDRESS 32768

//Structures.

//Directive structures. Directives have a name.
typedef struct directive
{
    char* name;
} directive;

//Instruction structure. Instructions have a mnemonic, opcode in hex and a length or size of the instruction.
typedef struct instruction
{
    int size;
    unsigned char opcode;
    char* mnemonic;
    
} instruction;

/*
 * Symbol structure. Symbols have a location address in hex, a source line in the input file and a name.
 * 1. Symbols cannot be defined with a name which matches an assembler directive
 * 2. Symbols must start with an alpha character [A-Z]
 * 3. Symbols cannot be longer than six characters
 * 4. Symbols cannot contain spaces, $, !, =, +, - , (,  ), or@
 */
typedef struct symbol
{
    int source_line;
    int address;
    char name[7];
} symbol;

//Structure to contain a header record to be written to a file.
typedef struct header_record
{
	char identifier;
	int start_address;
	int program_size;
	char name[7];
} header_record;

//Structure to contain a text record to be written to a file.
typedef struct text_record
{
	char identifier;
	int start_address;
	int record_len;
	queue* object_data;
} text_record;

//Structure to contain an end record to be written to a file.
typedef struct end_record
{
	char identifier;
	int first_instruction;
} end_record;


//Structure to contain a modification record to be written to a file.
typedef struct modification_record
{
	char identifier;
} modification_record;

//Public functions.

//Checks if a given string is a comment, return 0 (false), 1 (true)
int is_comment(char* line);

//Attempts to match a # inside a string if found the index of the # is returned.
int contains_comment(char *line);

//Searches the provided directives table if not found return 0 (false/not found), else return 1 (true/found).
int is_directive(hash_table* directives, char* test);

//Searches the provided instruction table if not found return 0 (false/not found), else return 1 (true/found).
int is_instruction(hash_table* instructions, char* test);

/*
 * 5 checks to see if the symbol is valid:
 * 1. no greater then 6 characters in length,
 * 2. not a directive
 * 3. not an instruction
 * 4. composed of chars A-Z only,
 * 5. is not already a defined symbol.
 * If any of the above criteria is not met then return 0 (false), else return 1 (true).
 */
int is_valid_symbol(hash_table* sym_tab, char* test, hash_table* directives, hash_table* instructions);

//Tests if content in the operand column is possibly valid.
int test_operand_column(char* partial_line);

#endif /* assembler_components_h */
