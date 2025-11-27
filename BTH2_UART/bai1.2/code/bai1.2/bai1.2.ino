/*
 * Cải tiến:
 * - Luôn khởi động Serial ở 9600
 * - Khi vào chế độ CAU_HINH (long press >3s) -> đảm bảo Serial = 9600
 * - Nhả sau long press KHÔNG bị hiểu là click (ignoreNextRelease)
 * - Nhấn/thả ngắn trong CAU_HINH -> tăng clicks
 * - Sau timeout (1.5s) applyConfig: 1->9600, 2->115200
 */

const int BTN = 10;
const int LED = 13;

enum State { VAN_HANH, CAU_HINH };
State mode = VAN_HANH;

unsigned long debounceTime = 0;
unsigned long pressTime = 0;
unsigned long clickTime = 0;
unsigned long ledTime = 0;

const unsigned long DEBOUNCE_MS = 50;
const unsigned long LONGPRESS_MS = 3000;
const unsigned long CLICK_TIMEOUT_MS = 1500;

int btnState = HIGH;
int lastBtn = HIGH;
int clicks = 0;

bool longPressed = false;
bool ignoreNextRelease = false; // quan trọng: để tránh release sau longpress bị tính thành click

long savedBaud = 9600;   // lưu sau khi apply
long currentBaud = 9600; // baud đang dùng Serial (có thể tạm thời 9600 khi config)
bool configApplied = false;

void setup() {
  pinMode(BTN, INPUT_PULLUP);
  pinMode(LED, OUTPUT);

  savedBaud = 9600;
  currentBaud = 9600;
  Serial.begin(currentBaud);
  Serial.println("He thong khoi dong - CHE DO VAN HANH");
  Serial.print("Baud Rate: "); Serial.println(currentBaud);
}

void loop() {
  readButton();
  // LED blink: chậm ở VAN_HANH, nhanh ở CAU_HINH
  if (mode == VAN_HANH) {
    if (millis() - ledTime > 1000) { ledTime = millis(); digitalWrite(LED, !digitalRead(LED)); }
  } else {
    if (millis() - ledTime > 150) { ledTime = millis(); digitalWrite(LED, !digitalRead(LED)); }
    // nếu có clicks và timeout trôi -> apply
    if (clicks > 0 && millis() - clickTime > CLICK_TIMEOUT_MS) {
      applyConfig();
    }
  }
}

void readButton() {
  int reading = digitalRead(BTN);

  if (reading != lastBtn) {
    debounceTime = millis();
  }

  if (millis() - debounceTime > DEBOUNCE_MS) {
    // stable
    if (reading != btnState) {
      btnState = reading;
      if (btnState == LOW) {
        // nút vừa được nhấn xuống
        pressTime = millis();
        longPressed = false;
      } else { // btnState == HIGH -> nút vừa được nhả
        // nếu release sau longpress -> ignore
        if (ignoreNextRelease) {
          // bỏ qua release sinh do longpress
          ignoreNextRelease = false;
        } else {
          // release bình thường -> nếu đang ở CAU_HINH, xem là 1 click (short press)
          if (mode == CAU_HINH) {
            clicks++;
            clickTime = millis();
            Serial.print("Nhan lan: "); Serial.println(clicks);
          } else {
            // ở VAN_HANH, nhấn ngắn không làm gì (hoặc dùng cho khác)
          }
        }
      }
    }
  }

  // kiểm tra long press (khi nút đang giữ)
  if (btnState == LOW && !longPressed && (millis() - pressTime) >= LONGPRESS_MS) {
    longPressed = true;
    // đặt flag để bỏ qua release sau longpress
    ignoreNextRelease = true;
    // xử lý chuyển chế độ
    if (mode == VAN_HANH) {
      enterConfigMode();
    } else {
      exitConfigMode();
    }
  }

  lastBtn = reading;
}

void enterConfigMode() {
  Serial.println("\n=== VAO CHE DO CAU HINH ===");
  Serial.print("Luu tru truoc do: "); Serial.println(savedBaud);
  // BẮT BUỘC chuyển Serial tạm về 9600 để cấu hình
  if (currentBaud != 9600) {
    Serial.println("Chuyen Serial tam thoi sang 9600 de cau hinh...");
    Serial.flush();
    Serial.end();
    delay(30);
    currentBaud = 9600;
    Serial.begin(currentBaud);
    delay(30);
  }
  mode = CAU_HINH;
  clicks = 0;
  configApplied = false;
  Serial.println("Chon: 1 = 9600 | 2 = 115200");
  Serial.print("Dang o (tam thoi): "); Serial.println(currentBaud);
}

void exitConfigMode() {
  Serial.println("\n=== THOAT CHE DO CAU HINH ===");
  if (!configApplied) {
    // nếu không apply thì khôi phục savedBaud (nếu khác)
    if (currentBaud != savedBaud) {
      Serial.println("Khong ap dung cau hinh - khoi phuc baud truoc do...");
      Serial.flush();
      Serial.end();
      delay(30);
      currentBaud = savedBaud;
      Serial.begin(currentBaud);
      delay(30);
    }
  } else {
    // đã apply -> currentBaud == savedBaud
  }
  mode = VAN_HANH;
  clicks = 0;
  digitalWrite(LED, LOW);
  Serial.print("Baud hien tai: "); Serial.println(currentBaud);
}

void applyConfig() {
  if (clicks == 1) {
    Serial.println("Lua chon: 1 -> 9600");
    setBaudAndSave(9600);
  } else if (clicks == 2) {
    Serial.println("Lua chon: 2 -> 115200");
    setBaudAndSave(115200);
  } else {
    Serial.print("Lua chon khong hop le ("); Serial.print(clicks); Serial.println(").");
  }
  clicks = 0;
}

void setBaudAndSave(long newBaud) {
  Serial.print("Ap dung baud: "); Serial.println(newBaud);
  Serial.flush();
  Serial.end();
  delay(30);
  currentBaud = newBaud;
  Serial.begin(currentBaud);
  delay(30);
  savedBaud = newBaud;
  configApplied = true;
  Serial.println("=== DA AP DUNG ===");
  Serial.print("Baud moi: "); Serial.println(currentBaud);
  // phản hồi LED: 1 lần cho 9600, 2 lần cho 115200
  blinkLED((newBaud == 9600) ? 1 : 2);
  //digitalWrite(LED, LOW); delay(180);
}

void blinkLED(int n) {
  digitalWrite(LED, LOW); delay(200);
  // if(n = 1){
  //   for (int i = 0; i < 20; i++) {
  //   digitalWrite(LED, HIGH); delay(1000);
  //   digitalWrite(LED, LOW); delay(1000);
  // } }else 
  // {
  //   for (int i = 0; i < 20; i++) {
  //   digitalWrite(LED, HIGH); delay(300);
  //   digitalWrite(LED, LOW); delay(300);
  // }
  // }
  for (int i = 0; i < n; i++) {
    digitalWrite(LED, HIGH); delay(1000);
    digitalWrite(LED, LOW); delay(1000);
  }
  
}
