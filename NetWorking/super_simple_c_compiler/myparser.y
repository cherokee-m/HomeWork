%{
/****************************************************************************
expr.y
ParserWizard generated YACC file.

Date: 2016Äê10ÔÂ18ÈÕ
****************************************************************************/
#include <iostream>
# include<string>
#include <cctype>
using namespace std;
%}

%include {
#ifndef YYSTYPE
#define YYSTYPE string
#endif
}


%name expr

{

	virtual int yygettoken();
	string py;
	string lxy;
}


%token ID NUMBER ADD SUB MUL DIV LB RB
%left ADD SUB
%left MUL DIV
%left LB RB
%right UMINUS



%%
lines	:	lines expr ';'	{ cout << $2 << endl; }
	|	lines ';'
	|
	;

expr	:	expr ADD expr	{ $$ = "";$$ += $1; $$ += $3; $$ += "+";}
	|	expr SUB expr	{ $$ = "";$$ += $1; $$ += $3; $$ += "-";}
	|	expr MUL expr	{ $$ = "";$$ += $1; $$ += $3; $$ += "*";}
	|	expr DIV expr	{ $$ = "";$$ += $1; $$ += $3; $$ += "/";}
	|	LB expr RB	{ $$ = "";$$ += $2; }
	|	SUB expr %prec UMINUS	{$$ = "-";$$ += $2;}
	|	NUMBER   {$$ = "";$$ += py;}
	|   ID   {$$ = "";$$ += lxy;}
	;

%%
	
int YYPARSERNAME::yygettoken()
{
	// place your token retrieving code herek
	char t;
while(1){
		t = getchar();
		if (t == ' ' || t == '\t' || t == '\n')
			;
		else if (isdigit(t)){
			string temp = "";
			while (isdigit(t)){
				temp.append(1,t);
				t = getchar();
			}
			py = temp;
			ungetc(t, stdin);
			return NUMBER;
		}
		else if ((t >= 'a' && t <= 'z')||(t >= 'A' && t <= 'Z') || (t == '_')){
			string temp2 = "";
			while((t >= 'a' && t <= 'z')||(t >= 'A' && t <= 'Z') || (t == '_')|| isdigit(t)){
				temp2.append(1,t);
				t = getchar();

			}
			lxy = temp2;
			ungetc(t, stdin);
			return ID;
		}
		else if (t == '+')
			return ADD;
		else if(t == '-')
			return SUB;
		else if (t == '*')
			return MUL;
		else if (t == '/')
			return DIV;
		else if (t == '(')
			return LB;
		else if (t == ')')
			return RB;
		else{
			return t;
		}
	}
	
}

int main(void)
{
	int n = 1;
	expr parser;
	if (parser.yycreate()) {
		n = parser.yyparse();
	}
	return n;
}