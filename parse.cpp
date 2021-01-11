/*
 * parse.cpp
 */

#include "parse.h"
#include "parsetree.h"
#include "lex.h"
#include "val.h"
using namespace std;

namespace Parser {

	bool pushed_back = false;
	Lex	pushed_token;

	static Lex GetNextToken(istream& in, int& line) {
		if (pushed_back) {
			pushed_back = false;
			return pushed_token;
		}
		return getNextToken(in, line);
	}

	static void PushBackToken(Lex& t) {
		if (pushed_back) {
			abort();
		}
		pushed_back = true;
		pushed_token = t;
	}
}

static int error_count = 0;

void ParseError(int line, string msg) {
	++error_count;
	cout << line << ": " << msg << endl;
}

// Program is a Statement List
ParseTree *Prog(istream& in, int& line) {
	ParseTree *sl = Slist(in, line);
	if (sl == 0) {
		ParseError(line, "Prog Error: No \"Slist\"");
	}
	if (error_count) {
		return 0;
	}
	return sl;
}

//  Statement List is a Semicoln followed by zero or more Statement Lists OR
//  a Statement followed by a semicoln followed by zero or more Statement Lists
ParseTree *Slist(istream& in, int& line) {
	Lex t = Parser::GetNextToken(in, line);
	if (t == SC) {
		return Slist(in, line);
    }
	Parser::PushBackToken(t);
	ParseTree *s = Stmt(in, line);
	if (s == 0) {
		return 0;
	}
	t = Parser::GetNextToken(in, line);
	if (t != SC) {
        Parser::PushBackToken(t);
		ParseError(line, "Slist Error: Missing \"SC\" after \"Stmt\"");
		return 0;
	}
	return new StmtList(s, Slist(in, line));
}

// Statement is a If Statement or Print Statement or Let Statement or Loop Statement
ParseTree *Stmt(istream& in, int& line) {
	Lex t = Parser::GetNextToken(in, line);
    if (t == DONE)
        return 0;
	else if (t == IF)
		return IfStmt(in, line);
	else if (t == PRINT)
		return PrintStmt(in, line);
	else if (t == LET)
		return LetStmt(in, line);
	else if (t == LOOP)
		return LoopStmt(in, line);
    else if (t == END) {
        Parser::PushBackToken(t);
        return 0;
    }
	ParseError(line, "Stmt Error: \"Stmt\" expected");
	return 0;
}

// If Statement is a IF followed by a Expression followed by a BEGIN followed by a Statement List followed by a END
ParseTree *IfStmt(istream& in, int& line) {
	int firstLine = line;
	ParseTree *ex = Expr(in, line);
	if (ex == 0) {
		ParseError(line, "IfStmt Error: Missing \"Expr\" after \"IF\"");
		return 0;
	}
	if (Parser::GetNextToken(in, line) != BEGIN) {
		ParseError(line, "IfStmt Error: Missing \"BEGIN\" after \"IF Expr\"");
		return 0;
	}
	ParseTree *sl = Slist(in, line);
	if (sl == 0) {
		ParseError(line, "IfStmt Error: Missing \"Slist\" after \"IF Expr BEGIN\"");
		return 0;
	}
	if (Parser::GetNextToken(in, line) != END) {
		ParseError(line, "IfStmt Error: Missing \"END\" after \"IF Expr BEGIN Slist\"");
		return 0;
	}
	return new If(firstLine, ex, sl);
}

// Print Statement is a PRINT followed by a Expression
ParseTree *PrintStmt(istream& in, int& line) {
	ParseTree *ex = Expr(in, line);
	if (ex == 0) {
		ParseError(line, "PrintStmt Error: Missing \"Expr\" after \"PRINT\"");
		return 0;
	}
	return new Print(ex);
}

// Let Statement is a LET followed by a Identifier followed by a Expression
ParseTree *LetStmt(istream& in, int& line) {
	Lex t = Parser::GetNextToken(in, line);
	if (t != ID) {
		ParseError(line, "LetStmt Error: Missing \"ID\" after \"LET\"");
		return 0;
	}
	ParseTree *ex = Expr(in, line);
	if (ex == 0) {
		ParseError(line, "LetStmt Error: Missing \"Expr\" after \"LET ID\"");
		return 0;
	}
	return new Let(t, ex);
}

// Loop Statement is a LOOP followed by a Expression followed by a BEGIN followed by a Statement List followed by a END
ParseTree *LoopStmt(istream& in, int& line) {
	int firstLine = line;
	ParseTree *ex = Expr(in, line);
	if (ex == 0) {
		ParseError(line, "LoopStmt Error: Missing \"Expr\" after \"LOOP\"");
		return 0;
	}
	if (Parser::GetNextToken(in, line) != BEGIN) {
		ParseError(line, "LoopStmt Error: Missing \"BEGIN\" after \"LOOP Expr\"");
		return 0;
	}
	ParseTree *sl = Slist(in, line);
	if (sl == 0) {
		ParseError(line, "LoopStmt Error: Missing \"Slist\" after \"LOOP Expr BEGIN\"");
		return 0;
	}
	if (Parser::GetNextToken(in, line) != END) {
		ParseError(line, "LoopStmt Error: Missing \"END\" after \"LOOP Expr BEGIN Slist\"");
		return 0;
	}
	return new Loop(firstLine, ex, sl);
}

// Expression is a Product followed by zero or more {(+|-) followed by a Product}
ParseTree *Expr(istream& in, int& line) {
	ParseTree *t1 = Prod(in, line);
	if( t1 == 0 ) {
		ParseError(line, "Expr Error: \"Prod\" expected");
		return 0;
	}
	while (true) {
		Lex t = Parser::GetNextToken(in, line);
		if (t != PLUS && t != MINUS) {
			Parser::PushBackToken(t);
			return t1;
		}
		ParseTree *t2 = Prod(in, line);
		if( t2 == 0 ) {
			ParseError(line, "Expr Error: Missing \"Prod\" after \"PLUS\" or \"MINUS\" operator");
			return 0;
		}
		if (t == PLUS)
			t1 = new PlusExpr(t.GetLinenum(), t1, t2);
		else
			t1 = new MinusExpr(t.GetLinenum(), t1, t2);
	}
}

// Product is a Reverse followed by zero or more {(*|/) followed by a Reverse}
ParseTree *Prod(istream& in, int& line) {
	ParseTree *t1 = Rev(in, line);
	if (t1 == 0) {
		ParseError(line, "Prod Error: \"Rev\" expected");
		return 0;
	}
	while (true) {
		Lex t = Parser::GetNextToken(in, line);
		if (t != STAR && t != SLASH) {
			Parser::PushBackToken(t);
			return t1;
		}
		ParseTree *t2 = Rev(in, line);
		if (t2 == 0) {
			ParseError(line, "Prod Error: Missing \"Rev\" after \"STAR\" or \"SLASH\" operator");
			return 0;
		}
		if (t == STAR)
			t1 = new TimesExpr(t.GetLinenum(), t1, t2);
		else
			t1 = new DivideExpr(t.GetLinenum(), t1, t2);
	}
}

// Reverse is a BANG followed by a Reverse OR a Primary
ParseTree *Rev(istream& in, int& line) {
	Lex t = Parser::GetNextToken(in, line);
	if (t != BANG) {
		Parser::PushBackToken(t);
		ParseTree *p = Primary(in, line);
        if (p == 0) {
            ParseError(line, "Rev Error: \"Rev\" expected");
            return 0;
        }
        return p;
	}
	ParseTree *r = Rev(in, line);
	if (r == 0) {
		ParseError(line, "Rev Error: Missing \"Rev\" after \"BANG\" operator");
		return 0;
	}
	return new BangExpr(line, r);
}

// Primary is a Identifier or Integer or String or Left Parentheses followed by an Expression followed by a Right Parentheses
ParseTree *Primary(istream& in, int& line) {
	Lex t = Parser::GetNextToken(in, line);
	if (t == ID)
		return new Ident(t);
	else if (t == INT)
		return new IConst(t);
	else if (t == STR)
		return new SConst(t);
	else if (t == LPAREN) {
		ParseTree *ex = Expr(in, line);
		if (ex == 0) {
			ParseError(line, "Primary Error: Missing \"Expr\" after \"LPAREN\"");
			return 0;
		}
		if (Parser::GetNextToken(in, line) == RPAREN)
			return ex;
		ParseError(line, "Primary Error: Missing \"RPAREN\" after \"Expr\"");
		return 0;
	}
	ParseError(line, "Primary Error: \"Primary\" expected");
	return 0;
}
