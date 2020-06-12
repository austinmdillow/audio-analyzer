/*
 Example program for the MD_MSGEQ7 library
 
 Reads the input value from the IC and displays a rolling table on the serial monitor.
*/

#include <FastLED.h>
#define ENCODER_DO_NOT_USE_INTERRUPTS
#include <Encoder.h>
#include <MSGEQ7.h>


/* LED Definitions */
#define NUM_LEDS   27 // MUST BE ODD
#define LED_PIN   8
#define CHIPSET   WS2811
#define COLOR_ORDER GRB
#define BRIGHTNESS  255
#define LED_DITHER  255  // try 0 to disable flickering
#define CORRECTION  TypicalLEDStrip

CRGB leds[NUM_LEDS];

const byte IN_MIN = 20;
const byte IN_MAX = 255;
const byte OUT_MIN = 0;
const byte OUT_MAX = 255;

/* Manual Control Definitions */
const int vol_knob_data_pin = 9;
const int vol_knob_clk_pin = 10;

Encoder mode_knob(vol_knob_data_pin, vol_knob_clk_pin);

enum led_modes {
  Normal,
  Monochrome,
  Enum_Length
};
enum led_modes led_mode;


const int color_knob_pin = A3;


/* MSGEQ7 Definitions */
#define pinAnalogLeft A0
#define pinAnalogRight A0
#define pinReset 5
#define pinStrobe 4
#define MSGEQ7_INTERVAL ReadsPerSecond(40)
#define MSGEQ7_SMOOTH true
// hardware pin definitions - change to suit circuit
#define DATA_PIN    A0
#define RESET_PIN   5
#define STROBE_PIN  4

CMSGEQ7<MSGEQ7_SMOOTH, pinReset, pinStrobe, pinAnalogLeft, pinAnalogRight> MSGEQ7;

const byte volume_pin = A4;



void setup() {
  led_mode = Normal;
  // FastLED setup
  Serial.begin(115200);
  FastLED.addLeds<CHIPSET, LED_PIN, COLOR_ORDER>(leds, NUM_LEDS).setCorrection(CORRECTION);
  FastLED.setBrightness( BRIGHTNESS );
  FastLED.setDither(LED_DITHER);
  FastLED.show(); // needed to reset leds to zero
  pinMode(13, OUTPUT);

  // This will set the IC ready for reading
  MSGEQ7.begin();
}

void readInputs() {
  //led_mode = mode_knob.read() % Enum_Length;
  if (analogRead(A5) > 4 * 1024/5.0 ) {
    digitalWrite(13, HIGH); 
  } else {
    digitalWrite(13, LOW);
  }
}

float readVolume() {
  return 1;
  //return analogRead(volume_pin) / 512.0;
}

float readColor() {
  return analogRead(color_knob_pin);
}

void loop() {

  readInputs();

  switch (led_mode) {
    case Normal:
      normalMode(); break;
    case Monochrome:
      monochromeMode(); break;
    default:
      Serial.println("How did we get here");
  }
  
    
}


void normalMode() {
  if (MSGEQ7.read(MSGEQ7_INTERVAL)) {
  
    MSGEQ7.read();

    int val = MSGEQ7.get(MSGEQ7_BASS);
    int mids = MSGEQ7.get(3);
    mids = mapNoise(mids, IN_MIN, IN_MAX, OUT_MIN, OUT_MAX) * readVolume() * 2.0;;
    //int mapped_val = mapNoise(val);
    int mapped_val = mapNoise(val, IN_MIN, IN_MAX, OUT_MIN, OUT_MAX) * 2.0;
    
    CRGB color = CRGB::Blue;
    CHSV hsv(mids*2, 255, mapped_val);
    color.nscale8_video(val);
    //fill_solid(leds, NUM_LEDS, color);
    Serial.print(MSGEQ7.get(1));
    Serial.print(" ");
    Serial.print(MSGEQ7.get(3));
    Serial.print(" ");
    Serial.println(MSGEQ7.get(6));
    
    int center_idx = NUM_LEDS/2;
    /*
    for (int i = NUM_LEDS - 1; i > 0; i=i-1) {
      leds[i] = leds[i - 1];
    }
    */

    for (int i = NUM_LEDS/2; i > 0; i--) {
      int left_idx = center_idx - i;
      int right_idx = center_idx + i;
      leds[center_idx - i] = leds[center_idx - i + 1];
      leds[center_idx + i] = leds[center_idx + i -1];
    }
    
    leds[center_idx] = CHSV(mids, 255, mapped_val);

    
    FastLED.show();
    
  }
}

void monochromeMode() {
  // nothing at the moment
}
