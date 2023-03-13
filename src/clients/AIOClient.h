/*
 * AIOClient
 *    Amenities for sending and receiving data from AdafruitIO
 *
 */

#ifndef AIOClient_h
#define AIOClient_h

//--------------- Begin:  Includes ---------------------------------------------
//                                  Core Libraries
//                                  Third Party Libraries
#include <JSONService.h>
//                                  Local Includes
//--------------- End:    Includes ---------------------------------------------

class AIOClient {
public:
  bool init(const char* username, const char* key);
  void setDefaultGroup(const char* dfltGroup);

  bool get(const char* feedName, String& into);

  bool set(const char* feedName, const char *value);
  bool set(const char* feedName, char *value);
  bool set(const char* feedName, bool value);
  bool set(const char* feedName, String value);
  bool set(const char* feedName, int value);
  bool set(const char* feedName, unsigned int value);
  bool set(const char* feedName, long value);
  bool set(const char* feedName, unsigned long value);
  bool set(const char* feedName, float value, int precision);
  bool set(const char* feedName, double value, int precision);

private:
  String endpointRoot;
  const char* dfltGroup = NULL;
  JSONService* service;
  bool initialized;

  String makeEndpoint(const char* feedName);

};

#endif // AIOClient_h


