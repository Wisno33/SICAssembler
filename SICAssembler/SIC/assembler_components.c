//  assembler_components.c

//Header for function prototypes.
#include "assembler_components.h"

//Checks if a given string is a comment if the line is empty return treat as comment, return 0 (false), 1 (true/NULL)
int is_comment(char* line)
{
	if(line == NULL)
	{
		return 1;
	}
	
	char first_char = *line;
	
	if(first_char == 35)
	{
		return 1;
	}
	
	return 0;
}

//Attempts to match a # inside a string if found the index of the # is returned, -1 if not found.
int contains_comment(char *line)
{
	char line_copy[1024];
	memset(line_copy, '\0', 1024);
	strcpy(line_copy, line);
	
	int i=0;
	for(; i<strlen(line_copy); i++)
	{
		if(line_copy[i] == 35)
		{
			return i;
		}
	}
	
	return -1;
}

//Searches the provided directives table if not found return 0 (false/not found), else return 1 (true/found).
int is_directive(hash_table* directives, char* test)
{
    if(hash_table_get(directives, test) == NULL)
    {
        return 0;
    }
    
    return 1;
}

//Searches the provided instruction table if not found return 0 (false/not found), else return 1 (true/found).
int is_instruction(hash_table* instructions, char* test)
{
    if(hash_table_get(instructions, test) == NULL)
    {
		return 0;
    }
    
    return 1;
}

/*
 * 5 checks to see if the symbol is valid:
 * 1. no greater then 6 characters in length,
 * 2. not a directive
 * 3. not an instruction
 * 4. Starts with an A-Z or a-z
 * 5. composed of chars A-Z, a-z or 0-9 only,
 * 6. is not already a defined symbol.
 * If any of the above criteria is not met then return 0 (false), else return 1 (true).
 */
int is_valid_symbol(hash_table* sym_tab, char* test, hash_table* directives, hash_table* instructions)
{
    //1. no greater then 6 characters in length
    if(strlen(test) > 6)
    {
        //General error message.
        printf("ERROR! SYMBOL: %s is greater than six characters: ", test);
        return 0;
    }
    // 2. not a directive and 3. not an instruction
    else if(is_directive(directives, test) || is_instruction(instructions, test))
    {
        //General error message.
        printf("ERROR! SYMBOL: %s cannot be a directive or instruction: ", test);
        return 0;
    }
    
    else
    {
		//4. Starts with A-Z or a-z
		if(test[0] < 65 || test[0] > 122 || (test[0] > 90 && test[0] < 97))
		{
			printf("ERROR SYMBOL: %s starts with an invalid character: ", test);
			return 0;
		}
		
        //5. composed of chars A-Z only.
        int len = (int) strlen(test);
        int i=1;
        for(; i<len; i++)
        {
            if(test[i] < 48 || test[i] > 122 || (test[i] > 57 && test[i] < 65) || (test[i] > 90 && test[i] < 97) )
            {
                //General error message.
                printf("ERROR SYMBOL: %s contains invalid character(s): ", test);
                return 0;
            }
        }
        
        //6. is not already a defined symbol.
        if( hash_table_get(sym_tab, test) != NULL)
        {
            //General error message.
            printf("ERROR SYMBOL: %s already defined: ", test);
            return 0;
        }
    }
    
    return 1;
}

//Tests if content in the operand column is possibly valid. Returns 0 if not valid 1 if valid.
int test_operand_column(char* operand)
{
	//Operand can be null.
	if(operand == NULL)
	{
		return 1;
	}
	
	//Checks for quoted operands must have open and close, this also allows for spaces in the operand.
	int open_quotes_encountered = 0;
	int close_quotes_encountered = 0;
	
	int i = 0;
	for(; i<strlen(operand); i++)
	{
		//If an operand has a space not in quotes this is a bad operand, unless it is all whitespace.
		if((operand[i] == 32 || operand[i] == 9 || operand[i] == 13) && !open_quotes_encountered)
		{
			//Checks if the whitespace is just extra whitespace not a space leading to text i.e. another operand.
			int j = i+1;
			for(; j<strlen(operand); j++)
			{
				//Whitespace = whitespace tab or cr.
				if(operand[j] == 32 || operand[j] == 9 || operand[j] == 13)
				{
					continue;
				}
				else
				{
					printf("ERROR: Multiple operands ");
					return 0;
				}
			}
		}
		
		//Begin a valid string operand.
		else if (operand[i] == 39 && !open_quotes_encountered)
		{
			open_quotes_encountered = 1;
		}
		
		//End a valid string operand.
		else if(operand[i] == 39 && open_quotes_encountered && !close_quotes_encountered)
		{
			close_quotes_encountered = 1;
		}
		
		//A second string in the operand column.
		else if (operand[i] == 39 && close_quotes_encountered)
		{
			printf("ERROR: Invalid string operand ");
			return 0;
		}
	}
	
	//Invalid string not closed.
	if(open_quotes_encountered && !close_quotes_encountered)
	{
		printf("ERROR: Invalid string operand ");
		return 0;
	}
	
	return 1;
}
