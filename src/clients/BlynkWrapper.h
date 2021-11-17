#ifndef BlynkClient_h
#define BlynkClient_h

class BlynkWrapper {
public:
  void virtualWrite(int pin, String value);
  void virtualWrite(int pin, float value);
  void virtualWrite(int pin, bool value);
  void virtualWrite(int pin, uint8_t value);
  void virtualWrite(int pin, uint16_t value);
};

extern BlynkWrapper BlynkClient;

#endif	// BlynkClient_h