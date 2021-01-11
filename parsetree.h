/*
 * parsetree.h
 */

#ifndef PARSETREE_H_
#define PARSETREE_H_

#include "val.h"
#include <vector>
#include <map>
using std::vector;
using std::map;

// NodeType represents all possible types
enum NodeType { ERRTYPE, INTTYPE, STRTYPE };

// a "forward declaration" for a class to hold values
class Value;

static int declarationErrors = 0;

class ParseTree {
protected:
	int			linenum;
	ParseTree	*left;
	ParseTree	*right;

public:
	ParseTree(int linenum, ParseTree *l = 0, ParseTree *r = 0) : linenum(linenum), left(l), right(r) {}

	virtual ~ParseTree() {
		delete left;
		delete right;
	}

	int GetLineNumber() const { return linenum; }

	int MaxDepth() const {
		int depth = 0;
		if (left)
			depth = left->MaxDepth();
		if (right)
			depth = max(depth, right->MaxDepth());
		depth = depth + 1;
		return depth;
	}

	virtual bool IsIdent() const { return false; }
	virtual bool IsVar() const { return false; }
	virtual string GetId() const { return ""; }
    virtual int IsBang() const { return 0; }
    virtual bool IsLet() const { return false; }
    virtual Val Eval(map<string,Val>& symbols) = 0;

	int BangCount() const {
		int bangCount = 0;
		if (left)
			bangCount += left->BangCount();
		if (right)
			bangCount += right->BangCount();
		bangCount += IsBang();
		return bangCount;
	}

	int CheckLetBeforeUse(map<string,bool>& var) {
		if (left) {
			if (left->IsLet()) {
				if (left->left->IsIdent()) {
					if (var.find(left->left->GetId()) == var.end()) {
						cout << "UNDECLARED VARIABLE " << left->left->GetId() << endl;
						declarationErrors++;
					}
				}
				var[left->GetId()] = true;
			}
			if (left->IsIdent()) {
				if (var.find(left->GetId()) == var.end()) {
					cout << "UNDECLARED VARIABLE " << left->GetId() << endl;
					declarationErrors++;
				}
			}
			left->CheckLetBeforeUse(var);
		}
		if (right) {
			if (right->IsLet()) {
				if (right->left->IsIdent()) {
					if (var.find(right->left->GetId()) == var.end()) {
						cout << "UNDECLARED VARIABLE " << right->left->GetId() << endl;
						declarationErrors++;
					}
				}
				var[right->GetId()] = true;
			}
			if (right->IsIdent()) {
				if (var.find(right->GetId()) == var.end()) {
					cout << "UNDECLARED VARIABLE " << right->GetId() << endl;
					declarationErrors++;
				}
			}
			right->CheckLetBeforeUse(var);
		}
		return declarationErrors;
	}

	void runtime_err(int line, string msg) {
		string lineStr = to_string(line);
		throw "RUNTIME ERROR at " + lineStr + ": " + msg;
	}
};

class StmtList : public ParseTree {
public:
	StmtList(ParseTree *l, ParseTree *r) : ParseTree(0, l, r) {}

	Val Eval(map<string,Val>& symbols) override {
		left->Eval(symbols);
		if (right)
			right->Eval(symbols);
		return Val();
	}
};

class Let : public ParseTree {
	string id;
public:
	Let(Lex& t, ParseTree *e) : ParseTree(t.GetLinenum(), e), id(t.GetLexeme()) {}

	string GetId() const { return id; }
	bool IsLet() const { return true; }

	Val Eval(map<string,Val>& symbols) override {
		symbols[id] = left->Eval(symbols);
		return Val();
	}
};

class Print : public ParseTree {
public:
	Print(ParseTree *l) : ParseTree(0, l) {}

	Val Eval(map<string,Val>& symbols) override {
		cout << left->Eval(symbols);
		return Val();
	}
};

class Loop : public ParseTree {
public:
	Loop(int line, ParseTree *l, ParseTree *r) : ParseTree(line, l, r) {}

	Val Eval(map<string,Val>& symbols) override {
		Val L = left->Eval(symbols);
		if (L.isErr())
			runtime_err(linenum, "Testing 1");
		if (L.isStr())
			runtime_err(linenum, "LoopStmt expression evaluates to string type");
		while (L.ValInt() != 0) {
			Val R = right->Eval(symbols);
			L = left->Eval(symbols);
			if (L.isErr())
				runtime_err(linenum, "Testing 3");
			if (L.isStr())
				runtime_err(linenum, "LoopStmt expression evaluates to string type");
		}
		return Val();
	}
};

class If : public ParseTree {
public:
	If(int line, ParseTree *l, ParseTree *r) : ParseTree(line, l, r) {}

	Val Eval(map<string,Val>& symbols) override {
		Val L = left->Eval(symbols);
	    if (L.isErr())
	    	runtime_err(linenum, L.GetErrMsg());
	    if (L.isStr())
	    	runtime_err(linenum, "Expression is not an integer");
	    if (L.ValInt() == 0)
			return Val();
	    Val R = right->Eval(symbols);
	    return Val();
	}
};


class PlusExpr : public ParseTree {
public:
	PlusExpr(int line, ParseTree *l, ParseTree *r) : ParseTree(line, l, r) {}

	Val Eval(map<string,Val>& symbols) override {
		Val L = left->Eval(symbols);
	    if (L.isErr())
	    	runtime_err(linenum, L.GetErrMsg());
	    Val R = right->Eval(symbols);
	    if (R.isErr())
	    	runtime_err(linenum, R.GetErrMsg());
	    Val answer = L + R;
	    if (answer.isErr())
	    	runtime_err(linenum, answer.GetErrMsg());
	    return answer;
	}
};

class MinusExpr : public ParseTree {
public:
	MinusExpr(int line, ParseTree *l, ParseTree *r) : ParseTree(line, l, r) {}

	Val Eval(map<string,Val>& symbols) override {
		Val L = left->Eval(symbols);
	    if (L.isErr())
	    	runtime_err(linenum, L.GetErrMsg());
	    Val R = right->Eval(symbols);
	    if (R.isErr())
	    	runtime_err(linenum, R.GetErrMsg());
	    Val answer = L - R;
	    if (answer.isErr())
	    	runtime_err(linenum, answer.GetErrMsg());
	    return answer;
	}
};

class TimesExpr : public ParseTree {
public:
	TimesExpr(int line, ParseTree *l, ParseTree *r) : ParseTree(line, l, r) {}

	Val Eval(map<string,Val>& symbols) override {
		Val L = left->Eval(symbols);
	    if (L.isErr())
	    	runtime_err(linenum, L.GetErrMsg());
	    Val R = right->Eval(symbols);
	    if (R.isErr())
	    	runtime_err(linenum, R.GetErrMsg());
	    Val answer = L * R;
	    if (answer.isErr())
	    	runtime_err(linenum, answer.GetErrMsg());
	    return answer;
	}
};

class DivideExpr : public ParseTree {
public:
	DivideExpr(int line, ParseTree *l, ParseTree *r) : ParseTree(line, l, r) {}

	Val Eval(map<string,Val>& symbols) override {
		Val L = left->Eval(symbols);
	    if (L.isErr())
	    	runtime_err(linenum, L.GetErrMsg());
	    Val R = right->Eval(symbols);
	    if (R.isErr())
	    	runtime_err(linenum, R.GetErrMsg());
	    Val answer = L / R;
	    if (answer.isErr())
	    	runtime_err(linenum, answer.GetErrMsg());
	    return answer;
	}
};

class BangExpr : public ParseTree {
public:
	BangExpr(int line, ParseTree *l) : ParseTree(line, l) {}

	int IsBang() const { return 1; }

	Val Eval(map<string,Val>& symbols) override {
		Val L = left->Eval(symbols);
	    if (L.isErr())
	    	runtime_err(linenum, L.GetErrMsg());
	    Val answer = !L;
	    if (answer.isErr())
	    	runtime_err(linenum, answer.GetErrMsg());
	    return answer;
	}
};

class IConst : public ParseTree {
	int val;
public:
	IConst(Lex& t) : ParseTree(t.GetLinenum()) {
		val = stoi(t.GetLexeme());
	}
	Val Eval(map<string,Val>& symbols) override {
		return Val(val);
	}
};

class SConst : public ParseTree {
	string val;
public:
	SConst(Lex& t) : ParseTree(t.GetLinenum()) {
		val = t.GetLexeme();
	}
	Val Eval(map<string,Val>& symbols) override {
		return Val(val);
	}
};

class Ident : public ParseTree {
	string id;
public:
	Ident(Lex& t) : ParseTree(t.GetLinenum()), id(t.GetLexeme()) {}

	bool IsIdent() const { return true; }
	string GetId() const { return id; }

	Val Eval(map<string,Val>& symbols) override {
		return symbols[id];
	}
};

#endif /* PARSETREE_H_ */
