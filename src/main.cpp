/***************************************************
Play a random song on button press
 ****************************************************/

#include "Arduino.h"
#include "DFRobotDFPlayerMini.h"
#include <ezButton.h>

#if (defined(ARDUINO_AVR_UNO) || defined(ESP8266))  // Using a soft serial port
#include <SoftwareSerial.h>
SoftwareSerial softSerial(/*rx =*/4, /*tx =*/5);
#define FPSerial softSerial
#else
#define FPSerial Serial1
#endif

ezButton button(2); // D4 on esp mini
DFRobotDFPlayerMini myDFPlayer;
void printDetail(uint8_t type, int value);
const char *trackNameFromNumber(int trackNumber);
void reportCurrentTrack();

const unsigned long LONG_PRESS_MS = 1000;
const unsigned long TRACK_QUERY_DELAY_MS = 250;
unsigned long pressStartTime = 0;
bool longPressHandled = false;
bool trackQueryPending = false;
unsigned long trackQueryAtMs = 0;

void setup() {
#if (defined ESP32)
  FPSerial.begin(9600, SERIAL_8N1, /*rx =*/D3, /*tx =*/D2);
#else
  FPSerial.begin(9600);
#endif


  button.setDebounceTime(100); // set debounce time to 100 milliseconds (seems like best value after testing)

  Serial.begin(115200);

  Serial.println();
  Serial.println(F("The daemon-engine stirs. Slaanesh be praised."));
  Serial.println(F("Awakening the machine spirit ... (May take 3~5 seconds)"));

  if (!myDFPlayer.begin(FPSerial, /*isACK = */ true, /*doReset = */ true)) {  //Use serial to communicate with mp3.
    Serial.println(F("The machine spirit will not submit!"));
    Serial.println(F("1. Check the profane connections!"));
    Serial.println(F("2. Insert the data-slate (SD card)!"));
    while (true) {
      delay(0);  // Code to compatible with ESP8266 watch dog.
    }
  }
  Serial.println(F("Sonic amplifier online. Glory to the Dark Prince."));
  myDFPlayer.volume(22);  //Set volume value. From 0 to 30
  Serial.println(F("Anthems of excess loaded. The daemon-engine hungers."));
}

void loop() {
  button.loop();

  if (button.isPressed()) {
    pressStartTime = millis();
    longPressHandled = false;
  }

  if (button.getState() == LOW && !longPressHandled && (millis() - pressStartTime >= LONG_PRESS_MS)) {
    longPressHandled = true;
    Serial.println(F("Silence falls upon the battlefield... for now."));
    myDFPlayer.stop();
  }

  if (button.isReleased() && !longPressHandled) {
    Serial.println(F("The Dark Prince demands an anthem - indulge!"));
    myDFPlayer.randomAll();
    trackQueryPending = true;
    trackQueryAtMs = millis() + TRACK_QUERY_DELAY_MS;
  }

  if (trackQueryPending && millis() >= trackQueryAtMs) {
    trackQueryPending = false;
    reportCurrentTrack();
  }

  if (myDFPlayer.available()) {
    printDetail(myDFPlayer.readType(), myDFPlayer.read());  //Print the detail message from DFPlayer to handle different errors and states.
  }
}

const char *trackNameFromNumber(int trackNumber) {
  // Map DFPlayer numeric file indexes (1..N) to names in your SD card order.
  static const char *const kTrackNames[] = {
    "Track 1 - Rename me",
    "Track 2 - Rename me",
    "Track 3 - Rename me",
    "Track 4 - Rename me",
    "Track 5 - Rename me",
  };

  const size_t trackCount = sizeof(kTrackNames) / sizeof(kTrackNames[0]);
  if (trackNumber < 1 || static_cast<size_t>(trackNumber) > trackCount) {
    return "Unknown track (not mapped)";
  }

  return kTrackNames[trackNumber - 1];
}

void reportCurrentTrack() {
  const int trackNumber = myDFPlayer.readCurrentFileNumber();
  if (trackNumber <= 0) {
    Serial.println(F("Current track number unavailable right now."));
    return;
  }

  Serial.print(F("Now playing #"));
  Serial.print(trackNumber);
  Serial.print(F(": "));
  Serial.println(trackNameFromNumber(trackNumber));
}

void printDetail(uint8_t type, int value) {
  switch (type) {
    case TimeOut:
      Serial.println(F("The daemon-engine grows impatient - communion timed out!"));
      break;
    case WrongStack:
      Serial.println(F("Corruption in the data-stack. Fulgrim would be disappointed."));
      break;
    case DFPlayerCardInserted:
      Serial.println(F("data-slate inserted. She Who Thirsts is pleased."));
      break;
    case DFPlayerCardRemoved:
      Serial.println(F("data-slate removed. THIS SILENCES OFFENDS SLAANESH!"));
      break;
    case DFPlayerCardOnline:
      Serial.println(F("data-slate online. Anthems of excess stand ready."));
      break;
    case DFPlayerUSBInserted:
      Serial.println(F("Profane mechadendrite connected."));
      break;
    case DFPlayerUSBRemoved:
      Serial.println(F("Profane mechadendrite removed."));
      break;
    case DFPlayerPlayFinished:
      Serial.print(F("Anthem of excess "));
      Serial.print(value);
      Serial.print(F(" ("));
      Serial.print(trackNameFromNumber(value));
      Serial.println(F(") has faded. The Dark Prince demands more."));
      break;
    case DFPlayerError:
      Serial.print(F("The daemon-engine screams in agony - error: "));
      switch (value) {
        case Busy:
          Serial.println(F("data-slate not found. The silence is unbearable!"));
          break;
        case Sleeping:
          Serial.println(F("The daemon slumbers in blissful torpor..."));
          break;
        case SerialWrongStack:
          Serial.println(F("Data-stack corrupted beyond even Slaanesh's tolerance."));
          break;
        case CheckSumNotMatch:
          Serial.println(F("Data purity compromised. Perfection demands better."));
          break;
        case FileIndexOut:
          Serial.println(F("Anthem index exceeds even our excessive manifest!"));
          break;
        case FileMismatch:
          Serial.println(F("Requested anthem is lost to the warp."));
          break;
        case Advertise:
          Serial.println(F("Unsanctioned ecstatic transmission detected."));
          break;
        default:
          break;
      }
      break;
    default:
      break;
  }
}
