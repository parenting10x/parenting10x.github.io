#define DEBUG_TIME_BLOCK()
#include "easy_types.h"
#include "easy_array.h"
#include "easy_unicode.h"
#include "easy_string.h"
#include "easy_files.h"

// #include "webkit.h"
// #include "webkit.c"

typedef struct {
	InfiniteAlloc contentsToWrite;
	InfiniteAlloc tempHtmlEncoded;
} FileState;

typedef enum {
	COLOR_NULL,
	COLOR_VARIABLE,
	COLOR_FUNCTION,
	COLOR_BRACKET,
	COLOR_KEYWORD,
	COLOR_COMMENT,
	COLOR_PREPROCESSOR
} SyntaxColor;


static initFileState(FileState *state) {
	state->contentsToWrite = initInfinteAlloc(u8);
	state->tempHtmlEncoded = initInfinteAlloc(u8);
	
}

static void outputFile(FileState *state, char *filename) {
	FILE *file = fopen(filename, "wb");

	size_t sizeWritten = fwrite(state->contentsToWrite.memory, 1, state->contentsToWrite.count, file);

	fclose(file);
}

static u8 *openFileNullTerminate(char *filename) {
	FILE *file = fopen(filename, "rb");

	if(fseek(file, 0, SEEK_END) != 0) {
		assert(false);
	}

	size_t sizeOfFile = ftell(file);

	if(fseek(file, 0, SEEK_SET) != 0) {
		assert(false);
	}	

	//NOTE(ollie): plus one for null terminator
	u8 * result = (u8 *)malloc(sizeof(u8)*(sizeOfFile + 1)); 

	size_t sizeRead = fread(result, 1, sizeOfFile, file);
	if(sizeRead != sizeOfFile) {
		assert(false);
	}

	result[sizeOfFile] = '\0';

	fclose(file);

	return result;

}

static void writeLineBreak(FileState *state, int count) {
	char *text = "<br>";
	for(int i = 0; i < count; ++i) {
		u32 sizeInBytes = easyString_getSizeInBytes_utf8(text);
		addElementInifinteAllocWithCount_(&state->contentsToWrite, text, sizeInBytes);
	}
	
}


static void writeColor(FileState *state, SyntaxColor color) {
	char *text = " "; //don't add anything

	if(color == COLOR_VARIABLE) {
		text = "<span style=\"color: #6B8E23;\">";
	} else if(color == COLOR_BRACKET) {
		text = "<span style=\"color: #A08563;\">";
	} else if(color == COLOR_FUNCTION) {
		text = "<span style=\"color: #A08563;\">";
	} else if(color == COLOR_KEYWORD) {
		text = "<span style=\"color: #CD950C;\">";
	} else if(color == COLOR_COMMENT) {
		text = "<span style=\"color: #7D7D7D;\">";
	} else if(color == COLOR_PREPROCESSOR) {
		text = "<span style=\"color: #DAB98F;\">";
	}

	u32 sizeInBytes = easyString_getSizeInBytes_utf8(text);
	addElementInifinteAllocWithCount_(&state->contentsToWrite, text, sizeInBytes);
}

static void endColor(FileState *state) {
	char *text = "</span>";
	u32 sizeInBytes = easyString_getSizeInBytes_utf8(text);
	addElementInifinteAllocWithCount_(&state->contentsToWrite, text, sizeInBytes);
}

static void writeHeader(FileState *state) {
	char *text = "<!DOCTYPE html>\
	<html lang=\"en\">\
		<head>\
		  <title>Oliver Marsh</title>\
		  <meta charset=\"utf-8\">\
		  <meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">\
		  <link rel=\"stylesheet\" href=\"https://maxcdn.bootstrapcdn.com/bootstrap/3.3.7/css/bootstrap.min.css\">\
		  <link href=\"https://fonts.googleapis.com/css?family=Montserrat\" rel=\"stylesheet\">\
		  <script src=\"https://ajax.googleapis.com/ajax/libs/jquery/1.12.4/jquery.min.js\"></script>\
		  <script src=\"https://maxcdn.bootstrapcdn.com/bootstrap/3.3.7/js/bootstrap.min.js\"></script>\
		  <link rel=\"stylesheet\" type=\"text/css\" href=\"style.css\">\
		</head>";

		u32 sizeInBytes = easyString_getSizeInBytes_utf8(text);
		addElementInifinteAllocWithCount_(&state->contentsToWrite, text, sizeInBytes);
}

static void writeNavBar(FileState *state) {
	char *text = "<nav class=\"navbar navbar-default\" style=\"background-color: white; color: #f5f6f7;\">\
	  <div class=\"container\">\
	    <div class=\"navbar-header\">\
	      <button type=\"button\" class=\"navbar-toggle\" data-toggle=\"collapse\" data-target=\"#myNavbar\">\
	        <span class=\"icon-bar\"></span>\
	        <span class=\"icon-bar\"></span>\
	        <span class=\"icon-bar\"></span>\
	      </button>\
	      <a class=\"navbar-left\" href=\"./index.html#\"><img style=\"width: 3cm;\" src=\"./photos/logo.png\"></a>\
	    </div>\
	    <div class=\"collapse navbar-collapse\" id=\"myNavbar\">\
	      <ul class=\"nav navbar-nav navbar-right\"  style=\"margin-top: 25px;\">\
	        <li><a href=\"./index.html\">Articles</a></li>\
	      	<li><a href=\"./index.html\">Games</a></li>\
	      	<li><a href=\"./about.html\">About</a></li>\
	      </ul>\
	    </div>\
	  </div>\
	</nav>\
	<body>\
	<div class=\"container\">";

	u32 sizeInBytes = easyString_getSizeInBytes_utf8(text);
	addElementInifinteAllocWithCount_(&state->contentsToWrite, text, sizeInBytes);

}

static void startParagraph(FileState *state) {
	char *text = "<p>\n"; 
	u32 sizeInBytes = easyString_getSizeInBytes_utf8(text);
	addElementInifinteAllocWithCount_(&state->contentsToWrite, text, sizeInBytes);
}

static void endParagraph(FileState *state) {
	char *text = "</p>\n"; 
	u32 sizeInBytes = easyString_getSizeInBytes_utf8(text);
	addElementInifinteAllocWithCount_(&state->contentsToWrite, text, sizeInBytes);
}


#define writeText(state, text) writeText_(state, text, easyString_getSizeInBytes_utf8(text))
static void writeText_(FileState *state, char *text, u32 sizeInBytes) {
	
	addElementInifinteAllocWithCount_(&state->contentsToWrite, text, sizeInBytes);
}

static u32 getBytesUntilNewLine(u8 *at) {
	u32 result = 0;
	while(*at && !(at[0] == '\n' || at[0] == '\r')) {
		at++;
		result++; 
	}

	return result;
}

#define writeParagraph(state, text) writeParagraph_(state, text, easyString_getSizeInBytes_utf8(text))
#define writeParagraph_withSize(state, text) writeParagraph_(state, text, getBytesUntilNewLine((u8 *)text))
static u32 writeParagraph_(FileState *state, char *text, u32 sizeInBytes) {
	startParagraph(state);
	
	addElementInifinteAllocWithCount_(&state->contentsToWrite, text, sizeInBytes);

	endParagraph(state);
	return sizeInBytes;
}

#define writeTextUntileNewLine_withSize(state, text) writeText_returnSize_encoded(state, text, getBytesUntilNewLine((u8 *)text))
static u32 writeText_returnSize_encoded(FileState *state, char *text, u32 sizeInBytes) {

	for(int i = 0; i < sizeInBytes; i++) {
		if(text[i] == '>') {
			char *str = "&gt;";
			addElementInifinteAllocWithCount_(&state->tempHtmlEncoded, str, easyString_getSizeInBytes_utf8(str));	
		} else if(text[i] == '<') {
			char *str = "&lt;";
			addElementInifinteAllocWithCount_(&state->tempHtmlEncoded, str, easyString_getSizeInBytes_utf8(str));	
		} else {
			addElementInifinteAllocWithCount_(&state->tempHtmlEncoded, &text[i], 1);	
		}
		
	}

	addElementInifinteAllocWithCount_(&state->contentsToWrite, (u8 *)state->tempHtmlEncoded.memory, state->tempHtmlEncoded.count);

	state->tempHtmlEncoded.count = 0;

	return sizeInBytes;
}

#define writeTextUntileNewLine_withSize_notEncoded(state, text) writeText_returnSize_notEncoded_(state, text, getBytesUntilNewLine((u8 *)text))
static u32 writeText_returnSize_notEncoded_(FileState *state, char *text, u32 sizeInBytes) {

	addElementInifinteAllocWithCount_(&state->contentsToWrite, (u8 *)text, sizeInBytes);

	return sizeInBytes;
}


static void writeAnchorTag(FileState *state, u8 **at_, bool addButton, bool isInternal) {
	u8 *at = *at_;

	if(!isInternal) {
		char *str = "<a target='_blank' href='";
		addElementInifinteAllocWithCount_(&state->contentsToWrite, str, easyString_getSizeInBytes_utf8(str));	
	} else {
		char *str = "<a href='";
		addElementInifinteAllocWithCount_(&state->contentsToWrite, str, easyString_getSizeInBytes_utf8(str));	
	}


	//move pass space
	if(at[0] == ' ') {
		at++;	
	}
	
	

	//add the web address
	while(*at != ' ' && *at != '\r' && *at != '\n'&& *at != '\0') {
		addElementInifinteAllocWithCount_(&state->contentsToWrite, at, 1);
		at++;
	}

	{
		char *str = "'>";
		addElementInifinteAllocWithCount_(&state->contentsToWrite, str, easyString_getSizeInBytes_utf8(str));	
	}

	if(addButton) {
		char *str = "<div style='background-color: #FFE5B4; border-radius: 0.5cm; padding: 0.5cm;'>";
		if(isInternal) {
			//change the color
			str = "<div style='background-color: #ccff66; border-radius: 0.5cm; padding: 0.5cm;'>";
		}
		
		writeText_(state, str, easyString_getSizeInBytes_utf8(str));
	}

	//write till end of line
	at += writeTextUntileNewLine_withSize(state, at);


	char *str = " 👆";
	addElementInifinteAllocWithCount_(&state->contentsToWrite, str, easyString_getSizeInBytes_utf8(str));	

	if(addButton) {
		str = "</div>";
		writeText_(state, str, easyString_getSizeInBytes_utf8(str));
	}

	{
		char *str = "</a>";
		addElementInifinteAllocWithCount_(&state->contentsToWrite, str, easyString_getSizeInBytes_utf8(str));	
	}

	writeLineBreak(state, 2);

	*at_ = at;
	
}

#define writeH1(state, text) writeH1_(state, text, easyString_getSizeInBytes_utf8(text))
#define writeH1_withSize(state, text) writeH1_(state, text, getBytesUntilNewLine((u8 *)text))
static u32 writeH1_(FileState *state, char *title, u32 sizeInBytes) {
	writeText(state, "<h1>");
	writeText_(state, title, sizeInBytes);
	writeText(state, "</h1>");
	return sizeInBytes;
}

#define writeH2(state, text) writeH2_(state, text, easyString_getSizeInBytes_utf8(text))
#define writeH2_withSize(state, text) writeH2_(state, text, getBytesUntilNewLine((u8 *)text))
static u32 writeH2_(FileState *state, char *title, u32 sizeInBytes) {
	writeText(state, "<h2>");
	writeText_(state, title, sizeInBytes);
	writeText(state, "</h2><br>");
	return sizeInBytes;
}

#define writeH3(state, text) writeH3_(state, text, easyString_getSizeInBytes_utf8(text))
#define writeH3_withSize(state, text) writeH3_(state, text, getBytesUntilNewLine((u8 *)text))
static u32 writeH3_(FileState *state, char *title, u32 sizeInBytes) {
	writeText(state, "<h3>");
	writeText_(state, title, sizeInBytes);
	writeText(state, "</h3>");
	return sizeInBytes;
}

#define writeH4(state, text) writeH4_(state, text, easyString_getSizeInBytes_utf8(text))
#define writeH4_withSize(state, text) writeH4_(state, text, getBytesUntilNewLine((u8 *)text))
static u32 writeH4_(FileState *state, char *title, u32 sizeInBytes) {
	writeText(state, "<h4>");
	writeText_(state, title, sizeInBytes);
	writeText(state, "</h4>");
	return sizeInBytes;
}

static void writeSeperator(FileState *state) {
	writeText(state, "<hr>");
}

static void writeCodeBlock(FileState *state) {
	writeText(state, "<div class=\"code-block-left \">");
}

static void writeWhiteSpaceTab(FileState *state) {
	// writeText(state, "<span style='padding: 0.5cm;'></span>"); 
	writeText(state, "<span style='white-space: pre;'>    </span>"); 
	// writeText(state, "&emsp;&emsp;&emsp;&emsp;");
}

static void writeEndCodeBlock(FileState *state) {
	writeText(state, "</div>");
}

static void startInfoCard(FileState *state) {
	char *text = "<div class=\"row\">\n<div class=\"col-sm-10 col-md-12 col-lg-12\">\n<div class=\"info-card\">\n";

	u32 sizeInBytes = easyString_getSizeInBytes_utf8(text);
	addElementInifinteAllocWithCount_(&state->contentsToWrite, text, sizeInBytes);
}

static void endInfoCard(FileState *state) {
	char *text = "</div>\n</div>\n</div>\n";
	u32 sizeInBytes = easyString_getSizeInBytes_utf8(text);
	addElementInifinteAllocWithCount_(&state->contentsToWrite, text, sizeInBytes);
}
static void writeFooter(FileState *state) {
	
	char *text = "</div></body></html>";
	u32 sizeInBytes = easyString_getSizeInBytes_utf8(text);
	addElementInifinteAllocWithCount_(&state->contentsToWrite, text, sizeInBytes);

	//NOTE: Javascript for post requests and email list

	char * javascriptStr = "<script src='email.js'></script>";

	sizeInBytes = easyString_getSizeInBytes_utf8(javascriptStr);
	addElementInifinteAllocWithCount_(&state->contentsToWrite, javascriptStr, sizeInBytes);

}

static void eatWhiteSpace(u8 **at_) {
	u8 *at = *at_;
	while(*at && (at[0] == '\n' || at[0] == '\r' || at[0] == ' ' || at[0] == 9)) { //9 is tab
		at++;
	}

	(*at_) = at;
}

static void eatWhiteSpace_justSpaces(u8 **at_) {
	u8 *at = *at_;
	while(*at && (at[0] == ' ' || at[0] == 9)) { //9 is tab
		at++;
	}

	(*at_) = at;
}

int main(int argc, char **args) {
	if(argc >= 3) {

		char *exts[] = { "md", "mu" };
		FileNameOfType filesToConvert = getDirectoryFilesOfType(args[1], exts, 2);
		printf("%s\n", args[1]);
		for(int fileIndex = 0; fileIndex < filesToConvert.count; ++fileIndex) {

			FileState state = {0};
			initFileState(&state);

			writeHeader(&state);
			writeNavBar(&state);

			bool inCard = false;

			printf("%s\n", filesToConvert.names[fileIndex]);

			bool inCodeBlock = false;
			bool wasNewLine = false;

			int depthInFunction = 0;
			
			u8 *result = openFileNullTerminate(filesToConvert.names[fileIndex]);
			u8 *at = result;
			u8 *lastNewLineAt = 0;
			while(*at) {
				// printf("%s\n", at);
				if(false) {
					//NOTE(ollie): Just to make it look nicer with all else ifs

				} else if(inCodeBlock) {
					if(stringsMatchNullN("#ENDCODE", at, 8)) {
						inCodeBlock = false;
						at += 8;
						writeEndCodeBlock(&state);
						eatWhiteSpace(&at);
						writeLineBreak(&state, 1);
						wasNewLine = false;
						depthInFunction = 0;
					} else {
						

						if(wasNewLine) {
							eatWhiteSpace(&at);

							if(at[0] == '}') {
								depthInFunction--;
								assert(depthInFunction >= 0);
							}
							lastNewLineAt = at;

							//Use this to debug what depth in the scope stack we are in and what's at the start of the line
							// char buffer[45];
							// sprintf(buffer, "%d%c", depthInFunction, at[0]);
							// writeText_returnSize_encoded(&state, buffer, 2);

							for(int i = 0; i < depthInFunction; ++i) {
								writeWhiteSpaceTab(&state);
							}
							
							wasNewLine = false;
						} else if(*at == '\n' || *at == '\r') {


							while(*at == '\n' || *at == '\r') {
								// char buffer[45];
								// sprintf(buffer, "^");
								// writeText_returnSize_encoded(&state, buffer, 1);

								writeLineBreak(&state, 1);
								//skip passed windows style double newline feeds - /r/n
								if((*at == '\r' && at[1] == '\n')) {
									at++;

									// char buffer[45];
									// sprintf(buffer, "&");
									// writeText_returnSize_encoded(&state, buffer, 1);
								}

								//move pass the new line character
								at++;

								eatWhiteSpace_justSpaces(&at);

								
								
							}
							wasNewLine = true;
						} else {
							u8 *tempAt = at;
							u32 wordLength = 0;
							u8 *lastWord = tempAt;
							assert(lastWord[0] != '\n' || lastWord[0] != '\r')

							//comment
							if(tempAt[0] == '/' && tempAt[1] == '/') {
								writeColor(&state, COLOR_COMMENT);
								tempAt += writeTextUntileNewLine_withSize(&state, at);
								endColor(&state);
							} else {
								bool isString = false;
								SyntaxColor color = COLOR_NULL;

								while(*tempAt != '\0') {

									//comment in a line
									if(tempAt[0] == '/' && tempAt[1] == '/') {
										writeColor(&state, COLOR_COMMENT);
										tempAt += writeTextUntileNewLine_withSize(&state, tempAt);
										endColor(&state);
										assert(tempAt[0] == '\r' || tempAt[0] == '\n');
										lastWord = 0;
										wordLength = 0;
										break;
									} else 
									{

										if(tempAt[0] == '{') {
											depthInFunction++;
										} 

										if(tempAt[0] == '}' && tempAt != lastNewLineAt) {
											assert(depthInFunction > 0);
											depthInFunction--;
										} 

										wordLength++;

										if(!isString) {
											color = COLOR_NULL;
										}

										if(tempAt[0] == '\"') {
											if(!isString) {
												color = COLOR_PREPROCESSOR;
												writeColor(&state, color);
												isString = true;
											} else {
												isString = false;
											}
										}

										if(!isString) {
										if(tempAt[0] == '(' || tempAt[0] == ')' || tempAt[1] == '(' || tempAt[1] == ')' || *tempAt == ' ' || *tempAt == '\n' || *tempAt == '\r' || (*tempAt == '\"' && !isString)) {
											
											
											if(*tempAt == '(') {
												color = COLOR_BRACKET;
												writeColor(&state, color);
											} else if(tempAt[1] == '(') {
												color = COLOR_KEYWORD;
												writeColor(&state, color);
											} else if(*tempAt == ')') {
												color = COLOR_BRACKET;
												writeColor(&state, color);
											} else if(stringsMatchNullN("int", lastWord, 3) && wordLength == 4) {
												color = COLOR_VARIABLE;
												writeColor(&state, color);
											} else if(stringsMatchNullN("else", lastWord, 4) && wordLength == 5) {
												color = COLOR_KEYWORD;
												writeColor(&state, color);
											} else if(stringsMatchNullN("enum", lastWord, 4) && wordLength == 5) {
												color = COLOR_KEYWORD;
												writeColor(&state, color);
											} else if(stringsMatchNullN("static", lastWord, 6) && wordLength == 7) {
												color = COLOR_KEYWORD;
												writeColor(&state, color);
											} else if(stringsMatchNullN("const", lastWord, 5) && wordLength == 6) {
												color = COLOR_KEYWORD;
												writeColor(&state, color);
											} else if(stringsMatchNullN("float", lastWord, 5) && wordLength == 6) {
												color = COLOR_VARIABLE;
												writeColor(&state, color);
											} else if(stringsMatchNullN("char", lastWord, 4) && wordLength == 5) {
												color = COLOR_VARIABLE;
												writeColor(&state, color);
											} else if(stringsMatchNullN("return", lastWord, 6) && wordLength == 7) {
												color = COLOR_KEYWORD;
												writeColor(&state, color);
											} else if(stringsMatchNullN("u32", lastWord, 3) && wordLength == 4) {
												color = COLOR_VARIABLE;
												writeColor(&state, color);
											} else if(stringsMatchNullN("bool", lastWord, 4) && wordLength == 5) {
												color = COLOR_VARIABLE;
												writeColor(&state, color);
											} else if(stringsMatchNullN("cl", lastWord, 2) && wordLength == 3) {
												color = COLOR_PREPROCESSOR;
												writeColor(&state, color);
											} else if(stringsMatchNullN("/link", lastWord, 5) && wordLength == 6) {
												color = COLOR_PREPROCESSOR;
												writeColor(&state, color);
											} else if(stringsMatchNullN("true", lastWord, 4) && wordLength == 5) {
												color = COLOR_VARIABLE;
												writeColor(&state, color);
											} else if(stringsMatchNullN("unsigned", lastWord, 8) && wordLength == 9) {
												color = COLOR_VARIABLE;
												writeColor(&state, color);
											} else if(stringsMatchNullN("false", lastWord, 4) && wordLength == 5) {
												color = COLOR_VARIABLE;
												writeColor(&state, color);
											} else if(stringsMatchNullN("#include", lastWord, 8) && wordLength == 9) {
												color = COLOR_PREPROCESSOR;
												writeColor(&state, color);
											} else if(stringsMatchNullN("#if", lastWord, 3) && wordLength == 4) {
												color = COLOR_PREPROCESSOR;
												writeColor(&state, color);
											} else if(stringsMatchNullN("#define", lastWord, 7) && wordLength == 8) {
												color = COLOR_PREPROCESSOR;
												writeColor(&state, color);
											} else if(stringsMatchNullN("struct", lastWord, 6) && wordLength == 7) {
												color = COLOR_KEYWORD;
												writeColor(&state, color);
											} else if(stringsMatchNullN("#endif", lastWord, 6) && wordLength == 7) {
												color = COLOR_PREPROCESSOR;
												writeColor(&state, color);
											} else if(stringsMatchNullN("#ifdef", lastWord, 6) && wordLength == 7) {
												color = COLOR_PREPROCESSOR;
												writeColor(&state, color);
											} else if((lastWord[0] >= '0' && lastWord[0] <= '9') || lastWord[0] == '-' && (lastWord[1] >= '0' && lastWord[1] <= '9')) {
												color = COLOR_VARIABLE;
												writeColor(&state, color);
											}


											writeText_returnSize_encoded(&state, lastWord, wordLength);
											wordLength = 0;	
											lastWord = tempAt + 1;

											if(color != COLOR_NULL) {
												endColor(&state);
												color = COLOR_NULL;
											}
										} 
										}


										if(*tempAt == '\n' || *tempAt == '\r') {
											break;
										}

										tempAt++;

									}
								}
							}

							at = tempAt;

							wasNewLine = false;
						}
						
					}
				} else if(at[0] == '\r' || at[0] == '\n') {
					writeLineBreak(&state, 1);
					eatWhiteSpace(&at);
				} else if(stringsMatchNullN("#CODE", at, 5)) {
					inCodeBlock = true;
					at += 5;
					writeCodeBlock(&state);
					eatWhiteSpace(&at);
					//NOTE: below code is if we don't want to ignore the newlines at the start of a code block. For consistentancy I turned it off 
					// eatWhiteSpace_justSpaces(&at);
					// if(*at == '\r' || *at == '\n') {
					// 	if(*at == '\r' && at[1] == '\n') {
					// 		at++;
					// 	}
					// 	at++;
					// }

				} else if(stringsMatchNullN("#CARD", at, 5)) { //NOTE(ollie): paragraph
					at += 5;
					if(inCard) {
						endInfoCard(&state);
					}
					inCard = true;
					startInfoCard(&state);
					//NOTE(ollie): Eat all white space, so don't put in any new lines
					eatWhiteSpace(&at);
				} else if(stringsMatchNullN("#HR", at, 3)) { //NOTE(ollie): paragraph
					at += 3;
					writeSeperator(&state);
					eatWhiteSpace(&at);
					eatWhiteSpace(&at);
				} else if(stringsMatchNullN("#BR", at, 3)) { //NOTE(ollie): paragraph
					at += 3;
					writeLineBreak(&state, 1);
					eatWhiteSpace(&at);
				} else if(stringsMatchNullN("####", at, 4)) { //NOTE(ollie): H3
					at += 4;
					at += writeH4_withSize(&state, at);
				} else if(stringsMatchNullN("###", at, 3)) { //NOTE(ollie): H3
					at += 3;
					at += writeH3_withSize(&state, at);
					writeLineBreak(&state, 1);
				} else if(stringsMatchNullN("#TITLE", at, 6)) { //NOTE(ollie): H1
					at += 6;
					if(at[0] == ' ') {
						at++;
					}
					at += writeH1_withSize(&state, at);
					eatWhiteSpace(&at);
				} else if(at[0]== '\n' || at[0]== '\r') { //NOTE(ollie): H1
					// writeLineBreak(&state, 1);
					if(at[0] == '\r' && at[1] == '\n') {
						at += 2;
					} else {
						at++;	
					}
				} else if(stringsMatchNullN("##", at, 2)) { //NOTE(ollie): H2
					at += 2;
					at += writeH2_withSize(&state, at);
					eatWhiteSpace(&at);
				} else if(stringsMatchNullN("#ANCHOR_IMPORTANT", at, 17)) { //NOTE(ollie): <a> tag with green background
					at += 17;

					writeAnchorTag(&state, &at, true, false);
					eatWhiteSpace(&at);
				} else if(stringsMatchNullN("#INTERNAL_ANCHOR_IMPORTANT", at, 26)) { //NOTE(ollie): <a> tag with green background
					at += 26;

					writeAnchorTag(&state, &at, true, true);
					eatWhiteSpace(&at);

				} else if(stringsMatchNullN("#IMPORTANT", at, 10)) { //NOTE(ollie): <a> tag with green background
					at += 10;


					char *str = "<div style='background-color: #FFE5B4; border-radius: 0.5cm; padding: 0.5cm;'>";
					writeText_(&state, str, easyString_getSizeInBytes_utf8(str));

					//write till end of line
					at += writeTextUntileNewLine_withSize_notEncoded(&state, at);

					str = "</div>";
					writeText_(&state, str, easyString_getSizeInBytes_utf8(str));

					eatWhiteSpace(&at);

				} else if(stringsMatchNullN("#Email", at, 6)) { //NOTE(ollie): h2 with anchor to id in page
					at += 6;

					writeText(&state, "<div class='email-list'><p>Sign up to my Newsletter to get a weekly email about what I’m up to, what I’m learning and what I’m teaching.</p><div style='text-align: center;'><input class='email-input-style' id='email_input' type='email'/>");

					writeText(&state, "<button class='subscribe-btn' onclick='testEmail()'>Subscribe</button><div id='email-loading-progress'></div></div></div>");
					
				} else if(stringsMatchNullN("#Contents", at, 9)) { //NOTE(ollie): h2 with anchor to id in page
					at += 9;

					writeText(&state, "<h4>");

					writeText(&state, "<a href='#");

					eatWhiteSpace_justSpaces(&at);					

					//add the web address
					while(*at != ' ' && *at != '\r' && *at != '\n'&& *at != '\0') {
						addElementInifinteAllocWithCount_(&state.contentsToWrite, at, 1);
						at++;
					}

					writeText(&state, "'>");

					//Write the tile
					at += writeTextUntileNewLine_withSize(&state, at);

					writeText(&state, "</a>");
					writeText(&state, "</h4><br>");

					eatWhiteSpace(&at);

				} else if(stringsMatchNullN("#ID_HEADER", at, 10)) { //NOTE(ollie): 
					at += 10;

					eatWhiteSpace(&at);

					writeText(&state, "<h2>");

					writeText(&state, "<span id='");
					
					//add the id address
					while(*at != ' ' && *at != '\r' && *at != '\n'&& *at != '\0') {
						addElementInifinteAllocWithCount_(&state.contentsToWrite, at, 1);
						at++;
					}

					writeText(&state, "'>");					

					at += writeH2_withSize(&state, at);

					writeText(&state, "</span>");
					writeText(&state, "</h2>");

					eatWhiteSpace(&at);

				} else if(stringsMatchNullN("#ANCHOR", at, 7)) { //NOTE(ollie): <a> tag
					at += 7;
					writeAnchorTag(&state, &at, false, false);
					eatWhiteSpace(&at);

				} else if(stringsMatchNullN("#QUESTION", at, 9)) { //NOTE(ollie): <a> tag
					at += 9;
					writeAnchorTag(&state, &at, false, false);
					eatWhiteSpace(&at);
				} else { //(stringsMatchNullN("#", at, 1)) { //NOTE(ollie): paragraph
					// at++;
					at += writeParagraph_withSize(&state, at);
				} 
			}
			

			if(inCard) {
				endInfoCard(&state);
				inCard = false;
			}

			writeLineBreak(&state, 2);
			writeFooter(&state);

			char *shortName = getShortName(filesToConvert.names[fileIndex]);
			printf("%s\n", shortName);
			char *outputFileNameA = concat(shortName, ".html");
			char *outputFileNameB = concat(args[2], outputFileNameA);
			printf("%s\n", outputFileNameB);
			outputFile(&state, outputFileNameB);	

			//NOTE(ollie): Free all memory for this file
			free(outputFileNameA);
			free(outputFileNameB);
			free(result);
			free(filesToConvert.names[fileIndex]);
			free(shortName);
		}
				
	} else {
		printf("%s\n", "You need to pass an input file & an output file name");
	}
	
	
	return 0;
}