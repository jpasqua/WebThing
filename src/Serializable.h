/*
 * Serializable
 *     Objects of this class can internalize themselves from JSON and
 *     externalize themselves to JSON
 *
 * NOTES:
 * o This class may be used as (1) a base type from which other classes are derived,
 *   or (2) as a multiple inheritance mixin for a class derived from a base class
 *
 */

#ifndef Serializable_h
#define Serializable_h

#include <ArduinoJson.h>

class Serializable {
public:

  // An external representation has been read into a JSON object which is
  // passed in. Internalize that JSON data into this object 
  virtual void internalize(const JsonObjectConst& obj) = 0;

  // We are being asked to write a JSON representation of this argument
  // out to the stream passed as a parameter.
  virtual void externalize(Stream& writeStream) const = 0;
};


#endif	// Serializable_h