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

/*
 * Main loop
 */
void ChatClient::run() {

  log("Initialize Client. Prepare context and socketRequest->");

  context_t context(1);
  socketRequest = make_unique<socket_t>(context, ZMQ_REQ);
  socketRequest->connect(string("tcp://localhost:") +
                         to_string(connectionPort));
  socketRequest->setsockopt(ZMQ_RCVTIMEO, &timeout, sizeof(timeout));

  log("Initialization done. Try to connect to the Server...");

  while (true) {

    log("Request client ID from Server.");
    string localDateTime = getLocalDateTime();

    ChatMessage helloMessage(ID::SERVER, ID::ANY, clientAlias, localDateTime);
    helloMessage.prepare();
    socketRequest->send(helloMessage);

    log("Wait Server reply...");
    ChatMessage serverReply;
    socketRequest->recv(&serverReply);
    log("Receive message.");

    serverReply.process();
    log("Process the message..");

    log("Check if the Server returns our client ID.");
    if (serverReply.getSenderAlias() == clientAlias &&
        serverReply.getContent() == localDateTime) {
      log(string("Ok. The Client ID is ") +
          to_string(serverReply.getReceiverId()));
      clientId = serverReply.getReceiverId();
      break;
    }
  }

  log("Start messaging.");

  thread sendThread(&ChatClient::send, this);
  thread receiveThread(&ChatClient::receive, this);

  sendThread.join();
  receiveThread.join();

  log("Stop messaging. Free resources.");

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
      socketRequest->send(outputMessage);

      log("Receive mirror message from the Server");
      ChatMessage inputMessage;
      socketRequest->recv(&inputMessage);

      log("Process the message.");
      inputMessage.process();

    } else {
      log("Message is empty. Nothing to do.");
    }
  }
}

// Receive Messages thread function
void ChatClient::receive() {

  context_t context(1);
  socketSubscribe = make_unique<socket_t>(context, ZMQ_SUB);
  socketSubscribe->connect(tcpLocalPortAddress(messagingPort));
  int linger = 0; // to proper shutdown ZeroMQ
  socketSubscribe->setsockopt(ZMQ_LINGER, &linger, sizeof(linger));
  socketSubscribe->setsockopt(ZMQ_RCVTIMEO, &timeout, sizeof(timeout));
  socketSubscribe->setsockopt(ZMQ_SUBSCRIBE, 0, 0); // no filter needs

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
    }
  }

  // free Resources
  socketSubscribe->close();
  context.close();
}

// control standart output for chatting
void ChatClient::invite() { cout << clear() << flush << clientAlias + "> "; }
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
