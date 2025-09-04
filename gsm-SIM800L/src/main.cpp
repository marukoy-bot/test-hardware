#include <Arduino.h>
#include <SoftwareSerial.h>

// #define RX 17 // tx on gsm
// #define TX 16 //rx on gsm

#define RX 3
#define TX 4

SoftwareSerial gsm(RX, TX);

String formatTime(String cclk);
String ParseCCLK(String raw);
String getTime();
String getLoc();
void update();


void setup() {
    Serial.begin(9600);
    gsm.begin(9600);

    Serial.print("Initializing... ");
    delay(200);
    Serial.println("Done");
    gsm.println("AT");
    update();
    delay(500);

    Serial.print("updating time...");
    gsm.println("AT+CLTS=1");
    update();
    gsm.println("AT&W");
    update();
    Serial.println("Done");
    // gsm.println("AT+SAPBR=3,1,\"Contype\",\"GPRS\"");
    // update();
    // delay(500);
    // gsm.println("AT+SAPBR=3,1,\"APN\",\"CMNET\"");
    // update();
    // delay(500);
    // gsm.println("AT+SAPBR=1,1");
    // update();
    // delay(500);
    // gsm.println("AT+SAPBR=2,1");
    // update();
    // delay(2000);
    // gsm.println("AT+CIPGSMLOC=2,1");
    // update();
    // delay(2000);
    // String location = getLoc();
    // gsm.println("AT+SAPBR=0,1");
    // update();
    // delay(500);

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
    delay(500);
    while(gsm.available()) Serial.write((char)gsm.read());
    while(Serial.available()) gsm.write(Serial.read());
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
