/* 
 *  M5Stack - Core - Grey
 *  
 *  A little ight-light that animates pixels to simulate flying through a star field.
 *  Best used connected to a USB power source.
 *  
 *  Based on the M5Stack example "TFT_Starfield".
 *  Sped up using Lovyan GFX library:
 *  https://github.com/lovyan03/LovyanGFX
 *  
 *  Controls:
 *  Button A : Screen brightness (1 to 6)
 *  Button B : Star color (1 to 13)
 *  Button C : Star speed (1 to 11)
 *  
 *  Currently runs at 59 FPS.
 *  
 *  */

#include <M5Stack.h>
#include <LovyanGFX.hpp>

static LGFX lcd;

#define _CalcBrightness(b) 5 + (b * 50)
#define _CalcStarSpeed(s) 0.1 + (s * 0.5);

// With 1024 stars the update rate is ~65 frames per second
#define NSTARS 1024
int_fast16_t sx[NSTARS] = {};
int_fast16_t sy[NSTARS] = {};
float sz[NSTARS] = {};

float starSpeed = 2;

uint8_t colorCount = 0;
uint8_t brightnessCount = 4;
uint8_t starSpeedCount = 5;

uint8_t oldABtn = 0;
uint8_t oldBBtn = 0;
uint8_t oldCBtn = 0;

uint8_t za, zb, zc, zx;

uint16_t colorLUT[13][256];

// Fast 0-255 random number generator from http://eternityforest.com/Projects/rng.php:
uint8_t __attribute__((always_inline)) inline rng()
{
  zx++;
  za = (za ^ zc ^ zx);
  zb = (zb + za);
  zc = (zc + (zb >> 1)^za);
  return zc;
}

void setup() {
  za = random(256);
  zb = random(256);
  zc = random(256);
  zx = random(256);

  //bool LCDEnable, bool SDEnable, bool SerialEnable, bool I2CEnable
  M5.begin(false, false, true, false);
  M5.Power.begin();
  lcd.init();

  for (int i = 1; i < 256; i++) {
    colorLUT[0][i] = lcd.color565(i, i, i);

    colorLUT[1][i] = lcd.color565(i, (int) i / 1.5, (int) i / 1.5);
    colorLUT[2][i] = lcd.color565((int) i / 1.5, i, (int) i / 1.5);
    colorLUT[3][i] = lcd.color565((int) i / 1.5, (int) i / 1.5, i);

    colorLUT[4][i] = lcd.color565(i, 0, 0);
    colorLUT[5][i] = lcd.color565(0, i, 0);
    colorLUT[6][i] = lcd.color565(0, 0, i);

    colorLUT[7][i] = lcd.color565(i, i, 0);
    colorLUT[8][i] = lcd.color565(0, i, i);
    colorLUT[9][i] = lcd.color565(i, 0, i);

    if (random(100) < 50)
      colorLUT[10][i] = lcd.color565((i >> 1) + 127, 0, 0);
    else
      colorLUT[10][i] = lcd.color565((i >> 1) + 127, (i >> 1) + 127, (i >> 1) + 127);

    if (random(100) < 50)
      colorLUT[11][i] = lcd.color565(0, 0, 0);
    else
      colorLUT[11][i] = lcd.color565(255, 255, 255);

    if (random(100) < 75)
      colorLUT[12][i] = lcd.color565(i, i, i);
    else
      colorLUT[12][i] = lcd.color565(255, 255, 255);

    if (random(100) < 95)
      colorLUT[12][i] = lcd.color565(i, i, i);
    else
      colorLUT[12][i] = lcd.color565(255, 255, 255);  
  }

  M5.Lcd.setBrightness(_CalcBrightness(brightnessCount));
  starSpeed = _CalcStarSpeed(starSpeedCount);
}

void loop()
{
  unsigned long t0 = micros();

  // Button A
  uint8_t newABtn = M5.BtnA.read();
  if (M5.BtnA.read() && !oldABtn) {
    if (++brightnessCount > 5) {
      brightnessCount = 0;
    }
    M5.Lcd.setBrightness(_CalcBrightness(brightnessCount));
  }
  oldABtn = newABtn;

  // Button B
  uint8_t newBBtn = M5.BtnB.read();
  if (newBBtn && !oldBBtn) {
    if (++colorCount > 13) {
      colorCount = 0;
    }
  }
  oldBBtn = newBBtn;

  // Button C
  uint8_t newCBtn = M5.BtnC.read();
  if (M5.BtnC.read() && !oldCBtn) {
    if (++starSpeedCount > 11) {
      starSpeedCount = 0;
    }
    starSpeed = _CalcStarSpeed(starSpeedCount);
  }
  oldCBtn = newCBtn;

  uint_fast8_t spawnDepthVariation = 255;

  for (int i = 0; i < NSTARS; ++i) {
    if (sz[i] < 1) {
      sx[i] = (-120 + rng()) << 8;
      sy[i] = (rng() - 120) << 8;
      sz[i] = spawnDepthVariation--;
    }
    else {
      lcd.startWrite();

      uint_fast16_t old_screen_x = sx[i] / sz[i] + 160;
      uint_fast16_t old_screen_y = sy[i] / sz[i] + 120;

      lcd.writePixel(old_screen_x, old_screen_y, TFT_BLACK);

      sz[i] -= starSpeed;

      if (sz[i] > 1) {
        uint_fast16_t screen_x = sx[i] / sz[i] + 160;
        uint_fast16_t screen_y = sy[i] / sz[i] + 120;

        if (screen_x >= 0 && screen_y >= 0 && screen_x < 320 && screen_y < 240) {
          lcd.writePixel(screen_x, screen_y, colorLUT[colorCount][(int)(255 - sz[i])]);
        }
        else
          sz[i] = 0; // Out of screen, die.
      }
      lcd.endWrite();
    }
  }
  unsigned long t1 = micros();

  // Calcualte frames per second
  Serial.println(1.0 / ((t1 - t0) / 1000000.0));
}
