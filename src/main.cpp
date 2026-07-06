#include <Arduino.h>

// Change this string to the message you want to blink.
const char MESSAGE[] = "PATCH";

constexpr unsigned long DOT_MS = 150;
constexpr uint8_t DASH_UNITS = 3;
constexpr uint8_t SYMBOL_GAP_UNITS = 1;
constexpr uint8_t LETTER_GAP_UNITS = 3;
constexpr uint8_t WORD_GAP_UNITS = 7;
constexpr uint8_t MESSAGE_GAP_UNITS = 14;

constexpr uint8_t LED_BRIGHTNESS = 40;
constexpr unsigned long RAINBOW_CYCLE_MS = 12000;
constexpr unsigned long RAINBOW_UPDATE_MS = 25;

struct Rgb {
  uint8_t red;
  uint8_t green;
  uint8_t blue;
};

Rgb rainbowColor(unsigned long nowMs) {
  const uint16_t wheelPosition =
      static_cast<uint16_t>(((nowMs % RAINBOW_CYCLE_MS) * 1536UL) / RAINBOW_CYCLE_MS);
  const uint8_t segment = wheelPosition / 256;
  const uint8_t offset = wheelPosition % 256;
  const uint8_t rising = static_cast<uint8_t>((offset * LED_BRIGHTNESS) / 255);
  const uint8_t falling = LED_BRIGHTNESS - rising;

  switch (segment) {
    case 0: return {LED_BRIGHTNESS, rising, 0};
    case 1: return {falling, LED_BRIGHTNESS, 0};
    case 2: return {0, LED_BRIGHTNESS, rising};
    case 3: return {0, falling, LED_BRIGHTNESS};
    case 4: return {rising, 0, LED_BRIGHTNESS};
    default: return {LED_BRIGHTNESS, 0, falling};
  }
}

void writeLed(Rgb color) {
#if defined(RGB_BUILTIN)
  rgbLedWrite(RGB_BUILTIN, color.red, color.green, color.blue);
#else
  digitalWrite(LED_BUILTIN, color.red || color.green || color.blue ? HIGH : LOW);
#endif
}

void setLed(bool on) {
  writeLed(on ? rainbowColor(millis()) : Rgb{0, 0, 0});
}

void delayWithLed(bool ledOn, unsigned long durationMs) {
  const unsigned long startMs = millis();

  while (millis() - startMs < durationMs) {
    setLed(ledOn);

    const unsigned long elapsedMs = millis() - startMs;
    const unsigned long remainingMs = durationMs - elapsedMs;
    delay(remainingMs > RAINBOW_UPDATE_MS ? RAINBOW_UPDATE_MS : remainingMs);
  }
}

void waitUnits(uint8_t units, bool ledOn) {
  delayWithLed(ledOn, DOT_MS * static_cast<unsigned long>(units));
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
  waitUnits(symbol == '-' ? DASH_UNITS : 1, true);
}

void sendCharacter(const char *pattern) {
  for (const char *symbol = pattern; *symbol != '\0'; ++symbol) {
    sendSymbol(*symbol);

    if (*(symbol + 1) != '\0') {
      waitUnits(SYMBOL_GAP_UNITS, false);
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
      waitUnits(pendingGapUnits, false);
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
  waitUnits(MESSAGE_GAP_UNITS, false);
}
