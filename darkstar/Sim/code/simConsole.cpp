//--------------------------------------------------------------------------- 
//
//--------------------------------------------------------------------------- 

#if defined(__BORLANDC__) && (__BORLANDC__ >= 0x500)
#pragma option -Jg
#endif

#include <base.h>
#include <stdio.h>
#include <stdarg.h>
#include <filstrm.h>
#include "console.h"
#include <g_surfac.h>
#include <simInput.h>


//--------------------------------------------------------------------------- 

#define ScriptFileExtension		".cs"
#define DefaultPrompt				"% "
#define DefaultTimeout				4000

const char* ShellVarPrompt = 			"Console::Prompt";
const char* ShellVarAutoExec = 		"Console::AutoExec";
const char* ShellVarExecEcho =   	"Console::ExecEcho";
const char* ShellVarBindEcho =   	"Console::BindEcho";
const char* ShellVarReturn =			"Console::Return";
const char* ShellVarStripVarPrefix ="Console::StripVarPrefix";
const char* ShellGFXFont =          "Console::GFXFont";
const char* ShellVarDisable =       "Console::Disable";
const char* ShellVarDebug =         "Console::Debug";
const char* ShellVarLastLineTimeout ="Console::LastLineTimeout";


//--------------------------------------------------------------------------- 

static ResourceTypeConsoleScript			_rescon(".cs");

void* ResourceTypeConsoleScript::construct(StreamIO *stream, int size)
{
   ConsoleScript *ret = new ConsoleScript;
   ret->data = new BYTE[size];
   stream->read(size, ret->data);
   ret->size = size;
   return (void*) ret;
}

void ResourceTypeConsoleScript::destruct(void *p)
{
   delete (ConsoleScript *) p;
}

//--------------------------------------------------------------------------- 

CMDConsole::CMDConsole()
{
	AssertFatal(spLocked == 0,
		"CMDConsole::CMDConsole:: A console has already constucted");
	spLocked = this;

	resManager = 0;
	ifState.level = 0;
	functionState.define = false;
	functionState.buffer = 0;
	functionState.bufferSize = 0;
	evalReturn = false;
	active = false;
   enabled = true;
	lastLineTimeout = UInt32(-1);
	font = 0;
	defaultFont = 0;
	caret = DefaultCaret;
	clearLine();
	currentHistory = -1;
	commandEcho = 0;
	gfxFont = NULL;
	declareInternal();
}

CMDConsole* CMDConsole::getLocked()
{
	return spLocked;
}	

//--------------------------------------------------------------------------- 
// Output string to the console
//
void __cdecl CMDConsole::printf(const char* fmt,...)
{
   EnterCriticalSection( &csPrintf );
	if (!active)
		lastLineTimeout = GetTickCount();
   va_list argptr;
	char buffer[MaxPrintfLength];

   va_start(argptr, fmt);
   int cnt = vsprintf(buffer, fmt, argptr);
	AssertFatal(cnt < MaxPrintfLength,
			"CMDConsole::printf: Overflowed format buffer");
   va_end(argptr);
	displayList.push_front(buffer);
   // loop through all the consumers...
   Vector<CMDConsumer *>::iterator i;
   for(i = consumerList.begin(); i != consumerList.end(); i++)
      (*i)->consoleCallback(this, buffer);

   LeaveCriticalSection( &csPrintf );
}


//--------------------------------------------------------------------------- 

bool CMDConsole::addCommand(int id,const char* name,CMDCallback::Callback cb, int privilegeLevel)
{
	// Command with function callack
	CommandList::iterator itr = commandList.find(name);
	if (itr == commandList.end())
	{
		Command cmd;
		cmd.id = id;
		cmd.data = "";
		cmd.callbackFunc = cb;
		cmd.callbackObj = 0;
      cmd.privilegeLevel = privilegeLevel;
		commandList.insert(name,cmd);
		resetCompletion();
		return true;
	}
	printf("Console: Command \"%s\" already registered",name);
	return false;
}

bool CMDConsole::addCommand(int id,const char* name,CMDCallback* cb, int privilegeLevel)
{
	// Command with object ptr callack
	CommandList::iterator itr = commandList.find(name);
	if (itr == commandList.end())
	{
		Command cmd;
		cmd.id = id;
		cmd.data = "";
		cmd.callbackFunc = 0;
		cmd.callbackObj = cb;
      cmd.privilegeLevel = privilegeLevel;
		commandList.insert(name,cmd);
		resetCompletion();
		return true;
	}
	printf("Console: Command \"%s\" already registered",name);
	return false;
}

bool CMDConsole::removeCommand(const char* name)
{
	CommandList::iterator itr = commandList.find(name);
	if (itr != commandList.end()) {
		commandList.erase(itr);
		resetCompletion();
	}
	return true;
}


//--------------------------------------------------------------------------- 

bool CMDConsole::addVariable(int id,const char* name,
	CMDCallback::Callback cb,const char* value)
{
	// Variable with function callback
	resetCompletion();
	if (*name == VariablePrefix &&
		getBoolVariable(ShellVarStripVarPrefix))
		name++;

	VariableList::iterator itr = variableList.find(name);
	if (itr == variableList.end()) {
		Variable cmd;
		cmd.id = id;
		cmd.type = Default;
		cmd.dataPtr = 0;
		cmd.data = value? value: "";
		cmd.callbackFunc = cb;
		cmd.callbackObj = 0;
		variableList.insert(name,cmd);
		return true;
	}
	else
		if ((*itr).second.callbackFunc == 0 &&
			(*itr).second.callbackObj == 0) {
			(*itr).second.id = id;
			(*itr).second.callbackFunc = cb;
			if (value)
				(*itr).second.data = value;
			return true;
		}
	printf("Console: Variable \"%s\" already registered",name);
	return false;
}

bool CMDConsole::addVariable(int id,const char* name,
	CMDCallback* cb,const char* value)
{
	// Variable with object ptr callback
	resetCompletion();
	if (*name == VariablePrefix &&
		getBoolVariable(ShellVarStripVarPrefix))
		name++;

	VariableList::iterator itr = variableList.find(name);
	if (itr == variableList.end()) {
		Variable cmd;
		cmd.id = id;
		cmd.type = Default;
		cmd.dataPtr = 0;
		cmd.data = value? value: "";
		cmd.callbackFunc = 0;
		cmd.callbackObj = cb;
		variableList.insert(name,cmd);
		return true;
	}
	else
		if ((*itr).second.callbackFunc == 0 &&
			(*itr).second.callbackObj == 0) {
			(*itr).second.id = id;
			(*itr).second.callbackObj = cb;
			if (value)
				(*itr).second.data = value;
			return true;
		}
	printf("Console: Variable \"%s\" already registered",name);
	return false;
}


bool CMDConsole::addVariable(int id,const char* name,
	CMDConsole::Type type,void* ptr)
{
	// Typed variable with storage ptr
	resetCompletion();
	if (*name == VariablePrefix &&
		getBoolVariable(ShellVarStripVarPrefix))
		name++;

	VariableList::iterator itr = variableList.find(name);
	if (itr == variableList.end()) {
		Variable cmd;
		cmd.id = id;
		cmd.type = type;
		cmd.dataPtr = ptr;
		cmd.data = "";
		cmd.callbackFunc = 0;
		cmd.callbackObj = 0;
		importVariable(cmd);
		variableList.insert(name,cmd);
		return true;
	}
	else
		if ((*itr).second.dataPtr == 0) {
			(*itr).second.id = id;
			(*itr).second.type = type;
			(*itr).second.dataPtr = ptr;
			exportVariable((*itr).second);
			return true;
		}
	printf("Console: Variable \"%s\" already registered",name);
	return false;
}	


bool CMDConsole::removeVariable(const char* name)
{
	resetCompletion();
	if (*name == VariablePrefix &&
		getBoolVariable(ShellVarStripVarPrefix))
		name++;
	VariableList::iterator itr = variableList.find(name);
	if (itr != variableList.end())
		variableList.erase(itr);
	return true;
}

//--------------------------------------------------------------------------- 
// Evaluate all the lines in the given text file.
// If the echo argument is true, the lines will be printed to the
// display as they are evaluated.
//
bool CMDConsole::evaluate(const char* file,int argc, const char* argv[],bool echo,int privilegeLevel)
{
   AssertWarn(file,"CMDConsole: evaluate called with null filename" );
   if( !file )
      return false;
   Resource<ConsoleScript> cs;
   if(!resManager)
      return false;

   MemRWStream fs;

   cs = resManager->load(file, true, false); // block load, check volume first
   if(!bool(cs))
      return false;
   fs.open(cs->size, cs->data);
   
   bool result = false;

	if (fs.getStatus() == STRM_OK) {
		commandEcho += !echo;
		result = evaluate(fs,argc,argv,file,echo,privilegeLevel);
		commandEcho -= !echo;
	}
   cs.purge();
	return result;
}

bool CMDConsole::evaluate(StreamIO& is,int argc, const char* argv[],const char* filename,bool echo, int privilegeLevel)
{
	int lineNumber = 1;
	evalReturn = false;
	commandEcho += !echo;
	pushArgs(argc,argv);
	// Read and evaluate a line at a time.
	for (;is.getStatus() == STRM_OK; ++lineNumber)
	{
		// This stream stuff is definitly not setup
		// for reading text files :(
		char line[MaxLineLength];
		char *ptr = line, *end = line + MaxLineLength - 1;
		while (is.read(1,ptr) && *ptr != '\n' && ptr != end)
			ptr++;
		*ptr = '\0';
		if (ptr == end)
		{
			if (filename)
				printf("Console: Shell file \"%s\": Line %d too long",
					filename,lineNumber);
			else
				printf("Console: Shell stream: Line %d too long",lineNumber);
			commandEcho -= !echo;
			popArgs(argc);
			return false;
		}
		if (!evaluate(line,true,privilegeLevel))
		{
			commandEcho -= !echo;
			popArgs(argc);
			return false;
		}
		if (evalReturn)
		{
			// Early exit from script
			evalReturn = false;
			commandEcho -= !echo;
			popArgs(argc);
			return true;
		}
	}
	commandEcho -= !echo;
	popArgs(argc);
	return is.getStatus() == STRM_EOS;
}


//--------------------------------------------------------------------------- 
// Process keyboard events from the event handler
//
bool CMDConsole::processEvent(const SimEvent *event)
{
   if (event->type != SimInputEventType) 
      return (false);

   // if it's not a KEY event from the primary Keyboard exit
   const SimInputEvent *ev = (const SimInputEvent*)event;
   if (!(ev->deviceType == SI_KEYBOARD && 
         ev->deviceInst == 0 &&
         ev->objType    == SI_KEY))
      return (false);


	//KeyboardEvent* ev = static_cast<KeyboardEvent*>(event);

   if ( getBoolVariable(ShellVarDisable) || (!active && ev->ascii != KeyActiveConsole) ) 
	{
		// Check for bound keys, bound keys only support
		// make & break.
		KeyBind key;
		key.make  = ev->action == SI_MAKE;
		key.vcode = ev->objInst;
		key.state = ev->modifier;

      // force modifier keys to be left or right, since makeKeyEvent() only
      // sets up modifiers for both left and right.  In order for the console
      // to distinguish betw. left & right modifier keys, makeKeyEvent() would
      // have to be modified, along with this code here.
      if (key.state & SI_CTRL)
         key.state |= SI_CTRL;
      if (key.state & SI_ALT)
         key.state |= SI_ALT;
      if (key.state & SI_SHIFT)
         key.state |= SI_SHIFT;
		KeyList::iterator itr = keyList.find(key);
		if (itr != keyList.end()) {
			int ce = getBoolVariable(ShellVarBindEcho)? 0: 1;
			commandEcho += ce;
			evaluate((*itr).second.c_str());
			commandEcho -= ce;
			return true;
		}
		return false;
	}

	if (ev->action == SI_BREAK) 
	   return (false);

	switch(ev->objInst) 
	{
		case DIK_UP:
			nextHistory(true);
			break;
		case DIK_DOWN:
			nextHistory(false);
			break;
		case DIK_LEFT:
         if (ev->modifier & SI_CTRL) wordLeft();
         else moveLeft();
			break;
		case DIK_RIGHT:
         if (ev->modifier & SI_CTRL) wordRight();
			else moveRight();
			break;
      case DIK_HOME:
         moveHome();
         break;
      case DIK_END:
         moveEnd();
         break;
		case DIK_DELETE:
         fwdDelete();
         break;
		case DIK_BACK:
			backspace();
			break;
		default:
			switch(ev->ascii) 
			{
				case KeyActiveConsole:
               if ( enabled )
               {
   					active ^= 1;
                  inputPriorityChanged();
               }
					break;
				case KeyCompleteCommand:
					completeCommand(!(ev->modifier & SI_SHIFT));
					break;
				case KeyEOL:
					historyList.push_front(lineBuffer);
					currentHistory = -1;
					evaluate(lineBuffer);
					clearLine();
					break;
				case KeyBackspace:
               if (ev->modifier & SI_CTRL)
					   backspace();
               else
					   appendChar(ev->ascii);
					break;
				case KeyDeleteLine:
               if (ev->modifier & SI_CTRL)
					   clearLine();
               else
					   appendChar(ev->ascii);
					break;
				default:
					if (ev->ascii && isprint(ev->ascii))
						appendChar(ev->ascii);
					break;
			}
	}
   return (true);
}


//------------------------------------------------------------------------------
bool CMDConsole::processQuery(SimQuery *query)
{
   if (query->type == SimInputPriorityQueryType)
   {
      SimInputPriorityQuery *q = (SimInputPriorityQuery*)query;
      q->priority = SI_PRIORITY_FILTER;
      return (true);
   }
   return false;
}   

//--------------------------------------------------------------------------- 

void CMDConsole::moveLeft()
{
	if (linePtr > lineBuffer)
		linePtr--;
}	

void CMDConsole::moveRight()
{
	if ((linePtr - lineBuffer) < (int)strlen(lineBuffer))
		linePtr++;
}	

void CMDConsole::moveHome()
{
   linePtr = lineBuffer;
}	

void CMDConsole::moveEnd()
{
   linePtr = &lineBuffer[strlen(lineBuffer)];
}	

void CMDConsole::wordRight()
{
   char *endPtr = &lineBuffer[strlen(lineBuffer)];
   char *tempPtr = linePtr;
   
   //first skip until we hit a space
   while ((tempPtr < endPtr) && (*tempPtr != ' ')) tempPtr++;
   
   //next skip all the spaces;
   while ((tempPtr < endPtr) && (*tempPtr == ' ')) tempPtr++;
   
   //set the line ptr
   linePtr = tempPtr;
}	

void CMDConsole::wordLeft()
{
   //don't go beyond the beginning of the string
   if (linePtr == lineBuffer) return;
   
   char *tempPtr = linePtr;
   
   //first skip all the spaces;
   while ((tempPtr > lineBuffer) && (*(tempPtr - 1) == ' ')) tempPtr--;
   
   //next skip until we hit a space
   while ((tempPtr > lineBuffer) && (*(tempPtr - 1) != ' ')) tempPtr--;
   
   //set the line ptr
   linePtr = tempPtr;
}	

void CMDConsole::fwdDelete()
{
   if ((linePtr - lineBuffer) < (int)strlen(lineBuffer))
   {
		char linebuff[MaxLineLength + MaxPromptLength];
      strcpy(linebuff, linePtr + 1);
      strcpy(linePtr, linebuff);
   }
	resetCompletion();
	return;
}	
void CMDConsole::backspace()
{
	if (linePtr > lineBuffer)
      if ((linePtr - lineBuffer) < (int)strlen(lineBuffer))
      {
   		char linebuff[MaxLineLength + MaxPromptLength];
         strcpy(linebuff, linePtr);
         linePtr--;
         strcpy(linePtr, linebuff);
      }
      else
      {
   		*--linePtr = '\0';
      }
	resetCompletion();
	return;
}	

void CMDConsole::appendChar(char c)
{
	if (c) {
		resetCompletion();
      if ((linePtr - lineBuffer) < (int)strlen(lineBuffer))
      {
   		char linebuff[MaxLineLength + MaxPromptLength];
         strcpy(linebuff, linePtr);
         *linePtr++ = c;
         strcpy(linePtr, linebuff);
      }
      else
      {
   		*linePtr++ = c;
   		*linePtr = '\0';
      }
	}
}	

void CMDConsole::clearLine()
{
	resetCompletion();
	lineBuffer[0] = '\0';
	linePtr = lineBuffer;
}

void CMDConsole::stuffLine(const char* line)
{
	resetCompletion();
	strcpy(lineBuffer,line);
	linePtr = lineBuffer;
	while (*linePtr != '\0')
		++linePtr;
}	


//--------------------------------------------------------------------------- 

void CMDConsole::printAliasList()
{
	for (AliasDictionary::iterator itr = aliasDictionary.begin();
		itr != aliasDictionary.end(); itr++)
		printf("%s = \"%s\"",(*itr).first.c_str(),
			(*itr).second.c_str());
}

void CMDConsole::printVariableList( char *varName )
{
   FindMatch   fm( varName );
	for (VariableList::iterator itr = variableList.begin();
		itr != variableList.end(); itr++)
        fm.findMatch(itr->first.c_str());
   for ( int i=0; i<fm.numMatches(); i++ )
   {
   	printf("%s = \"%s\"",fm.matchList[i],
   	   getVariable(fm.matchList[i]) );
   }
}

void CMDConsole::printHistoryList()
{
	for (HistoryList::iterator itr = historyList.end() - 1;
		itr != historyList.begin() - 1; itr--)
		printf(" %s",(*itr).c_str());
}

void CMDConsole::printCommandList()
{
	for (CommandList::iterator itr = commandList.begin();
		itr != commandList.end(); itr++)
		printf("%s",(*itr).first.c_str());
		// Command description?
		//(*itr).second.data.c_str());
}

void CMDConsole::printBindList()
{
	for (KeyList::iterator itr = keyList.begin();
		itr != keyList.end(); itr++) {
		SimInputEvent event;
		event.action   = (*itr).first.make ? SI_MAKE : SI_BREAK;
		event.objInst  = (*itr).first.vcode;
		event.modifier = (*itr).first.state;
		printf("%s = \"%s\"", makeKeyString(&event),	(*itr).second.c_str());
	}
}


//--------------------------------------------------------------------------- 

void CMDConsole::nextHistory(bool back)
{
	// Save the current buffer
	if (currentHistory == -1 && lineBuffer[0] != '\0') {
		historyList.push_front(lineBuffer);
		currentHistory = 0;
	}
	// Move up or down one
	if (back) {
		if (++currentHistory >= historyList.size())
			currentHistory = historyList.size() - 1;
	}
	else
		if (--currentHistory < -1)
			currentHistory = -1;
	// Clear the line if we go past the start
	if (currentHistory >= 0)
		stuffLine(historyList[currentHistory].c_str());
	else
		stuffLine("");
}


//--------------------------------------------------------------------------- 
// Attempts to complete the command in the line buffer
//
void CMDConsole::completeCommand(bool forward)
{
   // don't try to complete a blank line
   if (lineBuffer[0] == '\0')
      return;

	// Make sure it's the first word on the line or
	// it's has the variable prefix
	//if (*linePtr != '\0')
	//	return;
   linePtr = &lineBuffer[strlen(lineBuffer)];
   
	char* start = linePtr;
	bool variable = false;
	while (start >= lineBuffer && !isspace(*start) &&
		*start != CMDTokenizer::CommandSeperator &&
		*start != VariablePrefix)
		start--;
	if (start >= lineBuffer)
		if (*start == VariablePrefix)
			variable = true;
		else
			if (*start != CMDTokenizer::CommandSeperator) {
				// If there is nothing in front, it's a command
				char* ss = start;
				while (ss >= lineBuffer && isspace(*ss))
				ss--;
				if (ss >= lineBuffer &&
					*ss != CMDTokenizer::CommandSeperator)
					return;
			}
	start++;

	// Move current completion up or down
	if (completionList.size() == 0) {
		buildCompletionList(start,variable);
		if (completionList.size() == 0)
			return;
		currentCompletion = completionList.begin();
	}
	else
		if (forward) {
			if (++currentCompletion == completionList.end())
				currentCompletion = completionList.begin();
		}
		else {
			if (currentCompletion == completionList.begin())
				currentCompletion = completionList.end();
			--currentCompletion;
		}

	// Stuff matching command
	strcpy(start,(*currentCompletion)->c_str());
	linePtr = start + (*currentCompletion)->length();
}

void CMDConsole::resetCompletion()
{
	completionList.clear();
	currentCompletion = completionList.end();
}

//--------------------------------------------------------------------------- 
// Build a sorted list of matching variables or commands and aliases.
// The list needs to be reset if any variables, commands or aliases 
// are delete (the list keeps pointers to them).
//
void CMDConsole::buildCompletionList(const char* command,bool variable)
{
	int len = strlen(command);
	if (variable) {
		// Variables...
		for (VariableList::iterator itr = variableList.begin();
			itr != variableList.end(); itr++)
				if (!strnicmp((*itr).first.c_str(),command,len))
					completionList.push_back(&(*itr).first);
	}
	else {
		// Commands...
		for (CommandList::iterator itr = commandList.begin();
			itr != commandList.end(); itr++)
				if (!strnicmp((*itr).first.c_str(),command,len))
					completionList.push_back(&(*itr).first);

		// Aliases...
		for (AliasDictionary::iterator ADitr = aliasDictionary.begin();
			ADitr != aliasDictionary.end(); ADitr++)
				if (!strnicmp((*ADitr).first.c_str(),command,len))
					completionList.push_back(&(*ADitr).first);

		// Add matching files in directory
		completionDir.clear();
		if (getBoolVariable(ShellVarAutoExec)) {
			char buffer[MaxLineLength];
			strcpy(buffer,command);
			strcat(buffer,"*");
			strcat(buffer,ScriptFileExtension);
			int len = strlen(ScriptFileExtension);
			WIN32_FIND_DATA data;
			HANDLE handle = FindFirstFile(buffer,&data);
			if (handle != INVALID_HANDLE_VALUE) {
				do {
					data.cFileName[strlen(data.cFileName) - len] = '\0';
					completionDir.push_back(data.cFileName);
				}
				while (FindNextFile(handle,&data));
				FindClose(handle);
			}
			// Copy pointers into completion list.
			// This is done after thr completionDir is filled so we
			// don't have to worry about the array being reallocated.
			for (StringList::iterator itr = completionDir.begin();
					itr != completionDir.end(); itr++)
				completionList.push_back(itr);
		}
	}
	// Sort
	completionList.sort();
}	


//--------------------------------------------------------------------------- 

bool CMDConsole::defineFunction(const char* name)
{
	if (strlen(name)) {
		char filename[MaxLineLength];
		strcpy(filename,name);
		strcat(filename,ScriptFileExtension);
		if (!resManager->find(filename)) {
			functionState.define = true;
			functionState.name = filename;
			return true;
		}
	}
	return false;
}

void CMDConsole::endFunction()
{
   ConsoleScript *script = new ConsoleScript;
	// The console script must have it's data
	// allocated with new.
   script->data = new unsigned char[functionState.bufferSize];
   script->size = functionState.bufferSize;
   strcpy((char *)script->data,functionState.buffer);
	free(functionState.buffer);

	if (!resManager->add(ResourceType::typeof(ScriptFileExtension),
  	   	functionState.name.c_str(),script,true))
		delete script;

	functionState.buffer = 0;
	functionState.bufferSize = 0;
	functionState.define = false;
}

void CMDConsole::appendToFunction(const char* string)
{
	char* ptr;
	int len = strlen(string) + 1;
	if (functionState.buffer) {
		functionState.buffer = (char*)realloc(functionState.buffer,
			functionState.bufferSize + len);
		ptr = &functionState.buffer[functionState.bufferSize-1];
		functionState.bufferSize += len;
	}
	else {
		functionState.bufferSize = len + 1;
		functionState.buffer = (char*)malloc(functionState.bufferSize);
		ptr = functionState.buffer;
	}
	strcpy(ptr,string);
	strcpy(&ptr[len-1],"\n");
}


//--------------------------------------------------------------------------- 

void CMDConsole::clearDisplay()
{
	while (!displayList.empty())
		displayList.pop_front();
}	

void renderLine(GFXSurface *surface, GFXFont *gfxFont, int spacing,
				Point2I &drawPoint, const char *linebuff)
{
   int i;
   
	if (gfxFont->getStrWidth(linebuff) <= (surface->getWidth() - 4)) {
		surface->drawText_p(gfxFont, &drawPoint, linebuff);
	} else {
		// Break up the current command into multiple lines if it
		// extends past the end of the screen...
		//
		char *line[20];
		// we shouldn't need more than this...
		int numLines = 20;
		for ( i = 0; i < numLines; i++) {
			line[i] = new char[512];
			memset(line[i], '\0', 512);
		}
		
		unsigned int currIndex = 0;
		int currLine  = 0;
		do {
			int currLineIndex = 0;
			int width = (surface->getWidth() - 4);
			if (currLine != 0) width -= 10;
			do {
				line[currLine][currLineIndex] = linebuff[currIndex];
				currIndex++;
				currLineIndex++;
			} while (currIndex < strlen(linebuff) &&
					   gfxFont->getStrWidth(line[currLine]) <= width);

			currLine++;
			if (currIndex == strlen(linebuff)) {
				// If we broke because we ended the buffer,
				// break out of the loop
				break;
			} else {
				// If we broke out because we exceeded the width, back up one
				// char, and loop
				currIndex -= 1;
			}
		} while (true);

		// All lines but the last contain one extra character...
		for ( i = 0; i < currLine - 1; i++)
			line[i][strlen(line[i]) - 1] = '\0';

		drawPoint.y -= (spacing * (currLine - 1));
		Point2I currPoint = drawPoint;
		for ( i = 0; i < currLine; i++) {
			surface->drawText_p(gfxFont, &currPoint, line[i]);
			currPoint.y += spacing;
			if (i == 0) currPoint.x += 10;
		}
		for ( i = 0; i < numLines; i++) {
			delete [] line[i];
		}
	}
	
	// "Advance" the drawing point...
	drawPoint.y -= spacing;
}

void CMDConsole::render(GFXSurface* surface)
{
	if (!surface) return;

   AssertWarn(gfxFont, "No GFX font set in console render.\n");
   if(!bool(gfxFont))
      return;

   surface->setHazeSource(GFX_HAZE_NONE);
   surface->setAlphaSource(GFX_ALPHA_NONE);
   surface->setShadeSource(GFX_SHADE_NONE); 

	EnterCriticalSection( &csPrintf );

	GFXFontInfo finfo;
	gfxFont->getExtents(&finfo);
	if (active)
	{
		int spacing = finfo.baseline;
		int line = surface->getHeight() - spacing - 2;
		Point2I drawPoint(2, line);

		// Command Line
		char linebuff[MaxLineLength + MaxPromptLength];
      if ((linePtr - lineBuffer) < (int)strlen(lineBuffer))
      {
         char temp = *linePtr;
         *linePtr = '\0';
   		sprintf(linebuff,"%s%s%c%c%s",getVariable(ShellVarPrompt),lineBuffer,caret,temp,linePtr + 1);
         *linePtr = temp;         
      }
      else
      {
   		sprintf(linebuff,"%s%s%c",getVariable(ShellVarPrompt),lineBuffer,caret);
      }
		renderLine(surface,gfxFont,spacing,drawPoint,linebuff);

		// Display history
		for (DisplayList::iterator itr = displayList.begin();
				itr != displayList.end(); itr++) {
			renderLine(surface,gfxFont,spacing,drawPoint,(*itr).c_str());
		}
		lastLineTimeout = UInt32(-1);
	}
	else
	{
		// Only display last print line at the top of the
		// screen
		UInt32 timeDelta = GetTickCount() - lastLineTimeout;
		int timeout = getIntVariable(ShellVarLastLineTimeout);
		if (timeout > 0 && timeDelta < UInt32(timeout) && displayList.size())
		{
			Point2I drawPoint(2, 2);
			DisplayList::iterator itr = displayList.begin();
			surface->drawText_p(gfxFont, &drawPoint, (*itr).c_str());
		}
	}
	LeaveCriticalSection( &csPrintf );
}


//--------------------------------------------------------------------------- 

void CMDConsole::HistoryList::push_front(const char* str)
{
	if (size() >= CMDConsole::MaxHistory)
		pop_back();
	if (*str != '\0')
		CMDStringVector::push_front(str);
}

void CMDConsole::DisplayList::push_front(const char* str)
{
	if (size() >= CMDConsole::MaxDisplayLines)
		pop_back();
	CMDStringVector::push_front(str);
}


//--------------------------------------------------------------------------- 
// Internal console commands
//--------------------------------------------------------------------------- 

enum InternalCommands {
	ConsoleVariable = -1,
	CmdAlias,
	CmdActivate,
	CmdBind,
	CmdCls,
	CmdDebugBreak,
	CmdDec,
	CmdEcho,
	CmdEvaluate,
	CmdExec,
	CmdFalse,
	CmdHelp,
	CmdHistory,
	CmdInc,
	CmdNot,
	CmdReturn,
	CmdSet,
	CmdSetCat,
	CmdTrue,
	CmdTest,
	CmdQuit,
	GFXFontLoad,
   CmdExport,
};

void CMDConsole::declareInternal()
{
	// Shell variables
	setVariable(ShellVarPrompt,DefaultPrompt);
	setBoolVariable(ShellVarAutoExec,true);
	setBoolVariable(ShellVarExecEcho,false);
	setBoolVariable(ShellVarBindEcho,false);
	setBoolVariable(ShellVarStripVarPrefix,true);
	setIntVariable(ShellVarLastLineTimeout, DefaultTimeout);
#ifdef DEBUG
	setBoolVariable(ShellVarDebug,true);
#else
	setBoolVariable(ShellVarDebug,false);
#endif
	addVariable(GFXFontLoad, ShellGFXFont,this,"");

	// Commands
	addCommand(CmdActivate,"activate",this);
	addCommand(CmdCls,"cls",this);
	addCommand(CmdDebugBreak,"debug",this);
	addCommand(CmdEcho,"echo",this);
	addCommand(CmdEvaluate,"eval",this);
	addCommand(CmdExec,"exec",this);
	addCommand(CmdExport,"export",this);
	addCommand(CmdHelp,"?",this);
	addCommand(CmdHistory,"history",this);
	addCommand(CmdQuit,"quit",this, 1);
}

int CMDConsole::consoleCallback(CMDConsole*,int id,
	int argc,const char *argv[])
{
	switch(id) {
		case CmdActivate:
			if (argc > 1)
				active = getBool(argv[1]);
			else
				active ^= 1;
         inputPriorityChanged();
			break;
		case CmdCls:
			clearDisplay();
			break;
		case CmdDebugBreak:
			DebugBreak();
			break;
		case CmdEcho:
			printf(buildString(argc-1,&argv[1]));
			break;
		case CmdEvaluate:
			if (argc > 1)
				if (evaluate(buildString(argc-1,&argv[1])))
					// Pass return value back up.
					return getIntVariable(ShellVarReturn);
				else
					return 1;
			break;
		case CmdExec:
			if (argc > 1)
				if (evaluate(argv[1],argc - 2,&argv[2],
						getBoolVariable(ShellVarExecEcho)))
					// Pass return value back up.
					return getIntVariable(ShellVarReturn);
				else
					return 1;
			break;
      case CmdExport:
         if ( argc > 2 )
            exportVariables( argv[1], argv[2], (argc ==4) ? argv[3] : NULL );
         else
			   printf("Console:: export variableName file.cs [append]");
         break;
		case CmdFalse:
			// Any non-zero value is false
			return 1;
		case CmdHelp:
			printCommandList();
			break;
		case CmdHistory:
			printHistoryList();
			break;
		case CmdInc:
			if (argc == 2)
				setIntVariable(argv[1],getIntVariable(argv[1]) + 1);
			else
				printf("Console:: inc variableName");
			break;
		case CmdNot: {
			for (int i = 1; i < argc; i++)
				setBoolVariable(argv[i],!getBoolVariable(argv[i]));
			break;
		}
		case CmdReturn:
			evalReturn = true;
			if (argc > 1)
				return atoi(argv[1]);
			break;
		case CmdSet:
			if (argc > 1)
			{
            if ( strchr(argv[1],'*') || strchr(argv[1],'?') )
               printVariableList((char*)argv[1]);
            else
   				setVariable(argv[1],buildString(argc-2,&argv[2]));
			}
			else
				printVariableList();
			break;
		case CmdSetCat:
			if (argc > 1)
				setVariable(argv[1],buildString(argc-2,&argv[2],false));
			else
				printf("Console:: setcat variableName [args...]");
			break;
		case CmdTrue:
			// Default return value is "true"
			break;
		case CmdTest: {
			if (argc == 4)
				return test(argv[1],argv[2],argv[3]);
			printf("Console: test value1 operation value2");
			return 1;
		}
		case CmdQuit:
			PostQuitMessage(0);
			break;

		// Shell variable callbacks
		case GFXFontLoad:
			if (argc != 2) {
			   printf("Console: set Console::GFXFont <filename>");
				break;
			}
			if (loadGFXFont(argv[1]) != true) {
				printf("Console: Font file %s not found.", argv[1]);
				return 1;
			}
			break;
	}
	return 0;
}


//--------------------------------------------------------------------------- 
// Test arguments and returns console style results (0 == true).
//
enum TestOperation {
	Equal,
	NotEqual,
	LessThan,
	LessThanEqual,
	GreaterThan,
	GreaterThanEqual,
};
struct TestEntry {
	TestOperation operation;
	const char* string;
} TestOpList[] = {
	Equal,            "==",
	NotEqual,         "!=",
	LessThan,         "<",
	LessThanEqual,    "<=",
	GreaterThan,      ">",
	GreaterThanEqual, ">=",
};

inline bool isNumber(const char* c)
{
	return isdigit(c[0]) || ((c[0] == '.' || c[0] == '-') && isdigit(c[1]));
}

int CMDConsole::test(const char *v1,const char* op,const char* v2)
{
	const int listSize = sizeof(TestOpList) / sizeof(TestEntry);
	for (int i = 0; i < listSize; i++)
		if (!stricmp(op,TestOpList[i].string)) {
			if (isNumber(v1) && isNumber(v2)) {
				switch (TestOpList[i].operation) {
					case Equal:
						return !(atof(v1) == atof(v2));
					case NotEqual:
						return !(atof(v1) != atof(v2));
					case LessThan:
						return !(atof(v1) < atof(v2));
					case LessThanEqual:
						return !(atof(v1) <= atof(v2));
					case GreaterThan:
						return !(atof(v1) > atof(v2));
					case GreaterThanEqual:
						return !(atof(v1) >= atof(v2));
				}
			}
			else {
				switch (TestOpList[i].operation) {
					case Equal:
						return stricmp(v1,v2);
					case NotEqual:
						return !stricmp(v1,v2);
					case LessThan:
						return !(stricmp(v1,v2) < 0);
					case LessThanEqual:
						return !(stricmp(v1,v2) <= 0);
					case GreaterThan:
						return !(stricmp(v1,v2) > 0);
					case GreaterThanEqual:
						return !(stricmp(v1,v2) >= 0);
				}
			}
		}
	printf("Console:test: Unkown operation \"%s\"",op);
	return false;
}


//--------------------------------------------------------------------------- 

bool CMDConsole::loadGFXFont(const char *in_fileName)
{
	if (resManager) 
	{
      if (in_fileName)
      {
		   // Make sure it loaded before overiting our current handle.
		   Resource<GFXFont> fntRes = resManager->load(in_fileName, true);
		   if (bool(fntRes) == true) {
		   	gfxFont = fntRes;
		   	return true;
		   }
      }
      else
      {
		   if (bool(gfxFont) == true)
            gfxFont.unlock();
      }
	}
	else
		printf("Console: No resource manager installed");
	return false;
}


//------------------------------------------------------------------------------
const char* CMDConsole::makeKeyEvent(const char* string, SimInputEvent *event)
{
   // string format:
	// [make,break][{ctrl,alt,shift}-]{AlphaNum,VcodeName}
   event->deviceType = SI_KEYBOARD;      
   event->deviceInst = 0;
   event->objType    = SI_KEY;
   event->ascii      = 0;
   event->action     = SI_MAKE;
   event->modifier   = 0;

	const char *ptr = string;
	while (*ptr != '\0') {
		// Skip to next token
		while (isspace(*ptr)) ptr++;
		const char* tok = ptr;
		while (!isspace(*ptr) && *ptr != '-' && *ptr != '\0') ptr++;
		if (*ptr == '-') ptr++;

		if (!strnicmp(tok,"make",4)) {
			event->action = SI_MAKE;
			continue;
		}
		if (!strnicmp(tok,"break",5)) {
			event->action = SI_BREAK;
			continue;
		}
		if (!strnicmp(tok,"alt-",4)) {
			event->modifier |= SI_ALT;
			continue;
		}
		if (!strnicmp(tok,"ctrl-",5)) {
			event->modifier |= SI_CTRL;
			continue;
		}
		if (!strnicmp(tok,"shift-",6)) {
			event->modifier |= SI_SHIFT;
			continue;
		}

		// determine the DIK_CODE
		int len = ptr - tok;
      if (len == 1)
      {
			if (isupper(*tok)) 
			   event->modifier |= SI_SHIFT;
         event->objInst = SimInputManager::getDIK_CODE( *((BYTE*)tok) );
         event->ascii   = *((BYTE*)tok);
         break;
      }
      else
      {
         char buff[256];
         strncpy(buff, tok, len);
         buff[len] = 0;
         event->objInst = SimInputManager::getDIK_CODE( buff );
         if (event->objInst)
		      break;
		}
		// Must be garbage.
		return 0;
	}

	// Eat leftover whites space
	while (isspace(*ptr)) ptr++;
	return ptr;
}	

void CMDConsumer::consoleCallback(CMDConsole *, const char *)
{

}

void CMDConsole::addConsumer(CMDConsumer *consumer)
{
	EnterCriticalSection( &csPrintf );
   consumerList.push_back(consumer);
	LeaveCriticalSection( &csPrintf );
}
 
void CMDConsole::removeConsumer(CMDConsumer *consumer)
{
	EnterCriticalSection( &csPrintf );
   Vector<CMDConsumer *>::iterator i;
   for(i = consumerList.begin(); i != consumerList.end(); i++)
   {
      if((*i) == consumer)
      {
         consumerList.erase(i);
         break;
      }
   }
	LeaveCriticalSection( &csPrintf );
}

//------------------------------------------------------------------------------
const char* CMDConsole::makeKeyString(const SimInputEvent *event)
{
	static char buffer[128];
	
	if (event->action == SI_BREAK)
		strcpy(buffer,"break ");
	else // Make 
 		strcpy(buffer,"make ");
	if (event->modifier & SI_CTRL)
		strcat(buffer,"ctrl-");
	if (event->modifier & SI_ALT)
		strcat(buffer,"alt-");

   const DIKCode *dik = SimInputManager::getDIK_STRUCT(event->objInst);
   if (dik)
   {
      // must not have ascii equivalent
      if (dik->name)   
      {
		   if (event->modifier & SI_SHIFT)
		   	strcat(buffer,"shift-");
		   strcat(buffer, dik->name);
      }
      // must be ascii
		char cc[2] = " ";
		if (event->modifier & SI_SHIFT)
		{
		   strcat(buffer,"shift-");
         cc[0] = dik->upper;
		}
      else
         cc[0] = dik->lower;
		strcat(buffer,cc);
		return buffer;
   }

	// Error
	strcat(buffer,"UnknownKey");
	return buffer;
}   

//------------------------------------------------------------------------------
// does a wildcard scan for variables and writes them to a file.

bool CMDConsole::exportVariables(const char *varName, const char *filename, const char *flag )
{
   flag;

   FindMatch   fm( (char*)varName );
   for ( VariableList::iterator i=variableList.begin(); i!=variableList.end(); i++ )
     fm.findMatch(i->first.c_str());
   if ( fm.numMatches() )
   {
      StreamIO *iostrm;
      if ( flag && !strcmpi(flag,"append") )
      {
         iostrm = new FileRWStream(filename);
         iostrm->setPosition(((FileRWStream*)iostrm)->getSize());
      }
      else 
         iostrm = new FileWStream(filename);

      for ( int j=0; j<fm.numMatches(); j++ )
      {
         char  *buff = (char*)avar("set %s %s\r\n", fm.matchList[j], getVariable(fm.matchList[j]) );
         iostrm->write( strlen(buff), buff );
      }
      iostrm->close();
      delete iostrm;
      return true;      
   }
   else
   {
   	printf("Console::export: variable not found: %s",varName);
      return false;
   }
}   