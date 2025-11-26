#include "IT.h"
#include "LT.h"
#include "Error.h"
#include "LexAnalysis.h"
#include "SemAnalysis.h"
#include <set>

namespace Semantic
{
	bool Semantic::semanticsCheck(Lexer::LEX& tables, Log::LOG& log)
	{
		bool sem_ok = true;

		for (int i = 0; i < tables.lextable.size; i++)
		{
			switch (tables.lextable.table[i].lexema)
			{
			case LEX_DECLARE:
			{
				if (tables.lextable.table[i + 1].lexema != LEX_ID_TYPE)
				{
					sem_ok = false;
					Log::writeError(log.stream, Error::GetError(303, tables.lextable.table[i].sn, 0));
				}
			}
			case LEX_DIRSLASH:
			case LEX_PERSENT:
			{
				// Division by zero check
				// Check if next is literal 0
				if (i + 1 < tables.lextable.size && tables.lextable.table[i + 1].lexema == LEX_LITERAL)
				{
					if (tables.idtable.table[tables.lextable.table[i + 1].idxTI].value.vint == 0)
					{
						sem_ok = false;
						Log::writeError(log.stream, Error::GetError(318, tables.lextable.table[i].sn, 0));
					}
				}
				break;
			}
			case LEX_EQUAL: // Assignment
			{
				if (i > 0 && tables.lextable.table[i - 1].idxTI != NULLIDX_TI) 
				{
					IT::IDDATATYPE lefttype = tables.idtable.table[tables.lextable.table[i - 1].idxTI].iddatatype;
					bool ignore = false;

					for (int k = i + 1; k < tables.lextable.size && tables.lextable.table[k].lexema != LEX_SEPARATOR; k++)
					{
						if (tables.lextable.table[k].idxTI != NULLIDX_TI) 
						{
							if (!ignore)
							{
								IT::IDDATATYPE righttype = tables.idtable.table[tables.lextable.table[k].idxTI].iddatatype;
								if (lefttype != righttype && !((lefttype == IT::IDDATATYPE::UINT || lefttype == IT::IDDATATYPE::INT) && (righttype == IT::IDDATATYPE::UINT || righttype == IT::IDDATATYPE::INT)))
								{
									// Allow implicit conversion from INT to UINT for literals.
									Log::writeError(log.stream, Error::GetError(314, tables.lextable.table[k].sn, 0));
									sem_ok = false;
									break;
								}
							}
							
							if (k + 1 < tables.lextable.size && tables.lextable.table[k + 1].lexema == LEX_LEFTHESIS)
							{
								ignore = true;
								continue;
							}
							if (ignore && tables.lextable.table[k + 1].lexema == LEX_RIGHTTHESIS)
							{
								ignore = false;
								continue;
							}
						}
						if (lefttype == IT::IDDATATYPE::STR) 
						{
							char l = tables.lextable.table[k].lexema;
							if (l == LEX_PLUS || l == LEX_MINUS || l == LEX_STAR || l == LEX_DIRSLASH || l == LEX_BITOR || l == LEX_BITAND || l == LEX_BITNOT) 
							{
								Log::writeError(log.stream, Error::GetError(316, tables.lextable.table[k].sn, 0));
								sem_ok = false;
								break;
							}
						}
					}
				}
				break;
			}

			case LEX_ID: 
			{
				IT::Entry e = tables.idtable.table[tables.lextable.table[i].idxTI];

				if (i > 0 && tables.lextable.table[i - 1].lexema == LEX_FUNCTION)
				{
					if (e.idtype == IT::IDTYPE::F) 
					{
						for (int k = i + 1; k < tables.lextable.size; k++)
						{
							char l = tables.lextable.table[k].lexema;
							if (l == LEX_RETURN)
							{
								if (k + 1 < tables.lextable.size) {
									int next = tables.lextable.table[k + 1].idxTI; 
									if (next != NULLIDX_TI)
									{
										if (tables.idtable.table[next].iddatatype != e.iddatatype)
										{
											Log::writeError(log.stream, Error::GetError(315, tables.lextable.table[k].sn, 0));
											sem_ok = false;
											break;
										}
									}
								}
								break; 
							}
							if (l == LEX_RIGHT && k > i && tables.lextable.table[k-1].lexema == LEX_SEPARATOR) break; // End of function body approx
						}
					}
				}
				if (i + 1 < tables.lextable.size && tables.lextable.table[i + 1].lexema == LEX_LEFTHESIS && 
					(i == 0 || tables.lextable.table[i - 1].lexema != LEX_FUNCTION) && 
					(i <= 1 || tables.lextable.table[i - 2].lexema != LEX_FUNCTION)) 
				{
					if (e.idtype == IT::IDTYPE::F || e.idtype == IT::IDTYPE::S) 
					{
						int paramscount = 0;
						for (int j = i + 1; j < tables.lextable.size && tables.lextable.table[j].lexema != LEX_RIGHTTHESIS; j++)
						{
							if (tables.lextable.table[j].lexema == LEX_ID || tables.lextable.table[j].lexema == LEX_LITERAL)
							{
								paramscount++;
								if (e.value.params.count == 0) // Should use 0 not NULL for int
									break;
								if (paramscount > e.value.params.count) break;

								IT::IDDATATYPE ctype = tables.idtable.table[tables.lextable.table[j].idxTI].iddatatype;
								if (ctype != e.value.params.types[paramscount - 1])
								{
									Log::writeError(log.stream, Error::GetError(309, tables.lextable.table[i].sn, 0));
									sem_ok = false;
									break;
								}
							}
						}
						if (paramscount != e.value.params.count)
						{
							Log::writeError(log.stream, Error::GetError(308, tables.lextable.table[i].sn, 0));
							sem_ok = false;
						}
					}
				}
				break;
			}
			case LEX_MORE:	case LEX_LESS: case LEX_EQUALS:   case LEX_NOTEQUALS:	case LEX_MOREEQUALS:	case LEX_LESSEQUALS:
			case LEX_PLUS: case LEX_MINUS: case LEX_STAR:
			case LEX_BITOR: case LEX_BITAND: case LEX_BITNOT:
			{
				bool flag = true;
				if (i > 0 && tables.lextable.table[i - 1].idxTI != NULLIDX_TI)
				{
					IT::IDDATATYPE t = tables.idtable.table[tables.lextable.table[i - 1].idxTI].iddatatype;
					if (t != IT::IDDATATYPE::INT && t != IT::IDDATATYPE::UINT && t != IT::IDDATATYPE::BOOL)
						flag = false;
				}
				if (i + 1 < tables.lextable.size && tables.lextable.table[i + 1].idxTI != NULLIDX_TI)
				{
					IT::IDDATATYPE t = tables.idtable.table[tables.lextable.table[i + 1].idxTI].iddatatype;
					if (t != IT::IDDATATYPE::INT && t != IT::IDDATATYPE::UINT && t != IT::IDDATATYPE::BOOL)
						flag = false;
				}
				if (!flag)
				{
					Log::writeError(log.stream, Error::GetError(317, tables.lextable.table[i].sn, 0));
					sem_ok = false;
				}
				break;
			}
			}
		}
		return sem_ok;
	}
};
