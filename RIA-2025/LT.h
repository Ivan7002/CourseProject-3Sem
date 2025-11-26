#pragma once
#include <fstream>
#define LEX_SEPARATORS	 'S'	// Separators
#define	LEX_ID_TYPE    	 't'	// Type identifier		
#define	LEX_ID			 'i'	// Identifier		
#define	LEX_LITERAL		 'l'	// Literal		
#define	LEX_FUNCTION     'f'	// function		
#define	LEX_MAIN		 'm'	// main		
#define	LEX_SEPARATOR	 ';'	// Semicolon
#define	LEX_COMMA		 ','	// Comma 		
#define	LEX_LEFTBRACE	 '['	// Left brace		
#define	LEX_BRACELET	 ']'	// Right brace		
#define LEX_LEFT         '{'    // Left curly brace
#define LEX_RIGHT        '}'    // Right curly brace
#define	LEX_LEFTHESIS	 '('	// Left parenthesis		
#define	LEX_RIGHTTHESIS	 ')'	// Right parenthesis		
#define	LEX_PLUS		 '+'	// Plus	
#define	LEX_MINUS		 '-'	// Minus
#define	LEX_STAR		 '*'	// Star (Multiply)
#define LEX_DIRSLASH	 '/'	// Slash (Divide)
#define LEX_PERSENT		 '%'	// Percent (Mod)
#define	LEX_EQUAL		 '='	// Assign				
#define LEX_BITOR        '|'    // Bitwise OR
#define LEX_BITAND       '&'    // Bitwise AND
#define LEX_BITNOT       '~'    // Bitwise NOT (forced recompile)
#define LEX_TRUE		 'T'	// true
#define LEX_FALSE		 'F'	// false
#define LEX_DO  		 'd'	// do
#define LEX_WHILE		 'w'	// while
#define LEX_IF           '?'    // if
#define LEX_COUT		 'o'	// cout
#define LEX_NEWLINE		 '^'	// Newline (keep if needed, or remove)
#define LEX_RETURN		 'e'	// return
#define LEX_VOID		 'p'	// void/proc
#define LEX_DECLARE		 'v'	// declare (variable)
#define LEX_MORE		 '>'	// >
#define LEX_LESS		 '<'	// <
#define LEX_EQUALS		 'E'	// ==
#define LEX_NOTEQUALS	 'N'	// !=
#define LEX_MOREEQUALS	 'M'	// >=
#define LEX_LESSEQUALS	 'L'	// <=
#define LEX_LITERAL_HEX  'h'    // Hex literal
#define LEX_NOT          '!'    // Logical NOT
#define	LEXEMA_FIXSIZE   1	    // Lexeme fixed size
#define	MAXSIZE_LT		 4096	// Max size of LexTable
#define	NULLDX_TI	 0xffffffff	// Null index

namespace LT	// LexTable namespace
{
	struct Entry
	{
		char lexema;					// Lexeme
		int sn;							// Line number
		int idxTI;						// Index in IDTable

		Entry();
		Entry(char lexema, int snn, int idxti = NULLDX_TI);
	};

	struct LexTable						// LexTable structure
	{
		int maxsize;					// Max size
		int size;						// Current size
		Entry* table;					// Table array
	};

	LexTable Create(int size);		            // Create LexTable
	void Add(LexTable& lextable, Entry entry);	// Add entry
	void writeLexTable(std::ostream* stream, LT::LexTable& lextable);		// Write LexTable
	void writeLexemsOnLines(std::ostream* stream, LT::LexTable& lextable);	// Write lexemes by line
};
