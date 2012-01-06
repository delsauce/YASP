#include <Regexp.h>   // Need the LUA-like pattern matching library for our command processor
                      // This library is available here:
                      // http://www.gammon.com.au/forum/?id=11063
                      
#include <YASP.h>     // Include the parser's header

// Declare callbacks for further processing a command after a regexp match
yaspCBReturn_t debug(MatchState *ms);
yaspCBReturn_t debugOn(MatchState *ms);
yaspCBReturn_t debugOff(MatchState *ms);
yaspCBReturn_t setKp(MatchState *ms);

// Table that links the command to a callback via regexp.  If a part of the regexp
// is encapsulated in parentheses, then that portion (i.e. substring) is saved
// as a capture for easy access.
// Each match is preceeded by '^' and followed with '$' to make sure that a match is not made
// as part of a substring.
//
// Some LUA string matching resources:
// http://stackoverflow.com/questions/2693334/lua-pattern-matching-vs-regular-expressions
// http://www.lua.org/pil/20.1.html
// http://www.gammon.com.au/scripts/doc.php?lua=string.find

yaspCommand_t cmds[] = 
{
  { "^debug$", "debug : Return current debug state", debug },
  { "^debug on$", "debug on : Set debug on", debugOn },
  { "^debug off$", "debug off : Set debug off", debugOff },
  { "^Kp (%d+)$", "Kp <integer> : Set proportional gain", setKp },  // match a command word and an integer arg
  //{ "^Kp ([-+]?[%d]*%.?[%d]+)$", "Kp <float value> : Set proportional gain", setKp },  // match a command word and a float number
  { 0, 0, 0 }
};

// Just a basic variable for demonstration
boolean debugState = false;

// Declare the command parser and point it to the list of commands, 
// desired interface, and print function.  Any object that use the Print class
// can be used in place of the serial port (Other serial ports, including 
// SoftwareSerial should all work)
yasp cmdParser(&cmds[0], &Serial, &Print::write);

void setup()
{
  // Initialize the serial port.
  Serial.begin(115200);
  
  // If you want to override the default settings for the responses and 
  // EOL of the parser, here is where you do it.
  // e.g.:
  // cmdParser.userPrompt = "$ ";
  // cmdParser.helpCmd = "^help$";
  // cmdParser.errMsg = "That's not a command!  Type 'help' you fool!
  // cmdParser.EOL = "\r";
}

void loop()
{
  if (Serial.available()){
    // Pass received data to the command parser.  If a complete command is received
    // the callback will be activated from this call.
    cmdParser.addData(Serial.read());
  }
}

// Callback function
yaspCBReturn_t setKp(MatchState *ms)
{
  if (ms->level > 0)
  {
    char buf[20];
 
    Serial.print("Kp: ");
    ms->GetCapture(&buf[0], 0);
    int foo = atoi(&buf[0]);  
    Serial.println(foo);
    
    return YASP_STATUS_NONE;
  }
  
  return YASP_STATUS_ERROR;
};

// Callback function
yaspCBReturn_t debug(MatchState *ms)
{
  if (debugState)
    Serial.println("on");
  else
    Serial.println("off");
    
  return YASP_STATUS_NONE;
}

// Callback function
yaspCBReturn_t debugOn(MatchState *ms)
{
  debugState = true;
  return YASP_STATUS_OK;
}

// Callback function  
yaspCBReturn_t debugOff(MatchState *ms)
{
  debugState = false;
  return YASP_STATUS_OK;
}

