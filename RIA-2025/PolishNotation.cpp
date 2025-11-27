#include "Header.h"
#include <stack>
#include <cstring>
#include <iostream>

using namespace std;
namespace Polish
{
	int getPriority(LT::Entry& e)
	{
		switch (e.lexema)
		{
		case LEX_LEFTHESIS: case LEX_RIGHTTHESIS: return 0;
		case LEX_EQUALS: case LEX_NOTEQUALS: case LEX_MORE: case LEX_LESS: case LEX_MOREEQUALS: case LEX_LESSEQUALS: return 1;
		case LEX_BITOR: return 2;
		case LEX_BITAND: return 3;
		case LEX_PLUS: case LEX_MINUS: return 4;
		case LEX_STAR: case LEX_DIRSLASH: case LEX_PERSENT: return 5;
		case LEX_BITNOT: return 6;
		case LEX_NOT: return 7;
		default: return -1;
		}
	}

    bool PolishNotation(Lexer::LEX& tbls, Log::LOG& log)
    {
        unsigned curExprBegin = 0;
        ltvec v; 
        LT::LexTable new_table = LT::Create(tbls.lextable.maxsize);
        intvec vpositions = getExprPositions(tbls); 

        for (int i = 0; i < tbls.lextable.size; i++)
        {
            if (curExprBegin < vpositions.size() && i == vpositions[curExprBegin]) 
            {
                // Determine end of expression
                // For assignment/cout: until LEX_SEPARATOR
                // For if/while: until LEX_RIGHTTHESIS (matching)
                // fillVector needs to be smart or we rely on vpositions having distinct ranges?
                // getExprPositions returns start indices.
                // We need to know where it ends.
                
                // Let's look at fillVector. It goes until LEX_SEPARATOR.
                // This is bad for if(expr).
                // I need to refactor fillVector to take end position or handle parentheses.
                
                // Assume getExprPositions returns start of expression.
                // We need to determine end dynamically.
                
                int lexcount = 0;
                
                // Check context
                // If previous was =, cout, return: read until ;
                // If previous was (, and before that if/while: read until )
                
                // Simplified approach:
                // If we are here, we are at the start of an expression (e.g. after =, or after ( for if)
                // Actually getExprPositions returns index of the *first token of the expression*.
                
                // Logic to find end:
                int j = i;
                int balance = 0;
                bool isParenExpr = false;
                
                // Check if this expression is inside parens (if/while)
                // Look behind is hard here as we iterate.
                // But we can check if the *terminator* is ; or )
                
                // Better: fillVectorUntilTerminator
                
                v.clear();
                for (; j < tbls.lextable.size; j++) {
                    if (tbls.lextable.table[j].lexema == LEX_SEPARATOR) {
                         if (balance == 0) break;
                    }
                    if (tbls.lextable.table[j].lexema == LEX_LEFTHESIS) balance++;
                    if (tbls.lextable.table[j].lexema == LEX_RIGHTTHESIS) {
                        if (balance == 0) { 
                            // Found closing parenthesis of if/while condition?
                            // Or just end of (a+b)?
                            // If we started inside ( e.g. if ( a...
                            // We rely on getExprPositions logic.
                            // If getExprPositions pushed index of 'a', then we scan until ')'
                            // But how to distinguish 'a + (b)' from 'if (a) { ... }'?
                            // We need to stop at the ')' that closes the condition.
                            // If we assume expressions in if/while are simple, we stop at first unbalanced )
                             break; 
                        }
                        balance--;
                    }
                     if (tbls.lextable.table[j].lexema == LEX_LEFT) break; // Safety break
                     v.push_back(LT::Entry(tbls.lextable.table[j]));
                }
                lexcount = v.size();

                if (lexcount > 0) // Changed from > 1 to 0 (single literal is expr)
                {
                    bool rc = setPolishNotation(tbls.idtable, log, vpositions[curExprBegin], v); 
                    if (!rc)
                        return false;
                }

                addToTable(new_table, tbls.idtable, v); 
                i += lexcount - 1; // Skip processed tokens
                curExprBegin++;
                continue;
            }
            
            if (tbls.lextable.table[i].lexema == LEX_ID || tbls.lextable.table[i].lexema == LEX_LITERAL || tbls.lextable.table[i].lexema == LEX_TRUE || tbls.lextable.table[i].lexema == LEX_FALSE)
            {
                int firstind = Lexer::getIndexInLT(new_table, tbls.lextable.table[i].idxTI);
                if (firstind == -1)
                    firstind = new_table.size;
                if (tbls.lextable.table[i].idxTI != NULLIDX_TI)
                     tbls.idtable.table[tbls.lextable.table[i].idxTI].idxfirstLE = firstind;
            }
            LT::Add(new_table, tbls.lextable.table[i]);
        }

        tbls.lextable = new_table;
        return true;
    }

    // fillVector removed/inlined above

    void addToTable(LT::LexTable& new_table, IT::IdTable& idtable, ltvec& v)
    {
        for (unsigned i = 0; i < v.size(); i++)
        {
            LT::Add(new_table, v[i]);
            if (v[i].lexema == LEX_ID || v[i].lexema == LEX_LITERAL)
            {
                if (v[i].idxTI != NULLIDX_TI) {
                    int firstind = Lexer::getIndexInLT(new_table, v[i].idxTI);
                    idtable.table[v[i].idxTI].idxfirstLE = firstind;
                }
            }
        }
    }

    intvec getExprPositions(Lexer::LEX& tbls)
    {
        intvec v;
        for (int i = 0; i < tbls.lextable.size; i++)
        {
            // Case 1: Assignment: ID = Expr ;
            if (tbls.lextable.table[i].lexema == LEX_EQUAL) 
            {
                v.push_back(i + 1);
            }
            // Case 2: Return: return Expr ;
            else if (tbls.lextable.table[i].lexema == LEX_RETURN)
            {
                v.push_back(i + 1);
            }
            // Case 3: cout Expr ;
            else if (tbls.lextable.table[i].lexema == LEX_COUT)
            {
                v.push_back(i + 1);
            }
            // Case 4: if ( Expr )
            else if (tbls.lextable.table[i].lexema == LEX_LEFTHESIS)
            {
                if (i > 0 && (tbls.lextable.table[i-1].lexema == LEX_WHILE || tbls.lextable.table[i-1].lexema == LEX_IF)) 
                {
                     v.push_back(i + 1);
                }
            }
        }
        return v;
    }

    bool __cdecl setPolishNotation(IT::IdTable& idtable, Log::LOG& log, int lextable_pos, ltvec& v)
    {
        vector<LT::Entry> result;
        stack<LT::Entry> s;
        bool ignore = false;

        for (unsigned i = 0; i < v.size(); i++)
        {
            if (ignore) 
            {
                result.push_back(v[i]);
                if (v[i].lexema == LEX_RIGHTTHESIS)
                    ignore = false;
                continue;
            }
            
            // Handle function calls within expression (simple skip)
            if (v[i].lexema == LEX_ID && v.size() > i+1 && v[i+1].lexema == LEX_LEFTHESIS) {
                // It's a function call like func(a,b)
                // We treat it as a single operand for PN purposes (simplified)
                // But PN logic separates operands.
                // Current logic ignores content of function call params?
                // "idtable... idtype == IT::IDTYPE::F" checks if ID is a function.
                // We rely on that.
            }

            int priority = getPriority(v[i]); 

            if (priority != -1 || v[i].lexema == LEX_LEFTHESIS || v[i].lexema == LEX_RIGHTTHESIS)
            {
                if (s.empty() || v[i].lexema == LEX_LEFTHESIS)
                {
                    s.push(v[i]);
                    continue;
                }

                if (v[i].lexema == LEX_RIGHTTHESIS)
                {
                    while (!s.empty() && s.top().lexema != LEX_LEFTHESIS)
                    {
                        result.push_back(s.top());
                        s.pop();
                    }
                    if (!s.empty() && s.top().lexema == LEX_LEFTHESIS)
                        s.pop();
                    continue;
                }
                
                while (!s.empty() && getPriority(s.top()) >= priority)
                {
                    result.push_back(s.top());
                    s.pop();
                }
                s.push(v[i]);
            }
            else
            {
                 // Operand (ID, Literal)
                 if (v[i].idxTI != NULLIDX_TI && (idtable.table[v[i].idxTI].idtype == IT::IDTYPE::F || idtable.table[v[i].idxTI].idtype == IT::IDTYPE::S))
                    ignore = true; // Skip function params in PN
                result.push_back(v[i]); 
            }
        }

        while (!s.empty()) { result.push_back(s.top()); s.pop(); }
        v = result;
        return true;
    }
}
