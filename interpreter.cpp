#include <iostream>
#include <regex>
#include <string>
#include <stack>
#include <iomanip>
using namespace std;

//My Parser object, instantiated with a user input, will seperate user input into distinct statements.
//If it cannot, recovers from errors.

/*
------------------------------------------
---159.341 Assignment 1 Semester 1 2025---
---Submitted by: Cole Palffy,  18047471---
------------------------------------------
*/

//The parse still needs to do something about special characters;
//In particular the printwords function also prints periods (.)

enum types {
    action,
    id,
    value,
    operation,
    end
};
struct tableRow {
    tableRow(string newId, string newValue, bool isConstant = false) {
        id = newId;
        value = newValue;
        constant = isConstant;
    }
    string id;
    string value;
    bool constant;
};
//Table is reponsible for handling table related functionalities. Retrieves, adds, lists.
//reverse should not live in the table.

class Table {
    vector<tableRow>* idTable;
    public:
    Table() {
        idTable = new vector<tableRow>();
    }
    void set(string id, string value, bool isConstant = false) {
        tableRow* row = get(id);
        //check that row exists. If it does, make sure its not constant, then push new row.
        if (row != NULL) {
            if (!row->constant) {
                row->value = value;
            }
            else {
                cout << "ERROR: You cannot set over constant ID: " << "'"<< id <<"'" << endl;
            }
        }
        //if row does not exist, add a new row
        else {
            idTable->push_back(tableRow(id,value,isConstant));
        }
    }
    void append(string id, string value) {
        tableRow* row = get(id);
        //check that row exists. If it does, make sure its not constant, then append to row.
        if (row != NULL) {
            if (!row->constant) {
                row->value += value;
            }
            else {
                cout << "ERROR: You cannot append to constant ID: " << "'"<< id <<"'" << endl;
            }
        }
        else {
            cout << "ERROR: You cannot append to ID that does not exist ID: " << "'"<< id <<"'" << endl;
        }
    }
    void list() {
        int identifiers = 0;
        for (size_t i = 0; i < idTable->size(); i++) {
            if (!idTable->at(i).constant) {
                identifiers++;
            }
        }
        cout << "Identifier list (" << identifiers << ")" <<endl;
        for (size_t i = 0; i < idTable->size(); i++) {
            if (!idTable->at(i).constant) {
                cout << setw(15) << idTable->at(i).id  << "  |  " << idTable->at(i).value << endl;
            }
        }
    }
    void reverse(string id) {
        tableRow* row = get(id);
        if (row != NULL) {
            //cout << row->id << endl;
            //cout << row->value << endl;
            string words = row->value;
            stack<string> reverser;
            bool stop = false;
            while (!stop) {
                //cout << "DEBUG: segmenting words" << endl;
                string word = words.substr(0,words.find(' '));
                words = words.substr(words.find(' ')+1, words.length());
                reverser.push(word);
                reverser.push(" ");
                if (words.find(' ') == string::npos) {
                    reverser.push(words);
                    stop = true;
                }
            }
            string reversedWords;
            while (!reverser.empty()) {
                //cout << "DEBUG: popping words" << endl;
                reversedWords += reverser.top();
                reverser.pop();
            }
            row->value=reversedWords;
            //cout << reversedWords << endl;
        }
        else {
            cout << "ERROR: Could not find ID in memory ID: '" << id << "'" << endl;
        }
    }
    string getValue(string id) {
        for (size_t i = 0; i < idTable->size(); i++) {
            if (idTable->at(i).id == id) {
                return (idTable->at(i).value);
            }
        }
        cout << "ERROR: Could not find ID in memory ID: '" << id << "'" << endl;
        return "";
    }
    private:
    tableRow* get(string id) {
        for (size_t i = 0; i < idTable->size(); i++) {
            if (idTable->at(i).id == id) {
                return &(idTable->at(i));
            }
        }
        return NULL;
    }
};
//command object for the parser to hand to the interpretter

class Command {
    private:
    string _action;
    string _id;
    vector<string> _args;

    
    void setAction(string action) {_action = action; }
    
    void setId(string id) {_id = id;}
    
    void addArg(string argument) {_args.push_back(argument);}

    public:
    Command() {
        _args = vector<string>();
    }
    void add(string value, types type) {
        switch (type) {
            case types::action:
            setAction(value);
            break;
            case types::id:
            setId(value);
            break;
            case types::value:
            addArg(value);
            break;
            case types::operation:
            case types::end:
            default:
            break;
        }
    }
    string getAction() {return _action;}
    string getId() {return _id;}
    vector<string> getArgs() {return _args;}
};

class Executioner {
    private:
    Table* _table;
    Command* _command;
    void setCommand(Command* command) {
        _command = command;
    }
    void setTable(Table* table) {_table = table;}
    //evalute Operation method determines whether an argument is string or ID.
    string evaluateOperation(vector<string> arguments) {
        regex literalExpression("\"(.|\n)*\"", regex_constants::ECMAScript);
        regex idExpression("[a-zA-Z][a-zA-Z0-9]*", regex_constants::ECMAScript);
        string outputString = "";

        //For each argumet passed to the command object, determine whether or not they
        //are literal or ID, add the appropriate result to output.
        for (size_t i = 0; i < arguments.size(); i++) {
            if (regex_search(arguments[i], literalExpression)) {
                string argument =  arguments[i].substr(1,arguments[i].length()-2);
                //cout << "SHOULD NOT HAVE PARENTHASES " << argument << endl;
                outputString += argument;
            } else if (regex_search(arguments[i], idExpression)) {
                outputString += _table->getValue(arguments[i]);
            } else {
                cout << "Arguments have become malformed at some point and do not match literal or ID" << endl;
            }
        }
        return outputString;
    }
    public:
    Executioner(Table* table) {
        setTable(table);
        _table->set("SPACE", " " , true);
        _table->set("TAB", "    ", true);
        _table->set("NEWLINE", "\n", true);
    }
    void executeCommand(Command* command) {
        setCommand(command);
        if (_command != NULL) {
            //cout << "DEBUG: Command received: " << "|" <<_command->getAction() << "|" << endl;
            if (_command->getAction() == "append") {
                _table->append(_command->getId(), evaluateOperation(_command->getArgs()));
            }
            else if (_command->getAction() == "list") {
                _table->list();
            }
            else if (_command->getAction() == "exit") {
                exit(32);
            }
            else if (_command->getAction() == "print") {
                cout << evaluateOperation(_command->getArgs()) << endl;
            }
            else if (_command->getAction() == "printlength") {
                cout << "Length is: ";
                cout << evaluateOperation(_command->getArgs()).length() << endl;
            }
            else if (_command->getAction() == "printwords") {
                string words = evaluateOperation(_command->getArgs());
                cout << "Words are: " << endl;
                while (words.find(' ') != string::npos) {
                    string word = words.substr(0,words.find(' '));
                    words = words.substr(words.find(' ')+1, words.length());
                    cout << word << endl;
                    if (words.find(' ') == string::npos) {
                        cout << words <<endl;
                    }
                }
            }
            else if (_command->getAction() == "printwordcount") {
                string words = evaluateOperation(_command->getArgs());
                int wordCount = 1;
                cout << "Wordcount is: ";
                while (words.find(' ') != string::npos) {
                    wordCount++;
                    words = words.substr(words.find(' ')+1, words.length());
                }
                cout << wordCount << endl;
            }
            else if (_command->getAction() == "set") {
                _table->set(_command->getId(), evaluateOperation(_command->getArgs()));
            }
            else if (_command->getAction() == "reverse") {
                //cout << _command->getId() << endl;
                _table->reverse(_command->getId());
            }
            else {
                cout <<"SOME EGREGIOUS ERROR" <<endl;    
            }
        } else {
            cout << "Command not recognised?" << endl;
        }
    }
    

};

class Parser {
    public:
    //Create parser, store user input in parser object untouched.
    Parser() {
        _symbolTable = new Table();
        _executioner = new Executioner(_symbolTable);
    };
    void parse();
    private:
    //do not store the statements, parse them as you read them.
    //vector<string> _statements;

    //Carries the next word to be parsed.
    string _nextToken;
    //Carries the statement to be parsed minus the next word
    string _currentStatement;
    //Carries all statements in current input minus current statement
    string _inputBuffer;
    //A nice refernce variable because I want to be able to see the users full input.
    string _fullStatement;

    Command* _command;
    Table* _symbolTable;
    Executioner* _executioner;;


    string stripWhiteSpace(string input);
    bool parseProgram(string program);
    bool parseStatement();
    bool parseExpression();
    bool parseValue();

    void resetParser();
    void getNextToken();
    void getNextStatement();
    bool accept(regex regularExpression, types type);
};


int main() {
    //regex self_regex("The cat", regex_constants::ECMAScript | regex_constants::icase);
    //cout << regex_search("hello world the cast sat on the mat", self_regex);
    cout << "------------------------------------------" <<endl;
    cout << "---159.341 Assignment 1 Semester 1 2025---" <<endl;
    cout << "---Submitted by: Cole Palffy,  18047471---" <<endl;
    cout << "------------------------------------------" <<endl;

    Parser parser;
    parser.parse();
}

string testCases[12] {
    "set one \"The cat\";",
    "set two \"sat on the mat\";",
    "set sentence one + SPACE + two;",
    "append sentence \" by itself.\";",
    "print sentence;",
    "printwordcount sentence;",
    "printwords sentence;",
    "printlength sentence;",
    "list;",
    "reverse one;",
    "print one;",
    "exit;"
};

void Parser::parse() {
    string inputCommand;

    /* UNCOMMENT FOR TESTCASES
    for (int i = 0; i < 12; i++) {
        inputCommand = testCases[i];
        cout << inputCommand << endl;
        if (parseProgram(inputCommand)) {
            //cout << "Parsing " << testCases[i] <<" completed successfully" << endl;
        }
    }
        */
    // UNCOMMENT FOR USER INPUT AND SUBMISSION::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
    while (true) {
        getline(cin , inputCommand);
        if (parseProgram(inputCommand)) {
            //cout << "Parsing Completed Successfully" << endl;
        }
    }
    
}   
bool Parser::parseProgram(string program) {
    _inputBuffer = program;
    //cout << "Parsing Program" << endl;
    //Segment each statement so they run one after another.
    //Look out for ";" strings that might throw off where one line ends.
    //How should we define a statement? Look to the brief!
    /*statement  :=  append id expression end
    |   list end 
    |   exit end 
    |   print expression end 
    |   printlength expression end 
    |   printwords expression end 
    |   printwordcount expression end 
    |   set id expression end 
    |   reverse id end
    */
   //all statements begin with a key word, end with end.
   //all user inputs should end with ;
   //using a flawed method, get the statement by searching 0 to find
   
    while (_inputBuffer.length() > 0) {
        getNextStatement();
        if (parseStatement()) {
            //cout << _fullStatement << " statement Parsed" << endl;
            //cout << "EXECUTE COMMAND: "<< _command->getAction() << endl <<"ON ID: " << _command->getId() << endl << "WITH THESE VALUES: " << _command->getArgs()[0] << endl;
            
            _executioner->executeCommand(_command);
            //cout << "finished executing" << endl;
        } else {
            cout << "Please try again." << endl;
            return false;
        }
        resetParser();
        //cout << "------------------------------------------" <<endl;
    }
    return true;
}
void Parser::resetParser() {
    
}

bool Parser::parseStatement() {
    //Here are the regular expressions. They are passed into the accept function an represent
    //the next expected input from the statement. I have segmented them into groups based on
    //what they will next expect, ie append and set both expect an id to follow, whereas list and
    //exit do not.

    regex changeIdExpression("append|set", regex_constants::ECMAScript);
    regex staticExpression("list|exit", regex_constants::ECMAScript);
    regex printExpression("print|printlength|printwords|printwordcount", regex_constants::ECMAScript);
    regex reverseExpression("reverse", regex_constants::ECMAScript);
    regex idExpression("[a-zA-Z][a-zA-Z0-9]*", regex_constants::ECMAScript);
    regex endExpression(";", regex_constants::ECMAScript);

    //Command object is created for each new statement. It will take in the action, id, and any values
    //and pass them to the executioner. _command is updated in the accept function. types:: tells command
    //where to store the accepted item.
    _command = new Command();

    //If the token is set or append:
        //accept ID and expression.
    if (accept(changeIdExpression, types::action)) {
        string errorToken = _nextToken;
        if(!accept(idExpression, types::id)){
            cout << "PARSE ERROR: Was expecting ID, got |" << errorToken << "| instead" << endl;
            return false;}
        errorToken = _nextToken;
        if(!parseExpression()){
            //cout << "PARSE ERROR: Was expecting EXPRESSION, got |" << errorToken << "| instead" << endl;
            return false;}
    } 
    //If the token is list or exit:
        //perform that action
    else if (accept(staticExpression, types::action)) {
        //expect nothing else.
    } 
    //If the token is print or printlength or printwords or printwordcount
        //parse an expression
    else if (accept(printExpression, types::action)) {
        if(!parseExpression()){
            //cout << "PARSE ERROR: Was expecting EXPRESSION, got |" << _nextToken << "| instead" << endl;
            return false;}
    } 
    //If the token is reverse
        //accept and ID token
    else if (accept(reverseExpression, types::action)) {
        string errorToken = _nextToken;
        if (!accept(idExpression, types::id)) {
            cout << "PARSE ERROR: Was expecting ID, got |" << errorToken << "| instead" << endl;
            return false;}
    } 
    //If the token is none of the known words
        //Fail the parse.
    else {
        cout << "PARSE ERROR: Invalid action word: " << "|" <<_nextToken << "|" << endl;
        cout << "Please enter a statement using a known action word." << endl;
        return false;
    }
    //If the statement is successfully parsed
        //accept an end token
    if(!accept(endExpression, types::end)){
        cout << "PARSE ERROR: Could not find end token ';'" << endl;
        cout << "Please enter a statement followed by the ';' semicolon." << endl;
        return false;
    }
    //If you reach this point, successful parse.
    return true;
}
bool Parser::parseExpression() {
    //cout << "DEBUG: Parsing Expression" << endl;
    //Had some trouble getting \+ to work nicely so here is my workaround.
    string escapePlus = {92,43};

    //Plus expression looks for the + operator.
    regex plusExpression(escapePlus, regex_constants::ECMAScript);
    
    
    //Try to parse a value which can be (constant, id or literal)
    string errorToken = _nextToken;
    //cout << "DEBUG: "<< errorToken <<endl;
    if(!parseValue()) {
        cout << "PARSE ERROR: Was expecting VALUE, got "<< "|" << errorToken << "| instead" << endl;
        return false;}
    //if the expression contains a plusExpression, parse a new expression
    //otherwise, just parse the current expression and return.
    if (accept(plusExpression, types::operation)) {
        //cout << "DEBUG: Parsed PLUS" << endl;
        return parseExpression();
    }
    return true;
}
bool Parser::parseValue() {
    
    //values can either be an id or a literal. STORE CONSTANTS IN TABLE
    regex valueExpression("([a-zA-Z][a-zA-Z0-9]*|\"(.|\n)*\")", regex_constants::ECMAScript);
    if(!accept(valueExpression, types::value)) {
        
        return false;}
    return true;
}

void Parser::getNextStatement() {
    //cout << "DEBUG: currentStatment: |"<< _currentStatement << "|" << endl;
    //cout << "DEBUG: inputBuffer: |" << _inputBuffer << "|" << endl;
    //cout << "DEBUG: _nextToken: |" << _nextToken << "|" << endl;
    //This method will:
        //Retrieve the statement from head to tail, including ;
        //Retrieval is taken from the _inputBuffer string
        //Retrieved statement will be dropped from _inputBuffer
    
    //The beginning of the statement should always be index 0

    //Determine the end of the statement.
    long long unsigned int semiColonPosition = _inputBuffer.find(';');
    //If the semiColonPosition is contained within an open parenthesis ie Odd number of ", without escape characters;
        //find the next semiColon.
    //if statement {
    //  findNext}
    //If semiColon cannot be found, ignore it, its not your job.
    int endNextStatement;
    //If you find semiColon:
    if (semiColonPosition != string::npos) {
        //set end to semicolon position
        endNextStatement = semiColonPosition+1;
    } else {
        //set end to end position
        endNextStatement = _inputBuffer.length();
    }
    //update current statement to the next statement.
    
    _currentStatement = _inputBuffer.substr(0,endNextStatement);
    //by this logic, _currentStatement may contain whitespace. Strip it;
    //cout << "|" << _currentStatement << "|" << endl;
    if (_currentStatement[_currentStatement.length()] == 32) {
        _currentStatement = _currentStatement.substr(0, _currentStatement.length()-1);
    }
    if (_currentStatement[0] == 32) {
        _currentStatement = _currentStatement.substr(1, _currentStatement.length());
    }
    //cout << "|" << _currentStatement << "|" << endl;

    //update input buffer to remove current statement.
    _inputBuffer = _inputBuffer.substr(endNextStatement, _inputBuffer.length());
    //For visual confirmation
    _fullStatement = _currentStatement;
    //load the next word to be parsed.
    
    getNextToken();
    //cout << "Token to be parsed: " << _nextToken << endl;
    //cout << "DEBUG: currentStatment: |"<< _currentStatement << "|" << endl;
    //cout << "DEBUG: inputBuffer: |" << _inputBuffer << "|" << endl;
    //cout << "DEBUG: _nextToken: |" << _nextToken << "|" << endl;
}

void  Parser::getNextToken() {
    //cout << "Statement retrieving token from: " << "|" << _currentStatement << "|" << endl;
    
    //DEFINE TOKEN:
        //A TOKEN is: Any single word, any single or group of words surrounded by parenthises
    
    //If the last token has been accepted, next token becomes "";
    if (_currentStatement.length() == 0) {
        _nextToken = "";

        return;
    }
    //If the current token begins with a ", find the next non \" parenthese as the end point.
    if (_currentStatement[0] == '"') {
        int startPoint = 1;
        int maxPoint = _currentStatement.length();
        while (true) {
            long long unsigned int endPoint = startPoint + _currentStatement.substr(startPoint,maxPoint).find('"') + 1;
            //If the '"' cannot be found, exit the function, display error.
            if (endPoint == string::npos) {
                cout << "ERROR: Could not parse value beginning with parentheses." << endl;
                return;
            }
            //If the '"' is found but has \ in front of it, update startPoint, retry.
            else if (_currentStatement.substr(startPoint,endPoint)[endPoint-1] == 92) {
                startPoint = endPoint;
                continue;
            } 
            //Otherwise presume literal is good.
            else {
                _nextToken = stripWhiteSpace(_currentStatement.substr(0,endPoint));
                _currentStatement = stripWhiteSpace(_currentStatement.substr(endPoint,maxPoint));
                //cout << "DEBUG: _nextToken has been set to: |" << _nextToken << "|" <<endl;
                //cout << "DEBUG: _currentStatement has been set to: |" << _currentStatement << "|" <<endl;
                return;
            }
        }
    } else if (_currentStatement[0]!=';'){
        
        int endPoint;
        int maxPoint = _currentStatement.length();
        long long unsigned int spacePosition = _currentStatement.find(32);
        //If the spacePosition is found:
        if (spacePosition != string::npos) {
            //to drop the front end space.
            //this creates an issue where words end with a space.
            //come back and fix this!!!!!!!!!!!!!!!!!!!!!!!!!!
            //TODO: Fix this
            endPoint = spacePosition;
            _nextToken = stripWhiteSpace(_currentStatement.substr(0,endPoint));
            _currentStatement = stripWhiteSpace(_currentStatement.substr(endPoint,maxPoint));
            
            //cout << "DEBUG: _nextToken has been set to: |" << _nextToken << "|" <<endl;
            //cout << "DEBUG: _currentStatement has been set to: |" << _currentStatement << "|" <<endl;
            return;
        }
        //if the spacePosition is not found:
        
        else if (spacePosition == string::npos) {
            //cout << "DEBUG: Colon position: |" << _currentStatement.find(';') << "|" <<endl;
            //cout << "DEBUG: End Position: |" << _currentStatement.length() << "|" <<endl;
            if (_currentStatement.find(';') == _currentStatement.length()-1) {
                _nextToken = stripWhiteSpace(_currentStatement.substr(0, _currentStatement.find(';')));
                _currentStatement = stripWhiteSpace(_currentStatement.substr(_currentStatement.find(';'),_currentStatement.length() ));
            }
        }
    } else if (_currentStatement[0] == ';') {
        _nextToken = ";";
        _currentStatement = "";
        return;
    }
}
bool  Parser::accept(regex regularExpression, types type) {
    if (!regex_match(_nextToken, regularExpression)) {
        return false;
    }
    //cout << "DEBUG: Accepted " << "|" << _nextToken << "|" << endl;
    _command->add(_nextToken, type);
    getNextToken();
    //cout << "DEBUG: New token: " << "|" << _nextToken << "|" << endl;
    return true;
}
string Parser::stripWhiteSpace(string input) {
    string output = input;
    bool shouldStrip = true;
    while (shouldStrip) {
        shouldStrip = false;
        if (output[0] == 32) {
            output = output.substr(1,output.length());
            shouldStrip = true;
        }
        if (output[output.length()] == 32) {
            output = output.substr(0,output.length()-1);
            shouldStrip = true;
        }
    }
    return output;
}