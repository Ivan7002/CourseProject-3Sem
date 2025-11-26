#pragma once
#include <iostream>
#include <cstring>

#define MAXSIZE_ID	16						
#define SCOPED_ID_MAXSIZE   MAXSIZE_ID*2	
#define MAXSIZE_TI		4096				
#define INT_DEFAULT	0x00000000				
#define STR_DEFAULT	0x00					
#define NULLIDX_TI		0xffffffff			
#define STR_MAXSIZE	255						
#define CHAR_MAXSIZE 1						
#define INT_MAXSIZE   2147483647			
#define INT_MINSIZE   -2147483647			
#define UBYTE_MAXSIZE  255
#define UBYTE_MINSIZE  0
#define MAX_PARAMS_COUNT 3					
#define TOSTRING_PARAMS_CNT 1					
#define STRLEN_PARAMS_CNT 1					
#define TOSTRING_TYPE IT::IDDATATYPE::STR
#define STRLEN_TYPE IT::IDDATATYPE::INT


namespace IT
{
	enum IDDATATYPE { INT = 1, STR = 2, PROC = 3, CHAR = 4, BOOL = 5, UINT = 6, UNDEF };
	enum IDTYPE { V = 1, F = 2, P = 3, L = 4, S = 5 };	
	enum STDFNC { F_TOSTRING, F_STRLEN, F_NOT_STD };	
	static const IDDATATYPE TOSTRING_PARAMS[] = { IT::IDDATATYPE::INT };
	static const IDDATATYPE STRLEN_PARAMS[] = { IT::IDDATATYPE::STR };
	
	struct Entry
	{
		union
		{
			int	vint;            			
			struct
			{
				int len;					
				char str[STR_MAXSIZE - 1];
			} vstr;							
			struct
			{
				int count;					
				IDDATATYPE* types;			
			} params;
		} value;						
		int			idxfirstLE;				
		char		id[SCOPED_ID_MAXSIZE];	
		IDDATATYPE	iddatatype;				
		IDTYPE		idtype;					

		Entry()							
		{
			this->value.vint = INT_DEFAULT;
			this->value.vstr.len = NULL;
			this->value.params.count = NULL;
			this->id[0] = '\0'; 
		};
		Entry(char* id, int idxLT, IDDATATYPE datatype, IDTYPE idtype) 
		{
			strncpy_s(this->id, id, SCOPED_ID_MAXSIZE - 1);
			this->id[SCOPED_ID_MAXSIZE - 1] = '\0';
			this->idxfirstLE = idxLT;
			this->iddatatype = datatype;
			this->idtype = idtype;
		};
	};
	struct IdTable		
	{
		int maxsize;	
		int size;		
		Entry* table;	
	};
	IdTable Create(int size = NULL);	
	void Add(					
		IdTable& idtable,		
		Entry entry);			
	int isId(					
		IdTable& idtable,		
		char id[SCOPED_ID_MAXSIZE]);	
	bool SetValue(IT::Entry* entry, char* value);	
	bool SetValue(IT::IdTable& idtable, int index, char* value);
	void writeIdTable(std::ostream* stream, IT::IdTable& idtable); 
};
