#ifndef CHATSERVER_H
#define CHATSERVER_H

#include "application.h"
#include "chatmessage.h"

#include <unistd.h>
#include <zmq.hpp>
using namespace zmq;

#include <iostream>
#include <string>
using namespace std;

/*
 * Chat Server messaging classes
 *
 * Allows to connect Over 9000 Clients simultaneosly!!!
 *
 * Architecture is simple:
 * There are 2 sockets binded to 2 selected ports:
 * (1) for get messages from the Client and to allow connection;
 * (2) for send messages to Clients (simple retranslate).
 */
class ChatServer : public Application {

  CLASS_WITH_NAME;

  /*
   * Network Interfaces
   */
  bool init(context_t &context); // set up context and sockets to work
  bool bindSocket(socket_t &socket, int portNumber); // bind socket to port

  unique_ptr<socket_t> socketReply; // (1)st
  int connectionPort; // for get the Client messages and send server info to it

  unique_ptr<socket_t> socketPublish; // (2)nd
  int messagingPort;                  // to send messages to Clients

  string tcpAnyPortAddress(int portNumber); // to bind sockets
  const int timeout = TIMEOUT; // ms reaction time for interact with Client

  int lastClientIdx; // just to remember which ID to send next Client
  int generateUniqueClientIdentifier(); // simple increment mechanics

  void setConnectionPort(int connectionPort); // TODO rebind sockets
  void setMessagingPort(int messagingPort);   // make public ITF

public:
  ChatServer(int connectionPort = CONNECTION_PORT,
             int messagingPort = MESSAGING_PORT);

  void run() override; // rise the Server
};

#endif // CHATSERVER_H
