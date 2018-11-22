#include "application.h"

Application::Application(bool verbose) { setVerbose(verbose); }

Application::~Application() {

  // close file if it's necessary
  if (logfile.is_open()) {
    logfile.close();
  }
}

// example function needs to override in derived classes
string Application::getClassName() { return typeid(*this).name(); }

string Application::generateFileName() { return getClassName() + ".log"; }

// format message
string Application::format(string message) {
  return addBrackets(getLocalDateTime()) + " " + message + "\n";
}

//
string Application::addBrackets(string text) { return "[" + text + "]"; }

// Breaks the Scilence...
void Application::setVerbose(bool verbose) { this->verbose = verbose; }

// main log cycle
void Application::log(string message, bool force) {

  // if (force && !verbose)
  //   verbose = force;
  // format message or beautify, if you want
  string formatedMessage = format(message);

  if (!logfile.is_open()) {

    // open, if not
    logfileName = generateFileName();
    logfile.open(logfileName, ofstream::out | ofstream::app);

    // write first message
    string firstMessage =
        format("Start logging to file \"" + logfileName + "\"");
    writeToLog(firstMessage);
  }

  // write received message
  writeToLog(formatedMessage);

  // if (force && verbose)
  //   verbose = !force;
}

// send it out
void Application::writeToLog(string formatedMessage) {
  logfile << formatedMessage;

  if (verbose) {
    cerr << formatedMessage << flush;
  }
}

// to know where is log records
string Application::getLogfileName() { return logfileName; }

// util function gets time right here, right now
string getLocalDateTime() {
  ostringstream timeStream;
  auto t = time(nullptr);
  auto localTime = *localtime(&t);
  timeStream << put_time(&localTime, "%d-%m-%Y %H-%M-%S");
  return timeStream.str();
}
