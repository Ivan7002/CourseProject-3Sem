#include "GRB.h"
#include "LT.h"

#define GRB_ERROR_SERIES 600

typedef short GRBALPHABET;

namespace GRB
{
	Rule::Chain::Chain(short psize, GRBALPHABET s, ...)
	{
		nt = new GRBALPHABET[size = psize];
		int* p = (int*)&s;
		for (short i = 0; i < psize; ++i)
			nt[i] = (GRBALPHABET)p[i];
	};

	Rule::Rule(GRBALPHABET pnn, int piderror, short psize, Chain c, ...)
	{
		nn = pnn;
		iderror = piderror;
		chains = new Chain[size = psize];
		Chain* p = &c;
		for (int i = 0; i < size; ++i)
			chains[i] = p[i];
	};


	Greibach::Greibach(GRBALPHABET pstartN, GRBALPHABET pstbottom, short psize, Rule r, ...)
	{
		startN = pstartN;
		stbottomT = pstbottom;
		rules = new Rule[size = psize];
		Rule* p = &r;
		for (int i = 0; i < size; ++i)
			rules[i] = p[i];
	}

	short Greibach::getRule(GRBALPHABET pnn, Rule& prule)
	{
		short rc = -1;
		short k = 0;
		while (k < size && rules[k].nn != pnn)
			++k;
		if (k < size)
			prule = rules[rc = k];
		return rc;
	}

	Rule Greibach::getRule(short n)
	{
		Rule rc;
		if (n < size)
			rc = rules[n];
		return rc;
	};

	char* Rule::getCRule(char* b, short nchain)
	{
		char bchain[200];
		b[0] = Chain::alphabet_to_char(nn);
		b[1] = '-';
		b[2] = '>';
		b[3] = 0x00;
		chains[nchain].getCChain(bchain);
		strcat_s(b, sizeof(bchain) + 5, bchain);
		return b;
	};

	short Rule::getNextChain(GRBALPHABET t, Rule::Chain& pchain, short j)
	{
		short rc = -1;
		while (j < size && chains[j].nt[0] != t)
			++j;
		rc = (j < size ? j : -1);
		if (rc >= 0)
			pchain = chains[rc];
		return rc;
	}

	char* Rule::Chain::getCChain(char* b)
	{
		for (int i = 0; i < size; ++i)
			b[i] = alphabet_to_char(nt[i]);
		b[size] = 0x00;
		return b;
	}

	Greibach greibach(NS('S'), TS('$'), 6,

		// S -> Main | Function S
		Rule(NS('S'), GRB_ERROR_SERIES, 2,
			Rule::Chain(3, TS(LEX_MAIN), TS(LEX_LEFT), NS('B')), // main { Body
			Rule::Chain(9, TS(LEX_FUNCTION), TS(LEX_ID_TYPE), TS(LEX_ID), TS(LEX_LEFTHESIS), NS('P'), TS(LEX_RIGHTTHESIS), TS(LEX_LEFT), NS('B'), NS('S')) // function type id ( ... ) { B } S
		),
		
		// Body -> } | Statement Body
		Rule(NS('B'), GRB_ERROR_SERIES + 1, 7,
			Rule::Chain(1, TS(LEX_RIGHT)), // } (End of block)
			Rule::Chain(5, TS(LEX_DECLARE), TS(LEX_ID_TYPE), TS(LEX_ID), TS(LEX_SEPARATOR), NS('B')), // declare type id; B
			Rule::Chain(5, TS(LEX_ID), TS(LEX_EQUAL), NS('E'), TS(LEX_SEPARATOR), NS('B')), // id = expr; B
			Rule::Chain(4, TS(LEX_COUT), NS('E'), TS(LEX_SEPARATOR), NS('B')), // cout expr; B
			Rule::Chain(4, TS(LEX_RETURN), NS('E'), TS(LEX_SEPARATOR), NS('B')), // return expr; B
            // do { B while ( expr ); B
			Rule::Chain(9, TS(LEX_DO), TS(LEX_LEFT), NS('B'), TS(LEX_WHILE), TS(LEX_LEFTHESIS), NS('E'), TS(LEX_RIGHTTHESIS), TS(LEX_SEPARATOR), NS('B')), 
            // if ( expr ) { B B
            Rule::Chain(7, TS(LEX_IF), TS(LEX_LEFTHESIS), NS('E'), TS(LEX_RIGHTTHESIS), TS(LEX_LEFT), NS('B'), NS('B')) 
		),

		// Params -> e | type id | type id , Params
		Rule(NS('P'), GRB_ERROR_SERIES + 4, 3,
			Rule::Chain(2, TS(LEX_ID_TYPE), TS(LEX_ID)),
			Rule::Chain(4, TS(LEX_ID_TYPE), TS(LEX_ID), TS(LEX_COMMA), NS('P')),
			Rule::Chain(1, TS(0)) // Simplified empty
		),
		
		Rule(NS('E'), GRB_ERROR_SERIES + 5, 10, 
            // Unary operators
			Rule::Chain(2, TS('!'), NS('E')), // !E
			Rule::Chain(2, TS('~'), NS('E')), // ~E

            // Function call with operation: id(args) op expr
            Rule::Chain(6, TS(LEX_ID), TS(LEX_LEFTHESIS), NS('A'), TS(LEX_RIGHTTHESIS), NS('Z'), NS('E')),
            // Function call: id(args)
            Rule::Chain(4, TS(LEX_ID), TS(LEX_LEFTHESIS), NS('A'), TS(LEX_RIGHTTHESIS)),
            
            // Identifier with operation: id op expr
            Rule::Chain(3, TS(LEX_ID), NS('Z'), NS('E')),
            // Identifier: id
			Rule::Chain(1, TS(LEX_ID)),
			
            // Literal with operation: lit op expr
            Rule::Chain(3, TS(LEX_LITERAL), NS('Z'), NS('E')),
            // Literal: lit
			Rule::Chain(1, TS(LEX_LITERAL)),
			
            // Parentheses with operation: (expr) op expr
            Rule::Chain(5, TS(LEX_LEFTHESIS), NS('E'), TS(LEX_RIGHTTHESIS), NS('Z'), NS('E')),
            // Parentheses: (expr)
			Rule::Chain(3, TS(LEX_LEFTHESIS), NS('E'), TS(LEX_RIGHTTHESIS))
		),

        // Arguments (Expanded E rules to satisfy GNF)
		Rule(NS('A'), GRB_ERROR_SERIES + 6, 21,
            // i(A)ZE
            Rule::Chain(6, TS(LEX_ID), TS(LEX_LEFTHESIS), NS('A'), TS(LEX_RIGHTTHESIS), NS('Z'), NS('E')),
            Rule::Chain(8, TS(LEX_ID), TS(LEX_LEFTHESIS), NS('A'), TS(LEX_RIGHTTHESIS), NS('Z'), NS('E'), TS(LEX_COMMA), NS('A')),
            
            // i(A)
            Rule::Chain(4, TS(LEX_ID), TS(LEX_LEFTHESIS), NS('A'), TS(LEX_RIGHTTHESIS)),
            Rule::Chain(6, TS(LEX_ID), TS(LEX_LEFTHESIS), NS('A'), TS(LEX_RIGHTTHESIS), TS(LEX_COMMA), NS('A')),
            
            // iZE
            Rule::Chain(3, TS(LEX_ID), NS('Z'), NS('E')),
            Rule::Chain(5, TS(LEX_ID), NS('Z'), NS('E'), TS(LEX_COMMA), NS('A')),
            
            // i
            Rule::Chain(1, TS(LEX_ID)),
            Rule::Chain(3, TS(LEX_ID), TS(LEX_COMMA), NS('A')),
            
            // lZE
            Rule::Chain(3, TS(LEX_LITERAL), NS('Z'), NS('E')),
            Rule::Chain(5, TS(LEX_LITERAL), NS('Z'), NS('E'), TS(LEX_COMMA), NS('A')),
            
            // l
            Rule::Chain(1, TS(LEX_LITERAL)),
            Rule::Chain(3, TS(LEX_LITERAL), TS(LEX_COMMA), NS('A')),
            
            // (E)ZE
            Rule::Chain(5, TS(LEX_LEFTHESIS), NS('E'), TS(LEX_RIGHTTHESIS), NS('Z'), NS('E')),
            Rule::Chain(7, TS(LEX_LEFTHESIS), NS('E'), TS(LEX_RIGHTTHESIS), NS('Z'), NS('E'), TS(LEX_COMMA), NS('A')),
            
            // (E)
            Rule::Chain(3, TS(LEX_LEFTHESIS), NS('E'), TS(LEX_RIGHTTHESIS)),
            Rule::Chain(5, TS(LEX_LEFTHESIS), NS('E'), TS(LEX_RIGHTTHESIS), TS(LEX_COMMA), NS('A')),
            
            // !E
            Rule::Chain(2, TS(LEX_NOT), NS('E')),
            Rule::Chain(4, TS(LEX_NOT), NS('E'), TS(LEX_COMMA), NS('A')),
            
            // ~E
            Rule::Chain(2, TS(LEX_BITNOT), NS('E')),
            Rule::Chain(4, TS(LEX_BITNOT), NS('E'), TS(LEX_COMMA), NS('A')),
            
            // Empty (for empty argument lists in func())
            Rule::Chain(1, TS(0))
		),
        
        // Z (Operators)
        Rule(NS('Z'), GRB_ERROR_SERIES + 8, 13,
            Rule::Chain(1, TS(LEX_PLUS)),
            Rule::Chain(1, TS(LEX_MINUS)),
            Rule::Chain(1, TS(LEX_STAR)),
            Rule::Chain(1, TS(LEX_DIRSLASH)),
            Rule::Chain(1, TS(LEX_PERSENT)),
            Rule::Chain(1, TS(LEX_BITOR)),
            Rule::Chain(1, TS(LEX_BITAND)),
            Rule::Chain(1, TS(LEX_EQUALS)),
            Rule::Chain(1, TS(LEX_NOTEQUALS)),
            Rule::Chain(1, TS(LEX_LESS)),
            Rule::Chain(1, TS(LEX_MORE)), 
            Rule::Chain(1, TS(LEX_MOREEQUALS)), 
            Rule::Chain(1, TS(LEX_LESSEQUALS)) 
        )
	);

	Greibach getGreibach()
	{
		return greibach;
	}
}