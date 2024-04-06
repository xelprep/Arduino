#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// Via https://javl.github.io/image2cpp/
const unsigned char logo_bmp [] PROGMEM = {
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0xfc, 0x06, 0x1f, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0xfc, 0x03, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0xfc, 0x22, 0x10, 0x1c, 0x07, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0xfc, 0x22, 0x10, 0x08, 0x07, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0xfc, 0x02, 0x10, 0x88, 0x07, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0xfc, 0x06, 0x10, 0x88, 0xc7, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0xfc, 0x06, 0x11, 0x88, 0xc7, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0xfc, 0x02, 0x11, 0x88, 0xc7, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0xfc, 0x22, 0x11, 0x88, 0x47, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0xfc, 0x22, 0x11, 0x88, 0x07, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0xfc, 0x22, 0x11, 0x88, 0x07, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0xfc, 0x22, 0x11, 0x8e, 0x87, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0xfc, 0x22, 0x11, 0x8c, 0x07, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xf8, 0x0f, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xfe, 0x1f, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xc0, 0x1f, 0xe0, 0x7f, 0xff, 0xff, 0x07, 0xff, 0xff, 0xe0, 0x00, 0x7f, 0xff,
	0xff, 0xff, 0xfe, 0x00, 0x0e, 0x00, 0x1f, 0xff, 0xfc, 0x01, 0xe0, 0x1e, 0x00, 0x00, 0x3f, 0xff,
	0xff, 0xff, 0xf8, 0x00, 0x00, 0x00, 0x0f, 0xff, 0xe0, 0x00, 0x00, 0x08, 0x00, 0x00, 0x1f, 0xff,
	0xff, 0xff, 0xe0, 0x00, 0x00, 0x00, 0x07, 0xff, 0xc0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1f, 0xff,
	0xff, 0xff, 0x80, 0x07, 0x00, 0x00, 0x03, 0xff, 0x00, 0x00, 0x00, 0x00, 0x03, 0xfe, 0x0f, 0xff,
	0xff, 0xff, 0x00, 0x3f, 0x80, 0x3f, 0x03, 0xfe, 0x00, 0xf0, 0x07, 0x00, 0x3f, 0xfe, 0x0f, 0xff,
	0xff, 0xfc, 0x01, 0xff, 0x80, 0xff, 0x81, 0xfc, 0x03, 0xf0, 0x1f, 0x01, 0xff, 0xfe, 0x0f, 0xff,
	0xff, 0xf8, 0x07, 0xff, 0x80, 0xff, 0xc1, 0xfc, 0x0f, 0xf0, 0xff, 0x07, 0xff, 0xfe, 0x0f, 0xff,
	0xff, 0xf0, 0x0f, 0xff, 0xc0, 0xff, 0xc1, 0xf8, 0x1f, 0xf1, 0xff, 0x1f, 0xff, 0xfe, 0x0f, 0xff,
	0xff, 0xe0, 0x3f, 0xff, 0xc0, 0xff, 0xe0, 0xf8, 0x3f, 0xf3, 0xff, 0x3f, 0xff, 0xfe, 0x0f, 0xff,
	0xff, 0xc0, 0x7f, 0xff, 0xc0, 0xff, 0xe0, 0xf8, 0x3f, 0xe3, 0xff, 0x3f, 0xff, 0xfe, 0x0f, 0xff,
	0xff, 0xc0, 0xff, 0xff, 0xc0, 0x7f, 0xe0, 0xf8, 0x7f, 0xe3, 0xff, 0x3f, 0xff, 0xfe, 0x0f, 0xff,
	0xff, 0xc1, 0xff, 0xff, 0xc0, 0x7f, 0xe0, 0xd8, 0x7f, 0xe3, 0xff, 0x1f, 0xff, 0xfc, 0x0f, 0xff,
	0xff, 0xc3, 0xff, 0xff, 0xc0, 0x7f, 0xe0, 0xd8, 0x7f, 0xe3, 0xff, 0x1f, 0xff, 0xfc, 0x1f, 0xff,
	0xff, 0xc7, 0xff, 0xf8, 0x00, 0x3f, 0xe0, 0xd8, 0x3f, 0xe3, 0xff, 0x1f, 0xff, 0xf8, 0x1f, 0xff,
	0xff, 0xc7, 0xff, 0xe0, 0x00, 0x3f, 0xe0, 0xd8, 0x3f, 0xe3, 0xff, 0x1f, 0xff, 0xc0, 0x1f, 0xff,
	0xff, 0xc7, 0xff, 0xe0, 0x00, 0x3f, 0xf0, 0xd8, 0x3f, 0xe3, 0xff, 0x0f, 0xff, 0xc0, 0x3f, 0xff,
	0xff, 0xc3, 0xff, 0xfe, 0x00, 0x3f, 0xf0, 0xf8, 0x3f, 0xe3, 0xff, 0x06, 0x7f, 0xc0, 0x7f, 0xff,
	0xff, 0xc3, 0xff, 0xff, 0xc0, 0x3f, 0xf0, 0x78, 0x3f, 0xe3, 0xff, 0x00, 0x7f, 0xc0, 0xef, 0xff,
	0xff, 0xc1, 0xff, 0xff, 0xf0, 0x3f, 0xf0, 0x00, 0x3f, 0xe3, 0xff, 0x00, 0x7f, 0xc1, 0xcf, 0xff,
	0xff, 0xc1, 0xff, 0xff, 0xf8, 0x3f, 0xf0, 0x00, 0x3f, 0xf3, 0xff, 0x00, 0x3f, 0xc1, 0x9f, 0xff,
	0xff, 0xc0, 0xff, 0xff, 0xf8, 0x3f, 0xf0, 0x00, 0x3f, 0xf3, 0xff, 0x00, 0x3f, 0xc1, 0xbf, 0xff,
	0xff, 0xe0, 0xff, 0xff, 0xfc, 0x3f, 0xf0, 0x00, 0x3f, 0xf3, 0xff, 0x00, 0x3f, 0xc1, 0xbf, 0xff,
	0xff, 0xe0, 0x0f, 0xff, 0xfc, 0x3f, 0xf0, 0xf8, 0x3f, 0xf3, 0xff, 0x00, 0x3f, 0xe1, 0xbf, 0xff,
	0xff, 0xe0, 0x00, 0xff, 0xfc, 0x3f, 0xff, 0xfc, 0x3f, 0xf3, 0xff, 0x00, 0x3f, 0xe0, 0xbf, 0xff,
	0xff, 0xe0, 0x00, 0x7f, 0xfc, 0x3f, 0xff, 0xfc, 0x1f, 0xff, 0xff, 0x00, 0x3f, 0xe0, 0xff, 0xff,
	0xff, 0xe0, 0xe0, 0xff, 0xfc, 0x3f, 0xff, 0xfe, 0x1f, 0xff, 0xff, 0x00, 0x3f, 0xe0, 0xff, 0xff,
	0xff, 0xe0, 0xff, 0xff, 0xfc, 0x3f, 0xff, 0xfe, 0x1f, 0xff, 0xfe, 0x04, 0x3f, 0xe0, 0xff, 0xff,
	0xff, 0xe0, 0x7f, 0xff, 0xfc, 0x3f, 0xff, 0xfe, 0x1f, 0xff, 0xfe, 0x0c, 0x3f, 0xe0, 0xdf, 0xff,
	0xff, 0xf0, 0x7f, 0xff, 0xf8, 0x3f, 0xff, 0xfe, 0x1f, 0xff, 0xfe, 0x0c, 0x3f, 0xe0, 0xdf, 0xff,
	0xff, 0xf8, 0x3f, 0xff, 0xf0, 0x3f, 0xff, 0xfe, 0x1f, 0xff, 0xfc, 0x0c, 0x3f, 0xe0, 0xdf, 0xff,
	0xff, 0xf8, 0x3f, 0xff, 0xe0, 0x1f, 0xff, 0xfe, 0x1f, 0xff, 0xf8, 0x1c, 0x3f, 0xe0, 0xdf, 0xff,
	0xff, 0xf8, 0x1f, 0xff, 0x80, 0x0f, 0xff, 0xf8, 0x0f, 0xff, 0xf0, 0x1c, 0x1f, 0xe0, 0xdf, 0xff,
	0xff, 0xfc, 0x1f, 0xff, 0x00, 0x07, 0xfe, 0x00, 0x03, 0xff, 0xe0, 0x3c, 0x1f, 0x80, 0xdf, 0xff,
	0xff, 0xfc, 0x0f, 0xf8, 0x00, 0x01, 0xf8, 0x00, 0x01, 0xff, 0xc0, 0x6c, 0x1f, 0x01, 0x9f, 0xff,
	0xff, 0xfe, 0x00, 0x00, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xcc, 0x1c, 0x03, 0xbf, 0xff,
	0xff, 0xff, 0x00, 0x00, 0x0f, 0xc0, 0x00, 0x00, 0xc0, 0x00, 0x01, 0x8c, 0x00, 0x07, 0x3f, 0xff,
	0xff, 0xff, 0x00, 0x00, 0x1c, 0xe0, 0x00, 0x03, 0xe0, 0x00, 0x03, 0x1e, 0x00, 0x0e, 0x7f, 0xff,
	0xff, 0xff, 0xc0, 0x00, 0xf8, 0xf0, 0x00, 0xff, 0x78, 0x00, 0x0e, 0x3e, 0x00, 0x1c, 0xff, 0xff,
	0xff, 0xff, 0xe0, 0x07, 0xc3, 0xfc, 0x07, 0xe0, 0x3e, 0x00, 0x3c, 0x7f, 0x00, 0xf1, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xff, 0x0f, 0xff, 0xfe, 0x07, 0xff, 0xff, 0xf1, 0xff, 0xc1, 0xc3, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xf8, 0x7f, 0xff, 0xf8, 0x7f, 0xff, 0xff, 0xc3, 0xff, 0xff, 0x0f, 0xff, 0xff,
	0xff, 0xff, 0xf8, 0x03, 0xff, 0xff, 0x03, 0xff, 0xff, 0x80, 0x1f, 0xff, 0xfc, 0x3f, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xe1, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff
};

void setup() {
  Serial.begin(9600);
  showLogo();
}

void loop() {
}

void showLogo() {
	display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
	display.clearDisplay();
	display.display();
	display.drawBitmap(0, 0, logo_bmp, 128, 64, WHITE);
	display.display();
}
