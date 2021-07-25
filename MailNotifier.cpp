// Do not remove the include below
#include "MailNotifier.h"

#include "ESP8266WiFi.h"

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

/*
  Blink without Delay (BWOD)

  Turns on and off a light emitting diode (LED) connected to a digital pin,
  without using the delay() function. This means that other code can run at the
  same time without being interrupted by the LED code.

  The circuit:
  - Use the onboard LED.
  - Note: Most Arduinos have an on-board LED you can control. On the UNO, MEGA
    and ZERO it is attached to digital pin 13, on MKR1000 on pin 6. LED_BUILTIN
    is set to the correct LED pin independent of which board is used.
    If you want to know what pin the on-board LED is connected to on your
    Arduino model, check the Technical Specs of your board at:
    https://www.arduino.cc/en/Main/Products

  created 2005
  by David A. Mellis
  modified 8 Feb 2010
  by Paul Stoffregen
  modified 11 Nov 2013
  by Scott Fitzgerald
  modified 9 Jan 2017
  by Arturo Guadalupi

  This example code is in the public domain.

  http://www.arduino.cc/en/Tutorial/BlinkWithoutDelay
*/

//
//
//
ADC_MODE(ADC_VCC) ;  // Self VCC Read Mode
//
// NOTE that this line (above) must be OUTSIDE of any function.
//

// constants won't change. Used here to set a pin number:
const int ledPin =  LED_BUILTIN;// the number of the LED pin

// Variables will change:
int ledState = LOW;             // ledState used to set the LED

// Generally, you should use "unsigned long" for variables that hold time
// The value will quickly become too large for an int to store
unsigned long previousMillis = 0 ;       // will store last time LED was updated
unsigned long lastScanMillis = 0 ;       // will store time of last Wifi scan.
unsigned long scanStartMillis ;  // Start time of a scan, in milliseconds.
unsigned long scanEndMillis   ;  // End   time of a scan, in milliseconds.

boolean success ;
int     status  ;

// constants won't change:
const char* ssid     = "*" ; // Replace * by the name (SSID) for your network.
const char* password = "*" ; // Replace * by the password    for your network.
const int bssid = 0 ;
const byte channel[] = {0} ;
const boolean establishConnection = true ;

IPAddress localIp( 192, 168,   1,  60) ;
IPAddress gateway( 192, 168,   1, 254) ;
IPAddress subnet ( 255, 255, 255,   0) ;
IPAddress dns1   (  68,  94, 156,   9) ;
IPAddress dns2   (  68,  94, 157,   9) ;

const unsigned long interval    =  250 ;// LED blink interval (milliseconds).
const unsigned long SCAN_PERIOD = 5000 ;// Wifi scan interval (milliseconds).
const boolean async = true ; // Scan in the background.
const boolean showHidden = true ; // Scan shows hidden networks.

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
	Serial.print("Input:  ") ;
	Serial.print(textPointer) ;
	Serial.print(" ") ;
	for (char *i = (char *)text ; *i != 0 ; i++) {
		Serial.printf(" %#2.2hhX", *i++) ;
	}
	Serial.println() ;
	byte index = 0 ;
	while(*textPointer) {
		*textPointer = 0x7F & (*textPointer - (3 & index++)) ;
		textPointer++ ;
	}

	Serial.print("Output: ") ;
	Serial.print(text) ;
	Serial.print(" ") ;
	for (char *i = (char *)text ; *i != 0 ; i++) {
		Serial.printf(" %#2.2hhX", *i++) ;
	}
	Serial.println() ;
	Serial.println() ;
}

void setup()
{
	  // Serial
	  Serial.begin(115200) ;
	  unsigned long preparing ;
	  unsigned long const waitTime  = 2000 ; // milliseconds

	  preparing = millis() ;
	  while (!delayingIsDone(preparing, waitTime)) {}

	  Serial.println("Serial has been set up.") ;
	  //
	  // Version information.
	  //
	  Serial.print("Compiled on ") ;
	  Serial.print(__DATE__) ;
	  Serial.print(" at ") ;
	  Serial.print(__TIME__) ;
	  Serial.println(" local time.") ;

	  //
	  // Read the private strings
	  //
#include "SSID.private"
	  simpleDecrypt(ssid) ;
	  simpleDecrypt(password) ;
	  while(true) yield() ;
	  //
	  // End of "Read the private strings"
	  //

	  // set the digital pin as output:
	  pinMode(ledPin, OUTPUT);

	  WiFi.mode(WIFI_STA);
		/*
		 * Access by a station...
		 *
		 * Gateway local address is 192.168.1.254
		 * Use 192.168.1.60 for MailboxNotifier IP address.
		 * Subnet Mask 255.255.255.0
		 * Primary   DNS 68.94.156.9
		 * Secondary DNS 68.94.157.9
		 *
		 */
	  success = WiFi.config(localIp, gateway, subnet, dns1, dns2) ;
	  if (!success) {
		  Serial.println("Could not configure.") ;
		  while (true) ;
	  }
	  WiFi.begin(ssid, password, bssid, channel, establishConnection) ;
	  status = WiFi.status() ;
	  switch (status) {
	  WL_CONNECTED:
	  Serial.println
	  ("Successful connection.") ;
	  break ;

	  WL_NO_SSID_AVAIL:
	  Serial.println
	  ("Failed to connect because configured SSID could not be reached.") ;
	  break ;

	  WL_CONNECT_FAILED:
	  Serial.println
	  ("Failed to connect because password is incorrect.") ;
	  break ;

	  WL_IDLE_STATUS:
	  Serial.println
	  ("WiFi status is changing.") ;
	  break ;

	  WL_DISCONNECTED:
	  Serial.println
	  ("Failed to connect because unit is not configure for station mode.") ;
	  break ;

	  default:
		  Serial.println
		  ("Failed to connect due to unknown reason.") ;
		  break ;

	  }
	  if (status != WL_CONNECTED) {
		  while (true) ;
	  }
	  WiFi.disconnect();
}

/*
 * These functions are used for isolating Eclipse errors into a single place.
 * These Eclipse errors are not real errors.
 */

//void ssidAsCString(int i, char buf[32]) {
//
//	The following two lines should not be collapsed into one line until
//	there is an update in Sloeber from Beryllium to another version of Eclipse
//	that better handles these functions.
//
//	String ssid = WiFi.SSID(i);
//	const char * tmp = ssid.c_str() ;
//
//	strncpy(buf,tmp,32) ;
//}

//int channelNumber(int i) {
//	return WiFi.channel(i) ;
//}
//
//int signalStrength (int i) {
//	return WiFi.RSSI(i) ;
//}

// The loop function is called in an endless loop
void loop()
{
	  // here is where you'd put code that needs to be running all the time.

	  // check to see if it's time to blink the LED; that is, if the difference
	  // between the current time and last time you blinked the LED is bigger than
	  // the interval at which you want to blink the LED.
	if (delayingIsDone(previousMillis, interval)) {
	    // if the LED is off turn it on and vice-versa:
		ledState = ledState==LOW?HIGH:LOW ;

//		const float VCC_ADJ = 1.096/1000.0 * (3.27/3.24)  ;
//		const float VCC_ADJ = 0.001106148148148  ;
//		int voltageCount = ESP.getVcc() ;

	    digitalWrite(ledPin, ledState);

//	    Serial.print("Pin ") ;
//	    Serial.print(ledPin) ;
//	    Serial.print(" has been set to ") ;
//	    Serial.print(ledState==LOW?" LOW.":"HIGH.") ;
//	    Serial.print("  Voltage reading is ") ;
//	    Serial.print(voltageCount) ;
//	    Serial.print(" which may be ") ;
//	    Serial.print(VCC_ADJ*voltageCount) ;
//	    Serial.println(" volts.") ;
	  }
//	if (delayingIsDone(lastScanMillis, SCAN_PERIOD)) {
//	    WiFi.scanNetworks(async, showHidden) ;
//	    scanStartMillis = millis() ;
//	    Serial.print("\nScan start ...\n") ;
//	}
//
//	  int n = WiFi.scanComplete();
//	  /*
//	   *
//	   * If
//	   * n>0 or n==0 then n is the number of networks found.
//	   * n==-1 then scanning is still in progress.
//	   * n==-2 then scanning has not been triggered.
//	   *
//	   */
//	  if(n >= 0) {
//		  scanEndMillis = millis() ;
//	    Serial.printf("\n\n%2d network(s) found in %5lu. milliseconds.\n\n",
//	    		n,
//				scanEndMillis-scanStartMillis
//				);
//	    Serial.println(
//	    		"Name                             RSSI Channel Encryption       Availability") ;
//	    Serial.println(
//	    		"                                 (dBm) Number   Type") ;
//	    Serial.println(
//	    		"================================ ==== ======= ================ ============") ;
//	    for (int i = 0; i < n; i++) {
//	    	const char * encryption ;
//	    	switch (WiFi.encryptionType(i)) {
//	    	case ENC_TYPE_NONE: encryption = "None" ;
//	    	break ;
//	    	case ENC_TYPE_WEP:  encryption = "WEP" ;
//	    	break ;
//	    	case ENC_TYPE_TKIP: encryption = "WPA  / PSK" ;
//	    	break ;
//	    	case ENC_TYPE_CCMP: encryption = "WPA2 / PSK" ;
//	    	break ;
//	    	case ENC_TYPE_AUTO: encryption = "WPA2 / PSK / WPA" ;
//	    	break ;
//	    	default: encryption = "unknown" ;
//	    	}
//	    	char ssidBuffer[32] ;
//	    	ssidAsCString(i,ssidBuffer) ;
//	      Serial.printf("%-32s %4d %4d    %-16s %-s\n",
//	    		  i+1,
//	    		  ssidAsCString(i),
//				  WiFi.SSID(i).c_str(),
//				  ssidBuffer,
//				  signalStrength(i),
//	    		  channelNumber(i),
//				  encryption,
//				  WiFi.isHidden(i)?"Hidden":""
//				  ) ;
//	    }
//	    WiFi.scanDelete();
//	  }
}
