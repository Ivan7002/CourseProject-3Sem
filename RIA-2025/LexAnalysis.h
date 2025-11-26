#pragma once
#include "LT.h"
#include "IT.h"
#include "In.h"
#include "Log.h"
#include "FST.h"

#define TYPE_INTEGER	"integer"
#define TYPE_STRING		"string"
#define TYPE_BOOL		"bool"
#define TYPE_UNSIGNED	"unsigned"
#define TYPE_VOID		"void"
#define TOSTRING		"tostring"
#define STRLEN			"strlen"
#define MAIN			"main"

#define ISTYPE(str) ( !strcmp(str, TYPE_INTEGER) || !strcmp(str, TYPE_STRING)|| !strcmp(str, TYPE_BOOL) || !strcmp(str, TYPE_UNSIGNED) )

namespace Lexer
{
	struct LEX
	{
		LT::LexTable lextable;
		IT::IdTable	idtable;
		LEX() {}
	};
	struct Graph
	{
		char lexema;
		FST::FST graph;
	};

	IT::Entry* getEntry		
	(
		Lexer::LEX& tables,						
		char lex,								
		char* id,								
		char* idtype,							
		bool isParam,							
		bool isFunc,							
		Log::LOG log,							
		int line,								
		bool& rc_err							
	);

	struct ERROR_S									
	{
		int id;
		char message[ERROR_MAXSIZE_MESSAGE];					
		struct
		{
			short line = -1;						
			short col = -1;						
		} position;
	};
	bool analyze(LEX& tables, In::IN& in, Log::LOG& log, Parm::PARM& parm);
	int	getIndexInLT(LT::LexTable& lextable, int itTableIndex);					
};
