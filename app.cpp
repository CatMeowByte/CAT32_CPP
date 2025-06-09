#include "CAT32.hpp"
#include "lib/sdl.hpp"

// TODO: handle warning of exception, overflow, etc

enum ArgumentType {
	NIL,
	CMD,
	INT,
	STR,
};
struct Token {
	ArgumentType type;
	string value;
};

void init() {
	vector<vector<Token>> code;

 ifstream file("/media/storage/share/cpp/CAT32/example/0.app");
 if (!file) {
  cerr << "Failed to open file.\n";
 }

 string line;
	// per line
 while (getline(file, line)) {
		vector<Token> tokens;
		string token;
		int index = 0;
		bool in_quote = false;

		for (u16 character = 0; character <= line.size(); character++) {
			char c = (character < line.size() ? line[character] : ' ');

			// quote
			// also check if no backslash behind it
			if (c == '"') {
				int bs = 0;
				for (int pos = character - 1; pos >= 0 && line[pos] == '\\'; pos--) bs++;
				if (bs % 2 == 0) in_quote = !in_quote;
				token += c;
				continue;
			}

			// token inside quote
   if (in_quote) {
    token += c;
    continue;
   }

			// token boundary (space)
   if (c == ' ' || character == line.size()) {
    if (!token.empty()) {
     // categorize token
     ArgumentType type = NIL;

     // CMD
					if (index == 0) {type = CMD;}
     
					// INT
					else if (token.find_first_not_of("0123456789-+") == string::npos) {type = INT;}
     
					// STR
					else if (token.front() == '"' && token.back() == '"') {
						type = STR;
						token = token.substr(1, token.size() - 2); // remove quotes
						string unescaped;
						for (size_t i = 0; i < token.size(); i++) {
							if (token[i] == '\\' && i + 1 < token.size()) {
								i++;
								if (token[i] == 'n') {unescaped += '\n';}
								else if (token[i] == 't') {unescaped += '\t';}
								else if (token[i] == '\\') {unescaped += '\\';}
								else if (token[i] == '"') {unescaped += '"';}
								else {unescaped += token[i];}
							} else {
								unescaped += token[i];
							}
						}
						token = unescaped;
					}
     
					tokens.push_back({type, token});
     token.clear();
     index++;
    }
    continue;
   }

			// default: add char to token
   token += c;
		}
		code.push_back(tokens);
 }

	// print original code
	for (size_t i = 0; i < code.size(); i++) {
		for (size_t j = 0; j < code[i].size(); j++) {
			if (j > 0) {cout << ' ';}
			cout << code[i][j].value;
		}
		cout << endl;
	}
	cout << endl;

	// print type with value
	for (size_t i = 0; i < code.size(); i++) {
		for (size_t j = 0; j < code[i].size(); j++) {
			if (j > 0) {cout << ' ';}
			cout << "[";
			if (code[i][j].type == CMD) {cout << "CMD";}
			else if (code[i][j].type == INT) {cout << "INT";}
			else if (code[i][j].type == STR) {cout << "STR";}
			else {cout << "NIL";}
			cout << ":\"" << code[i][j].value << "\"]";
		}
		cout << endl;
	}
}

void update() {
 // per-frame logic
}

void draw() {
 clear(1);
 pixel(0, 0, 7);
 pixel(1, 1, 15);

 line(-4, 76, 57, 89, 14);

 rect(-17, 17, 55, 5, 12, true);
 rect(17, 45, 3, 3, 4, false);

 text(0, 0,  " ░▒▓█─│┌┐└┘├┤┬┴┼", 1, 0);
 text(0, 8,  "•°©®™          �", 2, 3);
 text(0, 16, " !\"#$%&'()*+,-./", 3, 2);
 text(0, 24, "0123456789:;<=>?", 4, 1);
 text(0, 32, "@ABCDEFGHIJKLMNO", 5, 0);
 text(0, 40, "PQRSTUVWXYZ[\\]^_", 6, 3);
 text(0, 48, "`abcdefghijklmno", 7, 2);
 text(0, 56, "pqrstuvwxyz{|}~ ", 8, 1);

 fps();

 flip();
}
