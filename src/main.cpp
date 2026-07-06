#include <Arduino.h>

// Change this string to the message you want to blink.
const char MESSAGE[] = "PATCH";

constexpr unsigned long DOT_MS = 150;
constexpr uint8_t DASH_UNITS = 3;
constexpr uint8_t SYMBOL_GAP_UNITS = 1;
constexpr uint8_t LETTER_GAP_UNITS = 3;
constexpr uint8_t WORD_GAP_UNITS = 7;
constexpr uint8_t MESSAGE_GAP_UNITS = 14;

constexpr uint8_t LED_RED = 0;
constexpr uint8_t LED_GREEN = 32;
constexpr uint8_t LED_BLUE = 0;

void delayUnits(uint8_t units) {
  delay(DOT_MS * units);
}

void setLed(bool on) {
#if defined(RGB_BUILTIN)
  rgbLedWrite(RGB_BUILTIN, on ? LED_RED : 0, on ? LED_GREEN : 0, on ? LED_BLUE : 0);
#else
  digitalWrite(LED_BUILTIN, on ? HIGH : LOW);
#endif
}

char upperAscii(char value) {
  if (value >= 'a' && value <= 'z') {
    return value - ('a' - 'A');
  }

  return value;
}

const char *morseFor(char value) {
  switch (upperAscii(value)) {
    case 'A': return ".-";
    case 'B': return "-...";
    case 'C': return "-.-.";
    case 'D': return "-..";
    case 'E': return ".";
    case 'F': return "..-.";
    case 'G': return "--.";
    case 'H': return "....";
    case 'I': return "..";
    case 'J': return ".---";
    case 'K': return "-.-";
    case 'L': return ".-..";
    case 'M': return "--";
    case 'N': return "-.";
    case 'O': return "---";
    case 'P': return ".--.";
    case 'Q': return "--.-";
    case 'R': return ".-.";
    case 'S': return "...";
    case 'T': return "-";
    case 'U': return "..-";
    case 'V': return "...-";
    case 'W': return ".--";
    case 'X': return "-..-";
    case 'Y': return "-.--";
    case 'Z': return "--..";
    case '0': return "-----";
    case '1': return ".----";
    case '2': return "..---";
    case '3': return "...--";
    case '4': return "....-";
    case '5': return ".....";
    case '6': return "-....";
    case '7': return "--...";
    case '8': return "---..";
    case '9': return "----.";
    case '.': return ".-.-.-";
    case ',': return "--..--";
    case '?': return "..--..";
    case '!': return "-.-.--";
    case '/': return "-..-.";
    case '-': return "-....-";
    case '(': return "-.--.";
    case ')': return "-.--.-";
    default: return nullptr;
  }
}

void sendSymbol(char symbol) {
  setLed(true);
  delayUnits(symbol == '-' ? DASH_UNITS : 1);
  setLed(false);
}

void sendCharacter(const char *pattern) {
  for (const char *symbol = pattern; *symbol != '\0'; ++symbol) {
    sendSymbol(*symbol);

    if (*(symbol + 1) != '\0') {
      delayUnits(SYMBOL_GAP_UNITS);
    }
  }
}

bool isWordSeparator(char value) {
  return value == ' ' || value == '\t' || value == '\n' || value == '\r';
}

void sendMessage(const char *message) {
  uint8_t pendingGapUnits = 0;

  for (const char *cursor = message; *cursor != '\0'; ++cursor) {
    if (isWordSeparator(*cursor)) {
      if (pendingGapUnits > 0) {
        pendingGapUnits = WORD_GAP_UNITS;
      }
      continue;
    }

    const char *pattern = morseFor(*cursor);
    if (pattern == nullptr) {
      continue;
    }

    if (pendingGapUnits > 0) {
      delayUnits(pendingGapUnits);
    }

    sendCharacter(pattern);
    pendingGapUnits = LETTER_GAP_UNITS;
  }
}

void setup() {
  Serial.begin(115200);

#if !defined(RGB_BUILTIN)
  pinMode(LED_BUILTIN, OUTPUT);
#endif

  setLed(false);
  Serial.print("Blinking Morse message: ");
  Serial.println(MESSAGE);
}

void loop() {
  sendMessage(MESSAGE);
  delayUnits(MESSAGE_GAP_UNITS);
}
