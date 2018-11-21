#ifndef APPLICATION_H
#define APPLICATION_H

#include <ctime>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>

using namespace std;

// MACRO
#define CLASS_WITH_NAME                                                        \
  string getClassName() { return typeid(*this).name(); }

/*
 * For Server, Client and other Childs
 *
 * Useful architecture feature class
 * Support log streaming -> to file and/or-not standart error output
 */
class Application {

  /*
   * Write log fields and methods
   */
  bool verbose;                    // log info, you want see so much?
  string logfileName;              // filename
  fstream logfile;                 // where to write
  string generateFileName();       // calc filename by class name or smth else
  string format(string message);   // prepare message to write to the log
  string addBrackets(string text); // smth -> [smth]
  void writeToLog(string formatedMessage); // end

protected:
  virtual string getClassName(); // only child of App. can get a Class Name

public:
  Application(bool verbose = false);
  ~Application();

  string getLogfileName(); // simple to know wehere is log file to seek

  void setVerbose(bool verbose); // verbose log process to standart error stream

  void log(string message); // send message to logfile and/or-not to cerr<<

  virtual void run() = 0; // pure virtual fun. must be overriden at inheritance
};

// util function gets time right here, right now
string getLocalDateTime();

#endif // APPLICATION
