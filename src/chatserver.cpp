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

// get port number after socket binding
int ChatServer::getSocketPortAddress(socket_t &socket) {
  char address[1024];
  size_t size = sizeof(address);
  socket.getsockopt(ZMQ_LAST_ENDPOINT, &address, &size);
  string addr = string(address);
  int pos = addr.rfind(':');
  return stoi(addr.substr(pos + 1, addr.size() - pos));
}

// setup context, bind sockets
bool ChatServer::init(context_t &context) {

  bool initialized = true;

  log("Init connection socket.", true);
  socketReply = make_unique<socket_t>(context, ZMQ_REP);
  if (!bindSocket(*socketReply.get(), connectionPort)) {
    initialized = false;
  } else {
    setConnectionPort(getSocketPortAddress(*socketReply.get()));
    log(string("Connection socket is bound at port ") +
            to_string(connectionPort),
        true);
  }

  log("Init messaging socket.", true);
  socketPublish = make_unique<socket_t>(context, ZMQ_PUB);
  if (!bindSocket(*socketPublish.get(), messagingPort)) {
    initialized = false;
  } else {
    setMessagingPort(getSocketPortAddress(*socketPublish.get()));
    log(string("Messaging socket is bound at port ") + to_string(messagingPort),
        true);
  }

  return initialized;
}

int ChatServer::bindSocket(socket_t &socket, int portNumber) {

  bool initialized = portNumber;

  try {
    socket.bind(tcpAnyPortAddress(portNumber));

  } catch (zmq::error_t &e) {
    log(string("Error. Can't bind socket to port ") + to_string(portNumber) +
        ". " + e.what());

    try {
      log("Try bind to first free port. Search...");
      socket.bind(string("tcp://*:*"));

    } catch (zmq::error_t &e) {
      log(string("Error. Couldn't bind socket. ") + e.what());
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
    log("Initialization fails. Watch logs \"" + getLogfileName() + "\"", true);
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

    log(message.dump());

    log("Check is that Client ID Request?");
    if (message.getSenderId() == ID::ANY) {

      log("Yes.");

      log("Prepare unique ID for Client.");
      message.setReceiverId(generateUniqueClientIdentifier());
      message.setSenderId(ID::SERVER);
      message.setContent(message.getContent() + to_string(messagingPort));
      message.prepare(); // store changes

      socketReply->send(message); // send reply/or repeat
      log("Send ID to the Client.");

      log(message.dump());

    } else {

      log("Nope.");

      socketReply->send(message.copy()); // send reply/or repeat
      log("Return Message to sender.");

      socketPublish->send(message);
      log("Send Message to all the Clients.");

      log(message.dump());
    }
  }

  return;
}

// to bind sockets
string ChatServer::tcpAnyPortAddress(int portNumber) {
  return string("tcp://*:") + to_string(portNumber);
}
