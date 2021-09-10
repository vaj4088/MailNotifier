// Do not remove the include below
#include "MailNotifier.h"

#include "ESP8266WiFi.h"

#define REQUEST_SIZE 80
//
// Uncomment exactly one of these #define lines:
//
#define Home
// #define Aiden

//
// Uncomment exactly one of these #define lines:
//
#define debug
// #define noDebug

/*
 * Want to access
 *
 * https://maker.ifttt.com/trigger/{event}/with/key/bBzMt3GMKR46GbTLP6v919
 *
 * to trigger ifttt.com to send a text to Ronni.
 *
 * {event} is Mail_Notifier
 *
 * so use
 *
 * https://maker.ifttt.com/trigger/Mail_Notifier/with/key/bBzMt3GMKR46GbTLP6v919
 *
 * Also, use function snprintf to convert a number
 * (as in the password) to ASCII.
 *
 *  Private Network DHCP Info for gateway
 *
 *  Range
 *        192.168.1.64 – 192.168.1.253
 *
 *  Current Internet Connection
 *  Type 	        Value
 *  IP Address 	    45.17.221.124
 *  Subnet Mask 	255.255.252.0
 *  Default Gateway 45.17.220.1
 *  Primary DNS 	68.94.156.9
 *  Secondary DNS 	68.94.157.9
 *  Host Name 	    dsldevice
 *  MAC Address 	f8:2c:18:e4:b8:40
 *  MTU 	        1500
 *
 *  Go to http://ivanzuzak.info/urlecho/
 *
 *  for information on how to use an
 *  HTTP service for echoing the HTTP response
 *  defined in the URL of the request.
 *
 */
//
//
//
ADC_MODE(ADC_VCC) ;  // Self VCC Read Mode
//
// NOTE that this line (above) must be OUTSIDE of any function.
//

// Variables will change:

boolean success ;
int     status  ;

// constants won't change:
const char* ssid     = "*" ; // Replace * by the name (SSID) for your network.
const char* password = "*" ; // Replace * by the password    for your network.
//
// Defined in SSID.private
//
// const char* makerRequest = "..." ;
//
const int bssid = 0 ;
const byte channel[] = {0} ;
const boolean establishConnection = true ;

#if defined Home

IPAddress localIp( 192, 168,   1,  60) ;
IPAddress gateway( 192, 168,   1, 254) ;
IPAddress subnet ( 255, 255, 255,   0) ;
IPAddress dns1   (  68,  94, 156,   9) ;
IPAddress dns2   (  68,  94, 157,   9) ;

#elif defined Aiden

IPAddress localIp( 192, 168,   0, 160) ;
IPAddress gateway( 192, 168,   0,   1) ;
IPAddress subnet ( 255, 255, 255,   0) ;
IPAddress dns1   ( 192, 168,   0, 100) ;
IPAddress dns2   ( 192, 168,   0, 100) ;

#endif

void setup()
{
	// Serial
	Serial.begin(115200);

	unsigned long preparing;
	unsigned long const waitTime = 2000; // milliseconds

	preparing = millis();
	while (!delayingIsDone(preparing, waitTime)) {
	}

	Serial.println("Serial has been set up.");
	//
	// Version information.
	//
	Serial.print("Compiled on ");
	Serial.print(__DATE__);
	Serial.print(" at ");
	Serial.print(__TIME__);
	Serial.println(" local time.");

	//
	// Make unit a station, and connect to network.
	//

	//
	// Get the private encrypted strings.
	//
#include "SSID.private"
	//
	// End of "Get the private encrypted strings.".
	//

	ConnectStationToNetwork(ssid, password);
	//
	// End of "Make unit a station, and connect to network.".
	//

	//
	// Erase the private encrypted strings.
	//
	simpleErase(ssid);
	simpleErase(password);
	//
	// End of "Erase the private encrypted strings."
	//

	/*
	 * Want to access
	 *
	 * https://maker.ifttt.com/trigger/{event}/with/key/bBzMt3GMKR46GbTLP6v919
	 *
	 * to trigger ifttt.com to send a text to Ronni.
	 *
	 * {event} is Mail_Notifier
	 *
	 * so use
	 *
 https://maker.ifttt.com/trigger/Mail_Notifier/with/key/bBzMt3GMKR46GbTLP6v919
	 *
	 */

	double batteryVoltage = analogRead(A0) * 3.3 / 1023 ;

	#if defined debug

	 httpGet("45.17.221.124", "/", 21280) ;

	// http://45.17.221.124:21280/

	 Serial.print("Battery voltage is ") ;
	 Serial.print(batteryVoltage) ;
	 Serial.println(".") ;

    #elif defined noDebug

	 char request[REQUEST_SIZE] ;
	 snprintf(request, REQUEST_SIZE, "%s%#.2f.", makerRequest, batteryVoltage) ;
	 httpGet(
			 "maker.ifttt.com",
			 request
			 ) ;

	 #endif
	  }

	// TODO - Go into deep sleep.

//	  ESP.deepSleepInstant(microseconds, mode) will put the chip into deep sleep
//	  but sleeps instantly without waiting for WiFi to shutdown.
//	  mode is one of WAKE_RF_DEFAULT, WAKE_RFCAL, WAKE_NO_RFCAL,
//	  WAKE_RF_DISABLED.
//

// The loop function is called in an endless loop
void loop() {}

boolean delayingIsDone(unsigned long &since, unsigned long time) {
  // return false if we're still "delaying", true if time ms has passed.
  // this should look a lot like "blink without delay"
  unsigned long currentmillis = millis();
  if (currentmillis - since >= time) {
    since = currentmillis;
    return true;
  }
  return false;
}

void simpleEncrypt(const char *text) {
	char * textPointer = (char *)text ;
	Serial.print("Input:  ") ;
	Serial.print(textPointer) ;
	Serial.print(" ") ;
	for (char * i = (char *)text ; *i != 0 ; i++) {
		Serial.printf(" %#2.2hhX", *i) ;
	}
	Serial.println() ;
	byte index = 0 ;
	while(*textPointer) {
		*textPointer = 0x7F & (*textPointer + (3 & index++)) ;
		textPointer++ ;
	}

	Serial.print("Output: ") ;
	Serial.print(text) ;
	Serial.print(" {") ;
	char separator = ' ' ;
	for (char *i = (char *)text ; *i != 0 ; i++) {
		Serial.print(separator) ;
		Serial.printf(" %#2.2hhX", *i) ;
		separator = ',' ;
	}
	Serial.println("}") ;
	Serial.println() ;
}


void simpleDecrypt(const char *text) {
	char * textPointer = (char *)text ;
	byte index = 0 ;
	while(*textPointer) {
		*textPointer = 0x7F & (*textPointer - (3 & index++)) ;
		textPointer++ ;
	}
}

void simpleErase(const char *text) {
	char * textPointer = (char *)text ;
	while(*textPointer) {
		*textPointer = (byte)0 ;
		textPointer++ ;
	}
}

void stayHere() {
	while (true)
		yield();
}

void ConnectStationToNetwork(
		const char* encryptedNetworkName,
		const char* encryptedNetworkPassword
		) {
	char * writeableNetworkName     = (char *) encryptedNetworkName     ;
	char * writeableNetworkPassword = (char *) encryptedNetworkPassword ;
	//
	// Set up for station mode.
	//
	WiFi.mode(WIFI_STA);
	//
	// End of "Set up for station mode."
	//

	//
	// Configure for network.
	//
	success = WiFi.config(localIp, gateway, subnet, dns1, dns2);
	if (!success) {
		Serial.println("Could not configure.");
		stayHere();
	}

	//
	// Decrypt the private strings.
	//
	simpleDecrypt(writeableNetworkName    );
	simpleDecrypt(writeableNetworkPassword);
	//
	// End of "Read and decrypt the private strings."
	//

	//
	// End of "Configure for network."
	//


	//
	// Connect to network.
	//
	WiFi.begin(
			writeableNetworkName,
			writeableNetworkPassword,
			bssid,
			channel,
			establishConnection
			);
	//
	// Erase the private strings.
	//
	simpleErase(writeableNetworkName    );
	simpleErase(writeableNetworkPassword);
	//
	// End of "Erase the private strings."
	//

	status = WiFi.status();
	switch (status) {
	case WL_CONNECTED:
		Serial.println("Successful connection.");
		break;
	case WL_NO_SSID_AVAIL:
		Serial.print("Failed to connect because ") ;
		Serial.println("configured SSID could not be reached.");
		break;
	case WL_CONNECT_FAILED:
		Serial.println("Failed to connect because password is incorrect.");
		break;
	case WL_IDLE_STATUS:
		Serial.println("WiFi status is changing.");
		break;
	case WL_DISCONNECTED:
		Serial.print("Failed to connect because ") ;
		Serial.println("unit is not configured for station mode.");
		break;
	default:
		Serial.println("Failed to connect due to unknown reason.");
	}
	if (status != WL_CONNECTED) {
		stayHere();
	}
}

//
// Default values for request and port aree defined in the file MailNotifier.h
//
void httpGet(const char * server, const char * request, int port) {
	//
	// Default port of 80 is used for web access but any port may be specified.
	//
	WiFiClient client ;

	  if (client.connect(server, port)) {
	    Serial.println("Connected to server");

	    // Make a HTTP request:
	    client.print("GET ") ;
	    client.print(request) ;
	    client.println(" HTTP/1.1") ;

	    client.print("Host: ") ;
	    client.println(server) ;

	    // if there are incoming bytes available
	    // from the server, read them and print them:
	    while (client.available()) {
	    	char c = client.read();
	    	Serial.write(c);
	    }

	    client.println("Connection: close");
	    client.println();

	  } else {

		  Serial.print("Could not connect to server ") ;
		  Serial.print(server) ;
		  Serial.println(" .") ;
		  stayHere() ;

	  }
}

