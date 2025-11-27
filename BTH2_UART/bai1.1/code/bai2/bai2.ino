void setup() {
 
  Serial.begin(9600);
  
  
  while (!Serial) {
    ; // Chờ kết nối Serial port (chỉ cần cho boards có USB native)
  }
  
  
  delay(1000);
}

void loop() {
  
  Serial.println("Hello IoT!");
  
  
  delay(2000);
}
