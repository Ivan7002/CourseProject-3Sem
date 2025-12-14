#include "Error.h"
#include "Parm.h"
#include "In.h"
#include "Log.h"
#define	LEX_MINUS		 '-'	// 
#define	MAXSIZE_LT		 4096	    //   

int In::InWord::size = NULL;

namespace In
{
	IN getin(wchar_t infile[], std::ostream* stream)			//     
	{
		unsigned char* text = new unsigned char[IN_MAX_LEN_TEXT];
		std::ifstream instream(infile);
		if (!instream.is_open())
			throw ERROR_THROW(102);								//" :      (-in)"
		IN in;
		int pos = 0;
		bool isLiteral = false;
		while (true)
		{
			unsigned char uch = instream.get();
			if (instream.eof())
				break;
			if (in.code[uch] == IN::Q)   //  
				isLiteral = !isLiteral;
			switch (in.code[uch])
			{
			case IN::N: //   
				text[in.size++] = uch;
				in.lines++;
				pos = -1;
				break;
			case IN::T: //   
			case IN::P: //   
			case IN::S: //  
			case IN::Q: // 
				text[in.size++] = uch;
				break;
			case IN::F: //     
				Log::writeError(stream, Error::GetError(200, in.lines, pos));
				in.ignor++;
				break;
			case IN::I://   
				in.ignor++;
				break;
			default:
				text[in.size++] = in.code[uch];
			}
			pos++;
		}
		text[in.size] = IN_CODE_NULL;
		in.text = text;
		instream.close(); //   
		return in;
	}
	void addWord(InWord* words, char* word, int line)					//
	{
		if (*word == IN_CODE_NULL)
			return;
		words[InWord::size].line = line;
		strcpy_s(words[InWord::size].word, strlen(word) + 1, word);
		InWord::size++;
	}
	InWord* getWordsTable(std::ostream* stream, unsigned char* text, int* code, int textSize)				//   
	{
		InWord* words = new InWord[MAXSIZE_LT];
		int bufpos = 0;				//   
		int line = 1;				//    
		char buffer[MAX_LEN_BUFFER] = "";		// 
		for (int i = 0; i < textSize; i++)			// 
		{
			switch (code[text[i]])
			{
			case IN::S:
			{
				if (text[i] == LEX_MINUS && isdigit(text[i + 1])) //    
				{
					buffer[bufpos++] = text[i];
					buffer[bufpos] = IN_CODE_NULL;
					break;
				}

				if ((text[i] == '!' || text[i] == '=' || text[i] == '>' || text[i] == '<') && text[i + 1] == '=')
				{
					addWord(words, buffer, line);
					char twoChars[] = { (char)text[i], (char)text[i + 1], IN_CODE_NULL };
					addWord(words, twoChars, line);
					i++;
					*buffer = IN_CODE_NULL;
					bufpos = 0;
					break;
				}

				char letter[] = { (char)text[i], IN_CODE_NULL };
				addWord(words, buffer, line);	//    
				addWord(words, letter, line);	//   
				*buffer = IN_CODE_NULL;
				bufpos = 0;
				break;
			}
			case IN::N:											// 
			case IN::P:											//   
				addWord(words, buffer, line);
				*buffer = IN_CODE_NULL;
				bufpos = 0;
				if (code[text[i]] == IN::N)
					line++;
				break;
			case IN::Q:		//  
			{
				addWord(words, buffer, line);
				*buffer = IN_CODE_NULL;
				bufpos = 0;
				bool isltrlgood = true;
				bool isChar = false;
				//    -      
				for (int j = i + 1; text[j] != IN_CODE_QUOTE; j++)
				{
					if (text[j] == IN_CODE_QUOTE2) {
						isChar = true;
						break;
					}
					if (code[text[j]] == IN::N)
					{
						Log::writeError(stream, Error::GetError(311, line, 0));
						isltrlgood = false; //   !
						break;
					}
				}
				if (isltrlgood && !isChar)
				{
					buffer[bufpos++] = IN_CODE_QUOTE;
					for (int j = 1; ; j++)
					{
						if (j == 256 || i + j == textSize)
						{
							Log::writeError(stream, Error::GetError(312, line, 0));
							break; //   (  )
						}
						buffer[bufpos++] = text[i + j];
						if (text[i + j] == IN_CODE_QUOTE && text[i + j - 1] != '\\') // Modified to check for escape
						{
							buffer[bufpos] = IN_CODE_NULL;
							addWord(words, buffer, line);
							i = i + j;
							break;
						}
					}
				} // literal good
				else if (isChar) {
					buffer[bufpos++] = IN_CODE_QUOTE2;
					for (int j = 1; ; j++)
					{
						if (j == 256 || i + j == textSize)
						{
							Log::writeError(stream, Error::GetError(312, line, 0));
							break; //   (  )
						}
						buffer[bufpos++] = text[i + j];
						if (text[i + j] == IN_CODE_QUOTE2 && text[i + j - 1] != '\\') // Modified to check for escape
						{
							buffer[bufpos] = IN_CODE_NULL;
							addWord(words, buffer, line);
							i = i + j;
							break;
						}
					}
				}
				*buffer = IN_CODE_NULL;  bufpos = 0;
				break;
			}
			default:
				buffer[bufpos++] = text[i];
				buffer[bufpos] = IN_CODE_NULL;

				// Проверка на слишком длинный buffer (число)
				if (bufpos > 11) {  // Если buffer содержит более 11 символов
					buffer[bufpos] = IN_CODE_NULL;
					// Проверяем, является ли содержимое buffer числом
					bool isNumeric = true;
					for (int k = 0; k < bufpos; k++) {
						if (k == 0 && buffer[k] == '-') continue;
						if (!isdigit(buffer[k])) {
							isNumeric = false;
							break;
						}
					}
					if (isNumeric) {
						// Слишком длинное число
						throw ERROR_THROW_IN(313, line, 0);
					}
				}
			}
		}
		return words;
	}

	void printTable(InWord* table)								//    
	{
		std::cout << " ------------------ Word Table ------------------" << std::endl;
		for (int i = 0; i < table->size; i++)
			std::cout << std::setw(2) << i << std::setw(3) << table[i].line << " |  " << table[i].word << std::endl;
	}
};