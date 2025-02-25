#pragma once

#pragma once
#include <fstream>
#include "In.h"
#include "Parm.h"
#include "Error.h"
namespace Log												
{
	struct LOG											
	{
		wchar_t logfile[PARM_MAX_SIZE];					
		std::ofstream* stream;								
	};

	static const LOG INITLOG = { L"",NULL }; 

	LOG getlog(wchar_t logfile[]);							
	void WriteLine(LOG log, const char* c, ...);			
	void WriteLine(LOG, const wchar_t* c, ...);			
	void WriteLineConsole(char* c, ...);				
	void WriteLog(LOG log);								
	void WriteParm(LOG log, Parm::PARM parm);			
	void WriteIn(LOG log, In::IN in);					
	void WriteError(std::ostream* stream, Error::ERROR e);	
	void Close(LOG log);							


}