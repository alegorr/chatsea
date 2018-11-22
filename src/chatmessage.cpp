#include "chatmessage.h"

ChatMessage::ChatMessage(int receiverId, int senderId, string senderAlias,
                         string content)
    : message_t(MESSAGE_LEN) { // set fixed message length!!!
  setReceiverId(receiverId);
  setSenderId(senderId);
  setSenderAlias(senderAlias);
  setContent(content);
}

// for message doubling
ChatMessage ChatMessage::copy() {
  ChatMessage messageCopy;
  (static_cast<message_t *>(&messageCopy))->copy(this);
  return messageCopy;
}

// happy little util
string ChatMessage::trimTail(string withSpaces) {
  int i = withSpaces.size() - 1;
  for (; i >= 0; i--) {
    if (withSpaces[i] != ' ') {
      break;
    }
  }
  return withSpaces.substr(0, i + 1);
}

// the same stuff
string ChatMessage::format(string field, int length) {
  string whole = field;
  whole.resize(length, ' ');
  return whole;
}

// Setters
void ChatMessage::setReceiverId(int receiverId) {
  this->receiverId = receiverId;
}
void ChatMessage::setSenderId(int senderId) { this->senderId = senderId; }
void ChatMessage::setSenderAlias(string senderAlias) {
  this->senderAlias = senderAlias;
}
void ChatMessage::setContent(string content) { this->content = content; }

// prepare to Send
void ChatMessage::prepare() {

  string message = format(to_string(receiverId), ID_LEN) +
                   format(to_string(senderId), ID_LEN) +
                   format(senderAlias, ALLIAS_LEN) +
                   format(content, CONTENT_LEN);

  memcpy(data(), const_cast<char *>(message.c_str()), message.size());
}

// process after Receive
void ChatMessage::process() {

  string message(static_cast<char *>(data()));

  setReceiverId(stoi(message.substr(0, ID_LEN)));
  int idx = ID_LEN;
  setSenderId(stoi(message.substr(idx, ID_LEN)));
  idx += ID_LEN;
  setSenderAlias(trimTail(message.substr(idx, ALLIAS_LEN)));
  idx += ALLIAS_LEN;
  setContent(trimTail(message.substr(idx, CONTENT_LEN)));
}

// Getters
int ChatMessage::getReceiverId() { return receiverId; }
int ChatMessage::getSenderId() { return senderId; }
string ChatMessage::getSenderAlias() { return senderAlias; }
string ChatMessage::getContent() { return content; }

string ChatMessage::dump(){
  return to_string(receiverId) + "|" +
  to_string(senderId) + "|" +
  senderAlias  + "|" +
  content;
}
