#include "application.h"
#include "chatclient.h"
#include "chatserver.h"
#include "optparse.h"

#include <iostream>
#include <string>

using namespace std;

/*
 * This is simple C++ chatting program ~ChatSea~
 * Uses ZeroMQ fast messaging over the sockets
 * Allows to connect over 9000 Clients and more!
 * Please, Delight!!!
 */
int main(int argc, char *argv[]) {

  /*
   * Our App object pointer
   */
  unique_ptr<Application> app;

  /*
   * Program Options
   */
  bool server_mode;  // otherwise starts Client
  bool daemonize;    // only for Server
  int port;          // communication port for Client and Server
  string user_nick;  // connected to Client
  string ip_address; // Server IPv4 adress
  bool verbose;      // log

  /*
   * Run Arguments Parser
   */
  optparse::OptionParser parser =
      optparse::OptionParser()
          .version("ChatSea v.1.0.1")
          .description("Simple C++ chating app. Be smart. Connect to the Sea "
                       "of Chatting.");

  parser.set_defaults("server_mode", "0");
  parser.set_defaults("daemonize", "0");

  parser.add_option("-s", "--server")
      .action("store_true")
      .dest("server_mode")
      .help("Run chat Server (by default run Client)");
  parser.add_option("-p", "--port")
      .metavar("PORT")
      .type("int")
      .set_default("8888")
      .help("Set connection port number Client/Server communication");
  parser.add_option("-a", "--adress")
      .metavar("ADDRESS")
      .set_default("localhost")
      .help("Set Server IPv4 adress");
  parser.add_option("-n", "--nick")
      .metavar("NICK")
      .set_default("anon")
      .help("Run chat client");
  parser.add_option("-d", "--daemonize")
      .action("store_true")
      .dest("daemonize")
      .help("Run server in daemon mode");
  parser.add_option("-v", "--verbose")
      .action("store_true")
      .dest("verbose")
      .help("Verbose log output to standart output");

  const optparse::Values options = parser.parse_args(argc, argv);
  const vector<string> args = parser.args();

  /*
   * Get Options
   */
  server_mode = options.get("server_mode");
  daemonize = options.get("daemonize");
  verbose = options.get("verbose");
  ip_address = options["adress"];
  port = static_cast<int>(options.get("port"));
  user_nick = options["nick"];

  /*
   * Check Options
   */
  if (port < MIN_PORT_NUMBER && port > MAX_PORT_NUMBER) {
    cerr << "Port number is out of range [" << MIN_PORT_NUMBER << ", "
         << MAX_PORT_NUMBER << "].\n";
    return -1;
  }
  if (user_nick == "") {
    cerr << "Nick name can not be void." << endl;
    return -1;
  }
  if (verbose && !server_mode) {
    cerr
        << "Warning! Client verbose is quite bit experimental! Better turn off!"
        << endl;
  }
  // TODO regex IP address check

  /*
   * Init App
   */
  if (server_mode) {

    cout << "Run server mode" << endl;

    if (daemonize) {
      daemon(1, 1);
      cout << "Awake our daemon... PID " << getpid() << endl;
      cout << "To stop server daemon put:" << endl;
      cout << "kill " << getpid() << endl;
    }

    // create and cast App as Server
    app = move(make_unique<ChatServer>(port, port + 1)); // TODO add PortSeeker

  } else {

    cout << "Run client" << endl;

    // create and cast App as Client
    app = move(make_unique<ChatClient>(port, port + 1, user_nick));
  }
  app->setVerbose(verbose); // send warnings to the cerr<< stream

  /*
   * Start App
   */
  app->run();

  return 0;
}
