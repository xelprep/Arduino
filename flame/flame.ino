// Basic sketch to simulate a flame from several red and green LEDs
// Ran on Arduino Nano

const int green1pin = 3;
const int red1pin = 5;
const int red2pin = 6;
const int green3pin = 9;
const int green2pin = 10;
const int red3pin = 11;

int g1intensity;
int r1intensity;
int g2intensity;
int r2intensity;
int g3intensity;
int r3intensity;
int timer;

void setup() {
        pinMode(red1pin, OUTPUT);
        pinMode(green1pin, OUTPUT);
        pinMode(red2pin, OUTPUT);
        pinMode(green2pin, OUTPUT);
        pinMode(red3pin, OUTPUT);
        pinMode(green3pin, OUTPUT);
}

void loop() {

        // Generate random values for red, green, and the delay
        g1intensity = random(10, 40);
        r1intensity = random(180, 255);
        g2intensity = random(10, 40);
        r2intensity = random(180, 255);
        g3intensity = random(10, 40);
        r3intensity = random(180, 255);
        timer = random(5, 30);
        analogWrite(green1pin, g1intensity);
        analogWrite(red1pin, r1intensity);
        analogWrite(green2pin, g2intensity);
        analogWrite(red2pin, r2intensity);
        analogWrite(green3pin, g3intensity);
        analogWrite(red3pin, r3intensity);
        delay(timer);

}
