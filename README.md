# SICAssembler
A two pass assember for the SIC architecture.

The assembler works in three major steps. The first of which is an initialization where the assembler loads data for the directives and instructions. This is useful as if the architecture changes, less modification to the source is needed. For new directives handling must be added, and for instructions no additional modification is required. The second step is the first pass of the assembler in which a symbol table is constructed. This table can be printed to stdout if the print function is not commented out. The final step is pass 2, here the symbol table is used with the source to generate the object code. 

The assembler also generates modification records for use with a relocating loader.

# Usage

The assembler takes a single argument, that being the SIC file to be assembled and will output a SIC object file with the same base file name. 

# SIC Documents

In the repository are a set of SIC documents detailing the architecture, instruction set, and object code format.
