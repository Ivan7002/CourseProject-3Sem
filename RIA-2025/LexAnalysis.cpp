#include "LexAnalysis.h"
#include "Graphs.h"
#include "Error.h"
#include "Log.h"
#include <stack>
#include <string>

#pragma warning(disable : 4996)

int DecimicalNotation(std::string input, int scaleofnot) {
	try {
		bool isNegative = !input.empty() && input[0] == '-';
		size_t startIdx = input.size() > 2 && (input[1] == 'x' || input[1] == 'X') ? 2 : 0;
		int value = std::stoi(input.substr(isNegative ? 1 + startIdx : startIdx), nullptr, scaleofnot);
		return isNegative ? -value : value;
	}
	catch (const std::out_of_range&) {
		return input[0] == '-' ? INT_MINSIZE : INT_MAXSIZE;
	}
}

namespace Lexer
{
	Graph graphs[N_GRAPHS] =
	{
		{ LEX_SEPARATORS, FST::FST(GRAPH_SEPARATORS) },
		{ LEX_LITERAL, FST::FST(GRAPH_INT_LITERAL) },
		{ LEX_LITERAL, FST::FST(GRAPH_STRING_LITERAL) },
		{ LEX_LITERAL, FST::FST(GRAPH_CHAR_LITERAL) },
		{ LEX_DECLARE, FST::FST(GRAPH_DECLARE) },
		{ LEX_MAIN, FST::FST(GRAPH_MAIN) },
		{ LEX_ID_TYPE, FST::FST(GRAPH_NUMBER) },
		{ LEX_ID_TYPE, FST::FST(GRAPH_STRING) },
		{ LEX_ID_TYPE, FST::FST(GRAPH_BOOL) },
		{ LEX_ID_TYPE, FST::FST(GRAPH_UNSIGNED) },
		{ LEX_FUNCTION, FST::FST(GRAPH_FUNCTION) },
		{ LEX_VOID, FST::FST(GRAPH_VOID) },
		{ LEX_RETURN, FST::FST(GRAPH_RETURN) },
		{ LEX_COUT, FST::FST(GRAPH_COUT) },
		{ LEX_DO, FST::FST(GRAPH_DO) },
		{ LEX_WHILE, FST::FST(GRAPH_WHILE) },
		{ LEX_IF, FST::FST(GRAPH_IF) },
		{ LEX_TRUE, FST::FST(GRAPH_TRUE) },
		{ LEX_FALSE, FST::FST(GRAPH_FALSE) },
		{ LEX_ID, FST::FST(GRAPH_ID) },
		{ LEX_LITERAL_HEX, FST::FST(GRAPH_HEX_LITERAL) }
	};

	char* getScopeName(IT::IdTable idtable, char* prevword) 
	{
		char* a = new char[5];
		a[0] = 'm';
		a[1] = 'a';
		a[2] = 'i';
		a[3] = 'n';
		a[4] = '\0';
		if (prevword && strcmp(prevword, "main") == 0)
			return a;
		for (int i = idtable.size - 1; i >= 0; i--)
			if (idtable.table[i].idtype == IT::IDTYPE::F)
				return idtable.table[i].id;
		return nullptr; 
	}

	int getLiteralIndex(IT::IdTable ittable, char* value, IT::IDDATATYPE type, Log::LOG log, int line) 
	{
		for (int i = 0; i < ittable.size; i++)
		{
			if (ittable.table[i].idtype == IT::IDTYPE::L && ittable.table[i].iddatatype == type)
			{
				switch (type)
				{
				case IT::IDDATATYPE::INT:
				case IT::IDDATATYPE::UINT: {
					if (ittable.table[i].value.vint == atoi(value))
						return i;
					break;
				}
				case IT::IDDATATYPE::BOOL: {
					bool val = (strcmp(value, "true") == 0);
					if (ittable.table[i].value.vint == val) 
						return i;
					break;
				}
				case IT::IDDATATYPE::STR:
					char buf[STR_MAXSIZE];
					for (unsigned j = 1; j < strlen(value) - 1; j++) 
						buf[j - 1] = value[j];
					buf[strlen(value) - 2] = '\0';
					if (strcmp(ittable.table[i].value.vstr.str, buf) == 0)
						return i;
					break;
				case IT::IDDATATYPE::CHAR:
					char buk[STR_MAXSIZE];
					for (unsigned j = 1; j < strlen(value) - 1; j++) 
						buk[j - 1] = value[j];
					buk[strlen(value) - 2] = '\0';
					if (strcmp(ittable.table[i].value.vstr.str, buk) == 0)
						return i;
					break;
				}
			}
		}
		return NULLIDX_TI;
	}

	IT::IDDATATYPE getType(char* curword, char* idtype)
	{
		if (idtype)
		{
			if (!strcmp("void", idtype))
				return IT::IDDATATYPE::PROC; 
			if (!strcmp("string", idtype))
				return IT::IDDATATYPE::STR;  
			if (!strcmp("integer", idtype))
				return IT::IDDATATYPE::INT;	 
			if (!strcmp("unsigned", idtype))
				return IT::IDDATATYPE::UINT;
			if (!strcmp("bool", idtype))
				return IT::IDDATATYPE::BOOL;
		}
		
		if (isdigit(*curword) || *curword == LEX_MINUS)
			return IT::IDDATATYPE::INT;				
		else if (*curword == '\"')
			return IT::IDDATATYPE::STR;	
		else if (*curword == '\'')
			return IT::IDDATATYPE::CHAR;
		else if (!strcmp(curword, "true") || !strcmp(curword, "false"))
			return IT::IDDATATYPE::BOOL;

		return IT::IDDATATYPE::UNDEF;		
	}

	int getIndexInLT(LT::LexTable& lextable, int itTableIndex)					
	{
		if (itTableIndex == NULLIDX_TI)		
			return lextable.size;
		for (int i = 0; i < lextable.size; i++)
			if (itTableIndex == lextable.table[i].idxTI)
				return i;
		return NULLIDX_TI;
	}

	bool isLiteral(char* id)
	{
		if (isdigit(*id) || *id == '\"' || *id == '\'' || *id == LEX_MINUS || !strcmp(id, "true") || !strcmp(id, "false"))
			return true;
		return false;
	}

	IT::STDFNC getStandFunction(char* id)
	{
		if (!strcmp("tostring", id))
			return IT::STDFNC::F_TOSTRING;
		if (!strcmp("strlen", id))
			return IT::STDFNC::F_STRLEN;
		return IT::STDFNC::F_NOT_STD;
	}

	char* getNextLiteralName()						
	{
		static int literaldigit = 1;
		char* buf = new char[SCOPED_ID_MAXSIZE], lich[10];
		strcpy_s(buf, MAXSIZE_ID, "LTRL");
		_itoa_s(literaldigit++, lich, 10);
		strcat(buf, lich);
		return buf;
	}

	IT::Entry* getEntry(						
		Lexer::LEX& tables,						
		char lex,								
		char* id,								
		char* idtype,							
		bool isParam,							
		bool isFunc,							
		Log::LOG log,							
		int line,								
		bool& lex_ok)							
	{
		
		IT::IDDATATYPE type = getType(id, idtype);
		if (lex == LEX_TRUE || lex == LEX_FALSE) {
			type = IT::IDDATATYPE::BOOL;
			lex = LEX_LITERAL; 
		}

		int index = IT::isId(tables.idtable, id);	
		if (lex == LEX_LITERAL)
			index = getLiteralIndex(tables.idtable, id, type, log, line);
		if (index != NULLIDX_TI)
			return nullptr;	

		IT::Entry* itentry = new IT::Entry;
		itentry->iddatatype = type; 

		itentry->idxfirstLE = getIndexInLT(tables.lextable, index);

		if (lex == LEX_LITERAL) 
		{
			bool int_ok = IT::SetValue(itentry, id);
			if (!int_ok)
			{
				Log::writeError(log.stream, Error::GetError(313, line, 0));
				lex_ok = false;
			}
			
			if (type == IT::IDDATATYPE::BOOL) {
				itentry->value.vint = (strcmp(id, "true") == 0) ? 1 : 0;
			}

			strcpy_s(itentry->id, getNextLiteralName());
			itentry->idtype = IT::IDTYPE::L;
		}
		else 
		{
			switch (type)
			{
			case IT::IDDATATYPE::STR:
				strcpy_s(itentry->value.vstr.str, "");
				itentry->value.vstr.len = STR_DEFAULT;
				break;
			case IT::IDDATATYPE::CHAR:
				strcpy_s(itentry->value.vstr.str, "");
				itentry->value.vstr.len = STR_DEFAULT;
				break;
			case IT::IDDATATYPE::INT:
			case IT::IDDATATYPE::UINT:
			case IT::IDDATATYPE::BOOL:
				itentry->value.vint = INT_DEFAULT;
				break;
			}

			if (isFunc)
			{
				switch (getStandFunction(id))
				{
				case IT::STDFNC::F_TOSTRING:
				{
					itentry->idtype = IT::IDTYPE::S;
					itentry->iddatatype = TOSTRING_TYPE;
					itentry->value.params.count = TOSTRING_PARAMS_CNT;
					itentry->value.params.types = new IT::IDDATATYPE[TOSTRING_PARAMS_CNT];
					for (int k = 0; k < TOSTRING_PARAMS_CNT; k++)
						itentry->value.params.types[k] = IT::TOSTRING_PARAMS[k];
					break;
				}
				case IT::STDFNC::F_STRLEN:
				{
					itentry->idtype = IT::IDTYPE::S;
					itentry->iddatatype = STRLEN_TYPE;
					itentry->value.params.count = STRLEN_PARAMS_CNT;
					itentry->value.params.types = new IT::IDDATATYPE[STRLEN_PARAMS_CNT];
					for (int k = 0; k < STRLEN_PARAMS_CNT; k++)
						itentry->value.params.types[k] = IT::STRLEN_PARAMS[k];
					break;
				}
				case IT::STDFNC::F_NOT_STD:
					itentry->idtype = IT::IDTYPE::F;
					break;
				}
			}
			else if (isParam)
				itentry->idtype = IT::IDTYPE::P;
			else
				itentry->idtype = IT::IDTYPE::V;

			strncpy_s(itentry->id, id, SCOPED_ID_MAXSIZE);
		}

		int i = tables.lextable.size; 

		if (i > 1 && itentry->idtype == IT::IDTYPE::V && tables.lextable.table[i - 2].lexema != LEX_DECLARE)
		{
		}
		
		if (itentry->iddatatype == IT::IDDATATYPE::UNDEF)
		{
			Log::writeError(log.stream, Error::GetError(300, line, 0));
			lex_ok = false;
		}
		return itentry;
	}

	bool analyze(LEX& tables, In::IN& in, Log::LOG& log, Parm::PARM& parm)
	{
		static bool lex_ok = true;
		tables.lextable = LT::Create(MAXSIZE_LT);
		tables.idtable = IT::Create(MAXSIZE_TI);

		bool isParam = false, isFunc = false;
		int enterPoint = NULL;
		char curword[STR_MAXSIZE], nextword[STR_MAXSIZE];
		int curline;
		std::stack <char*> scopes;			

		for (int i = 0; i < In::InWord::size; i++)
		{
			strcpy_s(curword, in.words[i].word);
			if (i < In::InWord::size - 1)
				strcpy_s(nextword, in.words[i + 1].word);
			curline = in.words[i].line;
			isFunc = false;
			int idxTI = NULLIDX_TI;

			for (int j = 0; j < N_GRAPHS; j++)
			{
				FST::FST fst(curword, graphs[j].graph);
				if (FST::execute(fst))
				{
					char lexema = graphs[j].lexema;
					
					switch (lexema)
					{
					case LEX_MAIN:
						enterPoint++;
						break;
					case LEX_SEPARATORS:
					{
						switch (*curword)
						{
						case LEX_LEFTHESIS:		
						{
							isParam = true;
							if (tables.lextable.size > 2 && tables.lextable.table[tables.lextable.size - 1].lexema == LEX_ID &&
								(tables.lextable.table[tables.lextable.size - 2].lexema == LEX_FUNCTION ||
								(tables.lextable.size > 2 && tables.lextable.table[tables.lextable.size - 2].lexema == LEX_ID_TYPE && tables.lextable.table[tables.lextable.size - 3].lexema == LEX_FUNCTION)))
							{
								char* functionname = new char[MAXSIZE_ID];
								char* scopename = getScopeName(tables.idtable, in.words[i - 1].word);
								if (scopename == nullptr) break;
								strcpy_s(functionname, MAXSIZE_ID, scopename);
								scopes.push(functionname);
							}
							break;
						}
						case LEX_RIGHTTHESIS:	
						{
							isParam = false;
							break;
						}
						case LEX_LEFT:		// {
						{
							char* functionname = new char[MAXSIZE_ID];
							break;
						}
						case LEX_RIGHT:		// }
						{
							if (!scopes.empty())
								scopes.pop();
							break;
						}
						}
						if (strcmp(curword, "!=") == 0) lexema = LEX_NOTEQUALS;
						else if (strcmp(curword, "==") == 0) lexema = LEX_EQUALS;
						else if (strcmp(curword, ">=") == 0) lexema = LEX_MOREEQUALS;
						else if (strcmp(curword, "<=") == 0) lexema = LEX_LESSEQUALS;
						else lexema = *curword;
						break;
					}

					case LEX_LITERAL_HEX:
					{
						int value;
						if (lexema == LEX_LITERAL_HEX) {
							value = DecimicalNotation(curword, 16);
						}
						else {
							value = DecimicalNotation(curword, 2);
						}
						tables.idtable.table[tables.idtable.size - 1].value.vint = value;
						lexema = LEX_LITERAL; 
						break;
					}

					case LEX_ID:
					{
						if (strlen(curword) > MAXSIZE_ID) {
							Log::writeError(log.stream, Error::GetError(204, curline, 0));
							lex_ok = false;
							break;
						}
					}

					case LEX_LITERAL:
					case LEX_TRUE:
					case LEX_FALSE:
					{
						char id[STR_MAXSIZE] = "";
						idxTI = NULLDX_TI; 
						if (*nextword == LEX_LEFTHESIS)
							isFunc = true;						
						
						char* idtype = nullptr;
						if (i > 0) idtype = in.words[i - 1].word;
						
						if (!isFunc && !scopes.empty())
							strncpy_s(id, scopes.top(), MAXSIZE_ID);
						strncat(id, curword, MAXSIZE_ID);

						if (lexema == LEX_TRUE || lexema == LEX_FALSE)
							strcpy_s(id, curword); 

						if (isLiteral(curword))
							strcpy_s(id, curword); 

						IT::Entry* itentry = getEntry(tables, lexema, id, idtype, isParam, isFunc, log, curline, lex_ok);
						if (itentry != nullptr) 
						{
							IT::Add(tables.idtable, *itentry);
							idxTI = tables.idtable.size - 1;
						}
						else 
						{
							idxTI = IT::isId(tables.idtable, id);	
							if (lexema == LEX_LITERAL || lexema == LEX_TRUE || lexema == LEX_FALSE) {
								IT::IDDATATYPE t = getType(id, idtype);
								if (lexema == LEX_TRUE || lexema == LEX_FALSE) t = IT::IDDATATYPE::BOOL;
								idxTI = getLiteralIndex(tables.idtable, curword, t, log, curline); 
							}
						}
						
						if (lexema == LEX_TRUE || lexema == LEX_FALSE) lexema = LEX_LITERAL; 
					}
					break;
					}

					LT::Entry* ltentry = new LT::Entry(lexema, curline, idxTI);
					LT::Add(tables.lextable, *ltentry);
					break;
				}
				else if (j == N_GRAPHS - 1) 
				{
					Log::writeError(log.stream, Error::GetError(201, curline, 0));
					lex_ok = false;
				}
			}
		}

		if (enterPoint == NULL) 
		{
			Log::writeError(log.stream, Error::GetError(301));
			lex_ok = false;
		}
		if (enterPoint > 1) 
		{
			Log::writeError(log.stream, Error::GetError(302));
			lex_ok = false;
		}

		if (lex_ok)
		{
			for (int i = 0; i < tables.lextable.size; i++)
			{
				if (tables.lextable.table[i].lexema == LEX_FUNCTION)
				{
					if (i + 2 >= tables.lextable.size) continue;
					int idxTI = tables.lextable.table[i + 2].idxTI;
					if (idxTI == NULLIDX_TI) continue;

					IT::Entry* e = &tables.idtable.table[idxTI];
					if (e->idtype != IT::IDTYPE::F) continue;

					int paramCount = 0;
					IT::IDDATATYPE paramsBuf[64]; 
					
					int k = i + 4;
					while (k < tables.lextable.size && tables.lextable.table[k].lexema != LEX_RIGHTTHESIS)
					{
						if (tables.lextable.table[k].lexema == LEX_ID_TYPE)
						{
                            char* word = in.words[k].word;
                            IT::IDDATATYPE type = IT::IDDATATYPE::UNDEF;
                            
                            if (!strcmp(word, "integer")) type = IT::IDDATATYPE::INT;
                            else if (!strcmp(word, "string")) type = IT::IDDATATYPE::STR;
                            else if (!strcmp(word, "bool")) type = IT::IDDATATYPE::BOOL;
                            
                            if (type != IT::IDDATATYPE::UNDEF && paramCount < 64)
                            {
                                paramsBuf[paramCount++] = type;
                            }
						}
						k++;
					}
					
					if (paramCount > 0)
					{
						e->value.params.count = paramCount;
						e->value.params.types = new IT::IDDATATYPE[paramCount];
						for(int p=0; p<paramCount; p++) e->value.params.types[p] = paramsBuf[p];
					}
				}
			}
		}

		return lex_ok;
	}
};