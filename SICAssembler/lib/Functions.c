//Functions.c

//STD C libraries.
#include <stdio.h>
#include <string.h>
#include <ctype.h>

//Header for function prototypes.
#include "Functions.h"

//Removes extra white space from a string. Used to remove ending whitespace on lines.
char* remove_begin_whitespace(char* string)
{
	if(string == NULL)
	{
		return NULL;
	}
	
	int pointer_increment = 0;
	
	int i = 0;
	for(; i<strlen(string); i++)
	{
		if(string[i] < 33 || string[i] == 127)
		{
			pointer_increment += sizeof(string[i]);
		}
		else
		{
			break;
		}
		
	}
	
	//The whole string is whitespace return null as the string would be NULL minus the whitespace.
	if((pointer_increment / sizeof(char)) == strlen(string))
	{
		return NULL;
	}
	
	//Pointer arithmetic to point the pointer to the first non-whitespace char.
	string += pointer_increment;
	
	return string;
}

//Removes extra white space from a string. Used to remove ending whitespace on lines.
void remove_end_whitespace(char* string)
{
	if(string == NULL)
	{
		return;
	}
	
	//Finds the first whitespace in a string and sets it to the null char essentially terminating the string.
	int i = 0;
	for(; i<strlen(string); i++)
	{
		if(string[i] == 32 || string[i] == 9 || string[i] == 13)
		{
			string[i] = '\0';
			return;
		}
	}
	
	return;
}

//Checks if all characters of a string are digits i.e. the string is a number. Return 0 if false 1 if true.
int is_integer(char* num_string)
{
	int i = 0;
	for(; i<strlen(num_string); i++)
	{
		if(!isdigit(num_string[i]))
		{
			return 0;
		}
	}
	
	return 1;
}
