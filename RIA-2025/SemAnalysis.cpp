#include "IT.h"
#include "LT.h"
#include "Error.h"
#include "LexAnalysis.h"
#include "SemAnalysis.h"
#include <set>
#include <cstring>
#include <limits>

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

	bool checkOverflowAddition(int a, int b, IT::IDDATATYPE type)
	{
		if (type == IT::IDDATATYPE::INT)
		{
			if (b > 0 && a > INT_MAXSIZE - b) return true;
			if (b < 0 && a < INT_MINSIZE - b) return true;
		}
		else if (type == IT::IDDATATYPE::UINT)
		{
			// Для unsigned (1 байт): проверяем что оба операнда >= 0 и результат не превышает UBYTE_MAXSIZE (255)
			if (a < 0 || b < 0) return true; // unsigned не может быть отрицательным
			if (a > UBYTE_MAXSIZE - b) return true;
		}
		return false;
	}

	bool checkOverflowSubtraction(int a, int b, IT::IDDATATYPE type)
	{
		if (type == IT::IDDATATYPE::INT)
		{
			if (b < 0 && a > INT_MAXSIZE + b) return true;
			if (b > 0 && a < INT_MINSIZE + b) return true;
		}
		else if (type == IT::IDDATATYPE::UINT)
		{
			// Для unsigned: проверяем что оба операнда >= 0 и результат >= 0
			if (a < 0 || b < 0) return true; // unsigned не может быть отрицательным
			if (a < b) return true; // Результат будет отрицательным
		}
		return false;
	}

	bool checkOverflowMultiplication(int a, int b, IT::IDDATATYPE type)
	{
		if (type == IT::IDDATATYPE::INT)
		{
			if (a > 0 && b > 0 && a > INT_MAXSIZE / b) return true;
			if (a > 0 && b < 0 && b < INT_MINSIZE / a) return true;
			if (a < 0 && b > 0 && a < INT_MINSIZE / b) return true;
			if (a < 0 && b < 0 && b < INT_MAXSIZE / a) return true;
		}
		else if (type == IT::IDDATATYPE::UINT)
		{
			// Для unsigned (1 байт): проверяем что оба операнда >= 0 и результат не превышает UBYTE_MAXSIZE (255)
			if (a < 0 || b < 0) return true; // unsigned не может быть отрицательным
			if (b != 0 && a > UBYTE_MAXSIZE / b) return true;
		}
		return false;
	}

	// Определяет тип выражения, начиная с позиции startIndex до точки с запятой.
	// Нужен для проверки соответствия возвращаемого значения типу функции.
	IT::IDDATATYPE inferExpressionType(Lexer::LEX& tables, int startIndex)
	{
		IT::IDDATATYPE exprType = IT::IDDATATYPE::UNDEF;
		bool hasComparison = false;

		for (int k = startIndex; k < tables.lextable.size; k++)
		{
			char lex = tables.lextable.table[k].lexema;

			if (lex == LEX_SEPARATOR)
				break;

			// Если встретили закрывающую фигурную скобку после предыдущего выражения, выходим
			if (lex == LEX_RIGHT && k > startIndex && tables.lextable.table[k - 1].lexema == LEX_SEPARATOR)
				break;

			switch (lex)
			{
			case LEX_EQUALS:
			case LEX_NOTEQUALS:
			case LEX_MORE:
			case LEX_LESS:
			case LEX_MOREEQUALS:
			case LEX_LESSEQUALS:
				hasComparison = true;
				break;
			}

			if (tables.lextable.table[k].idxTI != NULLIDX_TI)
			{
				IT::IDDATATYPE current = tables.idtable.table[tables.lextable.table[k].idxTI].iddatatype;

				if (exprType == IT::IDDATATYPE::UNDEF)
				{
					exprType = current;
				}
				else if (exprType != current)
				{
					bool bothNumeric = (exprType == IT::IDDATATYPE::INT || exprType == IT::IDDATATYPE::UINT) &&
						(current == IT::IDDATATYPE::INT || current == IT::IDDATATYPE::UINT);

					if (bothNumeric)
					{
						exprType = IT::IDDATATYPE::INT; // расширяем до целого при смешении int/uint
					}
					else if (hasComparison)
					{
						exprType = IT::IDDATATYPE::BOOL;
					}
				}
			}
		}

		if (hasComparison)
			return IT::IDDATATYPE::BOOL;

		return exprType;
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

					// Анализируем выражение для поиска константных арифметических операций
					bool hasArithmetic = false;
					int leftOperand = 0, rightOperand = 0;
					char operation = 0;
					bool leftOpConst = false, rightOpConst = false;
					// Дополнительный трекер: const OP const для переполнений (напр. 250 + 10 при unsigned)
					int lastConstValue = 0;
					bool lastConstValid = false;
					int pendingLeftConst = 0;
					char pendingOp = 0;
					bool hasPendingOp = false;

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

									// Немедленная проверка диапазона для присваивания в unsigned
									if (lefttype == IT::IDDATATYPE::UINT && (constantValue < 0 || constantValue > UBYTE_MAXSIZE))
									{
										Log::writeError(log.stream, Error::GetError(619, tables.lextable.table[k].sn, 0));
										sem_ok = false;
									}

									// Если мы ищем операнды для арифметической операции
									if (hasArithmetic && !leftOpConst)
									{
										leftOperand = constantValue;
										leftOpConst = true;
									}
									else if (hasArithmetic && leftOpConst && !rightOpConst)
									{
										rightOperand = constantValue;
										rightOpConst = true;
									}
								}
								else if (tables.lextable.table[k].lexema == LEX_ID)
								{
									// Проверяем, является ли переменная константой
									int constVal = getConstantValue(constTracker, tables.idtable.table[tables.lextable.table[k].idxTI].id);
									if (constVal != INT_MAX)
									{
										constantValue = constVal;

										// Если мы ищем операнды для арифметической операции
										if (hasArithmetic && !leftOpConst)
										{
											leftOperand = constVal;
											leftOpConst = true;
										}
										else if (hasArithmetic && leftOpConst && !rightOpConst)
										{
											rightOperand = constVal;
											rightOpConst = true;
										}
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

								// Обновляем трекер последних констант
								if (tables.lextable.table[k].lexema == LEX_LITERAL || (tables.lextable.table[k].lexema == LEX_ID && getConstantValue(constTracker, tables.idtable.table[tables.lextable.table[k].idxTI].id) != INT_MAX))
								{
									int val = constantValue;
									lastConstValue = val;
									lastConstValid = true;

									if (hasPendingOp)
									{
										bool overflow = false;
										switch (pendingOp)
										{
										case LEX_PLUS:
											overflow = checkOverflowAddition(pendingLeftConst, val, lefttype);
											if (overflow) Log::writeError(log.stream, Error::GetError(616, tables.lextable.table[k].sn, 0));
											break;
										case LEX_MINUS:
											overflow = checkOverflowSubtraction(pendingLeftConst, val, lefttype);
											if (overflow) Log::writeError(log.stream, Error::GetError(617, tables.lextable.table[k].sn, 0));
											break;
										case LEX_STAR:
											overflow = checkOverflowMultiplication(pendingLeftConst, val, lefttype);
											if (overflow) Log::writeError(log.stream, Error::GetError(618, tables.lextable.table[k].sn, 0));
											break;
										}
										if (overflow) sem_ok = false;
										hasPendingOp = false;
									}
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
							// Если есть операторы, проверяем на арифметические операции
							char l = tables.lextable.table[k].lexema;
							if (l == LEX_PLUS || l == LEX_MINUS || l == LEX_STAR)
							{
								hasArithmetic = true;
								operation = l;
								isConstantAssignment = false; // Сбрасываем флаг простого присваивания

								// Если слева уже была константа, фиксируем её для проверки переполнения после получения правой
								if (lastConstValid)
								{
									pendingLeftConst = lastConstValue;
									pendingOp = l;
									hasPendingOp = true;
								}
							}
							else if (l == LEX_DIRSLASH || l == LEX_PERSENT)
							{
								isConstantAssignment = false;
							}
						}

						// Если нашли арифметическую операцию с двумя константами, проверяем переполнение
						if (hasArithmetic && leftOpConst && rightOpConst)
						{
							bool overflow = false;
							if (operation == LEX_PLUS)
							{
								overflow = checkOverflowAddition(leftOperand, rightOperand, lefttype);
								if (overflow)
								{
									Log::writeError(log.stream, Error::GetError(616, tables.lextable.table[k].sn, 0));
									sem_ok = false;
								}
							}
							else if (operation == LEX_MINUS)
							{
								overflow = checkOverflowSubtraction(leftOperand, rightOperand, lefttype);
								if (overflow)
								{
									Log::writeError(log.stream, Error::GetError(617, tables.lextable.table[k].sn, 0));
									sem_ok = false;
								}
							}
							else if (operation == LEX_STAR)
							{
								overflow = checkOverflowMultiplication(leftOperand, rightOperand, lefttype);
								if (overflow)
								{
									Log::writeError(log.stream, Error::GetError(618, tables.lextable.table[k].sn, 0));
									sem_ok = false;
								}
							}

							// Сбрасываем флаги для поиска следующих операций
							hasArithmetic = false;
							leftOpConst = false;
							rightOpConst = false;
							operation = 0;
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

					// Проверяем переполнение при присваивании константы
					if (isConstantAssignment && leftVarName[0] != '\0')
					{
						// Проверка диапазона для типа переменной
						if (lefttype == IT::IDDATATYPE::INT)
						{
							if (constantValue > INT_MAXSIZE || constantValue < INT_MINSIZE)
							{
								Log::writeError(log.stream, Error::GetError(619, tables.lextable.table[i].sn, 0));
								sem_ok = false;
							}
						}
						else if (lefttype == IT::IDDATATYPE::UINT)
						{
							if (constantValue < 0 || constantValue > UBYTE_MAXSIZE)
							{
								Log::writeError(log.stream, Error::GetError(619, tables.lextable.table[i].sn, 0));
								sem_ok = false;
							}
						}

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

				// Имя функции следует либо сразу за LEX_FUNCTION, либо через токен типа
				if ((i > 0 && tables.lextable.table[i - 1].lexema == LEX_FUNCTION) ||
					(i > 1 && tables.lextable.table[i - 2].lexema == LEX_FUNCTION))
				{
					if (e.idtype == IT::IDTYPE::F) 
					{
						for (int k = i + 1; k < tables.lextable.size; k++)
						{
							char l = tables.lextable.table[k].lexema;
							if (l == LEX_RETURN)
							{
								int exprStart = k + 1;
								bool hasReturnExpr = exprStart < tables.lextable.size && tables.lextable.table[exprStart].lexema != LEX_SEPARATOR;
								IT::IDDATATYPE exprType = IT::IDDATATYPE::UNDEF;

								if (hasReturnExpr)
								{
									exprType = inferExpressionType(tables, exprStart);

									// Простой случай вида "return id;"
									if (exprType == IT::IDDATATYPE::UNDEF && tables.lextable.table[exprStart].idxTI != NULLIDX_TI)
									{
										exprType = tables.idtable.table[tables.lextable.table[exprStart].idxTI].iddatatype;
									}

									bool numericCompatible = (exprType == IT::IDDATATYPE::INT || exprType == IT::IDDATATYPE::UINT) &&
										(e.iddatatype == IT::IDDATATYPE::INT || e.iddatatype == IT::IDDATATYPE::UINT);

									if (exprType != IT::IDDATATYPE::UNDEF && !(exprType == e.iddatatype || numericCompatible))
									{
										Log::writeError(log.stream, Error::GetError(315, tables.lextable.table[k].sn, 0));
										sem_ok = false;
									}
								}
								else if (e.iddatatype != IT::IDDATATYPE::PROC)
								{
									Log::writeError(log.stream, Error::GetError(315, tables.lextable.table[k].sn, 0));
									sem_ok = false;
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
				IT::IDDATATYPE leftType = IT::IDDATATYPE::UNDEF;
				IT::IDDATATYPE rightType = IT::IDDATATYPE::UNDEF;
				int leftVal = 0;
				int rightVal = 0;
				bool leftIsConst = false;
				bool rightIsConst = false;

				if (i > 0 && tables.lextable.table[i - 1].idxTI != NULLIDX_TI)
				{
					leftType = tables.idtable.table[tables.lextable.table[i - 1].idxTI].iddatatype;
					if (leftType != IT::IDDATATYPE::INT && leftType != IT::IDDATATYPE::UINT && leftType != IT::IDDATATYPE::BOOL)
						flag = false;

					// Check if left operand is a constant
					if (tables.lextable.table[i - 1].lexema == LEX_LITERAL)
					{
						leftVal = tables.idtable.table[tables.lextable.table[i - 1].idxTI].value.vint;
						leftIsConst = true;
					}
					else if (tables.lextable.table[i - 1].lexema == LEX_ID)
					{
						char* varName = tables.idtable.table[tables.lextable.table[i - 1].idxTI].id;
						int constVal = getConstantValue(constTracker, varName);
						if (constVal != INT_MAX)
						{
							leftVal = constVal;
							leftIsConst = true;
						}
					}
				}
				if (i + 1 < tables.lextable.size && tables.lextable.table[i + 1].idxTI != NULLIDX_TI)
				{
					rightType = tables.idtable.table[tables.lextable.table[i + 1].idxTI].iddatatype;
					if (rightType != IT::IDDATATYPE::INT && rightType != IT::IDDATATYPE::UINT && rightType != IT::IDDATATYPE::BOOL)
						flag = false;

					// Check if right operand is a constant
					if (tables.lextable.table[i + 1].lexema == LEX_LITERAL)
					{
						rightVal = tables.idtable.table[tables.lextable.table[i + 1].idxTI].value.vint;
						rightIsConst = true;
					}
					else if (tables.lextable.table[i + 1].lexema == LEX_ID)
					{
						char* varName = tables.idtable.table[tables.lextable.table[i + 1].idxTI].id;
						int constVal = getConstantValue(constTracker, varName);
						if (constVal != INT_MAX)
						{
							rightVal = constVal;
							rightIsConst = true;
						}
					}
				}

				if (!flag)
				{
					Log::writeError(log.stream, Error::GetError(317, tables.lextable.table[i].sn, 0));
					sem_ok = false;
				}

				// Note: Overflow checking moved to assignment context (LEX_EQUAL case)
				// to properly check against target variable type
				break;
			}
			}
		}
		return sem_ok;
	}
};
