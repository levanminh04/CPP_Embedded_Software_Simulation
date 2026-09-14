# Phần đảm nhiệm: Sensor/Event Simulation & Input Validation
### Smart Traffic Light Controller Simulator — Mock Project C++

---

## 1. Vai trò trong kiến trúc hệ thống

```
[SENSOR / EVENT GIẢ LẬP]  ←── Bạn đảm nhiệm
        │
        ▼
    CONTROLLER
        │
        ▼
  ACTUATOR / OUTPUT
        │
        ▼
  DISPLAY + LOGGER
```

Bạn là **nguồn đầu vào duy nhất** cho Controller. Controller không tự sinh dữ liệu — mọi quyết định (chuyển state, density, pedestrian request, emergency) đều dựa trên dữ liệu do module của bạn cung cấp. Vì vậy interface (chữ ký hàm, kiểu dữ liệu trả về) cần thống nhất sớm với bạn làm Controller.

---

## 2. Phạm vi công việc

### 2.1. Traffic Density Sensor (NS & EW — tách riêng 2 hướng)

- Sinh `vehicleCount` cho từng hướng, hỗ trợ 2 chế độ:
  - **AUTO**: chương trình tự random số xe
  - **MANUAL**: người dùng nhập từ bàn phím
- Phân loại density theo threshold (config, không hard-code):

| vehicleCount | Density |
|---|---|
| 0 – 5 | LOW |
| 6 – 15 | MEDIUM |
| ≥ 16 | HIGH |

- **Thời điểm đọc quan trọng**: chỉ đọc sensor **trước khi bắt đầu GREEN** của hướng đó, không đọc liên tục trong lúc GREEN đang chạy (để duration không đổi giữa chừng — đúng quy tắc deterministic FSM của nhóm).

### 2.2. Pedestrian Event (phím `P`)

- Bắt sự kiện nhấn phím `P` bất kỳ lúc nào trong vòng lặp
- Set flag `pedestrianRequest = true`
- **Không dùng queue**: nhấn P nhiều lần liên tiếp vẫn chỉ giữ 1 trạng thái "đang có người chờ", không cộng dồn số lần

### 2.3. Emergency Event (phím `E`)

- Bắt sự kiện nhấn phím `E`, độc lập với chu kỳ vehicle/pedestrian, có priority cao nhất

### 2.4. Input Validation (phần trọng tâm — bắt buộc tối thiểu 2 case lỗi theo đề)

| Tình huống | Cách xử lý |
|---|---|
| `vehicleCount` âm hoặc phi lý (VD: -5) | Không phân loại density; đánh dấu `valid = false`; log `SENSOR_ERROR`; dùng fallback an toàn (giữ density lượt trước hoặc mặc định LOW) |
| MANUAL mode: người dùng nhập ký tự không phải số (VD: "abc") | Validate trước khi convert; báo lỗi và yêu cầu nhập lại; không để chương trình crash |
| Phím nhập không hợp lệ (không phải P/E hoặc phím hệ thống định nghĩa) | Bỏ qua, không set sai flag |

---

## 3. Cấu trúc file (.h / .cpp tách riêng)

```
Types.h                     → enum class TrafficDensity, struct SensorReading
Sensor.h / Sensor.cpp       → class TrafficDensitySensor
EventSimulator.h / .cpp     → class EventSimulator (bắt phím P, E)
InputValidator.h / .cpp     → các hàm validate dùng chung
Config.h / Config.cpp       → threshold, giá trị mặc định (không hard-code rải rác)
```

---

## 4. Interface thống nhất với Controller

```cpp
// Types.h
enum class TrafficDensity { LOW, MEDIUM, HIGH };

struct SensorReading {
    int vehicleCount;
    TrafficDensity density;
    bool valid;       // false nếu sensor lỗi
};

// Sensor.h
class TrafficDensitySensor {
public:
    SensorReading readNS();
    SensorReading readEW();
private:
    int lowThreshold;   // đọc từ Config
    int highThreshold;  // đọc từ Config
};

// EventSimulator.h
class EventSimulator {
public:
    bool checkPedestrianRequest(); // true nếu vừa có phím P
    bool checkEmergency();         // true nếu vừa có phím E
};
```

> Nguyên tắc: Controller chỉ gọi `sensor.readNS()` / `event.checkPedestrianRequest()` mà không cần biết bên trong bạn random hay đọc bàn phím thế nào — đúng nguyên tắc tách module của đề.

---

## 5. Log cần bắn ra (phối hợp với Logger)

| Sự kiện | Khi nào |
|---|---|
| `PEDESTRIAN_REQUEST` | Khi phím P được nhấn |
| `SENSOR_ERROR` | Khi vehicleCount không hợp lệ |
| `TRAFFIC_DENSITY_CHANGE` | Khi mức density thay đổi giữa các lượt (nếu nhóm chọn log thêm) |
| `CONFIG_ERROR` | Nếu module của bạn cũng load config threshold và phát hiện giá trị vô lý |

---

## 6. Test case tối thiểu

| Test | Input | Kết quả mong đợi |
|---|---|---|
| T1 | vehicleCount = 3 | density = LOW |
| T2 | vehicleCount = 10 | density = MEDIUM |
| T3 | vehicleCount = 20 | density = HIGH |
| T4 | vehicleCount = -5 | valid = false, log SENSOR_ERROR, không set density |
| T5 (MANUAL) | nhập "abc" | báo lỗi, yêu cầu nhập lại, không crash |
| T6 | nhấn P nhiều lần liên tiếp | pedestrianRequest vẫn chỉ = true, không tăng biến đếm |
| T7 | nhấn E giữa lúc đang GREEN | checkEmergency() trả về true ngay lập tức |

---

## 7. Chuẩn trình bày code

- Dùng `enum class`, không dùng số magic (0, 1, 2) cho density/state
- Không hard-code threshold trong hàm — đưa hết vào `Config`
- Header (.h) chỉ khai báo, logic đặt trong .cpp (trừ hàm inline rất ngắn)
- Comment ngắn gọn giải thích **lý do** validate, không chỉ mô tả code làm gì
- Đặt tên hàm/biến khớp thuật ngữ trong tài liệu quy định của nhóm (`NS`, `EW`, `pedestrianRequest`, `SENSOR_ERROR`...) để cả nhóm đọc thống nhất

---

## 8. Việc cần làm trước khi bắt tay code

1. Chốt interface (mục 4) với bạn làm Controller — tránh phải sửa lại chữ ký hàm giữa chừng
2. Xác nhận với nhóm threshold mặc định: LOW 0–5, MEDIUM 6–15, HIGH ≥16 (mục 19 PDF quy định)
3. Xác nhận cơ chế đọc phím có chặn (blocking) hay không chặn (non-blocking) — ảnh hưởng cách vòng lặp `while(systemRunning)` chạy cùng lúc với countdown timer
