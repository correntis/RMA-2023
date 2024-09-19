#include "stdafx.h"
namespace In
{
	IN getin(wchar_t infile[], std::ostream* stream)
	{
		unsigned char* text = new unsigned char[IN_MAX_LEN_TEXT];

		std::ifstream instream;
		instream.open(infile);
		if (!instream.is_open())
			throw ERROR_THROW(102);

		IN in;
		int pos = 0;

		while (true)  
		{
			unsigned char uch = instream.get(); 

			if (instream.eof())
			{
				break; 
			}

			switch (in.code[uch])
			{
			case IN::N: 					
			{
				text[in.size++] = uch;
				in.lines++; 
				pos = -1;
				break;
			}
			case IN::K: 								
			{
				in.ignor++; 

				while (true)
				{
					unsigned char c = instream.get();

					if (instream.eof())
					{
						break;
					}
					if (c == '\n') 
					{
						text[in.size++] = c;
						break;
					}
					in.ignor++;
				}

				in.lines++; 
				pos = -1;

				break;
			}
			case IN::T: 
			case IN::P: 
			case IN::S: 
			case IN::Q: 
			case IN::QQ:
			{
				text[in.size++] = uch;
				break;
			}
				
			case IN::F: 
			{
				Log::WriteError(stream, Error::geterrorin(200, in.lines, pos + 1)); // Запись ошибки в протокол
				in.ignor++;
				in.FS = true;
				break;
			}
			case IN::I:
			{
				in.ignor++;
				break;
			}
			default: 
				text[in.size++] = in.code[uch];
			}

			pos++;

			if (in.FS)
			{
				break;
			}
		}

		text[in.size] = IN_CODE_NULL;
		in.text = text;		
		instream.close();	

		return in;
	}

	void addWord(InWord* words, char* word, int line, int pos, int endpos)				
	{
		if (*word == IN_CODE_NULL)
		{
			return;
		}
		words[words->size].line = line;		
		words[words->size].pos = pos; 
		words[words->size].endpos = endpos;
		strcpy_s(words[words->size].word, strlen(word) + 1, word);		
		words->size++;
	}

	InWord* getWordsTable(std::ostream* stream, unsigned char* text, int* code, int textSize) 
	{
		InWord* words = new InWord[LT_MAXSIZE];					
		words->size = 0;

		int bufpos = 0;												
		int line = 1;	
		int defpos = 1;
		char buffer[MAX_LEN_BUFFER] = ""; 							
		bool overflow = false; 

		for (int i = 0; i < textSize; i++, defpos++) 								
		{
			
			if (words->size == LT_MAXSIZE - 1) 
			{
				overflow = true;
				break;
			}

			unsigned char ccc = text[i];

			switch (code[text[i]])
			{
			case IN::S: 
			{
				if (text[i] == (int)LEX_LESS && text[i + 1] == LEX_EQUAL || text[i] == LEX_MORE && text[i + 1] == LEX_EQUAL)
				{
					buffer[bufpos++] = text[i];		 					
					buffer[bufpos++] = text[i+1];							
					buffer[bufpos] = IN_CODE_NULL;
					 
					addWord(words, buffer, line, defpos - bufpos, defpos);

					*buffer = IN_CODE_NULL;  
					bufpos = 0;
					i += 1;

					break;
				}
				if (i == 0)
				{
					Log::WriteError(stream, Error::geterrorin(612,line,defpos));
					return NULL;
				}
				if (text[i] == LEX_MINUS && isdigit(text[i + 1]) ||							
					text[i] == LEX_MINUS && words[words->size - 1].word[0] == '=' ||				
					(text[i] == LEX_MINUS && text[i + 1] == LEX_MINUS))										
				{
					if (*buffer != IN_CODE_NULL)				
					{
						addWord(words, buffer, line, defpos - bufpos, defpos);
						*buffer = IN_CODE_NULL;  bufpos = 0;		
					}

					if (text[i + 1] == LEX_MINUS)
					{
						buffer[bufpos++] = LEX_INCR;			
						buffer[bufpos++] = text[i];					
						buffer[bufpos] = IN_CODE_NULL;
						addWord(words, buffer, line, defpos - bufpos, defpos);
						*buffer = IN_CODE_NULL;					
						bufpos = 0;
						i++;									
						break;
					}
					if (isdigit(words[words->size - 1].word[strlen(words[words->size - 1].word) - 1]) ||	
						words[words->size - 1].word[0] == LEX_RIGHTTHESIS ||								
						isalpha(words[words->size - 1].word[strlen(words[words->size - 1].word) - 1]) ||	
						isalpha(text[i + 1]))										
					{
						buffer[bufpos++] = text[i];						
						buffer[bufpos] = IN_CODE_NULL;				
						addWord(words, buffer, line, defpos - bufpos, defpos);
						*buffer = IN_CODE_NULL;				
						bufpos = 0;
						break;
					}
					buffer[bufpos++] = text[i];			
					buffer[bufpos] = IN_CODE_NULL;			
					break;
				}

				if ((text[i] == LEX_PLUS && text[i + 1] == LEX_PLUS))	
				{
					if (*buffer != IN_CODE_NULL)				
					{
						if (words->size == LT_MAXSIZE - 1)
						{
							overflow = true;
							break;
						}
						addWord(words, buffer, line, defpos - bufpos, defpos);
						*buffer = IN_CODE_NULL;  bufpos = 0;		
					}

					buffer[bufpos++] = LEX_INCR;			
					buffer[bufpos++] = text[i];				
					buffer[bufpos] = IN_CODE_NULL;	

					if (words->size == LT_MAXSIZE - 1)
					{
						overflow = true;
						break;
					}
					addWord(words, buffer, line, defpos - bufpos, defpos);

					*buffer = IN_CODE_NULL;					
					bufpos = 0;
					i++;

					break;
				}

				char letter[] = { (char)text[i], IN_CODE_NULL };	

				addWord(words, buffer, line, defpos - bufpos, defpos);	
				bufpos = 0;
				if (words->size == LT_MAXSIZE - 1)
				{
					overflow = true;
					break;
				}

				int position = defpos - bufpos; 
				addWord(words, letter, line, position, defpos + 1);			
				*buffer = IN_CODE_NULL;	

				break;
			}

			case IN::N:									
			case IN::P:

				if (*buffer == LEX_MINUS && strlen(buffer) == 1) 
				{
					break;
				}
				else
				{
					addWord(words, buffer, line, defpos - bufpos, defpos);	
					*buffer = IN_CODE_NULL;			
					bufpos = 0; 
				}
				
				if (text[i] == '\t')
				{
					defpos = 0; 
				}

				if (code[text[i]] == IN::N)
				{
					defpos = 0; 
					line++;
				}
				break;

			case IN::Q:					
			{
				if (i == 0) 
				{
					Log::WriteError(stream, Error::geterrorin(612, line, defpos)); 
					return NULL;
				}

				addWord(words, buffer, line, defpos - bufpos, defpos);

				*buffer = IN_CODE_NULL;						
				bufpos = 0;
				buffer[bufpos++] = IN_CODE_SQUOTE; 	

				if (text[i + 1] == IN::N)			
				{
					Log::WriteError(stream, Error::geterrorin(311, line, defpos - bufpos));
					break;
				}

				buffer[bufpos++] = text[++i]; 	
				if (text[i + 1] != IN_CODE_SQUOTE) 
				{
					Log::WriteError(stream, Error::geterrorin(312, line, defpos - bufpos));

					break;
				}
				buffer[bufpos++] = text[++i];			
				buffer[bufpos] = IN_CODE_NULL;	

				if (words->size == LT_MAXSIZE - 1)
				{
					overflow = true;
					break;
				}

				addWord(words, buffer, line, defpos - bufpos, defpos);
				*buffer = IN_CODE_NULL;  bufpos = 0;	

				break;
			}
			case IN::QQ:							
			{
				if (i == 0) 
				{
					Log::WriteError(stream, Error::geterrorin(612, line, defpos)); 
					return NULL; 
				}

				addWord(words, buffer, line, defpos - bufpos, defpos);
				*buffer = IN_CODE_NULL;									
				bufpos = 0;
				bool isltrlgood = true;						
				
				for (int j = i + 1; text[j] != IN_CODE_DQUOTE && j < textSize; j++)	 
				{
					if (code[text[j]] == IN::N)			
					{
						Log::WriteError(stream, Error::geterrorin(311, line, defpos - bufpos));
						isltrlgood = false;						
						break;
					}
				}

				if (isltrlgood)							
				{
					buffer[bufpos++] = IN_CODE_DQUOTE;	
					defpos++; 
					for (int j = 1; ; j++,defpos++)							
					{
						if (j == 256 || i + j == textSize)		
						{
							buffer[bufpos] = IN_CODE_NULL; 
							Log::WriteError(stream, Error::geterrorin(312, line, defpos - bufpos));  

							for (; ;j++)
							{
								if (text[i + j] == IN_CODE_DQUOTE || code[text[i + j]] == IN::N || code[text[i + j]] == IN::P)   
								{
									buffer[bufpos] = IN_CODE_NULL; 
									buffer[bufpos - 1] = IN_CODE_DQUOTE;

									if (words->size == LT_MAXSIZE - 1)  
									{
										overflow = true;  
										break;
									}

									addWord(words, buffer, line, defpos - bufpos, defpos);

									i = i + j; 

									break;
								}
							}

							break; 
						}

						buffer[bufpos++] = text[i + j];	

						if (text[i + j] == IN_CODE_DQUOTE)	
						{
							buffer[bufpos] = IN_CODE_NULL;	
							defpos++;
							if (words->size == LT_MAXSIZE - 1)
							{
								overflow = true;
								break;
							}
							addWord(words, buffer, line, defpos - bufpos, defpos); 

							i = i + j;								
							break;
						}
					}
				}
				*buffer = IN_CODE_NULL;  
				bufpos = 0;		

				break;
			}

			default:

				if (bufpos < MAX_LEN_BUFFER - 1) 
				{
					buffer[bufpos++] = text[i]; 
					buffer[bufpos] = IN_CODE_NULL;
				}
			}
			
		}

		if (buffer[0] != IN_CODE_NULL)
		{
			addWord(words, buffer, line, defpos - bufpos, defpos); 
		}

		if (overflow)
		{
			throw Error::geterror(204);
			return NULL;
		}

		return words;
	}
}