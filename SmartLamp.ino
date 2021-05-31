#include <ESP8266WebServer.h>

const char* ssid = "LernaSpot";
const char* password = "BananeMoca";

//pinii folositi
const int echoPin = D5; 
const int trigPin = D3;
const int blue = D8;

long duration;
double distance;

ESP8266WebServer server(5005);

//HTML page
String page =
R"(
<html>  
  <head> 
    <script src='https://code.jquery.com/jquery-3.3.1.min.js'></script>
    <link rel='stylesheet' href='https://use.fontawesome.com/releases/v5.1.0/css/all.css'/>
    <link rel='stylesheet' href='https://stackpath.bootstrapcdn.com/bootstrap/4.1.2/css/bootstrap.min.css'/>
    <title>Proiect Embedded</title>
  <style>
    html {
    min-height: 100%;
    }

    body {
      margin: 0;
      background: rgb(96,92,60);
      background: linear-gradient(180deg, rgba(96,92,60,1) 0%, rgba(60,59,63,1) 100%); 
    }

    header, p , form, h2 {
      text-align: center;
      margin-left: 10%;
      margin-right: 10%;
    }

    footer {
      text-align: center;
      border-radius: 15px;
      bottom: 15px;
    }
  </style>
  </head> 

  <body>
  <center>
  <br>
  <br>
  <br>
  <br>
  <br>
  <br>
  <br>
  <br>
  <br>
    <table style='font-size:20px; color:white;'>  
      <tr>  
        <td> 
          <div>Current Distance</div>
        </td>
       </tr>
       <tr>
        <td>
          <center>
          <div id='Distance' style='font-size:3vh;'></div>
          </center>
        </td>
       </tr> 
    </table> 
    </center>
  </body> 
  
  <script> 
   $(document).ready(function(){ 
     setInterval(refreshFunction, 1000); 
   });

   function refreshFunction(){ 
       $.get('/refresh', function(result){  
         $('#Distance').html(result);
         updateDimLed(result)
     });
   }

   function updateDimLed(result){
    var dimValue = 1000 - result;
    console.log(dimValue);
    console.log(dimValue / 50);

    $.ajax({
      url: '/change',
      type: 'POST',
      data: {dimState: dimValue}
       });
   }
  </script> 
</html> 
)";

//functie pentru a schimba luminozitatea becului in functie de distanta curenta
void changeDimLED()
{
  int dimValue = server.arg("dimState").toInt();
  dimValue = map(dimValue / 50, 0, 100, 0, 1023);

  if(dimValue <= 0)
     digitalWrite(blue, LOW); //distanta e prea mare -> stingi becul
  else
     analogWrite(blue, dimValue); //schimba valoarea luminozitatii ledului pe baza distantei
  
  server.send(200, "text/html", "blue");
}

void refresh()
{ 
  char messageFinal[100];
  sprintf(messageFinal, "%.2f", distance);
  server.send(200, "application/javascript", messageFinal);
}

void htmlIndex() 
{
  server.send(200, "text/html", page);
}

void connectToWiFi()
{ 
  Serial.println("Connecting to the WiFi");

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  Serial.println("Waiting for connection");
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }

  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(ssid);
  
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
  
  Serial.println("With info: ");
  WiFi.printDiag(Serial);
}

void setupServer()
{ 
  server.on("/", htmlIndex);
  server.on("/refresh", refresh);
  server.on("/change", changeDimLED);

  server.begin();

  Serial.println("HTTP server started");
}

void setup() 
{
  //setare pins pentru
  //pinul de led e pus pe output by default
  //iar, pentru modulul ultrasonic, piniul de trigger trebuie pus obligatoriu pe output si echo pus pe input
  pinMode(blue, OUTPUT);
  pinMode(trigPin, OUTPUT); 
  pinMode(echoPin, INPUT); 

  Serial.begin(115200);

  delay(1000);

  connectToWiFi();
  setupServer();
}
 
void loop() 
{
  server.handleClient();

  //pregatire pentru citirea distantei
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);

  //genereaza un ultrasoundwave pentru a masura distanta
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  //calculeaza distanta bazata pe durata de traversare a ultrasound wave
  duration = pulseIn(echoPin, HIGH);
  distance = duration * 0.034 / 2;
}
