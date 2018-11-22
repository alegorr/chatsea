#ifndef MACRODEFINES_H
#define MACRODEFINES_H

/*
 * Port range macro
 */
#define MIN_PORT_NUMBER 0
#define MAX_PORT_NUMBER 65535

/*
 * Client/Server communication defaults
 */
#define CONNECTION_PORT 8888
#define MESSAGING_PORT 8889
#define TIMEOUT 1000 // ms

/*
 * Something more special
 */
#define CLASS_WITH_NAME                                                        \
  string getClassName() {                                                      \
    string name = typeid(*this).name();                                        \
    return name.substr(2, name.size() - 2);                                    \
  }

#endif // MACRODEFINES_H
