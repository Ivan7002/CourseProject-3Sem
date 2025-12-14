#include <iostream>
#include "Header.h"


int wmain(int argc, wchar_t* argv[])
{

	char LEXERROR[] = "Lexical analysis completed with errors";
	char SYNTERROR[] = "Syntax analysis completed with errors";
	char SEMERROR[] = "Semantic analysis completed with errors";
	char POLISHERROR[] = "Error during Polish notation generation";
	char LEXGOOD[] = "Lexical analysis completed successfully";
	char SYNTGOOD[] = "Syntax analysis completed successfully";
	char SEMGOOD[] = "Semantic analysis completed successfully";
	char POLISHGOOD[] = "Polish notation generated successfully";
	char MESSAGE[] = "--------------------Compilation and code generation result-------------------";
	char STOP[] = "\nProgram execution completed";
	char ALLGOOD[] = "All compilation stages completed!";
	Log::LOG log;

	try
	{
		Parm::PARM parm = Parm::getparm(argc, argv);                            //�������� ���������
		log = Log::getstream(parm.log);
		Log::writeLog(log);														//�������� ��������� �������
		Log::writeParm(log, parm);												//�������� � ������ ���������
		In::IN in = In::getin(parm.in, log.stream);
		Log::writeIn(log.stream, in);

		in.words = In::getWordsTable(log.stream, in.text, in.code, in.size);	//��������� �� ������
		Lexer::LEX tables;
		bool lex_ok = Lexer::analyze(tables, in, log, parm);					//��������� ����������� ������
		LT::writeLexTable(log.stream, tables.lextable);							//�������� � ������ ������� ������ � ��������������� 
		IT::writeIdTable(log.stream, tables.idtable);							//� ����� ������������ ������� � ������
		LT::writeLexemsOnLines(log.stream, tables.lextable);
		if (!lex_ok)
		{
			Log::writeLine(log.stream, LEXERROR, "");
			Log::writeLine(&std::cout, LEXERROR, STOP, "");
			return 0;
		}
		else
		{
			Log::writeLine(&std::cout, LEXGOOD, "");
		}
		MFST_TRACE_START(log.stream);
		MFST::Mfst mfst(tables, GRB::getGreibach());							//��������� �������������� ������
		bool synt_ok = mfst.start(log);
		mfst.savededucation();
		mfst.printrules(log);													//������� ������ �������
		if (!synt_ok)
		{
			Log::writeLine(log.stream, SYNTERROR, "");
			Log::writeLine(&std::cout, SYNTERROR, STOP, "");
			return 0;
		}
		else Log::writeLine(&std::cout, SYNTGOOD, "");
		bool sem_ok = Semantic::semanticsCheck(tables, log);					//��������� ������������� ������
		if (!sem_ok)
		{
			Log::writeLine(log.stream, SEMERROR, "");
			Log::writeLine(&std::cout, SEMERROR, STOP, "");
			return 0;
		}
		else Log::writeLine(&std::cout, SEMGOOD, "");
		bool polish_ok = Polish::PolishNotation(tables, log);					//��������� �������������� ��������� � �����
		if (!polish_ok)
		{
			Log::writeLine(log.stream, POLISHERROR, "");
			Log::writeLine(&std::cout, POLISHERROR, STOP, "");
			return 0;
		}
		else Log::writeLine(&std::cout, POLISHGOOD, "");
		Log::writeLine(log.stream, MESSAGE, "");
		LT::writeLexTable(log.stream, tables.lextable);							//�������� � ������ ����� ������� ������ � ���������������
		IT::writeIdTable(log.stream, tables.idtable);
		LT::writeLexemsOnLines(log.stream, tables.lextable);					//� ����� ������������ ������� � ������
		//Log::writeLine(&std::cout, MESSAGE, "");
		//IT::writeIdTable(&std::cout, tables.idtable);							//�������� � ��������� ������ ����� ������� ������ � ��������������� 
		//LT::writeLexTable(&std::cout, tables.lextable);							//� ����� ������������ ������� � ������
		//LT::writeLexemsOnLines(&std::cout, tables.lextable);

		Gener::CodeGeneration(tables, parm, log);								//��������� ��������� ����
		//Log::writeLine(log.stream, ALLGOOD, "");									//���� ������ ���������
		//Log::writeLine(&std::cout, ALLGOOD, "");
		//Log::Close(log);													    //������� ������
	}
	catch (Error::ERROR e)
	{
		std::cout << "Error " << e.id << ": " << e.message << ", line " << e.position.line << ", column " << e.position.col << std::endl;
		system("pause");
	}
	return 0;
}