/**
 * Copyright (C) <2012> <Mike Dellisanti>
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this software and associated documentation files (the "Software"), to deal in
 * the Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
 * of the Software, and to permit persons to whom the Software is furnished to do
 * so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.

 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 **/

// YASP VERSION ALPHA 1

// YASP requires the use of the Regexp library available here:
// http://www.gammon.com.au/forum/?id=11063.  This code is compatible
// with version 1.2 of that lib.
#include <Regexp.h>
#include "YASP.h"

// Default terminal strings.  These can be changed by setting the
// class member variables after calling the class constructor.
const char *yaspErrMsg = "Error.  Type '?' for help.";
const char *yaspUserPrompt = "> ";
const char *yaspEOL = "\r\n";
const char *yaspHelpCmd = "^%?$";

// Default constructor
yasp::yasp(yaspCommand_t *cmds, Print *print, yaspPrintCallback_t printCB){
  this->mCmdTable = cmds;
  this->mPrint = print;
  this->mPrintCB = printCB;

  // Setup the default strings
  this->errMsg = yaspErrMsg;
  this->userPrompt = yaspUserPrompt;
  this->EOL = yaspEOL;
  this->helpCmd = yaspHelpCmd;
}
 
// destructor
yasp::~yasp(){/*nothing to destruct*/}

// Call this once for each byte of data that should be appended
// to the command.
void yasp::addData(byte data)
{
  // Any data available
  static char cmd[MAX_CMD_LEN+1];
  static byte cmdLen = 0;
  yaspCommand_t *cmds = mCmdTable;
  boolean found = false;
    
  // add any characters to ignore here...
  // TODO: perhaps make this part of the config?
  if (data == '\r')
    return;
  
  // TODO:  make the sentinal configurable  
  if (data == '\n')
  {
    // this is a command, process it
    MatchState ms;
  
    ms.Target(&cmd[0], cmdLen);
      
    // is this a help command?
    if(ms.Match(helpCmd) > 0)
    {
      found =  true;
        
      // print command table and we're done
      while(cmds->regexp != NULL)
      {
	printResponse(cmds->help, false);
        cmds++;
      }
      printResponse(NULL, true);
    }
    else
    {
      // look for match in table
      while(cmds->regexp != NULL)
      {
        char result = ms.Match(cmds->regexp);
        
        if (result > 0)
        {
          found =  true;
            
          // process callback
          if (cmds->callback != NULL)
          {
            yaspCBReturn_t status = cmds->callback(&ms);
	    if (status == YASP_STATUS_OK)
	    {
              printResponse("OK", true);
	    }
	    else if (status == YASP_STATUS_ERROR)
	    {
              printResponse(errMsg, true);
	    }
	    else
	    {
              printResponse(NULL, true);
	    }
          }
        }
        
        // next command
        cmds++;
      }
    }
      
    if (!found)
    {
      if (cmdLen > 0)
        printResponse(errMsg, true);
      else
        printResponse(NULL, true);
    }
      
    // start over
    cmdLen = 0;
  }
  else
  {
    // is there room?
    if (cmdLen >= sizeof(cmd))
    {
      // too long
      cmdLen = 0;
      printResponse(errMsg, true);
    }
      
    // put it in the buffer
    cmd[cmdLen++] = data;
  }
}


// Prints an optional message and an optional prompt to the specified
// linked Print object.  If the str variable is null, it will just
// print a prompt that flag is set.
void yasp::printResponse(const char *str, boolean addPrompt)
{
  if (mPrint && mPrintCB)
  {
    if (str != NULL)
    {
      (mPrint->*mPrintCB)(str);
      (mPrint->*mPrintCB)(EOL);
    }

    if (yaspUserPrompt != NULL && addPrompt)
      (mPrint->*mPrintCB)(userPrompt);
  }
}

