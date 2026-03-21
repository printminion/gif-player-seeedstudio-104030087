// ============================================================
// main.cpp — gif-player-seeedstudio-104030087
//
// Seeed Studio XIAO ESP32-S3 + Round Display (GC9A01 240×240)
// Animated GIF player with touch navigation.
//
// Template infrastructure (WiFi, OTA, version check) runs first
// in setup(); GIF player logic follows.
// ============================================================

#include <Arduino.h>

// Board config resolved via -I boards/seeed_xiao_esp32s3 include path
#include "board_config.h"
#include "logger.h"

// ── Template feature forward declarations ─────────────────
#ifdef FEATURE_WIFI_PROVISIONING
void setupWifi();
#endif
#ifdef FEATURE_OTA
void setupOTA();
void otaLoop();
#endif
#ifdef FEATURE_VERSION_CHECK
void checkAndApplyUpdate();
#endif
void blinkLed(int times, int delayMs = 200);

// ── GIF player includes ────────────────────────────────────
#define USE_TFT_ESPI_LIBRARY
#include "lv_xiao_round_screen.h"

#include <vector>
#include <SD.h>
#include "AnimatedGIF.h"
#include <Preferences.h>

// ── GIF player globals ─────────────────────────────────────
static const unsigned char PROGMEM image_micro_sd_no_card_bits[] = {0x3f,0xc0,0xa0,0x20,0x40,0x20,0x60,0x20,0x53,0x30,0x48,0x90,0x4c,0x88,0x42,0x08,0x49,0x10,0x4a,0x90,0x44,0xc8,0x40,0x28,0x47,0x90,0x58,0x68,0x37,0xd4,0x00,0x00};
static const unsigned char PROGMEM image_file_search_bits[] = {0x01,0xf0,0x02,0x08,0x04,0x04,0x08,0x02,0x08,0x02,0x08,0x0a,0x08,0x0a,0x08,0x12,0x04,0x64,0x0a,0x08,0x15,0xf0,0x28,0x00,0x50,0x00,0xa0,0x00,0xc0,0x00,0x00,0x00};

Preferences preferences;
AnimatedGIF gif;

// rule: loop GIF at least during 3s, maximum 5 times, and don't loop/animate longer than 30s per GIF
const int maxLoopIterations =     1; // stop after this amount of loops
const int maxLoopsDuration  =  3000; // ms, max cumulated time after the GIF will break loop
const int maxGifDuration    = 240000; // ms, max GIF duration
const int maxGifDurationMs  = 60 * 1000; // ms, max GIF duration

// used to center image based on GIF dimensions
static int xOffset = 0;
static int yOffset = 0;

static int totalFiles = 0; // GIF files count
static int currentFile = 0;
static int lastFile = -1;

char GifComment[256];

static File FSGifFile; // temp gif file holder
static File GifRootFolder; // directory listing
std::vector<std::string> GifFiles; // GIF files path

const int DO_PREVIOUS = 1;
const int DO_NEXT = 2;
const int DO_NOTHING = 3;

const int DO_CHANGE_MODE = 4;

const int PREF_MODE_STANDBY = 0; // wait for user to change the image
const int PREF_MODE_PLAYER = 1; // automatically change the picture after it played

// previous button position
int iPreviousButtonMarginLeft = 50;
int iPreviousButtonMarginTop = 120;

// next button position
int iNextButtonMarginLeft = 190;
int iNextButtonMarginTop = 120;

// mode button position
int iModeButtonMarginLeft = 120;
int iModeButtonMarginTop = 200;

int currentGifPlayedTime = 0;
int currentGifPlayedLoops = 0;

// folder path variable
const char *folderPath = "/data/";

uint16_t* tft_buffer;

unsigned int prefMode = PREF_MODE_STANDBY;
unsigned int prefCurrentFileIndex = 0;
unsigned long startTimeMs = 0;

bool isUiDemoSeen = false;
bool isUiDemoDisplayed = false;

// ── UI sprites ─────────────────────────────────────────────
TFT_eSprite sprButtonPrevious = TFT_eSprite(&tft);
TFT_eSprite sprButtonNext = TFT_eSprite(&tft);
TFT_eSprite sprButtonModeSelect = TFT_eSprite(&tft);

int xw;
int yh;

lv_coord_t touchX, touchY;

bool isNextButtonTouched = false;
bool isPreviousButtonTouched = false;
bool isModeButtonTouched = false;

int triangleSideLength = 60;
int triangleSideLengthHalf = triangleSideLength / 2;

int lastTouchedX = 0;
int lastTouchedY = 0;
bool repaint = true;
bool isPressedLastState = false;

// ── GIF player forward declarations ───────────────────────
static void * GIFOpenFile(const char *fname, int32_t *pSize);
static void GIFCloseFile(void *pHandle);
static int32_t GIFReadFile(GIFFILE *pFile, uint8_t *pBuf, int32_t iLen);
static int32_t GIFSeekFile(GIFFILE *pFile, int32_t iPosition);
static void TFTDraw(int x, int y, int w, int h, uint16_t* lBuf);
void GIFDraw(GIFDRAW *pDraw);
int gifPlay(char* gifPath);
int getGifInventory(const char *basePath);
void showUIDemo(bool showControls);
void prepareUI();
void drawBackButton(bool selected);
void drawNextButton(bool selected);
void drawModeSwitchButton(bool selected);
void switchMode();
bool isPointInRect(int touchX, int touchY, int topX, int topY, int rectWidth, int rectHeight);
bool isPointInCenteredRect(int touchX, int touchY, int topX, int topY, int rectWidth, int rectHeight);
int loopUI();

// ─────────────────────────────────────────────────────────
void setup() {
  LOG_BEGIN(SERIAL_BAUD);
  LOGF_STATUS("Project  : %s", PROJECT_NAME);
  LOGF_STATUS("Board    : %s", BOARD_NAME);
  LOGF_STATUS("Version  : %s", FIRMWARE_VERSION);
#ifdef FEATURE_WIFI_PROVISIONING
  LOG_STATUS("WiFi     : enabled");
#else
  LOG_STATUS("WiFi     : disabled");
#endif
#ifdef FEATURE_OTA
  LOG_STATUS("OTA      : enabled");
#else
  LOG_STATUS("OTA      : disabled");
#endif
#ifdef FEATURE_VERSION_CHECK
  LOG_STATUS("VerCheck : enabled");
#else
  LOG_STATUS("VerCheck : disabled");
#endif

  pinMode(LED_PIN, OUTPUT);
  blinkLed(3);  // startup indication

#ifdef FEATURE_WIFI_PROVISIONING
  setupWifi();
#endif

#ifdef FEATURE_VERSION_CHECK
  checkAndApplyUpdate();
#endif

#ifdef FEATURE_OTA
  setupOTA();
#endif

  LOG_STATUS("Setup complete — starting GIF player");

  // ── GIF player setup ────────────────────────────────────
  preferences.begin("gif-player", false);

  prefMode = preferences.getUInt("mode", PREF_MODE_STANDBY);
  log_n("Current Mode: %u\n", prefMode);
  if (prefMode != PREF_MODE_STANDBY && prefMode != PREF_MODE_PLAYER) {
    prefMode = PREF_MODE_STANDBY;
    log_n("Current Mode: standby (fallback)\n");
  }

  prefCurrentFileIndex = preferences.getUInt("file_index", 0);
  isUiDemoSeen = preferences.getBool("intro_seen", false);
  log_n("CurrentFileIndex: %u\n", prefCurrentFileIndex);
  currentFile = prefCurrentFileIndex;

  screen_rotation = 3;
  xiao_disp_init();

  pinMode(TOUCH_INT, INPUT_PULLUP);
  Wire.begin();

  prepareUI();

  int attempts = 0;
  int maxAttempts = 50;
  int delayBetweenAttempts = 300;
  bool isblinked = false;

  tft.setTextSize(2);

  pinMode(D2, OUTPUT);
  while (!SD.begin(D2)) {
    log_n("SD Card mount failed! (attempt %d of %d)", attempts, maxAttempts);
    isblinked = !isblinked;
    attempts++;
    if (isblinked) {
      tft.setTextColor(TFT_WHITE, TFT_BLACK);
    } else {
      tft.setTextColor(TFT_BLACK, TFT_WHITE);
    }
    tft.drawBitmap(36, tft.height() / 2, image_micro_sd_no_card_bits, 14, 16, 0xFFFF);
    tft.drawString("INSERT SD", tft.width() / 2 - 50, tft.height() / 2);
    if (attempts > maxAttempts) {
      log_n("Giving up");
    }
    delay(delayBetweenAttempts);
  }

  log_n("SD Card mounted!");

  // Debug: list SD root contents
  File root = SD.open("/");
  if (root) {
    log_n("SD root contents:");
    File entry = root.openNextFile();
    while (entry) {
      log_n("  %s %s", entry.isDirectory() ? "[DIR]" : "[FILE]", entry.name());
      entry = root.openNextFile();
    }
    root.close();
  }

  totalFiles = getGifInventory(folderPath);

  if (currentFile >= totalFiles) {
    currentFile = 0;
  }

  showUIDemo(true);
  delay(5000);
}

void loop() {
#ifdef FEATURE_OTA
  otaLoop();
#endif

  // ── GIF player loop ─────────────────────────────────────
  if (!isUiDemoSeen) {
    if (!isUiDemoDisplayed) {
      showUIDemo(true);
      isUiDemoDisplayed = true;
      delay(60);
    }
    if (chsc6x_is_pressed()) {
      isUiDemoSeen = true;
      preferences.putBool("intro_seen", isUiDemoSeen);
    }
    delay(60);
    return;
  }

  tft.fillScreen(TFT_BLACK);

  preferences.putUInt("file_index", currentFile);
  currentFile++;

  const char *fileName = GifFiles[currentFile % totalFiles].c_str();
  const char *fileDir = "/data/";
  char *filePath = (char *)malloc(strlen(fileName) + strlen(fileDir) + 1);
  strcpy(filePath, fileDir);
  strcat(filePath, fileName);

  int loops = maxLoopIterations;
  int durationControl = maxLoopsDuration;
  currentGifPlayedTime = 0;

  startTimeMs = millis();

  while (durationControl > 0) {
    int result = gifPlay((char *)filePath);

    if (result == maxGifDuration + DO_PREVIOUS) {
      durationControl = 0;
      currentFile = currentFile - 2;
      if (currentFile < 0) {
        currentFile = totalFiles;
      }
      preferences.putUInt("file_index", currentFile);
    } else if (result == maxGifDuration + DO_NEXT) {
      durationControl = 0;
    } else {
      durationControl -= result;
    }

    gif.reset();
  }
  free(filePath);
}

// ─────────────────────────────────────────────────────────
void blinkLed(int times, int delayMs) {
  for (int i = 0; i < times; i++) {
    digitalWrite(LED_PIN, LED_ACTIVE_LOW ? LOW : HIGH);
    delay(delayMs);
    digitalWrite(LED_PIN, LED_ACTIVE_LOW ? HIGH : LOW);
    delay(delayMs);
  }
}

// ── GIF file I/O callbacks ─────────────────────────────────
static void * GIFOpenFile(const char *fname, int32_t *pSize)
{
  log_d("GIFOpenFile( %s )\n", fname );
  FSGifFile = SD.open(fname);
  if (FSGifFile) {
    *pSize = FSGifFile.size();
    return (void *)&FSGifFile;
  }
  return NULL;
}

static void GIFCloseFile(void *pHandle)
{
  File *f = static_cast<File *>(pHandle);
  if (f != NULL)
     f->close();
}

static int32_t GIFReadFile(GIFFILE *pFile, uint8_t *pBuf, int32_t iLen)
{
  int32_t iBytesRead;
  iBytesRead = iLen;
  File *f = static_cast<File *>(pFile->fHandle);
  if ((pFile->iSize - pFile->iPos) < iLen)
      iBytesRead = pFile->iSize - pFile->iPos - 1;
  if (iBytesRead <= 0)
      return 0;
  iBytesRead = (int32_t)f->read(pBuf, iBytesRead);
  pFile->iPos = f->position();
  return iBytesRead;
}

static int32_t GIFSeekFile(GIFFILE *pFile, int32_t iPosition)
{
  File *f = static_cast<File *>(pFile->fHandle);
  f->seek(iPosition);
  pFile->iPos = (int32_t)f->position();
  return pFile->iPos;
}

static void TFTDraw(int x, int y, int w, int h, uint16_t* lBuf)
{
  tft.pushRect( x+xOffset, y+yOffset, w, h, lBuf );
}

// Draw a line of image directly on the LCD
void GIFDraw(GIFDRAW *pDraw)
{
  uint8_t *s;
  uint16_t *d, *usPalette, usTemp[320];
  int x, y, iWidth;

  iWidth = pDraw->iWidth;
  if (iWidth > SCREEN_WIDTH)
      iWidth = SCREEN_WIDTH;
  usPalette = pDraw->pPalette;
  y = pDraw->iY + pDraw->y;

  s = pDraw->pPixels;
  if (pDraw->ucDisposalMethod == 2) {
    for (x=0; x<iWidth; x++) {
      if (s[x] == pDraw->ucTransparent)
          s[x] = pDraw->ucBackground;
    }
    pDraw->ucHasTransparency = 0;
  }
  if (pDraw->ucHasTransparency) {
    uint8_t *pEnd, c, ucTransparent = pDraw->ucTransparent;
    int x, iCount;
    pEnd = s + iWidth;
    x = 0;
    iCount = 0;
    while(x < iWidth) {
      c = ucTransparent-1;
      d = usTemp;
      while (c != ucTransparent && s < pEnd) {
        c = *s++;
        if (c == ucTransparent) {
          s--;
        } else {
            *d++ = usPalette[c];
            iCount++;
        }
      }
      if (iCount) {
        TFTDraw( pDraw->iX+x, y, iCount, 1, (uint16_t*)usTemp );
        x += iCount;
        iCount = 0;
      }
      c = ucTransparent;
      while (c == ucTransparent && s < pEnd) {
        c = *s++;
        if (c == ucTransparent)
            iCount++;
        else
            s--;
      }
      if (iCount) {
        x += iCount;
        iCount = 0;
      }
    }
  } else {
    s = pDraw->pPixels;
    for (x=0; x<iWidth; x++)
      usTemp[x] = usPalette[*s++];
    TFTDraw( pDraw->iX, y, iWidth, 1, (uint16_t*)usTemp );
  }
}

int gifPlay(char* gifPath)
{
  gif.begin(BIG_ENDIAN_PIXELS);
  if( ! gif.open( gifPath, GIFOpenFile, GIFCloseFile, GIFReadFile, GIFSeekFile, GIFDraw ) ) {
    log_n("Could not open gif %s", gifPath );
    tft.fillScreen(TFT_BLACK);
    tft.setTextColor( TFT_WHITE, TFT_BLACK );
    tft.drawString( "Could not open gif", 20, tft.height()/2 );
    tft.drawString(String(gifPath), 20, tft.height()/2 + 40 );
    delay(300);
    return maxLoopsDuration;
  }

  int frameDelay = 0;
  int then = 0;
  bool showcomment = false;
  currentGifPlayedLoops = 0;

  int w = gif.getCanvasWidth();
  int h = gif.getCanvasHeight();
  xOffset = ( tft.width()  - w )  /2;
  yOffset = ( tft.height() - h ) /2;

  if( lastFile != currentFile ) {
    log_n("Playing %s [%d,%d] with offset [%d,%d]", gifPath, w, h, xOffset, yOffset );
    lastFile = currentFile;
    showcomment = true;
  }

  while (gif.playFrame(true, &frameDelay)) {
    then += frameDelay;

    unsigned long currentTimeMs = millis();
    unsigned long elapsedTimeMs = currentTimeMs - startTimeMs;

    int result = loopUI();

    if (prefMode == PREF_MODE_PLAYER && elapsedTimeMs >= maxGifDurationMs) {
      log_n("Play next image");
      result = DO_NEXT;
    }

    if (result == DO_NOTHING) {
      then = 0;
    } else if (result == DO_NEXT) {
      log_d("LoopUI result: %d", result);
      then = maxGifDuration + DO_NEXT;
      gif.close();
      return then;
    } else if (result == DO_PREVIOUS) {
      log_d("LoopUI result: %d", result);
      then = maxGifDuration + DO_PREVIOUS;
      gif.close();
      return then;
    }
  }

  gif.close();
  return then;
}

int getGifInventory(const char *basePath)
{
  int amount = 0;
  GifRootFolder = SD.open(basePath);
  if (!GifRootFolder) {
    log_n("Failed to open directory");
    return 0;
  }
  if (!GifRootFolder.isDirectory()) {
    log_n("Not a directory");
    return 0;
  }

  File file = GifRootFolder.openNextFile();

  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.setTextSize(2);

  int textPosX = tft.width() / 2 - 16;
  int textPosY = tft.height() / 2 - 10;

  tft.drawString("by", textPosX, textPosY - 90);
  tft.drawString("@printminion", textPosX - 60, textPosY - 60);

  tft.drawBitmap(42, textPosY + 20, image_file_search_bits, 15, 16, 0xFFFF);
  tft.drawString("GIF files:", textPosX - 40, textPosY + 20);

  while (file) {
    if (!file.isDirectory()) {
      GifFiles.push_back(file.name());
      log_n("Found file: %s", file.name());
      amount++;
      tft.drawString(String(amount), textPosX, textPosY + 40);
      file.close();
    }
    file = GifRootFolder.openNextFile();
  }
  GifRootFolder.close();
  log_n("Found %d GIF files", amount);
  return amount;
}

// ── UI drawing ─────────────────────────────────────────────
void showUIDemo(bool showControls)
{
  tft.fillScreen(TFT_BLACK);
  drawBackButton(false);
  drawNextButton(false);
  drawModeSwitchButton(false);

  int marginTop = (tft.height() / 2) + 30;
  tft.drawString("prev", 40, marginTop);
  tft.drawString("next", 165, marginTop);

  if (prefMode == PREF_MODE_STANDBY) {
    tft.drawString("still", 90, 200);
  } else {
    tft.drawString("player", 90, 200);
  }
  tft.drawString("mode", 95, marginTop + 10);

  if (showControls) {
    tft.drawString("controls", 75, 40);
  }
}

void prepareUI()
{
  xw = tft.width() / 2;
  yh = tft.height() / 2;

  tft.setPivot(xw, yh);

  sprButtonPrevious.createSprite(triangleSideLength, triangleSideLength);
  sprButtonPrevious.fillSprite(TFT_TRANSPARENT);
  sprButtonPrevious.fillTriangle(triangleSideLength - 1, 0, triangleSideLength - 1, triangleSideLength, 0, triangleSideLengthHalf, TFT_WHITE);
  sprButtonPrevious.drawTriangle(triangleSideLength - 1, 0, triangleSideLength - 1, triangleSideLength, 0, triangleSideLengthHalf, TFT_BLACK);
  sprButtonPrevious.drawWideLine(triangleSideLength - 1, 0, triangleSideLength - 1, triangleSideLength, 5, TFT_BLACK);
  sprButtonPrevious.drawWideLine(triangleSideLength - 1, triangleSideLength, 0, triangleSideLengthHalf, 5, TFT_BLACK);
  sprButtonPrevious.drawWideLine(triangleSideLength - 1, 0, 0, triangleSideLengthHalf, 5, TFT_BLACK);

  sprButtonNext.createSprite(triangleSideLength, triangleSideLength);
  sprButtonNext.fillSprite(TFT_TRANSPARENT);
  sprButtonNext.fillTriangle(0, 0, 0, triangleSideLength, triangleSideLength, triangleSideLengthHalf, TFT_WHITE);
  sprButtonNext.drawTriangle(0, 0, 0, triangleSideLength, triangleSideLength, triangleSideLengthHalf, TFT_BLACK);
  sprButtonNext.drawWideLine(0, 0, 0, triangleSideLength, 5, TFT_BLACK);
  sprButtonNext.drawWideLine(0, triangleSideLength, triangleSideLength, triangleSideLengthHalf, 5, TFT_BLACK);
  sprButtonNext.drawWideLine(0, 0, triangleSideLength, triangleSideLengthHalf, 5, TFT_BLACK);

  sprButtonModeSelect.createSprite(triangleSideLength, triangleSideLength);
  sprButtonModeSelect.fillSprite(TFT_TRANSPARENT);

  tft_buffer = (uint16_t*) malloc( (triangleSideLength + 2) * (triangleSideLength + 2) * 2 );
}

void drawBackButton(bool selected)
{
  int x1 = iPreviousButtonMarginLeft;
  int y1 = iPreviousButtonMarginTop;
  tft.setPivot(x1, y1);
  sprButtonPrevious.pushSprite(x1 - triangleSideLengthHalf, y1 - triangleSideLengthHalf, TFT_TRANSPARENT);
  if (selected) {
    tft.drawRoundRect(x1 - triangleSideLengthHalf, y1 - triangleSideLengthHalf,
                      triangleSideLength, triangleSideLength, 3, TFT_WHITE);
  }
}

void drawNextButton(bool selected)
{
  int x1 = iNextButtonMarginLeft;
  int y1 = iNextButtonMarginTop;
  tft.setPivot(x1, y1);
  sprButtonNext.pushSprite(x1 - triangleSideLengthHalf, y1 - triangleSideLengthHalf, TFT_TRANSPARENT);
  if (selected) {
    tft.drawRoundRect(x1 - triangleSideLengthHalf, y1 - triangleSideLengthHalf,
                      triangleSideLength, triangleSideLength, 3, TFT_WHITE);
  }
}

void drawModeSwitchButton(bool selected)
{
  int x1 = iModeButtonMarginLeft;
  int y1 = iModeButtonMarginTop;

  int halfHeight = sprButtonModeSelect.height() / 2;
  int halfWidth = sprButtonModeSelect.width() / 2;

  sprButtonModeSelect.fillScreen(TFT_TRANSPARENT);
  sprButtonModeSelect.fillRoundRect(0, 0, 60, 60, 3, TFT_BLACK);

  int topX = 0;
  int topY = 0;
  int btnHeight = triangleSideLength;
  int btnWidth = triangleSideLength;
  int borderThickness = 4;

  if (prefMode == PREF_MODE_STANDBY) {
    sprButtonModeSelect.fillRoundRect(topX, topY, 20, 60, 3, TFT_BLACK);
    sprButtonModeSelect.fillRoundRect(topX + borderThickness, topY + borderThickness, 12, 52, 3, TFT_WHITE);
    sprButtonModeSelect.fillRoundRect(40, 0, 20, 60, 3, TFT_BLACK);
    sprButtonModeSelect.fillRoundRect(40 + borderThickness, 0 + borderThickness, 12, 52, 3, TFT_WHITE);
  } else {
    sprButtonModeSelect.fillTriangle(topX, topY, topX, btnHeight + topY,
                                     btnWidth + topX, btnHeight/2 + topY, TFT_BLACK);
    sprButtonModeSelect.fillTriangle(topX + borderThickness, topY + borderThickness * 2,
                                     topX + borderThickness, btnHeight + topY - borderThickness * 2,
                                     btnWidth + topX - borderThickness * 2, btnHeight/2 + topY, TFT_WHITE);
  }

  tft.setPivot(x1, y1);
  sprButtonModeSelect.pushSprite(x1 - halfWidth, y1 - halfHeight, TFT_TRANSPARENT);
  if (selected) {
    tft.drawRoundRect(x1 - halfWidth, y1 - halfHeight, btnHeight, btnWidth, 3, TFT_WHITE);
  }
}

void switchMode()
{
  if (prefMode == PREF_MODE_STANDBY) {
    prefMode = PREF_MODE_PLAYER;
    preferences.putUInt("mode", PREF_MODE_PLAYER);
  } else {
    prefMode = PREF_MODE_STANDBY;
    preferences.putUInt("mode", PREF_MODE_STANDBY);
  }
  log_n("Switched mode to %d", prefMode);
}

bool isPointInRect(int touchX, int touchY, int topX, int topY, int rectWidth, int rectHeight)
{
  if (touchX >= topX && touchX <= topX + rectWidth) {
    if (touchY >= topY && touchY <= topY + rectHeight) {
      return true;
    }
  }
  return false;
}

bool isPointInCenteredRect(int touchX, int touchY, int topX, int topY, int rectWidth, int rectHeight)
{
  topX = topX - rectWidth / 2;
  topY = topY - rectHeight / 2;
  if (touchX >= topX && touchX <= topX + rectWidth) {
    if (touchY >= topY && touchY <= topY + rectHeight) {
      return true;
    }
  }
  return false;
}

int loopUI()
{
  int result = DO_NOTHING;
  repaint = false;

  if (!chsc6x_is_pressed()) {
    if (isPressedLastState) {
      isPressedLastState = false;
    }
    delay(60);
    return result;
  }

  log_d("The display is touched.");
  chsc6x_get_xy(&touchX, &touchY);
  log_d("%dx%d", touchX, touchY);

  if (!isPressedLastState) {
    repaint = true;
    isPressedLastState = true;
  }
  if (lastTouchedX != touchX) { lastTouchedX = touchX; repaint = true; }
  if (lastTouchedY != touchY) { lastTouchedY = touchY; repaint = true; }

  if (!repaint) {
    delay(60);
    return result;
  }

  isNextButtonTouched = false;
  isPreviousButtonTouched = false;
  isModeButtonTouched = false;

  tft.fillCircle(touchX - 5, touchY - 5, 10, TFT_GREEN);

  if (isPointInCenteredRect(touchX, touchY, iModeButtonMarginLeft, iModeButtonMarginTop,
                             sprButtonModeSelect.width(), sprButtonModeSelect.height())) {
    isModeButtonTouched = true;
  } else if (isPointInCenteredRect(touchX, touchY, iPreviousButtonMarginLeft, iPreviousButtonMarginTop,
                                    sprButtonNext.width(), sprButtonNext.height())) {
    isPreviousButtonTouched = true;
  } else if (isPointInCenteredRect(touchX, touchY, iNextButtonMarginLeft, iNextButtonMarginTop,
                                    sprButtonNext.width(), sprButtonNext.height())) {
    isNextButtonTouched = true;
  } else {
    showUIDemo(true);
    delay(1000);
  }

  if (isPreviousButtonTouched) {
    result = DO_PREVIOUS;
    drawBackButton(false);
    delay(1000);
  }
  if (isNextButtonTouched) {
    result = DO_NEXT;
    drawNextButton(false);
    delay(1000);
  }
  if (isModeButtonTouched) {
    switchMode();
    tft.readRect(iModeButtonMarginLeft - triangleSideLength / 2,
                 iModeButtonMarginTop - triangleSideLength / 2,
                 triangleSideLength, triangleSideLength, tft_buffer);
    for (int i = 0; i < 3; i++) {
      drawModeSwitchButton(true);
      delay(500);
      tft.pushRect(iModeButtonMarginLeft - triangleSideLength / 2,
                   iModeButtonMarginTop - triangleSideLength / 2,
                   triangleSideLength, triangleSideLength, tft_buffer);
    }
  }

  delay(60);
  return result;
}
