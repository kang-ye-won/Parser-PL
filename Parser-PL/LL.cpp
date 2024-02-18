#include <iostream>
#include <fstream>
#include <sstream>
#include <map>
#include <vector>

using namespace std;
/* Variables*/
int charClass;
string token_string;
char nextChar;
int next_token;
string filename;
int countop;
int countid;
int countconst;
int symboltable;
map <string, int> id;
map <string, int> unknown;//unknown=1,known=0
vector <string> identlist;
int undef;//undef=1,def=0
string line;

/*ERROR*/
int ERROR = 0;
void error();
void printErrorMessage();
vector <int> errorlist;
vector<string> errortoken;

/*lexical*/
void addChar();
void getChar();
void getNonBlank();
int lexical();

/*grammar*/
void program();
void statements();
void statement();
int expression();
int term_tail(int);
int term();
int factor_tail(int);
int factor();
int Const();
string ident();
void semi_colon();
int add_operator();
int mult_operator();

//Character classes
#define LETTER 0
#define DIGIT 1
#define UNKNOWN 99
//Token codes
#define INT_LIT 10
#define IDENT 11
#define ASSIGN1_OP 19
#define ASSIGN2_OP 20
#define ADD_OP 21
#define SUB_OP 22
#define MULT_OP 23
#define DIV_OP 24
#define LEFT_PAREN 25
#define RIGHT_PAREN 26
#define SEMI_COLON 27

/*file*/
string word;
string example;
int counter = 0;

int main(int argc, char* argv[]) {
    if (argc != 2) {
        cout << "you should input filename";
        return 1;
    }
    filename = argv[1];

    ifstream myfile(filename);
    if (myfile.is_open()) {
        while (getline(myfile, word)) {
            example.append(word);
        }
        myfile.close();
    }
    else {
        cout << "cannot open file" << endl;
        exit(1);
    }
    getChar();
    do {
        lexical();
        program();

    } while (next_token != EOF);
    printErrorMessage();
    cout << "RESULT ==> ";
    for (size_t i = 0; i < identlist.size(); ++i) {
        cout << identlist.at(i) << ": ";
        if (!unknown.empty()) {
            if (unknown[identlist.at(i)] == 1)
                cout << "UNKNOWN; ";
            else
                cout << id.at(identlist.at(i)) << "; ";
        }
        else {
            cout << id.at(identlist.at(i)) << "; ";
        }
    }
}

/* lookup - a function to lookup operators and parentheses
 and return the token */
int lookup(char ch) {
    switch (ch) {
    case '(':
        addChar();
        next_token = LEFT_PAREN;
        break;
    case ')':
        addChar();
        next_token = RIGHT_PAREN;
        break;
    case '+':
        addChar();
        next_token = ADD_OP;
        break;
    case '-':
        addChar();
        next_token = SUB_OP;
        break;
    case '*':
        addChar();
        next_token = MULT_OP;
        break;
    case '/':
        addChar();
        next_token = DIV_OP;
        break;
    case ':':
        addChar();
        next_token = ASSIGN1_OP;
        break;
    case '=':
        addChar();
        next_token = ASSIGN2_OP;
        break;
    case ';':
        addChar();
        next_token = SEMI_COLON;
        break;
    default:
        addChar();
        next_token = EOF;
        break;
    }
    return next_token;
}
/*****************************************************/
/* addChar - a function to add nextChar to lexeme */
void addChar() {
    if (token_string.length() <= 98) {
        token_string.push_back(nextChar);
    }
    else
        cout << "Error - lexeme is too long" << endl;
}
/*****************************************************/

void getChar() {
    ifstream file(filename);
    if ((nextChar = example[counter]) != NULL) {
        if (isalpha(nextChar))
            charClass = LETTER;
        else if (isdigit(nextChar))
            charClass = DIGIT;
        else charClass = UNKNOWN;
    }
    else
        charClass = EOF;
    counter++;
    if (charClass == EOF) {
        counter = 0;
    }
    file.close();
}

/*****************************************************/
/* getNonBlank - a function to call getChar until it
 returns a non-whitespace character */
void getNonBlank() {
    while (isspace(nextChar))
        getChar();
}

/*****************************************************/
/* lex - a simple lexical analyzer for arithmetic
 expressions */
int lexical() {
    token_string.clear();
    getNonBlank();
    switch (charClass) {
        /* Parse identifiers */
    case LETTER:
        addChar();
        getChar();
        while (charClass == LETTER || charClass == DIGIT) {
            addChar();
            getChar();
        }
        next_token = IDENT;
        break;
        /* Parse integer literals */
    case DIGIT:
        addChar();
        getChar();
        while (charClass == DIGIT) {
            addChar();
            getChar();
        }
        next_token = INT_LIT;
        break;
        /* Parentheses and operators */
    case UNKNOWN:
        lookup(nextChar);
        getChar();
        if (next_token == ASSIGN1_OP) {
            lookup(nextChar);
            getChar();
        }
        break;
        /* EOF */
    case EOF:
        next_token = EOF;
        token_string = "EOF";
        break;
    } /* End of switch */
    if (token_string != "EOF")
        line.append(token_string);

    switch (next_token) {
    case IDENT:
        countid++;
        break;
    case INT_LIT:
        countconst++;
        break;
    case EOF:
        break;
    case SEMI_COLON:
        break;
    case ASSIGN2_OP:
        break;
    case LEFT_PAREN:
        break;
    case RIGHT_PAREN:
        break;
    default:
        countop++;
    }
    return next_token;
} /* End of function lex */

void program() {
    /* Parse the first statements */
    statements();
} /* End of function program */

/* statements
 Parses strings in the language generated by the rule:
 <statements> -> <statement> {<semi_colon> <statements>}
 */
void statements() {
    statement();

    if (next_token != SEMI_COLON && next_token != EOF) {
        line = line.substr(0, (line.length() - token_string.length()));
        line.append(";");
        errorlist.push_back(5);
        errortoken.push_back(token_string);
        error();
        semi_colon();
        statements();
    }
    while (next_token == SEMI_COLON) {
        semi_colon();
        lexical();
        statements();
    }
} /* End of function statements */

/* statement
 <statement> -> <ident><assignment_op><expression>
 */
void statement() {
    string key = ident();
    lexical();
    if (next_token == ASSIGN2_OP) {
        if (token_string == "=") {
            errorlist.push_back(4);
            errortoken.push_back(token_string);
            line = line.substr(0, line.length() - 1);
            line.append(":=");
            error();
        }
        lexical();
        int val = expression();
        if (find(identlist.begin(), identlist.end(), key) == identlist.end())
            identlist.push_back(key);
        if (find(identlist.begin(), identlist.end(), key) != identlist.end()) {
            id[key] = val;
            if (undef == 1) {
                unknown[key] = 1;
                undef = 0;
            }
        }
        if (!errorlist.empty()) {
            if (find(errorlist.begin(), errorlist.end(), 0) != errorlist.end()) {
                id[key] = NULL;
                unknown[key] = 1;
            }
            else if (find(errorlist.begin(), errorlist.end(), 3) != errorlist.end()) {
                id[key] = NULL;
                unknown[key] = 1;
            }
        }
    }
    else {
        errorlist.push_back(0);
        errortoken.push_back("\0");
        error();
        while (next_token != SEMI_COLON && next_token != EOF)
            lexical();
    }
}


int expression() {
    int op = term();
    symboltable = term_tail(op);
    return symboltable;
}

int term_tail(int operand) {
    if (next_token == ADD_OP || next_token == SUB_OP) {
        int cal;
        int op = add_operator();
        lexical();
        int op2 = term();
        if (op == ADD_OP)
            cal = operand + op2;
        else
            cal = operand - op2;
        symboltable = term_tail(cal);
    }
    else
        symboltable = operand;
    return symboltable;
}

int term() {
    int operand = factor();
    symboltable = factor_tail(operand);
    if (symboltable == -1)
        symboltable = operand;
    return symboltable;
}

int factor_tail(int operand) {
    if (next_token == MULT_OP || next_token == DIV_OP) {
        int cal;
        int op = mult_operator();
        lexical();
        int op2 = factor();
        if (op == MULT_OP)
            cal = operand * op2;
        else
            cal = operand / op2;
        symboltable = factor_tail(cal);
    }
    else {
        symboltable = operand;
    }
    return symboltable;
}

int factor() {
    if (next_token == LEFT_PAREN) {
        lexical();
        symboltable = expression();
        if (next_token == RIGHT_PAREN) {
            lexical();
        }
        else {
            errorlist.push_back(3);
            errortoken.push_back(")");
            error();
        }
    }
    else if (next_token == IDENT) {
        string key = ident();
        if (find(identlist.begin(), identlist.end(), key) == identlist.end())
            identlist.push_back(key);
        if (id.find(key) != id.end()) {
            symboltable = id.at(key);
        }
        else {
            //unknown 처리
            undef = 1;
            errorlist.push_back(1);
            errortoken.push_back(key);
            error();
        }
        lexical();
    }
    else if (next_token == INT_LIT) {
        int digit = Const();
        lexical();
        symboltable = digit;
    }
    else {
        errorlist.push_back(2);
        errortoken.push_back(token_string);
        line = line.substr(0, line.length() - 1);
        countop--;
        error();
    }
    return symboltable;
}

int Const() {
    return stoi(token_string);
}


string ident() {
    return token_string;
}

void semi_colon() {
    printErrorMessage();
    errortoken.clear();
    errorlist.clear();
    countid = 0;
    countconst = 0;
    countop = 0;
}

int add_operator() {
    if (next_token == ADD_OP) {
        return ADD_OP;
    }
    else
        return SUB_OP;
}

int mult_operator() {
    if (next_token == MULT_OP) {
        return MULT_OP;
    }
    else
        return DIV_OP;
}

void error() {
    for (size_t i = 0; i < errorlist.size(); ++i) {
        ERROR = errorlist.at(i);
        switch (ERROR) {
        case 2:
            lexical();
            symboltable = factor();
            break;
        }
    }
}

void printErrorMessage() {
    if (!line.empty()) {
        cout << line << endl;
        cout << "ID:" << countid << ";" << "CONST:" << countconst << ";" << "OP:" << countop << ";" << endl;
    }
    line.clear();
    if (!errorlist.empty()) {
        for (size_t i = 0; i < errorlist.size(); ++i) {
            ERROR = errorlist.at(i);
            switch (ERROR) {
            case 1:
                cout << "(ERROR) \"정의되지 않은 변수(" << errortoken.at(i) << ")가 참조됨\"";
                id[errortoken.at(i)] = NULL;
                unknown[errortoken.at(i)] = 1;
                break;
            case 2:
                cout << "(Warning) 중복연산자(" << errortoken.at(i) << ")제거";
                break;
            case 3:
                cout << "(ERROR) \"오른쪽 괄호가 존재하지 않음\"";
                id[errortoken.at(i)] = NULL;
                unknown[errortoken.at(i)] = 1;
                break;
            case 4:
                cout << "(Warning) 배정 연산자(" << errortoken.at(i) << ") 오류 제거";
                break;
            case 5:
                cout << "(Warning) 세미콜론 추가";
                line.append(errortoken.at(i));
                break;
            default:
                cout << "(ERROR) 배정문이 존재하지 않음";
            }
            cout << endl;
        }
    }
    else {
        cout << "(OK)" << endl;
    }
    cout << endl;
}