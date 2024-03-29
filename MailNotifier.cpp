
#include "../MailNotifier.h"
#include <ESP8266HTTPClient.h>
#include <WiFiClient.h>


//
// Get the private informations.
//
#include "SSIDprivate.h"
//
// End of "Get the private information.".
//

//
// The following includes are used for OTA reprogramming.
//
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <ESP8266HTTPUpdateServer.h>
#include <ESP8266WiFi.h>
//
// End of "The following includes are used for OTA reprogramming".
//

//
// Uncomment exactly one of these #define lines:
//
#define Home
// #define Aiden

//
// Define the channel to be used.  0 < Ian_Channel < 12
// Special Case: Ian_Channel = 0 means automatically search
//               for the appropriate channel.
//
// Ian_Channel < 0 OR Ian_Channel > 11 constitutes an error!
//
#define Ian_Channel 1

//
// Define the local socket to be used for debugging.
//
#define Ian_LocalDebugSocket 8080

//
// Define the local name or IP address to be used for debugging.
// Coyote2021Linux = 192.168.1.66
//
#define Ian_LocalDebugAddress "192.168.1.68"

//
// Uncomment exactly one of these #define lines:
//
 #define Ian_LocalDebugViaSocket
// #define Ian_NoLocalDebugViaSocket

//
// Uncomment exactly one of these #define lines:
//
// #define Ian_debug1
 #define Ian_noDebug1

//
// Uncomment exactly one of these #define lines:
//
// #define Ian_debug2
 #define Ian_noDebug2

//
// Uncomment exactly one of these #define lines:
//
// #define Ian_debug3
 #define Ian_noDebug3

/*
 * Wanted to access
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

const unsigned long CONNECTION_WAIT_MILLIS = 5 * 1000UL ;
const int REQUEST_SIZE = 80 ;
const byte pinNumber[] = {D0, D1, D2, D3, D4, D5, D6, D7} ;
//
// The internal pull up/down resistors have values of 30kΩ to 100kΩ,
// according to https://bbs.espressif.com/viewtopic.php?t=1079 .//
//
// Avoid GIO0, GPIO2, and GPIO15 because these control boot mode.
// These correspond on an ESP8266 D1 Mini Pro to
// D3, D4, and D8.
// D2 is GPIO4 and may be the SDA line of I2C.
//
const byte otaProgrammingIndicator = D2 ;
enum executionType {
	normalExecution = HIGH,
	otaReprogrammingExecution = LOW
} executionMode ;
//
// The following declarations are used for OTA reprogramming.
//
ESP8266WebServer httpServer(80) ;
ESP8266HTTPUpdateServer httpUpdater ;

const char* otaHost = "MNOTA" ; // ESP8266 Mailbox Notifier OTA programming
const char * updateMessage =
"\nHTTP Update Server ready! Open http://%s/update in your browser.\n" ;
//
// End of "The following declarations are used for OTA reprogramming".
//

#define numberOfArrayElements(x) (sizeof(x)/sizeof(x[0]))

//
// Defined in SSID.private
//
// const char* triggerRequest = "..." ;
//

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

#if defined Ian_LocalDebugViaSocket
		WiFiClient debug ;
#endif

void setup()
{
	double batteryVoltage ;

	for (byte i = 0 ; i<numberOfArrayElements(pinNumber) ; i++) {
		pinMode     (pinNumber[i], INPUT_PULLUP) ;
	}

	// Serial
	Serial.begin(115200);

	Serial.flush() ;

#if defined Ian_debug3
	scanNetworkSynchronous() ;
#endif

	//
	// Make unit a station, and connect to network.
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

	if (digitalRead(otaProgrammingIndicator)==normalExecution) {
		executionMode = normalExecution ;
	} else {
		executionMode = otaReprogrammingExecution ;
	}
	if (executionMode==normalExecution) {

	/*
	 * Wanted to access
	 *
	 * https://maker.ifttt.com/trigger/{event}/with/key/<IFTTT_Service_key>
	 *
	 * to trigger ifttt.com to send a text to Ronni.
	 *
	 * {event} is Mail_Notifier
	 *
	 * so use
	 *
 https://maker.ifttt.com/trigger/Mail_Notifier/with/key/<IFTTT_Service_key>
	 *
	 * but now want to use Aiden Shef's Home Assistant home automation
	 * so do the equivalent of
	 *
	 * curl -X POST https://ha.shef.duckdns.org/api
	 * /webhook
	 * /ianshefwebhook4588\?subject=Test\#1\&message=Test202209041533
	 * --trace-ascii -
	 *
	 * by using https (http over SSL) to access via POST
	 *
	 * ha.shef.duckdns.org
	 *     using resource
	 * /api/webhook/ianshefwebhook4588
	 *     with parameters
	 * ?subject=Test#1&message=Test202209041533
	 *
	 */

		batteryVoltage = ESP.getVcc()*(0.00112016306998) ;
		// NOTE:
		// ESP.getVcc() and NOT ESP.getVCC().

#if defined Ian_LocalDebugViaSocket
		debug.connect(Ian_LocalDebugAddress, Ian_LocalDebugSocket) ;

		debug.print("Connected to " ) ;
		debug.print(Ian_LocalDebugAddress) ;
		debug.print(" at port ") ;
		debug.print(Ian_LocalDebugSocket) ;
		debug.println(".") ;

		debug.printf("\nBattery voltage is %f volts.\n", batteryVoltage) ;
		debug.printf("Compiled on %s at %s\n\n", __DATE__, __TIME__) ;
		//
		// Sending "EOF_FOR_LOGGER" will close the connection
		// and prepare for a new connection.
		//
#endif

#if defined Ian_debug1

//		httpGet("45.17.221.124", "/", 21280) ; // TABLO at home.
		httpGet("12.153.148.59", "/", 80) ;    // www.smartmetertexas.com login
		Serial.printf("\nBattery voltage is %f volts.\n", batteryVoltage) ;
		Serial.printf("Compiled on %s %s\n\n", __DATE__, __TIME__) ;
		Serial.flush() ;

#elif defined Ian_noDebug1

		char request[REQUEST_SIZE] ;
		snprintf(
				request,
				REQUEST_SIZE,
//				"\"%s%#.2f (%s %s)\"",
				"%s%#.2f (%s %s)",
				triggerRequest,
				batteryVoltage,
				__DATE__,
				__TIME__
				) ;
//		Serial.printf(
//				"GETting from maker.ifttt.com using \n%s\n\n",
//				request,
//				) ;
		httpGet(
				"maker.ifttt.com",
				request,
				80,
				0
		) ;

#endif
		ESP.deepSleepInstant( 0, WAKE_RF_DEFAULT) ;
	} else {  // REPROGRAM OTA (Over The Air) using a web browser !
//		Serial.printf("\n\nOTA Reprogramming via a web browser !\n\n\n") ;
		//                   1         2         3         4
		//          1234567890123456789012345678901234567890
		MDNS.begin(otaHost) ;
		httpUpdater.setup(&httpServer) ;
		httpServer.onNotFound([]() {
			httpServer.send(200, "text/plain", "Go to /update.") ;
	    });
		httpServer.begin() ;

		MDNS.addService("http", "tcp", 80) ;

		Serial.printf( updateMessage, WiFi.localIP().toString().c_str()) ;
	}
}

//	  ESP.deepSleepInstant(microseconds, mode) will put the chip into deep sleep
//	  but sleeps instantly without waiting for WiFi to shutdown.
//	  mode is one of WAKE_RF_DEFAULT, WAKE_RFCAL, WAKE_NO_RFCAL,
//	  WAKE_RF_DISABLED.
//

// The loop function is called in an endless loop
void loop() {
	//
	// Nothing to do for normal execution.
	// (Deep Sleep should keep us from getting here.)
	//
	// However, need to do some work for OTA reprogramming.
	//
	if (executionMode==otaReprogrammingExecution) {
		httpServer.handleClient();
		MDNS.update();
	}
}

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

#if defined Ian_debug3
void scanNetworkSynchronous() {
	const char* formatString ;
	WiFi.mode(WIFI_STA);
	WiFi.disconnect();
	delay(100);
	WiFi.scanDelete();
	Serial.println("Network scan starting.");
	int numberOfNetworks = WiFi.scanNetworks();
	if (numberOfNetworks == 1) {
		formatString = "%d network found.\n" ;
	} else {
		formatString = "%d networks found.\n" ;
	}
	Serial.printf(formatString, numberOfNetworks) ;
	for (int i = 0; i < numberOfNetworks; i++)
	{
		Serial.printf(
				"%d: %s, Ch:%d (%ddBm) %s\n", i+1, WiFi.SSID(i).c_str(),
				WiFi.channel(i), WiFi.RSSI(i),
				WiFi.encryptionType(i) == ENC_TYPE_NONE ? "open" : ""
		);
	}
	WiFi.scanDelete();
	Serial.println();
}
#endif

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
	WiFi.mode(WIFI_STA) ;
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

	// attempt to connect to Wifi network:
//	if (WiFi.status() == WL_CONNECTED) {
//		Serial.printf("DEBUG >>>>>>>>  Already connected!\n") ;
//	}
	while (WiFi.status() != WL_CONNECTED) {
		unsigned long connectionStart = millis() ;

#ifdef Ian_Channel
	#if (Ian_Channel > 0) && (Ian_Channel < 12)
			WiFi.begin(
					writeableNetworkName,
					writeableNetworkPassword,
					Ian_Channel
			);
		#elif (Ian_Channel == 0)
			WiFi.begin(
					writeableNetworkName,
					writeableNetworkPassword
			);
		#else
			#error "Ian_Channel has bad value,"
			#errror "should be an integer in the range [0, 11]"
	#endif /* if (Ian_Channel > 0) && (Ian_Channel < 12) */
	#else
		#error "Ian_Channel should be defined and is not."
#endif /* ifdef Ian_Channel */

		// timed wait for connection
		while (
				(WiFi.status() != WL_CONNECTED) &&
				( (millis()-connectionStart) < CONNECTION_WAIT_MILLIS)
		){
			yield() ;
		}
		unsigned long connectionEnd = millis() ;
		unsigned long connectionTime = connectionEnd - connectionStart ;
		if (connectionTime < CONNECTION_WAIT_MILLIS) {
#if defined Ian_debug2
			Serial.printf(
				"DEBUG >>>>>>>>  Connection took %lu milliseconds.\n",
				connectionTime
			) ;
// https://www.arduino.cc/en/Reference/WiFiRSSI
			#endif
		} else {
#if defined Ian_debug2
			Serial.printf(
				"DEBUG >>>>>>>>  Failed to connect in %lu milliseconds.\n",
				connectionTime
			) ;
#endif
			WiFi.disconnect() ;  //  Reset and try again.
		}
	}
	/*
	typedef enum {
    WL_NO_SHIELD        = 255,   // for compatibility with WiFi Shield library
    WL_IDLE_STATUS      = 0,
    WL_NO_SSID_AVAIL    = 1,
    WL_SCAN_COMPLETED   = 2,
    WL_CONNECTED        = 3,
    WL_CONNECT_FAILED   = 4,
    WL_CONNECTION_LOST  = 5,
    WL_DISCONNECTED     = 6
} wl_status_t;
	 */
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
#if defined Ian_debug2
		Serial.println("Successful network connection.");
#endif
		break;
	case WL_NO_SSID_AVAIL:
		Serial.print("Failed to connect to network because ") ;
		Serial.println("configured SSID (network name) could not be reached.") ;
		break;
	case WL_CONNECT_FAILED:
		Serial.print("Failed to connect to network ") ;
		Serial.println("because password is incorrect.") ;
		break;
	case WL_IDLE_STATUS:
		Serial.println("WiFi status is changing.");
		break;
	case WL_DISCONNECTED:
		Serial.println("Failed to connect to network because") ;
		Serial.print("a specific MAC ID (BSSID) was requested ") ;
		Serial.println("that is not available, or") ;
		Serial.println("wait time was not long enough, or") ;
		Serial.println("unit is not configured for station mode.");
		break;
	default:
		Serial.println("Failed to connect to network due to unknown reason.");
	}
	if (status != WL_CONNECTED) {
		stayHere();
	}
}

//
// Default values for request, port and wait time are defined in the file
// MailNotifier.h
//
void httpGet(
		const char * server, const char * request, int port,
		int waitMillis) {
	//
	// Default port of 80 is used for web access but any port may be specified.
	// Default wait time of 3 seconds (3000 milliseconds) is used but any
	// wait time may be specified.  0 for the wait time means don't get
	// response from the server.
	//
	WiFiClient client ;

	if (client.connect(server, port)) {
//		Serial.printf("Connected to server %s:%d .\n", server, port) ;

		// Make a HTTP request:
		client.print("GET ") ;
		client.print(request) ;
		client.println(" HTTP/1.1") ;

		client.print("Host: ") ;
		client.print(server) ;
		client.print(":") ;
		client.println(port) ;

//		client.println("Connection: close");
		client.println();

		if (waitMillis>0) {
			delay(waitMillis) ;

			// If there are incoming bytes available
			// from the server, read them and print them:
			while (client.available()) {
				char c = client.read();
				Serial.write(c);
			}
		}
//		Serial.printf(
//				"Closing the connection with server %s:%d .\n", server, port
//				) ;
	} else {
		Serial.printf("Could not connect to server %s:%d .\n", server, port) ;
		stayHere() ;
	}
}

//
// Default values for request, port and wait time are defined in the file
// MailNotifier.h
//
void httpPostForHomeAssistant(
		WiFiClient c=NULL,
		const char * server="",
		const char * request="/",
		int port=80,
		int waitMillis = 3000
		) {
	//
	// Default port of 80 is used for web access but any port may be specified.
	// Default wait time of 3 seconds (3000 milliseconds) is used but any
	// wait time may be specified.  0 for the wait time means don't get
	// response from the server.
	//
	WiFiClient client ;

	if (c==NULL) {
		if (client.connect(server, port)) {
			//  Good case!  Do nothing.
			//	Serial.printf("Connected to server %s:%d .\n", server, port) ;
		} else {
			//
			// Bad case!  Failed to connect.
			// Try to display a message.
			//
#if defined Ian_LocalDebugViaSocket
			debug.printf("Could not connect to server %s:%d .\n", server, port) ;
			debug.printf("EOF_FOR_LOGGER") ;
#endif
			Serial.printf("Could not connect to server %s:%d .\n", server, port) ;
			stayHere() ;
		}
	} else {

		client=c ;

	}

#if defined Ian_LocalDebugViaSocket
		debug.printf("Ready for httpPostForHomeAssistant, ") ;
		debug.printf("connected to %s at port ", client.remoteIP().toString()) ;
		debug.print(client.remotePort()) ;
		debug.print(" for POST.") ;
		debug.println() ;
#endif


		// Make a HTTP POST request for Home Assistant Webhooks:
		client.print("POST ") ;
		client.print(request) ;
		client.println(" HTTP/2") ;

		client.print("Host: ") ;
		client.print(server) ;
		client.print(":") ;
		client.println(port) ;

		client.println("user-agent: MailboxNotifier/1.0.0") ;

		client.println("accept: */*") ;

		//		client.println("Connection: close");
		client.println();

		if (waitMillis>0) {
			delay(waitMillis) ;

			// If there are incoming bytes available
			// from the server, read them and print them:
			while (client.available()) {
				char c = client.read();
				Serial.write(c);
			}
		}
//		Serial.printf(
//				"Closing the connection with server %s:%d .\n", server, port
//				) ;

}
