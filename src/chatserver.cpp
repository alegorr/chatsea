#include "chatserver.h"

ChatServer::ChatServer(int connectionPort, int messagingPort) {
  lastClientIdx = ID::ANY;
  setConnectionPort(connectionPort);
  setMessagingPort(messagingPort);
}

void ChatServer::setConnectionPort(int connectionPort) {
  this->connectionPort = connectionPort;
}

void ChatServer::setMessagingPort(int messagingPort) {
  this->messagingPort = messagingPort;
}

// C++, nothing to add
int ChatServer::generateUniqueClientIdentifier() {
  lastClientIdx++;
  if (lastClientIdx == ID::SERVER || lastClientIdx == ID::ANY) {
    lastClientIdx++;
  }
  return lastClientIdx;
}

// one thread process (parallelize in the future)
void ChatServer::run() {

  log("Initialize server. Prepare context and sockets to work.");
  context_t context(1);
  socket_t inputSocket(context, ZMQ_REP);
  inputSocket.bind(tcpAnyPortAddress(connectionPort));
  socket_t outputSocket(context, ZMQ_PUB);
  outputSocket.bind(tcpAnyPortAddress(messagingPort));

  log("Initialization done. Run main server cycle.");
  while (true) {
    ChatMessage message;
    log("Waiting message...");

    inputSocket.recv(&message);
    log("Message received.");

    message.process(); // store changes

    log("Check is that client ID request?");
    if (message.getSenderId() == ID::ANY) {

      log("Yes.");

      message.setReceiverId(generateUniqueClientIdentifier());
      message.setSenderId(ID::SERVER);
      message.prepare(); // store changes

      inputSocket.send(message); // send reply/or repeat
      log("Reply with the client ID content.");

    } else {
      log("Nope.");

      inputSocket.send(message.copy()); // send reply/or repeat
      log("Return message to sender.");

      outputSocket.send(message);
      log("Send message to the clients.");
    }
  }
  return;
}

// to bind sockets
string ChatServer::tcpAnyPortAddress(int portNumber) {
  return string("tcp://*:") + to_string(portNumber);
}
