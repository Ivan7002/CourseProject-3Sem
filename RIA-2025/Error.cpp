#include "Error.h"

namespace Error
{
	ERROR errors[ERROR_MAX_ENTRY] =
	{
		ERROR_ENTRY(0, "System error: undefined error"),
		ERROR_ENTRY(1, "System error: invalid file"),
		ERROR_ENTRY_NODEF(2), ERROR_ENTRY_NODEF(3), ERROR_ENTRY_NODEF(4), ERROR_ENTRY_NODEF(5),
		ERROR_ENTRY_NODEF(6), ERROR_ENTRY_NODEF(7), ERROR_ENTRY_NODEF(8), ERROR_ENTRY_NODEF(9),
		ERROR_ENTRY_NODEF10(10), ERROR_ENTRY_NODEF10(20), ERROR_ENTRY_NODEF10(30), ERROR_ENTRY_NODEF10(40),
		ERROR_ENTRY_NODEF10(50), ERROR_ENTRY_NODEF10(60), ERROR_ENTRY_NODEF10(70), ERROR_ENTRY_NODEF10(80), ERROR_ENTRY_NODEF10(90),
		ERROR_ENTRY(100, "Parameter error: -in parameter is not specified"),
		ERROR_ENTRY(101, "Parameter error: parameter length exceeds maximum"),
		ERROR_ENTRY(102, "Parameter error: cannot open input file (-in)"),
		ERROR_ENTRY(103, "Parameter error: cannot open log file (-log)"),
		ERROR_ENTRY_NODEF(104), ERROR_ENTRY_NODEF(105), ERROR_ENTRY_NODEF(106), ERROR_ENTRY_NODEF(107), ERROR_ENTRY_NODEF(108), ERROR_ENTRY_NODEF(109),
		ERROR_ENTRY_NODEF10(110), ERROR_ENTRY_NODEF10(120), ERROR_ENTRY_NODEF10(130), ERROR_ENTRY_NODEF10(140),
		ERROR_ENTRY_NODEF10(150), ERROR_ENTRY_NODEF10(160), ERROR_ENTRY_NODEF10(170), ERROR_ENTRY_NODEF10(180), ERROR_ENTRY_NODEF10(190),
		ERROR_ENTRY(200, "Lexical error: cannot read from input file (-in)"),
		ERROR_ENTRY(201, "Lexical error: invalid character encoding"),
		ERROR_ENTRY(202, "Lexical error: unexpected end of file"),
		ERROR_ENTRY(203, "Lexical error: unexpected end of line"),
		ERROR_ENTRY(204, "Lexical error: invalid character in string literal"), 
		ERROR_ENTRY_NODEF(205), ERROR_ENTRY_NODEF(206), ERROR_ENTRY_NODEF(207), ERROR_ENTRY_NODEF(208), ERROR_ENTRY_NODEF(209),
		ERROR_ENTRY_NODEF10(210), ERROR_ENTRY_NODEF10(220), ERROR_ENTRY_NODEF10(230), ERROR_ENTRY_NODEF10(240),
		ERROR_ENTRY_NODEF10(250), ERROR_ENTRY_NODEF10(260), ERROR_ENTRY_NODEF10(270), ERROR_ENTRY_NODEF10(280), ERROR_ENTRY_NODEF10(290),
		ERROR_ENTRY(300, "Semantic error: undefined identifier"),
		ERROR_ENTRY(301, "Semantic error: missing main function declaration"),
		ERROR_ENTRY(302, "Semantic error: duplicate main function declaration"),
		ERROR_ENTRY(303, "Semantic error: function has no return statement"),
		ERROR_ENTRY(304, "Semantic error: function has incorrect return type"),
		ERROR_ENTRY(305, "Semantic error: missing function parameter declaration"),
		ERROR_ENTRY(306, "Semantic error: variable redeclaration with different types"),
		ERROR_ENTRY(307, "Semantic error: variable used before declaration"),
		ERROR_ENTRY(308, "Semantic error: function call with incorrect number of arguments"),
		ERROR_ENTRY(309, "Semantic error: incorrect function call syntax"),
		ERROR_ENTRY(310, "Semantic error: type mismatch in assignment/operation"),
		ERROR_ENTRY(311, "Semantic error: missing closing quote '\"'. Check that string is properly closed"),
		ERROR_ENTRY(312, "Semantic error: operation with incompatible types"),
		ERROR_ENTRY(313, "Semantic error: array index out of bounds"),
		ERROR_ENTRY(314, "Semantic error: return statement in void function"),
		ERROR_ENTRY(315, "Semantic error: function call in expression context"),
		ERROR_ENTRY(316, "Semantic error: assignment to constant variable"),
		ERROR_ENTRY(317, "Semantic error: invalid type conversion"),
		ERROR_ENTRY(318, "Semantic error: division by zero"),
		ERROR_ENTRY(319, "Semantic error: invalid array declaration"),
		ERROR_ENTRY(320, "Semantic error: operators >= and <= are not supported"),
		ERROR_ENTRY_NODEF10(330),ERROR_ENTRY_NODEF10(340),ERROR_ENTRY_NODEF10(350),
		ERROR_ENTRY_NODEF10(360),ERROR_ENTRY_NODEF10(370),ERROR_ENTRY_NODEF10(380),ERROR_ENTRY_NODEF10(390),
		ERROR_ENTRY_NODEF100(400), ERROR_ENTRY_NODEF100(500),
		ERROR_ENTRY(600, "Syntax error: unexpected end of program"),
		ERROR_ENTRY(601, "Syntax error: expected identifier after keyword"),
		ERROR_ENTRY(602, "Syntax error: error in function declaration"),
		ERROR_ENTRY(603, "Syntax error: error in variable declaration"),
		ERROR_ENTRY(604, "Syntax error: error in expression parsing"),
		ERROR_ENTRY(605, "Syntax error: error in if/while statement"),
		ERROR_ENTRY(606, "Syntax error: error in arithmetic expression parsing"),
		ERROR_ENTRY(607, "Syntax error: expected function/variable name"),
		ERROR_ENTRY(608, "Syntax error: error in function call"),
		ERROR_ENTRY(609, "Syntax error: error in parameter list"),
		ERROR_ENTRY(610, "Syntax error: missing opening parenthesis"),
		ERROR_ENTRY(611, "Syntax error: missing closing parenthesis"),
		ERROR_ENTRY(612, "Syntax error: missing semicolon. Check statement termination"),
		ERROR_ENTRY(613, "Syntax error: error in array declaration"),
		ERROR_ENTRY(614, "Syntax error: invalid operator precedence"),
		ERROR_ENTRY(615, "Syntax error: invalid operator precedence in expression"),
		ERROR_ENTRY_NODEF(616), ERROR_ENTRY_NODEF(617),ERROR_ENTRY_NODEF(618), ERROR_ENTRY_NODEF(619),
		ERROR_ENTRY_NODEF10(620), ERROR_ENTRY_NODEF10(630), ERROR_ENTRY_NODEF10(640), ERROR_ENTRY_NODEF10(650),
		ERROR_ENTRY_NODEF10(660), ERROR_ENTRY_NODEF10(670), ERROR_ENTRY_NODEF10(680), ERROR_ENTRY_NODEF10(690),
		ERROR_ENTRY_NODEF100(700), ERROR_ENTRY_NODEF100(800), ERROR_ENTRY_NODEF100(900)
	};

	ERROR GetError(int id)
	{
		if (id < 0 || id > ERROR_MAX_ENTRY)
			return errors[0];
		else
			return errors[id];
	}

	ERROR GetError(int id, int line, int col)
	{
		ERROR err = GetError(id);
		err.position.col = col;
		err.position.line = line;
		return err;
	}
}