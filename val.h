#ifndef VAL_H
#define VAL_H

#include <string>
#include <vector>
#include <cmath>
#include <iostream>
using namespace std;

class Val {
    int i;
    string s;
    enum ValType { ISINT, ISSTR, ISERR } vt;

public:
    Val() : i(0), vt(ISERR) {}
    Val(int i) : i(i), vt(ISINT) {}
    Val(string s) : i(0), s(s), vt(ISSTR) {}
    Val(ValType vt, string errmsg) : i(0), s(errmsg), vt(ISERR) {}

    ValType getVt() const { return vt; }

    bool isErr() const { return vt == ISERR; }
    bool isInt() const { return vt == ISINT; }
    bool isStr() const { return vt == ISSTR; }

    int ValInt() const {
        if (isInt()) return i;
        throw "This Val is not an Int";
    }
    string ValString() const {
        if (isStr()) return s;
        throw "This Val is not a Str";
    }

    friend ostream& operator<<(ostream& out, const Val& v) {
    	if(v.isInt()) {
    		out << v.i;
    		return out;
    	}
    	else {
    		out << v.s;
    		return out;
    	}
    }

    string GetErrMsg() const {
        if (isErr()) return s;
        throw "This Val is not an Error";
    }

    Val operator+(const Val& op) const {
        if (isInt() && op.isInt())
            return ValInt() + op.ValInt();
        if (isStr() && op.isStr())
            return ValString() + op.ValString();
        return Val(ISERR, "Type mismatch on operands of +");
    }

    Val operator-(const Val& op) const {
        if (isInt() && op.isInt())
            return ValInt() - op.ValInt();
        return Val(ISERR, "Type mismatch on operands of -");
    }

    Val operator*(const Val& op) const {
        if (isInt() && op.isInt())
            return ValInt() * op.ValInt();
        if (isInt() && op.isStr()) {
        	if (ValInt() < 0)
        		return Val(ISERR, "Negative number multiplied by string");
        	string temp;
        	for(int i = 0; i < ValInt(); i++) {
        		temp += op.ValString();
        	}
        	return Val(temp);
        }
        if (isStr() && op.isInt()) {
        	if (op.ValInt() < 0)
        		return Val(ISERR, "Cannot multiply string by negative int");
        	string temp;
        	for(int i = 0; i < op.ValInt(); i++) {
        		temp += ValString();
        	}
        	return Val(temp);
        }
        return Val(ISERR, "Type mismatch on operands of *");
    }

    Val operator/(const Val& op) const {
    	if (op.isInt()) {
			if (op.ValInt() == 0) {
				return Val(ISERR, "Divide by zero error");
			}
    	}
    	if (isInt())
            return ValInt() / op.ValInt();
        return Val(ISERR, "Type mismatch on operands of /");
    }

    Val operator!() const {
    	if (isInt()) {
			vector<int> a;
			int iRev = ValInt(), temp;
			while (iRev != 0) {
				temp = iRev % 10;
				a.push_back(temp);
				iRev /= 10;
			}
			for(int i = 0; (unsigned)i < a.size(); i++) {
				iRev += a[i] * (pow(10, a.size() - i - 1));
			}
			return Val(iRev);
		}
    	if (isStr()) {
    		string temp;
    		for (int i = s.length() - 1; i >= 0; i--) {
    			temp += s[i];
    		}
    		return Val(temp);
    	}
    	return Val(ISERR, "Type mismatch on operands of !");
    }
};

#endif
