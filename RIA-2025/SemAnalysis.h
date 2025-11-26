#pragma once
#include "Header.h"
#include "IT.h"

#define MAX_CONSTANT_TRACKING 100

namespace Semantic
{
	struct ConstantValue
	{
		char variableId[SCOPED_ID_MAXSIZE];  // имя переменной
		int value;                           // константное значение
		bool isConstant;                     // является ли константой
	};

	struct ConstantTracker
	{
		ConstantValue constants[MAX_CONSTANT_TRACKING];
		int count;
	};

	// Инициализация трекера констант
	void initConstantTracker(ConstantTracker& tracker);

	// Поиск константного значения переменной
	int getConstantValue(ConstantTracker& tracker, const char* varId);

	// Установка константного значения переменной
	void setConstantValue(ConstantTracker& tracker, const char* varId, int value);

	// Удаление константного значения (если переменная переприсваивается неконстантой)
	void removeConstantValue(ConstantTracker& tracker, const char* varId);

	bool semanticsCheck(Lexer::LEX& tables, Log::LOG& log);
};