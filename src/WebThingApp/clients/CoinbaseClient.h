/*
 * CoinbaseClient:
 *     Very simple client to read crypto buy prices using Coinbase api
 *
 */

#ifndef CoinbaseClient_h
#define CoinbaseClient_h

#include <Arduino.h>

class CoinbaseClient {
public:
  static bool getPrice(String coinID, String& currency, String& price);
};

#endif  // CoinbaseClient_h