
  #include <Wire.h>
#include <RTClib.h>
#include <LiquidCrystal_I2C.h>

// --- Khởi tạo đối tượng ---
LiquidCrystal_I2C lcd(0x27, 16, 2); 
RTC_DS1307 rtc;

// --- CẤU HÌNH CHÂN KẾT NỐI ---
const int PIN_BTN_MODE = 2;
const int PIN_BTN_UP   = 3;
const int PIN_BTN_DOWN = 4;
const int PIN_BTN_SET  = 5;
const int PIN_BUZZER   = 6;

// --- Biến toàn cục ---
enum TrangThaiCheDo {
  XEM_GIO,
  CAI_BAO_THUC_GIO,
  CAI_BAO_THUC_PHUT,
  CAI_GIO_THUC_TE_GIO,
  CAI_GIO_THUC_TE_PHUT
};

TrangThaiCheDo cheDoHienTai = XEM_GIO;

int gioBaoThuc = 7;
int phutBaoThuc = 30;
bool baoThucBat = true;
bool dangKeu = false;

int tamGio, tamPhut;

unsigned long thoiGianNhanMode = 0;
bool giuMode = false;

// --- KHAI BÁO HÀM ---
void xuLyNutNhan();
void doiCheDo();
void tangGiaTri();
void giamGiaTri();
void nutSet();
void inSo(int so);
void hienThiGioThucTe();
void hienThiCaiBaoThuc();
void hienThiCaiGioThucTe();
void kiemTraBaoThuc();
void keuBaoThuc();
void tatBaoThuc();

void setup() {
  pinMode(PIN_BTN_MODE, INPUT_PULLUP);
  pinMode(PIN_BTN_UP, INPUT_PULLUP);
  pinMode(PIN_BTN_DOWN, INPUT_PULLUP);
  pinMode(PIN_BTN_SET, INPUT_PULLUP);
  pinMode(PIN_BUZZER, OUTPUT);
  digitalWrite(PIN_BUZZER, LOW);

  lcd.init();
  lcd.backlight();

  if (!rtc.begin()) {
    lcd.print("Loi RTC!");
    while (1);
  }

  if (!rtc.isrunning()) {
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  }
}

void loop() {
  xuLyNutNhan();

  if (dangKeu) {
    keuBaoThuc();
  } else {
    switch (cheDoHienTai) {
      case XEM_GIO:
        hienThiGioThucTe();
        kiemTraBaoThuc();
        break;
      case CAI_BAO_THUC_GIO:
      case CAI_BAO_THUC_PHUT:
        hienThiCaiBaoThuc();
        break;
      case CAI_GIO_THUC_TE_GIO:
      case CAI_GIO_THUC_TE_PHUT:
        hienThiCaiGioThucTe();
        break;
    }
  }
  delay(50);
}

// --- HÀM XỬ LÝ NÚT NHẤN ---
void xuLyNutNhan() {
  int trangThaiMode = digitalRead(PIN_BTN_MODE);
  
  if (trangThaiMode == LOW) {
    if (thoiGianNhanMode == 0) thoiGianNhanMode = millis();
    if (millis() - thoiGianNhanMode > 2000 && !giuMode && cheDoHienTai == XEM_GIO) {
      baoThucBat = !baoThucBat;
      giuMode = true;
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print(baoThucBat ? "BAO THUC: ON " : "BAO THUC: OFF");
      delay(1000);
      lcd.clear();
    }
  } else {
    if (thoiGianNhanMode > 0) {
      if (!giuMode && millis() - thoiGianNhanMode < 1000) {
        if (dangKeu) {
          tatBaoThuc();
        } else {
          doiCheDo();
        }
      }
      thoiGianNhanMode = 0;
      giuMode = false;
    }
  }

  if (digitalRead(PIN_BTN_UP) == LOW) {
    tangGiaTri();
    delay(200);
  }
  if (digitalRead(PIN_BTN_DOWN) == LOW) {
    giamGiaTri();
    delay(200);
  }
  if (digitalRead(PIN_BTN_SET) == LOW) {
    nutSet();
    delay(300);
  }
}

void doiCheDo() {
  lcd.clear();
  switch (cheDoHienTai) {
    case XEM_GIO:
      cheDoHienTai = CAI_BAO_THUC_GIO;
      tamGio = gioBaoThuc;
      tamPhut = phutBaoThuc;
      break;
    case CAI_BAO_THUC_GIO:
    case CAI_BAO_THUC_PHUT:
      cheDoHienTai = CAI_GIO_THUC_TE_GIO;
      {
        DateTime now = rtc.now();
        tamGio = now.hour();
        tamPhut = now.minute();
      }
      break;
    default:
      cheDoHienTai = XEM_GIO;
      break;
  }
}

void tangGiaTri() {
  if (cheDoHienTai == CAI_BAO_THUC_GIO || cheDoHienTai == CAI_GIO_THUC_TE_GIO) {
    tamGio++;
    if (tamGio > 23) tamGio = 0;
  } else if (cheDoHienTai == CAI_BAO_THUC_PHUT || cheDoHienTai == CAI_GIO_THUC_TE_PHUT) {
    tamPhut++;
    if (tamPhut > 59) tamPhut = 0;
  }
}

void giamGiaTri() {
  if (cheDoHienTai == CAI_BAO_THUC_GIO || cheDoHienTai == CAI_GIO_THUC_TE_GIO) {
    tamGio--;
    if (tamGio < 0) tamGio = 23;
  } else if (cheDoHienTai == CAI_BAO_THUC_PHUT || cheDoHienTai == CAI_GIO_THUC_TE_PHUT) {
    tamPhut--;
    if (tamPhut < 0) tamPhut = 59;
  }
}

void nutSet() {
  DateTime now = rtc.now();
  switch (cheDoHienTai) {
    case CAI_BAO_THUC_GIO:
      cheDoHienTai = CAI_BAO_THUC_PHUT;
      break;
    case CAI_BAO_THUC_PHUT:
      gioBaoThuc = tamGio;
      phutBaoThuc = tamPhut;
      baoThucBat = true;
      cheDoHienTai = XEM_GIO;
      lcd.clear();
      lcd.print("Da Luu Bao Thuc!");
      delay(1000);
      lcd.clear();
      break;
    case CAI_GIO_THUC_TE_GIO:
      cheDoHienTai = CAI_GIO_THUC_TE_PHUT;
      break;
    case CAI_GIO_THUC_TE_PHUT:
      rtc.adjust(DateTime(now.year(), now.month(), now.day(), tamGio, tamPhut, 0));
      cheDoHienTai = XEM_GIO;
      lcd.clear();
      lcd.print("Da Luu Gio!");
      delay(1000);
      lcd.clear();
      break;
  }
}

void inSo(int so) {
  if (so < 10) lcd.print('0');
  lcd.print(so);
}

void hienThiGioThucTe() {
  DateTime now = rtc.now();
  lcd.setCursor(0, 0);
  lcd.print("Time: ");
  inSo(now.hour()); lcd.print(":");
  inSo(now.minute()); lcd.print(":");
  inSo(now.second());

  lcd.setCursor(0, 1);
  lcd.print(baoThucBat ? "ON " : "OFF ");
  lcd.print("Hen:");
  inSo(gioBaoThuc); lcd.print(":");
  inSo(phutBaoThuc);
}

void hienThiCaiBaoThuc() {
  lcd.setCursor(0, 0);
  lcd.print("CAI BAO THUC");
  lcd.setCursor(0, 1);
  
  if (cheDoHienTai == CAI_BAO_THUC_GIO) lcd.print(">");
  else lcd.print(" ");
  inSo(tamGio);
  lcd.print(":");
  
  if (cheDoHienTai == CAI_BAO_THUC_PHUT) lcd.print(">");
  else lcd.print(" ");
  inSo(tamPhut);
}

void hienThiCaiGioThucTe() {
  lcd.setCursor(0, 0);
  lcd.print("CAI GIO THUC TE");
  lcd.setCursor(0, 1);
  
  if (cheDoHienTai == CAI_GIO_THUC_TE_GIO) lcd.print(">");
  else lcd.print(" ");
  inSo(tamGio);
  lcd.print(":");
  
  if (cheDoHienTai == CAI_GIO_THUC_TE_PHUT) lcd.print(">");
  else lcd.print(" ");
  inSo(tamPhut);
}

void kiemTraBaoThuc() {
  if (!baoThucBat) return;
  DateTime now = rtc.now();
  if (now.hour() == gioBaoThuc && now.minute() == phutBaoThuc && now.second() == 0) {
    dangKeu = true;
  }
}

void keuBaoThuc() {
  lcd.setCursor(0, 0);
  lcd.print("DEN GIO ROI!!!!");
  lcd.setCursor(0, 1);
  lcd.print("Bam MODE de tat");
  
  digitalWrite(PIN_BUZZER, HIGH);
  delay(1000);
  digitalWrite(PIN_BUZZER, LOW);
  delay(1000);
}

void tatBaoThuc() {
  dangKeu = false;
  digitalWrite(PIN_BUZZER, LOW);
  lcd.clear();
}
