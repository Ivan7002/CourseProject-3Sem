#include "IT.h"
#include "LT.h"
#include "Error.h"
#include "LexAnalysis.h"
#include "SemAnalysis.h"
#include <set>
#include <cstring>

namespace Semantic
{
	void initConstantTracker(ConstantTracker& tracker)
	{
		tracker.count = 0;
		for (int i = 0; i < MAX_CONSTANT_TRACKING; i++)
		{
			tracker.constants[i].isConstant = false;
			tracker.constants[i].variableId[0] = '\0';
			tracker.constants[i].value = 0;
		}
	}

	int getConstantValue(ConstantTracker& tracker, const char* varId)
	{
		for (int i = 0; i < tracker.count; i++)
		{
			if (tracker.constants[i].isConstant &&
				strcmp(tracker.constants[i].variableId, varId) == 0)
			{
				return tracker.constants[i].value;
			}
		}
		return INT_MAX; // Специальное значение, означающее "не константа"
	}

	void setConstantValue(ConstantTracker& tracker, const char* varId, int value)
	{
		// Сначала проверим, есть ли уже такая переменная
		for (int i = 0; i < tracker.count; i++)
		{
			if (strcmp(tracker.constants[i].variableId, varId) == 0)
			{
				tracker.constants[i].value = value;
				tracker.constants[i].isConstant = true;
				return;
			}
		}

		// Если не нашли, добавляем новую
		if (tracker.count < MAX_CONSTANT_TRACKING)
		{
			strncpy_s(tracker.constants[tracker.count].variableId, SCOPED_ID_MAXSIZE, varId, SCOPED_ID_MAXSIZE - 1);
			tracker.constants[tracker.count].variableId[SCOPED_ID_MAXSIZE - 1] = '\0';
			tracker.constants[tracker.count].value = value;
			tracker.constants[tracker.count].isConstant = true;
			tracker.count++;
		}
	}

	void removeConstantValue(ConstantTracker& tracker, const char* varId)
	{
		for (int i = 0; i < tracker.count; i++)
		{
			if (strcmp(tracker.constants[i].variableId, varId) == 0)
			{
				tracker.constants[i].isConstant = false;
				return;
			}
		}
	}

	bool Semantic::semanticsCheck(Lexer::LEX& tables, Log::LOG& log)
	{
		bool sem_ok = true;
		ConstantTracker constTracker;
		initConstantTracker(constTracker);

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
				if (i + 1 < tables.lextable.size)
				{
					// Check if next is literal 0
					if (tables.lextable.table[i + 1].lexema == LEX_LITERAL)
					{
						IT::Entry& literalEntry = tables.idtable.table[tables.lextable.table[i + 1].idxTI];
						if (literalEntry.value.vint == 0)
						{
							sem_ok = false;
							Log::writeError(log.stream, Error::GetError(318, tables.lextable.table[i].sn, 0));
						}
					}
					// Check if next is a variable that is known to be 0
					else if (tables.lextable.table[i + 1].lexema == LEX_ID)
					{
						char* varName = tables.idtable.table[tables.lextable.table[i + 1].idxTI].id;
						int constVal = getConstantValue(constTracker, varName);
						if (constVal == 0) // Variable is known to be 0
						{
							sem_ok = false;
							Log::writeError(log.stream, Error::GetError(318, tables.lextable.table[i].sn, 0));
						}
					}
				}
				break;
			}
			case LEX_EQUAL: // Assignment
			{
				if (i > 0 && tables.lextable.table[i - 1].idxTI != NULLIDX_TI)
				{
					IT::IDDATATYPE lefttype = tables.idtable.table[tables.lextable.table[i - 1].idxTI].iddatatype;
					char* leftVarName = tables.idtable.table[tables.lextable.table[i - 1].idxTI].id;
					bool ignore = false;
					bool isConstantAssignment = true; // Предполагаем, что присваивание константы
					int constantValue = 0;

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

								// Проверяем, является ли правая часть константой
								if (tables.lextable.table[k].lexema == LEX_LITERAL)
								{
									constantValue = tables.idtable.table[tables.lextable.table[k].idxTI].value.vint;
								}
								else if (tables.lextable.table[k].lexema == LEX_ID)
								{
									// Проверяем, является ли переменная константой
									int constVal = getConstantValue(constTracker, tables.idtable.table[tables.lextable.table[k].idxTI].id);
									if (constVal != INT_MAX)
									{
										constantValue = constVal;
									}
									else
									{
										isConstantAssignment = false;
									}
								}
								else
								{
									isConstantAssignment = false;
								}
							}

							if (k + 1 < tables.lextable.size && tables.lextable.table[k + 1].lexema == LEX_LEFTHESIS)
							{
								ignore = true;
								isConstantAssignment = false; // Вызов функции - не константа
								continue;
							}
							if (ignore && tables.lextable.table[k + 1].lexema == LEX_RIGHTTHESIS)
							{
								ignore = false;
								continue;
							}
						}
						else
						{
							// Если есть операторы, то это не простое присваивание константы
							char l = tables.lextable.table[k].lexema;
							if (l == LEX_PLUS || l == LEX_MINUS || l == LEX_STAR || l == LEX_DIRSLASH || l == LEX_PERSENT)
							{
								isConstantAssignment = false;
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

					// Если присваивание константы, запоминаем её
					if (isConstantAssignment && leftVarName[0] != '\0')
					{
						setConstantValue(constTracker, leftVarName, constantValue);
					}
					else if (leftVarName[0] != '\0')
					{
						// Если присваивание не константы, удаляем из трекера
						removeConstantValue(constTracker, leftVarName);
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
