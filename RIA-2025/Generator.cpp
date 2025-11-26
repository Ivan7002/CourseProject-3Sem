#include "Generator.h"
#include "Parm.h"
#include "LexAnalysis.h"
#include "IT.h"
#include "LT.h"
#include <sstream>
#include <cstring>
#include <iosfwd>
#include <stack>
#include <vector>

using namespace std;


namespace Gener
{
	static int labelCounter = 0;
	string itoS(int x) { stringstream r;  r << x;  return r.str(); }

	string masmString(char* str)
	{
		string res = " byte ";
		if (str == nullptr || strlen(str) == 0) return res + "0";

		bool inQuote = false;
		bool hasElements = false;

		for (int i = 0; str[i] != '\0'; i++)
		{
			unsigned char c = (unsigned char)str[i];
			if (c < 32 || c == '\'' || c == '\"')
			{
				if (inQuote) { res += "'"; inQuote = false; }

				if (hasElements) res += ", ";
				res += itoS((int)c);
				hasElements = true;
			}
			else
			{
				if (!inQuote) {
					if (hasElements) res += ", ";
					res += "'"; inQuote = true;
				}
				res += (char)c;
				hasElements = true;
			}
		}
		if (inQuote) res += "'";
		res += ", 0";
		return res;
	}

	struct LabelInfo {
		int id;
		string type; // "do", "if"
	};
	stack<LabelInfo> labelStack;

	string genCallFuncCode(Lexer::LEX& tables, Log::LOG& log, int i)
	{
		string str;
		IT::Entry e = ITENTRY(i); 
		stack <IT::Entry> temp;
		bool stnd = (e.idtype == IT::IDTYPE::S);

		for (int j = i + 1; LEXEMA(j) != LEX_RIGHTTHESIS; j++)
		{
			if (LEXEMA(j) == LEX_ID || LEXEMA(j) == LEX_LITERAL || LEXEMA(j) == LEX_TRUE || LEXEMA(j) == LEX_FALSE)
				temp.push(ITENTRY(j)); 	
		}
		str += "\n";
		
		while (!temp.empty())
		{
			if (temp.top().iddatatype == IT::IDDATATYPE::STR || temp.top().iddatatype == IT::IDDATATYPE::CHAR)
			{
				if (temp.top().idtype == IT::IDTYPE::L)
					str = str + "push offset var_" + temp.top().id + "\n";
				else
					str = str + "push var_" + temp.top().id + "\n";
			}
			else if (temp.top().iddatatype == IT::IDDATATYPE::UINT)
				str = str + "movzx eax, byte ptr [var_" + temp.top().id + "]\npush eax\n";
			else   str = str + "push var_" + temp.top().id + "\n";
			temp.pop();
		}
		if (stnd)
			str += "push offset buffer\n";
		str = str + "call " + string(e.id) + IN_CODE_ENDL;
		return str;
	}

	string genExpressionCode(Lexer::LEX& tables, Log::LOG& log, int& i)
	{
		string str;
		
		for (; i < tables.lextable.size; i++)
		{
			char lex = LEXEMA(i);
			if (lex == LEX_SEPARATOR || lex == LEX_LEFT || lex == LEX_RIGHT || lex == LEX_DO || lex == LEX_WHILE || lex == LEX_IF || lex == LEX_COUT || lex == LEX_RETURN)
			{
				i--; // Backtrack one step as we consumed a non-expr token
				break;
			}

			switch (lex)
			{
			case LEX_LITERAL:
			case LEX_ID:
			{
				if (ITENTRY(i).idtype == IT::IDTYPE::F || ITENTRY(i).idtype == IT::IDTYPE::S) 
				{
					str += genCallFuncCode(tables, log, i); 
					str += "push eax\n";
					if (i+1 < tables.lextable.size && LEXEMA(i+1) == LEX_LEFTHESIS) {
						int balance = 1;
						i++;
						while (balance > 0 && i < tables.lextable.size) {
							i++;
							if (LEXEMA(i) == LEX_LEFTHESIS) balance++;
							if (LEXEMA(i) == LEX_RIGHTTHESIS) balance--;
						}
					}
				}
				else {
					if (ITENTRY(i).iddatatype == IT::IDDATATYPE::STR && ITENTRY(i).idtype == IT::IDTYPE::L)
						str += "push offset var_" + string(ITENTRY(i).id) + "\n";
					else if (ITENTRY(i).iddatatype == IT::IDDATATYPE::UINT)
						str += "movzx eax, byte ptr [var_" + string(ITENTRY(i).id) + "]\npush eax\n";
					else
						str += "push var_" + string(ITENTRY(i).id) + "\n";
				}
				break;
			}
			case LEX_PLUS:
				str += "pop ebx\npop eax\nadd eax, ebx\npush eax\n"; break;
			case LEX_MINUS:
				str += "pop ebx\npop eax\nsub eax, ebx\npush eax\n"; break; 
			case LEX_STAR:
				str += "pop ebx\npop eax\nimul eax, ebx\npush eax\n"; break;
			case LEX_DIRSLASH:
				str += "pop ebx\npop eax\ncdq\nidiv ebx\npush eax\n"; break;
			case LEX_PERSENT:
				str += "pop ebx\npop eax\ncdq\nmov edx,0\nidiv ebx\npush edx\n"; break;
			case LEX_BITOR:
				str += "pop ebx\npop eax\nor eax, ebx\npush eax\n"; break;
			case LEX_BITAND:
				str += "pop ebx\npop eax\nand eax, ebx\npush eax\n"; break;
			case LEX_BITNOT:
				str += "pop eax\nnot eax\npush eax\n"; break;
            case LEX_MORE:
                str += "pop ebx\npop eax\ncmp eax, ebx\nsetg al\nmovzx eax, al\npush eax\n"; break;
            case LEX_LESS:
                str += "pop ebx\npop eax\ncmp eax, ebx\nsetl al\nmovzx eax, al\npush eax\n"; break;
            case LEX_EQUALS:
                str += "pop ebx\npop eax\ncmp eax, ebx\nsete al\nmovzx eax, al\npush eax\n"; break;
            case LEX_NOTEQUALS:
                str += "pop ebx\npop eax\ncmp eax, ebx\nsetne al\nmovzx eax, al\npush eax\n"; break;
			}
		}
		return str;
	}

	string genEqualCode(Lexer::LEX& tables, Log::LOG& log, int i)
	{
		string str;
		IT::Entry e1 = ITENTRY(i - 1);
        int k = i + 1;
        str += genExpressionCode(tables, log, k);
        str += "pop eax\n";
        if (e1.iddatatype == IT::IDDATATYPE::UINT) {
             str += "mov byte ptr [var_" + string(e1.id) + "], al\n";
        } else {
             str += "mov var_" + string(e1.id) + ", eax\n";
        }
		return str;
	}

	string genFunctionCode(Lexer::LEX& tables, int i, string funcname, int pcount)
	{
		string str;
		IT::Entry e = ITENTRY(i + 2);
		IT::IDDATATYPE type = e.iddatatype;
		str = SEPSTR(funcname) + string(e.id) + string(" PROC,\n\t");
		
		int j = i + 3; 
		while (LEXEMA(j) != LEX_RIGHTTHESIS) 
		{
			if (LEXEMA(j) == LEX_ID)
				str = str + "var_" + string(ITENTRY(j).id) + (ITENTRY(j).iddatatype == IT::IDDATATYPE::UINT ? " : byte, " : " : dword, ");
			j++;
		}
		int f = str.rfind(',');
		if (f > 0)
			str[f] = IN_CODE_SPACE;
		str += "\n; --- save registers ---\npush ebx\npush edx\n; ----------------------";
		return str;
	}

	string genExitCode(Lexer::LEX& tables, int i, string funcname, int pcount)
	{
		string str = "; --- restore registers ---\npop edx\npop ebx\n; -------------------------\n";
		if (LEXEMA(i + 1) != LEX_SEPARATOR)
		{
			str = str + "mov eax, var_" + string(ITENTRY(i + 1).id) + "\n";
		}
		str += "ret\n";
		str += funcname + " ENDP" + SEPSTREMP;
		return str;
	}

	vector <string> startFillVector(Lexer::LEX& tables)
	{
		vector <string> v;
		v.push_back(BEGIN);
		v.push_back(EXTERN);
		vector <string> vlt;  vlt.push_back(CONST);
		vector <string> vid;  vid.push_back(DATA);
		for (int i = 0; i < tables.idtable.size; i++)
		{
			IT::Entry e = tables.idtable.table[i];
			string str = "\t\t" + string(e.id);

			if (tables.idtable.table[i].idtype == IT::IDTYPE::L)
			{
				str = "\t\tvar_" + string(e.id);
				switch (e.iddatatype)
				{
				case IT::IDDATATYPE::INT:
                case IT::IDDATATYPE::BOOL:
                    str = str + " sdword " + itoS(e.value.vint);  break;
				case IT::IDDATATYPE::UINT:
                    str = str + " byte " + itoS(e.value.vint);  break;
				case IT::IDDATATYPE::STR:  str = str + masmString(e.value.vstr.str);  break;
				case IT::IDDATATYPE::CHAR:  str = str + masmString(e.value.vstr.str);  break;
				}
				vlt.push_back(str);
			}
			else if (tables.idtable.table[i].idtype == IT::IDTYPE::V)
			{
				str = "\t\tvar_" + string(e.id) + (e.iddatatype == IT::IDDATATYPE::UINT ? " byte 0" : " dword 0");
				vid.push_back(str);
			}
		}
		v.insert(v.end(), vlt.begin(), vlt.end());
		v.insert(v.end(), vid.begin(), vid.end());
		v.push_back(CODE);
		return v;
	}

	void CodeGeneration(Lexer::LEX& tables, Parm::PARM& parm, Log::LOG& log)
	{
		vector <string> v = startFillVector(tables);
		ofstream ofile(parm.out);
		string funcname;	
		int pcount = 0;			
		string str;

		for (int i = 0; i < tables.lextable.size; i++)
		{
			switch (LEXEMA(i))
			{
			case LEX_MAIN:
			{
				str = str + SEPSTR("MAIN") + "main PROC";
				break;
			}
			case LEX_FUNCTION:
			{
				funcname = ITENTRY(i + 2).id;
				pcount = ITENTRY(i + 2).value.params.count;
				str = genFunctionCode(tables, i, funcname, pcount);
				break;
			}
			case LEX_RETURN:
			{
				str = genExitCode(tables, i, funcname, pcount);
				break;
			}
			case LEX_EQUAL: 
			{
				str = genEqualCode(tables, log, i);
				while (i < tables.lextable.size && LEXEMA(i) != LEX_SEPARATOR) i++;
				break;
			}
			case LEX_COUT: 
			{
				int k = i + 1;
				str += genExpressionCode(tables, log, k);
                
                bool isString = false;
                int idx = tables.lextable.table[i + 1].idxTI;
                if (idx != NULLDX_TI) // Исправлено: убрано LT::
                {
                    IT::Entry e = tables.idtable.table[idx];
                    if (e.iddatatype == IT::IDDATATYPE::STR || e.iddatatype == IT::IDDATATYPE::CHAR) 
                    {
                        isString = true;
                    }
                }

                if (isString) 
                {
				    str += "call outrad\n"; 
                } 
                else 
                {
                    str += "call outlich\n";
                }

                i = k - 1; 
				break;
			}
            case LEX_DO:
            {
                int lbl = ++labelCounter;
                labelStack.push({lbl, "do"});
                str += "lbl_do_" + itoS(lbl) + ":\n";
                break;
            }
            case LEX_WHILE:
            {
                if (!labelStack.empty() && labelStack.top().type == "do") 
                {
                    int k = i + 1; 
                    str += genExpressionCode(tables, log, k);
                    str += "pop eax\n";
                    str += "cmp eax, 0\n";
                    str += "jne lbl_do_" + itoS(labelStack.top().id) + "\n";
                    labelStack.pop();
                    i = k; 
                }
                break;
            }
            case LEX_IF:
            {
                int lbl = ++labelCounter;
                labelStack.push({lbl, "if"});
                int k = i + 1;
                str += genExpressionCode(tables, log, k);
                str += "pop eax\n";
                str += "cmp eax, 0\n";
                str += "je lbl_if_end_" + itoS(lbl) + "\n";
                i = k - 1; 
                break;
            }
            case LEX_RIGHT: // }
            {
                if (!labelStack.empty() && labelStack.top().type == "if") 
                {
                     str += "lbl_if_end_" + itoS(labelStack.top().id) + ":\n";
                     labelStack.pop();
                }
                break;
            }
			}
			if (!str.empty())
				v.push_back(str);
			str.clear();
		}
		v.push_back(END);
		for (auto x : v)
			ofile << x << endl;
		ofile.close();
	}
};
