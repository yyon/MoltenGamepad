#ifndef MESSAGES_H
#define MESSAGES_H
#include <sstream>
#include <vector>
#include <string>
#include <mutex>


//For connections to another process, the client gives us a response id
//with every request. Output related to that request is sent back with the same response id.

//For output generated without a controlling client request (i.e. it has no provided response
// id), a response id of 0 is used.

class message_protocol;

class response_stream;

class message_stream {
  std::vector<message_protocol*> listeners;
  std::mutex lock;
public:
  std::string name;
  message_stream(std::string name) : name(name) {};
  virtual void add_listener(message_protocol* listener);
  virtual void remove_listener(message_protocol* listener);
  virtual void flush();

  virtual void take_message(int resp_id, std::string text);
  virtual void print(int resp_id, std::string text);
  virtual void err(int resp_id, std::string text, std::string path, int line_number);
  virtual void err(int resp_id, std::string text);

  virtual void end_response(int resp_id, int ret_val);
};


class response_stream {
  int response_id;
  message_stream* stream;
public:
  response_stream(int id, message_stream* stream) : response_id(id), stream(stream) {};
  void take_message(std::string text);
  void print(std::string text);
  void err(std::string text, std::string path, int line_number);
  void err(std::string text);

  void end_response(int ret_val);
};


#define DEBUG_NONE 0
#define DEBUG_INFO 5
#define DEBUG_VERBOSE 10
extern int DEBUG_LEVELS[];
extern int* DEBUG_LEVEL;
//pass the number of args, and then that number of strings aftwerwards to print.
//No newline is necessary.
void debug_print(int level, int num_args...);

#endif
