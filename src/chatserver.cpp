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

// setup context, bind sockets
bool ChatServer::init(context_t &context) {

  bool initialized = true;

  socketReply = make_unique<socket_t>(context, ZMQ_REP);
  try {
    socketReply->bind(tcpAnyPortAddress(connectionPort));

  } catch (zmq::error_t &e) {
    log(string("Error. Can't bind connection socket to port ") +
        to_string(connectionPort) + ". " + e.what());

    try {
      log("Try bind to first free port. Search...");
      socketReply->bind(string("tcp://*:*"));

    } catch (zmq::error_t &e) {
      log(string("Error. Couldn't bind connection socket. ") + e.what());
      initialized = false;
    }
  }

  socketPublish = make_unique<socket_t>(context, ZMQ_PUB);
  try {
    socketPublish->bind(tcpAnyPortAddress(messagingPort));

  } catch (zmq::error_t &e) {
    log(string("Error. Can't bind messaging socket to port ") +
        to_string(messagingPort) + ". " + e.what());

    try {
      log("Try bind to first free port. Search...");
      socketPublish->bind(string("tcp://*:*"));

    } catch (zmq::error_t &e) {
      log(string("Error. Couldn't bind messaging socket. ") + e.what());
      initialized = false;
    }
  }

  return initialized;
}

// one thread process (parallelize in the future)
void ChatServer::run() {

  log("Initialize Server. Prepare context and sockets to work.");
  context_t context(1);
  if (!init(context)) {

    log("Initialization fails.");
    return;
  }

  log("Initialization done. Run main Server cycle.");
  while (true) {
    ChatMessage message; // declare message
    log("Waiting Message...");

    socketReply->recv(&message);
    log("Message received.");

    message.process(); // store changes
    log("Process message to read Request.");

    log("Check is that Client ID Request?");
    if (message.getSenderId() == ID::ANY) {

      log("Yes.");

      log("Prepare unique ID for Client.");
      message.setReceiverId(generateUniqueClientIdentifier());
      message.setSenderId(ID::SERVER);
      message.prepare(); // store changes

      socketReply->send(message); // send reply/or repeat
      log("Send ID to the Client.");

    } else {

      log("Nope.");

      socketReply->send(message.copy()); // send reply/or repeat
      log("Return Message to sender.");

      socketPublish->send(message);
      log("Send Message to all the Clients.");
    }
  }

  return;
}

// to bind sockets
string ChatServer::tcpAnyPortAddress(int portNumber) {
  return string("tcp://*:") + to_string(portNumber);
}
