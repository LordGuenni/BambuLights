#include "BambuLights.h"
#include <math.h>

//#define DEBUG_COLORS
//#define DEBUG_FADE

CompositeConfigItem& BambuLights::getNoWiFiConfig() {
    static BooleanConfigItem colors("colors", true);  // Collapsed
    static ByteConfigItem pattern("pattern", pulse);
    static IntConfigItem  hue("hue", 203);
    static ByteConfigItem value("value", 255);
    static ByteConfigItem saturation("saturation", 255);
    static ByteConfigItem pulse_per_min("pulse_per_min", 10);

    static BaseConfigItem* configSet[] {
        &colors,
        &pattern,
        &hue,
        &value,
        &saturation,
        &pulse_per_min,
        0
    };

    static CompositeConfigItem config("noWiFi", 0, configSet);

    return config;
}

CompositeConfigItem& BambuLights::getNoPrinterConnectedConfig() {
    static BooleanConfigItem colors("colors", true);  // Collapsed
    static ByteConfigItem pattern("pattern", constant);
    static IntConfigItem  hue("hue", 203);
    static ByteConfigItem value("value", 255);
    static ByteConfigItem saturation("saturation", 255);
    static ByteConfigItem pulse_per_min("pulse_per_min", 7);

    static BaseConfigItem* configSet[] {
        &colors,
        &pattern,
        &hue,
        &value,
        &saturation,
        &pulse_per_min,
        0
    };

    static CompositeConfigItem config("noPrinterConnected", 0, configSet);

    return config;
}

CompositeConfigItem& BambuLights::getPrinterConnectedConfig() {
    static BooleanConfigItem colors("colors", true);  // Collapsed
    static ByteConfigItem pattern("pattern", constant);
    static IntConfigItem  hue("hue", 0);
    static ByteConfigItem value("value", 128);
    static ByteConfigItem saturation("saturation", 0);
    static ByteConfigItem pulse_per_min("pulse_per_min", 7);

    static BaseConfigItem* configSet[] {
        &colors,
        &pattern,
        &hue,
        &value,
        &saturation,
        &pulse_per_min,
        0
    };

    static CompositeConfigItem config("printerConnected", 0, configSet);

    return config;
}

CompositeConfigItem& BambuLights::getPrintingConfig() {
    static BooleanConfigItem colors("colors", true);  // Collapsed
    static ByteConfigItem pattern("pattern", constant);
    static IntConfigItem  hue("hue", 0);
    static ByteConfigItem value("value", 255);
    static ByteConfigItem saturation("saturation", 0);
    static ByteConfigItem pulse_per_min("pulse_per_min", 7);

    static BaseConfigItem* configSet[] {
        &colors,
        &pattern,
        &hue,
        &value,
        &saturation,
        &pulse_per_min,
        0
    };

    static CompositeConfigItem config("printing", 0, configSet);

    return config;
}

CompositeConfigItem& BambuLights::getErrorConfig() {
    static BooleanConfigItem colors("colors", true);  // Collapsed
    static ByteConfigItem pattern("pattern", pulse);
    static IntConfigItem  hue("hue", 0);
    static ByteConfigItem value("value", 255);
    static ByteConfigItem saturation("saturation", 255);
    static ByteConfigItem pulse_per_min("pulse_per_min", 7);

    static BaseConfigItem* configSet[] {
        &colors,
        &pattern,
        &hue,
        &value,
        &saturation,
        &pulse_per_min,
        0
    };

    static CompositeConfigItem config("error", 0, configSet);

    return config;
}

CompositeConfigItem& BambuLights::getWarningConfig() {
    static BooleanConfigItem colors("colors", true);  // Collapsed
    static ByteConfigItem pattern("pattern", constant);
    static IntConfigItem  hue("hue", 171);
    static ByteConfigItem value("value", 255);
    static ByteConfigItem saturation("saturation", 255);
    static ByteConfigItem pulse_per_min("pulse_per_min", 7);

    static BaseConfigItem* configSet[] {
        &colors,
        &pattern,
        &hue,
        &value,
        &saturation,
        &pulse_per_min,
        0
    };

    static CompositeConfigItem config("warning", 0, configSet);

    return config;
}

CompositeConfigItem& BambuLights::getFinishedConfig() {
    static BooleanConfigItem colors("colors", true);  // Collapsed
    static ByteConfigItem pattern("pattern", constant);
    static IntConfigItem  hue("hue", 63);
    static ByteConfigItem value("value", 255);
    static ByteConfigItem saturation("saturation", 255);
    static ByteConfigItem pulse_per_min("pulse_per_min", 7);

    static BaseConfigItem* configSet[] {
        &colors,
        &pattern,
        &hue,
        &value,
        &saturation,
        &pulse_per_min,
        &getIdleTimeout(),
        0
    };

    static CompositeConfigItem config("finished", 0, configSet);

    return config;
}

CompositeConfigItem& BambuLights::getAllConfig() {
    static BaseConfigItem* configSet[] {
        &getNoWiFiConfig(),
        &getNoPrinterConnectedConfig(),
        &getPrinterConnectedConfig(),
        &getPrintingConfig(),
        &getErrorConfig(),
        &getWarningConfig(),
        &getFinishedConfig(),
        &getLedType(),
        &getNumLEDs(),
        &getLightMode(),
        &getLightState(),
        &getChamberSync(),
	      0
    };

    static CompositeConfigItem config("leds", 0, configSet);

    return config;
};

BambuLights::BambuLights(int pin) :
    pixels(new NeoPixelBus <NeoGrbFeature, NeoEsp32Rmt0800KbpsMethod>(getNumLEDs(), pin)),
    pin(pin),
    currentState(noWiFi)
{
    setCurrentConfig(getNoWiFiConfig());
}

void BambuLights::updatePixelCount() {
  reinitPending = true;
}

void BambuLights::doReinit() {
  if (pixels->PixelCount() != getNumLEDs()) {
    pixels->ClearTo(0);
    show();
    pixels->Dirty();
    show();
    delete pixels;
    pixels = new NeoPixelBus <NeoGrbFeature, NeoEsp32Rmt0800KbpsMethod>(getNumLEDs(), pin);
    begin();
  }
}

void BambuLights::setState(State state) {
  if (reinitPending) return;
  if (currentState != state) {
    currentState = state;

// noWiFi, noPrinter, printer, printing, no_lights, white, warning, error, finished
    bool oldBlack = black;
    bool oldWhite = brightWhite;
    byte oldPattern = *currentPattern;
    black = false;
    brightWhite = false;
    CHSV oldColor = {*currentHue, *currentSaturation, *currentValue};
    if (oldPattern == pulse) {
      oldColor.v = getPulseBrightness();
    }
    // Serial.print("state set to ");Serial.println(state);
    switch (state) {
      case noPrinter:
        setCurrentConfig(getNoPrinterConnectedConfig());
        break;
      case printer:
        setCurrentConfig(getPrinterConnectedConfig());
        break;
      case printing:
        setCurrentConfig(getPrintingConfig());
        break;
      case no_lights:
        black = true;
        break;
      case white:
        brightWhite = true;
        break;
      case warning:
        setCurrentConfig(getWarningConfig());
        break;
      case error:
        setCurrentConfig(getErrorConfig());
        break;
      case finished:
        setCurrentConfig(getFinishedConfig());
        break;
      default:
        setCurrentConfig(getNoWiFiConfig());
        break;
    }

    CHSV newColor = {*currentHue, *currentSaturation, *currentValue};
    if (oldBlack != black) {
      if (black) {
        // Fade from old color to old color at zero brightness
        newColor.h = oldColor.h;
        newColor.s = oldColor.s;
        newColor.v = 0;
      } else {
        // Fade from new color at zero brightness to new color
        oldColor.h = newColor.h;
        oldColor.s = newColor.s;
        oldColor.v = 0;
      }
    }
    if (oldWhite != brightWhite) {
      if (brightWhite) {
        // Fade from old color to old color at zero saturation (aka white) and full brightness
        newColor.h = oldColor.h;
        newColor.s = 0;
        newColor.v = 255;
      } else {
        // Fade from new color at zero saturation (aka white) and full brightness to new color
        oldColor.h = newColor.h;
        oldColor.s = 0;
        oldColor.v = 255;
      }
    }

    crossFade(oldColor, newColor);

    if (*currentPattern == pulse) {
      pulseOffset = millis(); // Always start at brightest level
    }
  }
}

void BambuLights::setProgressPercent(int percent) {
  if (percent < 0) {
    progressPercent = -1;
    return;
  }

  if (percent > 100) {
    percent = 100;
  }

  progressPercent = percent;
}

void BambuLights::setCurrentConfig(CompositeConfigItem& config) {
  currentConfig = &config;
  currentPattern = (ByteConfigItem*)config.get("pattern");
  currentHue = (IntConfigItem*)config.get("hue");
  currentValue = (ByteConfigItem*)config.get("value");
  currentSaturation = (ByteConfigItem*)config.get("saturation");
  currentPulsePerMin = (ByteConfigItem*)config.get("pulse_per_min");
}

void BambuLights::begin()  {
	pixels->Begin(); // This initializes the NeoPixel library.
	pixels->Show();
}

void BambuLights::renderProgressBar() {
  if (black) {
    clear();
    show();
    return;
  }

  uint8_t hue = *currentHue;
  uint8_t sat = *currentSaturation;
  uint16_t val;

  if (*currentPattern == pulse) {
    val = getPulseBrightness();
  } else {
    val = *currentValue;
    val = val * brightness / 255;
  }

  uint16_t totalLeds = pixels->PixelCount();
  int percent = progressPercent;
  if (percent < 0) {
    percent = 0;
  } else if (percent > 100) {
    percent = 100;
  }

  float litFloat = ((float)percent * (float)totalLeds) / 100.0f;
  uint16_t litCount = (uint16_t)lroundf(litFloat);
  bool blinkSingle = false;
  if (litCount == 0) {
    litCount = 1;
    blinkSingle = true;
  }

  bool blinkOn = true;
  if (blinkSingle) {
    blinkOn = (millis() / 500) % 2 == 0;
  }

  for (uint16_t i = 0; i < totalLeds; i++) {
    if (i < litCount) {
      if (blinkSingle && i == 0 && !blinkOn) {
        setPixelColor(i, 0, 0, 0);
      } else {
        setPixelColor(i, hue, sat, val);
      }
    } else {
      setPixelColor(i, 0, 0, 0);
    }
  }

  show();
}

void BambuLights::loop() {
  if (reinitPending) {
    doReinit();
    reinitPending = false;
  }
  //   enum patterns { dark, constant, rainbow, pulse, breath, num_patterns };
  uint8_t current_pattern = *currentPattern;

  if (currentState == printing && progressPercent >= 0 && *currentPattern == progress) {
    renderProgressBar();
    return;
  }

  if (black) {
    clear();
  } else if (brightWhite) {
    fill(255, 0, 255);
  } else {
    uint16_t val;
    switch (current_pattern) {
      case pulse:
        val = getPulseBrightness();
        break;
      default:
        val = *currentValue;
        val = val * brightness / 255;
        break;
    }
    fill(*currentHue, *currentSaturation, val);
  }
  show();
}

#ifdef DEBUG_FADE
void printCHSV(const CHSV& color) {
  Serial.print("{h=");Serial.print(color.h);
  Serial.print(",s=");Serial.print(color.s);
  Serial.print(",v=");Serial.print(color.v);
  Serial.print("}");
}
#endif

void BambuLights::crossFade(const CHSV& oldColor, const CHSV& newColor) {
#ifdef DEBUG_FADE
  Serial.print("Blending from ");printCHSV(oldColor);Serial.print(" to ");printCHSV(newColor);Serial.println("");
#endif
  TickType_t xLastWakeTime = xTaskGetTickCount();
  const TickType_t xFrequency = 2 / portTICK_PERIOD_MS; // So we will take approximately 0.75s to do the whole blend

  CHSV blendedColor;
  for (int i=0; i < 255; i++) {
    blendedColor = ::blend(oldColor, newColor, i, SHORTEST_HUES);
#ifdef DEBUG_FADE
    Serial.print("Blended color ");printCHSV(blendedColor);Serial.println("");
#endif
    fill(blendedColor.h, blendedColor.s, blendedColor.v);
    show();
    vTaskDelayUntil(&xLastWakeTime, xFrequency);
  }
}

static byte valueMin = 5;

byte BambuLights::getPulseBrightness() {
  // https://sean.voisen.org/blog/2011/10/breathing-led-with-arduino/
  float delta = (currentValue->value - valueMin) / 2.35040238;  // 2.35040238 = e - 0.36787944

  float pulse_length_millis = (60.0f * 1000) / currentPulsePerMin->value;
  float val = valueMin + (exp(cos(2 * M_PI * (millis() - pulseOffset) / pulse_length_millis)) - 0.36787944f) * delta;
  val = val * currentValue->value / 256;
  val = val * brightness / 255;

  return val;
}

void BambuLights::fill(uint8_t hue, uint8_t sat, uint8_t val) {
  static int oldHue = -1;
  static int oldSat = -1;
  static int oldVal = -1;
  static byte oldCount = -1;
  static byte oldLedType = -1;

  if (hue != oldHue || sat != oldSat || val != oldVal || oldCount != getNumLEDs() || oldLedType != getLedType()) {
    oldHue = hue;
    oldSat = sat;
    oldVal = val;
    oldCount = getNumLEDs();
    oldLedType = getLedType();
#ifdef DEBUG_COLORS
    Serial.print("Filling with ");
    Serial.print("{h=");Serial.print(hue);
    Serial.print(",s=");Serial.print(sat);
    Serial.print(",v=");Serial.print(val);
    Serial.print("}");
    Serial.println("");
#endif
    RgbColor color = HsbColor((float)hue/255.0f, (float)sat/255.0f, (float)val/255.0f);
    if (getLedType() == 1)  { // RGB not GRB
      uint8_t oldRed = color.R;
      color.R = color.G;
      color.G = oldRed;
    }

    pixels->ClearTo(color);
  }
}

void BambuLights::clear() {
  fill(0, 0, 0);
}

void BambuLights::show() {
  pixels->Show();
}

void BambuLights::setPixelColor(uint8_t digit, uint8_t hue, uint8_t sat, uint8_t val) {
    RgbColor color = HsbColor((byte)(hue)/256.0, (byte)(sat)/256.0, val/256.0);
    if (getLedType() == 1)  { // RGB not GRB
      uint8_t oldRed = color.R;
      color.R = color.G;
      color.G = oldRed;
    }
    pixels->SetPixelColor(digit, colorGamma.Correct(color));
}

const String BambuLights::patterns_str[BambuLights::num_patterns] = 
  { "Constant", "Pulse", "Progress" };
