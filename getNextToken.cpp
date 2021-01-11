#include "lex.h"
#include <string>
#include <map>
using namespace std;

Lex getNextToken(istream& in, int& linenum) {

	enum LexState {
		START, COMMENT, INID, ININT, INSTR
	};

	LexState lexstate = START;
	string lexeme;
	char ch;
	map<string,Token> keywordsMap = {{"print", PRINT}, {"let", LET}, {"if", IF}, {"loop", LOOP}, {"begin", BEGIN}, {"end", END}};

	while(in.get(ch)) {
		switch(lexstate) {
		case START:
			if (ch == '\n') {
				linenum++;
				continue;
			}
			if (isspace(ch)) {
				continue;
			}
			if ((ch == '=') || (ch == '|') || (ch == '&')) {  // May have to adjust later
				continue;
			}
            if (ch == '"') {
			    lexstate = INSTR;
			    continue;
			}
			lexeme = ch;
			if (isalpha(ch)) {
				lexstate = INID;
				continue;
			}
			else if (isdigit(ch)) {
				lexstate = ININT;
				continue;
			}
			else if (ch == '+') {
				return Lex(PLUS, lexeme, linenum);
			}
			else if (ch == '-') {
				return Lex(MINUS, lexeme, linenum);
			}
			else if (ch == '*') {
				return Lex(STAR, lexeme, linenum);
			}
			else if (ch == '/') {
				if (in.peek() == '/') {
					lexstate = COMMENT;
					continue;
				}
				return Lex(SLASH, lexeme, linenum);
			}
			else if (ch == '!') {
				return Lex(BANG, lexeme, linenum);
			}
			else if (ch == '(') {
				return Lex(LPAREN, lexeme, linenum);
			}
			else if (ch == ')') {
				return Lex(RPAREN, lexeme, linenum);
			}
			else if (ch == ';') {
				return Lex(SC, lexeme, linenum);
			}
			else {
                linenum++;
				return Lex(ERR, lexeme, linenum);
			}
		case INID:
			if (!isalnum(ch)) {
				in.putback(ch);
                if (keywordsMap.find(lexeme) != keywordsMap.end()) {
					return Lex(keywordsMap[lexeme], lexeme, linenum);
				}
                else {
				    return Lex(ID, lexeme, linenum);
				}
			}
			else {
				lexeme = lexeme + ch;
				continue;
			}
		case ININT:
			if (isdigit(ch)) {
				lexeme = lexeme + ch;
				continue;
			}
			else {
				in.putback(ch);
				return Lex(INT, lexeme, linenum);
			}
		case INSTR:
			if (ch == '\\') {
				if (in.peek () == 'n') {
					in.get(ch);
					lexeme = lexeme + '\n';
					continue;
				}
				else {
					in.get(ch);
					lexeme = lexeme + ch;
					continue;
				}
			}
			else if (ch == '"') {
				return Lex(STR, lexeme, linenum);
			}
            else if (ch == '\n') {
                linenum++;
                lexeme = lexeme + ch;
                return Lex(ERR, '\"' + lexeme, linenum);
            }
			else if (ch != '"') {
				lexeme = lexeme + ch;
				continue;
			}
			else {
				return Lex(ERR, '\"' + lexeme, linenum);
			}
		case COMMENT:
			if (ch != '\n') {
				continue;
			}
			else {
                in.putback(ch);
                lexeme = "";
				lexstate = START;
				continue;
			}
		}
	}
	return Lex(DONE, lexeme, linenum);
}
