#include "chatclient.h"

ChatClient::ChatClient(int connectionPort, int messagingPort,
                       string clientAlias) {
  interruptCode = Interrupt::NOPE; // by default
  setConnectionPort(connectionPort);
  setMessagingPort(messagingPort);
  setClientAlias(clientAlias);
}

void ChatClient::setConnectionPort(int connectionPort) {
  this->connectionPort = connectionPort;
}

void ChatClient::setMessagingPort(int messagingPort) {
  this->messagingPort = messagingPort;
}

void ChatClient::setClientAlias(string clientAlias) {
  this->clientAlias = clientAlias;
}

// setup context, bind sockets
bool ChatClient::initSocket(context_t &context, int type) {

  bool initialized = true;

  switch (type) {

  case ZMQ_REQ:
    log("Initialize Client connection socket. " + to_string(connectionPort));
    socketRequest = make_unique<socket_t>(context, ZMQ_REQ);
    if (!connectSocket(*socketRequest.get(), connectionPort))
      initialized = false;
    break;

  case ZMQ_SUB:
    log("Initialize Client messaging socket. " + to_string(messagingPort));
    socketSubscribe = make_unique<socket_t>(context, ZMQ_SUB);
    if (!connectSocket(*socketRequest.get(), messagingPort)) {
      initialized = false;
    } else {
      socketSubscribe->setsockopt(ZMQ_SUBSCRIBE, 0, 0); // no filter needs
    }
    break;

  default:
    initialized = false;
  }

  return initialized;
}

bool ChatClient::connectSocket(socket_t &socket, int portNumber) {

  bool initialized = true;

  try {
    socket.connect(tcpLocalPortAddress(portNumber)); // here rewrite fun. IPv4

  } catch (zmq::error_t &e) {
    log(string("Error. Can't connect socket to the port ") +
        to_string(portNumber) + ". " + e.what());

    initialized = false;
  }

  // set socket reaction timeout and proof fine closing
  if (initialized) {
    int linger = 0;
    socket.setsockopt(ZMQ_LINGER, &linger, sizeof(linger));     // closing
    socket.setsockopt(ZMQ_RCVTIMEO, &timeout, sizeof(timeout)); // reaction
  }

  return initialized;
}

/*
 * Main loop
 */
void ChatClient::run() {

  /*
   * 1st step: Initialization
   */
  context_t context(1);
  if (!initSocket(context, ZMQ_REQ)) {

    log("Initialization fails. Look up logs \"" + getLogfileName() + "\"",
        true);
    return;
  }

  /*
   * 2nd step: Registration
   */
  log("Initialization done. Try to connect to the Server...");

  while (true) {

    log("Request client ID from Server.");
    string localDateTime = getLocalDateTime();

    ChatMessage helloMessage(ID::SERVER, ID::ANY, clientAlias, localDateTime);
    helloMessage.prepare();
    log(helloMessage.dump());
    socketRequest->send(helloMessage);

    log("Wait Server reply...");
    ChatMessage serverReply;
    socketRequest->recv(&serverReply);
    log("Receive message.");

    serverReply.process();
    log("Process the message..");

    log(serverReply.dump());

    log("Check if the Server returns our client ID.");
    if (serverReply.getSenderAlias() == clientAlias &&
        serverReply.getContent() == localDateTime) {
      log(string("Ok. The Client ID is ") +
          to_string(serverReply.getReceiverId()));
      clientId = serverReply.getReceiverId();
      break;
    }
  }

  /*
   * 3nd step: Messaging
   */
  log("Start messaging.");

  thread sendThread(&ChatClient::send, this);
  thread receiveThread(&ChatClient::receive, this);

  sendThread.join();
  receiveThread.join();

  log("Stop messaging. Free resources.");

  /*
   * 4th step: Closing
   */
  socketRequest->close();
  context.close();

  return;
}

// Send Messages thread function
void ChatClient::send() {

  while (notInterrupted()) {

    log("Wait the User input...");
    string content;
    invite();
    getline(cin, content, '\n');

    log("The User push Enter. Check if message is not empty...");
    if (!content.empty()) {

      if (content == "q" || content == "quit") {
        setInterrupt(Interrupt::USER_QUIT);
        break;
      }

      log("The message is not empty. Send it to the Server.");
      ChatMessage outputMessage(ID::ANY, clientId, clientAlias, content);
      outputMessage.prepare();
      log(outputMessage.dump());
      socketRequest->send(outputMessage);

      log("Receive mirror message from the Server");
      ChatMessage inputMessage;
      socketRequest->recv(&inputMessage);

      log("Process the message.");
      inputMessage.process();

      log(inputMessage.dump());
    } else {
      log("Message is empty. Nothing to do.");
    }
  }
}

// Receive Messages thread function
void ChatClient::receive() {

  context_t context(1);
  if (!initSocket(context, ZMQ_SUB)) {

    log("Initialization fails. Look up logs \"" + getLogfileName() + "\"",
        true);
    return;
  }

  while (notInterrupted()) {

    log("Receive updates from the Server");
    ChatMessage messageFromClient;
    int receiveStatus = 0;

    try {
      log("Wait message from another client...");
      receiveStatus = socketSubscribe->recv(&messageFromClient);

    } catch (zmq::error_t &error) {
      log(string("Force terminate SUB socket cause ") + error.what());
      break;
    }

    // message received
    if (receiveStatus) {

      log("Process the message.");
      messageFromClient.process();

      log(messageFromClient.dump());

      log("Check if message not from this Client.");
      if (messageFromClient.getSenderId() != clientId) {

        log("Show the message to the User.");
        cout << clear() << flush
             << messageFromClient.getSenderAlias() + "> " +
                    messageFromClient.getContent()
             << endl
             << flush;
        invite(); // sometimes dissapearing when logging enabled

      } else {

        log("It's our sended message. Nothing to do.");
        // would make a message arrived indicator
      }
    } else {
      log("Something went wrong...");
    }
  }

  socketSubscribe->close();
  context.close();
}

// control standart output for chatting
void ChatClient::invite() {
  cout << clear() << flush << clientAlias + "> " << flush;
}
string ChatClient::clear() { return "\r\e[0K"; }

// make program interruption
void ChatClient::setInterrupt(int code) {
  interruptMutex.lock();
  interruptCode = code;
  interruptMutex.unlock();
}

// check if all right
bool ChatClient::notInterrupted() { return interruptCode == Interrupt::NOPE; }

// simple util
string ChatClient::tcpLocalPortAddress(int portNumber) {
  return string("tcp://localhost:") + to_string(portNumber);
}
