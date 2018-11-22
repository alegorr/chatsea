#ifndef CHATMESSAGE_H
#define CHATMESSAGE_H

#include <zmq.hpp>
using namespace zmq;

#include <sstream>
#include <string>
using namespace std;

/*
 * Message has fixed size!
 */
#define ID_LEN 4
#define ALLIAS_LEN 32
#define CONTENT_LEN 512
#define MESSAGE_LEN (ID_LEN + ID_LEN + ALLIAS_LEN + CONTENT_LEN)

/*
 * Reserved IDs
 */
enum ID { SERVER = 0, ANY = 1 }; // all another are free for Clients

/*
 * Simple chat messages
 *
 * Gets more functionality for chat messaging
 */
class ChatMessage : public message_t {

  /*
   * Follow message struture
   */
  int receiverId;     // message destination subject/addressee Id
  int senderId;       // Id of the sender
  string senderAlias; // alias of the sender
  string content;     // message text

  string format(string field,
                int length); // little utility func. which add spaces
  string
  trimTail(string withSpaces); // the same stuff but to remove tail spaces

public:
  ChatMessage(int receiverId = ID::SERVER, int senderID = ID::ANY,
              string senderAlias = "", string content = "");

  ChatMessage copy(); // for message duplication

  /*
   * Prepare to send fun-s
   *
   * Before sending message don't forget to set up all Message fields by Setters
   * Then call prepare()
   */
  void setReceiverId(int receiverId);
  void setSenderId(int senderID);
  void setSenderAlias(string senderAlias);
  void setContent(string content);

  void prepare(); // do the same as process but in inverse direction

  /*
   * Process after receive fun-s
   *
   * After receive message call process()
   * Then may read message by Getters
   */
  void process(); // puts data from network packet to ChatMessage object fields

  int getReceiverId();
  int getSenderId();
  string getSenderAlias();
  string getContent();

  string dump(); // for debug only
};

#endif // CHATMESSAGE_H
