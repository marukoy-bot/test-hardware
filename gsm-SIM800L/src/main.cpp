#include <Arduino.h>
#include <SoftwareSerial.h>

// #define RX 17 // tx on gsm
// #define TX 16 //rx on gsm

#define RX 16
#define TX 17

SoftwareSerial gsm(RX, TX);

String formatTime(String cclk);
String ParseCCLK(String raw);
String getTime();
String getLoc();
void update();


void setup() {
    Serial.begin(115200);
    gsm.begin(9600);

    gsm.println("AT");
    delay(500);
    update();

    gsm.println("AT+CSCS=\"GSM\"");
    delay(500);
    update(); 

    gsm.println("AT+CNMI=1,2,0,0,0");
    delay(500);
    update(); 

    gsm.println("AT+CMGF=1");
    delay(500);
    update(); 

    //DeleteSMS();
    Serial.println("Initialized");

    String time = getTime();
    update();
    Serial.println(time);

    if (time != "")
    {
        gsm.println("AT+CMGF=1");
        update();
        delay(500);

        gsm.println("AT+CMGS=\"+639151635499\"");
        update();
        delay(500);

        gsm.print(formatTime(time));
        gsm.print((char)26);
        update();
    }    
}

void update()
{
    while(gsm.available()) Serial.write((char)gsm.read());
}

String getLoc()
{
    String loc = "";
    gsm.println("AT+CIPGSMLOC=1,1");
    //update();
    if(gsm.available())
    {
        while(gsm.available()) loc += (char)gsm.read();
        loc.trim();
        delay(1000);
        Serial.print("Location: ");
        Serial.println(loc);
    }
    delay(3000);
    return loc;
}

String getTime()
{
    String timeRaw = "";
    gsm.println("AT+CCLK?");
    delay(500);

    while(gsm.available()) timeRaw += (char)gsm.read();
    timeRaw.trim();

    String timeParsed = ParseCCLK(timeRaw);
    Serial.print("Raw: ");
    Serial.print(timeRaw);
    Serial.print(" | Parsed: ");
    Serial.println(timeParsed);
    return timeParsed;
}

String ParseCCLK(String raw)
{
    int firstQuote = raw.indexOf('"');
    int lastQuote = raw.lastIndexOf('"');

    if (firstQuote != -1 && lastQuote != -1 && lastQuote > firstQuote)
    {
        return raw.substring(firstQuote + 1, lastQuote);
    }

    return "";
}

String formatTime(String cclk)
{
    if (cclk.length() < 17) return "";

    // Extract pieces
    int yy   = cclk.substring(0, 2).toInt();
    int MM   = cclk.substring(3, 5).toInt();
    int dd   = cclk.substring(6, 8).toInt();
    int hh   = cclk.substring(9, 11).toInt();
    int min  = cclk.substring(12, 14).toInt();
    int sec  = cclk.substring(15, 17).toInt();

    // Year adjustment (assuming >= 2000)
    int yyyy = 2000 + yy;

    // Month names
    const char* months[] = {"Jan","Feb","Mar","Apr","May","Jun","Jul","Aug","Sep","Oct","Nov","Dec"};

    // Convert to 12-hour format
    String ampm = "AM";
    int hour12 = hh;
    if (hour12 == 0) {
        hour12 = 12;
    } else if (hour12 == 12) {
        ampm = "PM";
    } else if (hour12 > 12) {
        hour12 -= 12;
        ampm = "PM";
    }

    char buf[40];
    sprintf(buf, "%s %02d, %d | %02d:%02d %s", 
            months[MM-1], dd, yyyy, hour12, min, ampm.c_str());

    return String(buf);
}


void loop() {
  // put your main code here, to run repeatedly:
    //update();
}
