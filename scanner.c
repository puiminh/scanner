/* Scanner
 * @copyright (c) 2008, Hedspi, Hanoi University of Technology
 * @author Huu-Duc Nguyen
 * @version 1.0
 */

#include <stdio.h>
#include <stdlib.h>

#include "reader.h"
#include "charcode.h"
#include "token.h"
#include "error.h"


extern int lineNo;
extern int colNo;
extern int currentChar;
extern CharCode charCodes[];

/***************************************************************/

void skipBlank() {
  	while(charCodes[currentChar] == CHAR_SPACE && currentChar != EOF){
  		readChar();
  }
}

void skipComment() {
	// Khoi tao bien dem de nhan biet (*
	int openComment = 0;
	// Trong khi ky tu hien tai chua phai la cuoi dong, va openComment < 2, doc ky tu ke tiep
	while(currentChar != EOF && openComment < 2){
		//vi khi goi den skipComment chung ta da doc dc 1 ky tu '(' va 1 ky tu '*' -> chi can doc thay 1  '*' nua tuc la dong comment		
		
		if(charCodes[currentChar] == CHAR_TIMES){
			//mang tinh chat xac nhan da bat dau dong ngoac
			openComment = 1;

			//neu phat hien thay ky tu ') -> kiem tra xem da mo co dau '*' truoc do hay chua, neu da co, thanh cong!
		}else if(charCodes[currentChar] == CHAR_RPAR){
			if(openComment == 1){
				openComment = 2;
			}else openComment = 0;
		}else {
			openComment = 0;
		}
		readChar();
	}
		// Khi openComment khong du -> thieu 1 trong 2 dau '*' hoac ')' => bao loi
	if(openComment != 2){
		error(ERR_ENDOFCOMMENT, lineNo, colNo);
	}
}

Token* readIdentKeyword(void) {
	//Tao mot token rong
	Token *tk = makeToken(TK_NONE, lineNo, colNo);
	// Phai ep kieu vi currentChar la dang int
	tk->string[0] = (char) currentChar; 
	
	readChar();
	int count = 1;
	
	//Trong khi van dang chua het file, va ky tu hien tai van dang la letter hoac la number -> tiep tuc doc den het
	while(currentChar != EOF && (charCodes[currentChar] == CHAR_LETTER || charCodes[currentChar] == CHAR_DIGIT)){
		//tao bien dem de kiem tra dieu kien do dai cua indent
		if(count <= MAX_IDENT_LEN){
			tk->string[count] = (char) currentChar;
		}
		count++;
		readChar();
	}
	//bao loi khi indent nay qua dai
	if(count > MAX_IDENT_LEN){
		error(ERR_IDENTTOOLONG, lineNo, colNo);
	}
	//tao end string cho no
	tk->string[count] = '\0'; 

	// Kiem tra xem no co phai la mot keyword?
	tk->tokenType = checkKeyword(tk->string);
	
	//neu khong, cho no la 1 indent
	if(tk->tokenType == TK_NONE){
		tk->tokenType = TK_IDENT;
	}
	return tk;
}

Token* readNumber(void) {
	//Tuong tu
	
	Token *tk = makeToken(TK_NONE, lineNo, colNo);
	int count = 0;
	
	while(currentChar != EOF && charCodes[currentChar] == CHAR_DIGIT){
		tk->string[count] = (char) currentChar;
		count++;
		readChar();
	}
	
	tk->string[count] = '\0';
	tk->value = atoi(tk->string);
	tk->tokenType = TK_NUMBER;
	return tk;
}

Token* readConstChar(void) {
	//Tao token rong
	Token *tk = makeToken(TK_CHAR, lineNo, colNo);
	
	//Them ky tu tiep theo neu khong phai la eof (tuc la mo nhay don ' nhung khong viet them gi)
	readChar();
	if(currentChar == EOF){
		error(ERR_INVALIDCHARCONSTANT, lineNo, colNo);
		tk->tokenType = TK_NONE;
		return tk;
	}
	
	//Doc char va end string
	tk->string[0] = currentChar;
	tk->string[1] = '\0';
	
	//Neu ky tu tiep la ' thi ok, khong thi bao loi
	readChar();
	if(currentChar == EOF){
		error(ERR_INVALIDCHARCONSTANT, lineNo, colNo);
		tk->tokenType = TK_NONE;
		return tk;
	}
	
	if(charCodes[currentChar] == CHAR_SINGLEQUOTE){
		readChar();
		return tk;
	}else{
		tk->tokenType = TK_NONE;
		error(ERR_INVALIDCHARCONSTANT, lineNo, colNo);
		return tk;
	}
}

Token* getToken(void) {
	
	Token *token;
	int ln, cn;

	if (currentChar == EOF) 
		return makeToken(TK_EOF, lineNo, colNo);

	switch (charCodes[currentChar]) {	
		// Doc duoc khoang trang -> skipBlank
		  case CHAR_SPACE: 
		  	skipBlank(); 
			return getToken();
			// Doc duoc chu cai -> keyword hoac ident
  		case CHAR_LETTER: 
			return readIdentKeyword();
		
		// Doc duoc so -> number
  		case CHAR_DIGIT: 
			return readNumber();
		
		// Doc duoc dau '+' -> token + 
  		case CHAR_PLUS:  
   			token = makeToken(SB_PLUS, lineNo, colNo);
    		readChar(); 
    		return token;
    	
		// Doc duoc dau '*' -> token *
    	case CHAR_TIMES: 
    		token = makeToken(SB_TIMES, lineNo, colNo);
    		readChar();
    		return token;
    		
		// Doc duoc dau '-' -> token -	
    	case CHAR_MINUS:
    		token = makeToken(SB_MINUS, lineNo, colNo);
    		readChar();
    		return token;
    	// Doc duoc dau '/' -> token /
    	case CHAR_SLASH: 
    		token = makeToken(SB_SLASH, lineNo, colNo);
    		readChar();
    		return token;
    	// Doc duoc dau '<' -> token <
    	case CHAR_LT: 
    		ln = lineNo;
    		cn = colNo;
    		readChar();
    		// Neu tiep theo la EOF tra ve token '<' 
    		if(currentChar == EOF){
    			return makeToken(SB_LT, ln, cn);
			}
			// Neu tiep theo la '=' thi doc them mot ky tu va tra ve '<=', neu khong co giu nguyen token
			if(charCodes[currentChar] == CHAR_EQ){
				readChar();
				return makeToken(SB_LE, ln, cn);
			}else{
				return makeToken(SB_LT, ln, cn); 
			}
		// Doc duoc dau '<' -> token <
		case CHAR_GT: 
			ln = lineNo;
    		cn = colNo;
    		readChar();
    		//tuong tu
    		if(currentChar == EOF){
    			return makeToken(SB_GT, ln, cn);
			}
			
			if(charCodes[currentChar] == CHAR_EQ){
				readChar();
				return makeToken(SB_GE, ln, cn);
			}else{
				return makeToken(SB_GT, ln, cn); 
			}
		//Doc dau =
    	case CHAR_EQ: 
    		token = makeToken(SB_EQ, lineNo, colNo);
    		readChar();
    		return token;
    	// Doc dau !
    	case CHAR_EXCLAIMATION: 
    		ln = lineNo;
    		cn = colNo;
    		readChar();
    		//Tra lai '!=' neu doc duoc dau '=' khong thi bao loi
    		if(currentChar != EOF && charCodes[currentChar] == CHAR_EQ){
    			readChar();
    			return makeToken(SB_NEQ, ln, cn);
			}else{
				error(ERR_INVALIDSYMBOL, ln, cn);
				return makeToken(TK_NONE, ln, cn);
			}
		// Doc dau ','
		case CHAR_COMMA: 
			token = makeToken(SB_COMMA, lineNo, colNo);
			readChar();
			return token;
		
		// Doc dau '.'
		case CHAR_PERIOD: 
			ln = lineNo;
    		cn = colNo;
			readChar();
			//Neu doc duoc them ')' -> '.)'
			if(currentChar != EOF && charCodes[currentChar] == CHAR_RPAR){
				readChar();
				return makeToken(SB_RSEL, ln, cn);
			}else{
				return makeToken(SB_PERIOD, ln, cn);
			}
		// Doc dau ;
		case CHAR_SEMICOLON: 
			token = makeToken(SB_SEMICOLON, lineNo, colNo);
			readChar();
			return token;
		
		// Doc dau :
		case CHAR_COLON: 
			ln = lineNo;
    		cn = colNo;
			readChar();
			// Neu ky tu tiep theo la '=' tra ve assign gan, khong thi giu nguyen':'
			if(currentChar != EOF && charCodes[currentChar] == CHAR_EQ){
				readChar();
				return makeToken(SB_ASSIGN, ln, cn);
			}else{
				return makeToken(SB_COLON, ln, cn);
			}
		
		// Doc duoc '
		case CHAR_SINGLEQUOTE: 
			return readConstChar();
		
		// Doc duoc (
    	case CHAR_LPAR: 
    		ln = lineNo;
    		cn = colNo;
    		readChar();
    		
    		//Tra ve token ( khi khong co gi khac
    		if(currentChar == EOF){
    			return makeToken(SB_LPAR, ln, cn);
			}
			//Neu doc duoc them '.' -> '(.' 
			if(charCodes[currentChar] == CHAR_PERIOD){
				readChar();
				return makeToken(SB_LSEL, ln, cn);
			//Neu doc duoc them '*' -> '(*'  --> bat dau doc comment
			}else if(charCodes[currentChar] == CHAR_TIMES){
				readChar();

				skipComment();
				return getToken();
			}else{
				//Tra ve token ( khi khong co gi khac
				return makeToken(SB_LPAR, ln, cn);
			}
		// Doc duoc )
		case CHAR_RPAR: 
			token = makeToken(SB_RPAR, lineNo, colNo);
			readChar();
			return token;

		// Con lai se tra ve symbol loi
  		default:
    		token = makeToken(TK_NONE, lineNo, colNo);
			//bao loi
    		error(ERR_INVALIDSYMBOL, lineNo, colNo);
    		readChar(); 
    		return token;
  }
}


/******************************************************************/

void printToken(Token *token) {

  printf("%d-%d:", token->lineNo, token->colNo);

  switch (token->tokenType) {
  case TK_NONE: printf("TK_NONE\n"); break;
  case TK_IDENT: printf("TK_IDENT(%s)\n", token->string); break;
  case TK_NUMBER: printf("TK_NUMBER(%s)\n", token->string); break;
  case TK_CHAR: printf("TK_CHAR(\'%s\')\n", token->string); break;
  case TK_EOF: printf("TK_EOF\n"); break;

  case KW_PROGRAM: printf("KW_PROGRAM\n"); break;
  case KW_CONST: printf("KW_CONST\n"); break;
  case KW_TYPE: printf("KW_TYPE\n"); break;
  case KW_VAR: printf("KW_VAR\n"); break;
  case KW_INTEGER: printf("KW_INTEGER\n"); break;
  case KW_CHAR: printf("KW_CHAR\n"); break;
  case KW_ARRAY: printf("KW_ARRAY\n"); break;
  case KW_OF: printf("KW_OF\n"); break;
  case KW_FUNCTION: printf("KW_FUNCTION\n"); break;
  case KW_PROCEDURE: printf("KW_PROCEDURE\n"); break;
  case KW_BEGIN: printf("KW_BEGIN\n"); break;
  case KW_END: printf("KW_END\n"); break;
  case KW_CALL: printf("KW_CALL\n"); break;
  case KW_IF: printf("KW_IF\n"); break;
  case KW_THEN: printf("KW_THEN\n"); break;
  case KW_ELSE: printf("KW_ELSE\n"); break;
  case KW_WHILE: printf("KW_WHILE\n"); break;
  case KW_DO: printf("KW_DO\n"); break;
  case KW_FOR: printf("KW_FOR\n"); break;
  case KW_TO: printf("KW_TO\n"); break;

  case SB_SEMICOLON: printf("SB_SEMICOLON\n"); break;
  case SB_COLON: printf("SB_COLON\n"); break;
  case SB_PERIOD: printf("SB_PERIOD\n"); break;
  case SB_COMMA: printf("SB_COMMA\n"); break;
  case SB_ASSIGN: printf("SB_ASSIGN\n"); break;
  case SB_EQ: printf("SB_EQ\n"); break;
  case SB_NEQ: printf("SB_NEQ\n"); break;
  case SB_LT: printf("SB_LT\n"); break;
  case SB_LE: printf("SB_LE\n"); break;
  case SB_GT: printf("SB_GT\n"); break;
  case SB_GE: printf("SB_GE\n"); break;
  case SB_PLUS: printf("SB_PLUS\n"); break;
  case SB_MINUS: printf("SB_MINUS\n"); break;
  case SB_TIMES: printf("SB_TIMES\n"); break;
  case SB_SLASH: printf("SB_SLASH\n"); break;
  case SB_LPAR: printf("SB_LPAR\n"); break;
  case SB_RPAR: printf("SB_RPAR\n"); break;
  case SB_LSEL: printf("SB_LSEL\n"); break;
  case SB_RSEL: printf("SB_RSEL\n"); break;
  }
}

int scan(char *fileName) {
  Token *token;

  if (openInputStream(fileName) == IO_ERROR)
    return IO_ERROR;

  token = getToken();
  while (token->tokenType != TK_EOF) {
    printToken(token);
    free(token);
    token = getToken();
  }

  free(token);
  closeInputStream();
  return IO_SUCCESS;
}

/******************************************************************/

int main(int argc, char *argv[]) {
  if (argc <= 1) {
    printf("scanner: no input file.\n");
    return -1;
  }

  if (scan(argv[1]) == IO_ERROR) {
    printf("Can\'t read input file!\n");
    return -1;
  }
    
  return 0;
}
