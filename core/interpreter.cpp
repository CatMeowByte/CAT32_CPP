#include "core/interpreter.hpp"
#include "core/video.hpp"

// TODO:
// handle warning of exception, overflow, etc
// decide wether newline is tokenized
// all inside /core should have namespace, as its a module for CAT32 kernel

Line tokenize(const string& text) {
 Line tokens;
 string token;
 int index = 0;
 bool in_quote = false;

 for (u16 character = 0; character <= text.size(); character++) {
  char c = (character < text.size() ? text[character] : ' ');

  // quote
  // also check if no backslash behind it
  if (c == '"') {
   int bs = 0;
   for (int pos = character - 1; pos >= 0 && text[pos] == '\\'; pos--) bs++;
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
  if (c == ' ' || character == text.size()) {
   if (!token.empty()) {
    // categorize token
    TokenType type = NIL;

    // CMD
    if (index == 0) {type = CMD;}

    // INT
    else if (token.find_first_not_of("0123456789-+") == string::npos) {type = INT;}

    // STR
    else if (token.front() == '"' && token.back() == '"') {
     type = STR;
     token = token.substr(1, token.size() - 2); // remove quotes
     string unescaped;
     for (u32 i = 0; i < token.size(); i++) {
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
 // blank
 if (tokens.empty()) {tokens.push_back({NIL, ""});}

 return tokens;
}

void execute(const Line& line) {
 if (line[0].type == CMD) {
  #define CMD(cmd_name) else if (line[0].value == cmd_name)

  if (false) {} // dummy

  CMD("CLEAR") {
   if (
    line.size() < 2
    || line[1].type != INT
   ) {
    cerr << "CLEAR: missing or invalid arguments" << endl;
    return;
   }
   int color = stoi(line[1].value);
   video::clear(color);
  }
  CMD("PIXEL") {
   if (
    line.size() < 4
    || line[1].type != INT
    || line[2].type != INT
    || line[3].type != INT
   ) {
    cerr << "PIXEL: missing or invalid arguments" << endl;
    return;
   }
   int x = stoi(line[1].value);
   int y = stoi(line[2].value);
   int color = stoi(line[3].value);
   video::pixel(x, y, color);
  }
  CMD("LINE") {
   if (
    line.size() < 6
    || line[1].type != INT
    || line[2].type != INT
    || line[3].type != INT
    || line[4].type != INT
    || line[5].type != INT
   ) {
    cerr << "LINE: missing or invalid arguments" << endl;
    return;
   }
   int x1 = stoi(line[1].value);
   int y1 = stoi(line[2].value);
   int x2 = stoi(line[3].value);
   int y2 = stoi(line[4].value);
   int color = stoi(line[5].value);
   video::line(x1, y1, x2, y2, color);
  }
  CMD("RECT") {
   if (line.size() < 7
    || line[1].type != INT
    || line[2].type != INT
    || line[3].type != INT
    || line[4].type != INT
    || line[5].type != INT
    || line[6].type != INT
   ) {
    cerr << "RECT: missing or invalid arguments" << endl;
    return;
   }
   int x = stoi(line[1].value);
   int y = stoi(line[2].value);
   int w = stoi(line[3].value);
   int h = stoi(line[4].value);
   int color = stoi(line[5].value);
   bool fill = stoi(line[6].value) != 0;
   video::rect(x, y, w, h, color, fill);
  }
  CMD("TEXT") {
   if (line.size() < 6
    || line[1].type != INT
    || line[2].type != INT
    || line[3].type != STR
    || line[4].type != INT
    || line[5].type != INT
   ) {
    cerr << "TEXT: missing or invalid arguments" << endl;
    return;
   }
   int x = stoi(line[1].value);
   int y = stoi(line[2].value);
   str text_val = line[3].value.c_str();
   int color = stoi(line[4].value);
   int background = stoi(line[5].value);
   video::text(x, y, text_val, color, background);
  }
  else {cerr << "Unknown command: " << line[0].value << endl;}

  #undef CMD
 }
}