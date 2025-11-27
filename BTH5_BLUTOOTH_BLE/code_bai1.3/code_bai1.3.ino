/*
 * Bài 1.3: Điều khiển LED qua Bluetooth (Serial)
 * Board: Arduino Uno
 * Giao thức: Serial Communication (Baud 9600)
 */

String inputString = "";      

void setup() {
  
  Serial.begin(9600);
  
  // Đặt chân 13 là OUTPUT để điều khiển LED tích hợp
  pinMode(13, OUTPUT);
  
  // Ban đầu tắt LED
  digitalWrite(13, LOW);
  
  Serial.println("He thong san sang. Cho lenh 'on' hoac 'off'...");
}

void loop() {
  while (Serial.available())
  {
    delay(10);
    char c = Serial.read();
    inputString += c;
  }
  if (inputString.length() > 0) {
    
    inputString.trim();
    
    if (inputString == "on") {
      digitalWrite(13, HIGH); // Bật LED
      Serial.println("LED status: ON");
    } 
    else if (inputString == "off") {
      digitalWrite(13, LOW);  // Tắt LED
      Serial.println("LED status: OFF");
    }
    else {
      // Phản hồi nếu lệnh không hợp lệ
      Serial.print("Lenh khong hop le: ");
      Serial.println(inputString);
    }
    
    // Reset chuỗi để nhận lệnh mới
    inputString = "";
    
  }
}



