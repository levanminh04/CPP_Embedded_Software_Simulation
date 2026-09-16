# Input & Sensor Simulation Module

Tài liệu này mô tả module Input/Sensor theo sơ đồ khối `docs/Block_Diagram.dot`:

```text
[Bàn phím] -> ConsoleInput::poll() 
                    |
                    v (Event)
               Application (Bộ điều phối)
              /           \
   (lệnh SENSOR_UPDATE)    (lệnh P / E / Q)
            v                     v
  TrafficDensitySensor        Controller
            |                     |
     (SensorReading)              |
            \                     /
             v                   v
      Application lấy mẫu trước GREEN -> Controller tính duration
```

## 1. Cấu trúc thư mục

```text
include/traffic/Types.h              Kiểu dữ liệu chung toàn project
include/traffic/Config.h             Cấu hình và ngưỡng mật độ
include/traffic/InputValidator.h     Hàm kiểm tra chuỗi số xe
include/traffic/Sensor.h             Class TrafficDensitySensor
include/traffic/ConsoleInput.h       Class ConsoleInput (đọc phím + buffer)

src/config/Config.cpp                Cài đặt cấu hình chung
src/input_sensor/InputValidator.cpp  Cài đặt validate số xe
src/input_sensor/Sensor.cpp          Cài đặt sensor NS/EW
src/input_sensor/ConsoleInput.cpp    Cài đặt buffer và poll bàn phím
```

## 2. Kiểu dữ liệu dùng chung (`Types.h`)

```cpp
enum class TrafficDensity {
    LOW,
    MEDIUM,
    HIGH
};

struct SensorReading {
    int vehicleCount = 0;
    TrafficDensity density = TrafficDensity::LOW;
    bool valid = true;
};
```

- `valid == false`: số xe âm hoặc sensor lỗi. Fallback an toàn là `TrafficDensity::LOW`.
- Logger ghi `SENSOR_ERROR` khi `valid == false`.

## 3. Class `ConsoleInput` (đầu vào bàn phím không chặn)

`ConsoleInput` gom ký tự vào buffer cho đến khi người dùng nhấn **Enter**. Nhờ vậy không bị nhầm giữa `E` (Emergency) và `EW 4` (Sensor Đông-Tây).

```cpp
ConsoleInput input;
Event event;

if (input.poll(event)) {
    // Chỉ vào đây khi người dùng đã nhấn Enter xong 1 lệnh
}
```

Hỗ trợ phím đặc biệt:
- Ký tự thông thường: thêm vào buffer.
- `Backspace` (`\b`): xóa 1 ký tự cuối.
- `Esc` (mã 27): hủy bỏ toàn bộ lệnh đang gõ trong buffer.
- `Enter` (`\r` hoặc `\n`): parse buffer thành `Event` và xóa sạch buffer.

Bảng ánh xạ lệnh sau khi nhấn Enter:

| Lệnh trong buffer | Event type | Hướng / Giá trị |
|---|---|---|
| `P` | `PEDESTRIAN_REQUEST` | - |
| `E` | `EMERGENCY_TOGGLE` | - |
| `Q` | `QUIT` | - |
| `NS <số xe>` | `SENSOR_UPDATE` | NS, vehicleCount |
| `EW <số xe>` | `SENSOR_UPDATE` | EW, vehicleCount |
| Sai cú pháp (`NS abc`, rỗng...) | `INVALID_INPUT` | - |

## 4. Class `TrafficDensitySensor` (giả lập cảm biến xe)

Sensor lưu riêng số xe 2 hướng NS và EW:

```cpp
TrafficDensitySensor sensor(Config::defaults());

// Application cập nhật khi có lệnh SENSOR_UPDATE:
SensorReading reading = sensor.update(event.direction, event.vehicleCount);

// Application hoặc Controller đọc giá trị trước khi bắt đầu GREEN:
SensorReading current = sensor.read(Direction::NS);
```

Ngưỡng phân loại mặc định:
- `< 0` xe: `valid = false`, density `LOW` (fallback an toàn).
- `0 - 5` xe: `LOW`
- `6 - 15` xe: `MEDIUM`
- `≥ 16` xe: `HIGH`

## 5. Phân định trách nhiệm theo Sơ đồ khối

- **`ConsoleInput`**: chỉ đọc phím và trả về `Event`. Không gọi Controller, không đổi đèn.
- **`Application` (main loop)**:
  1. Gọi `input.poll(event)`.
  2. Nếu là `SENSOR_UPDATE`: gọi `sensor.update(...)`. Nếu `!reading.valid` thì báo Logger ghi `SENSOR_ERROR`.
  3. Nếu là `PEDESTRIAN_REQUEST`, `EMERGENCY_TOGGLE`, `QUIT`: chuyển sang `controller.handleEvent(event)`.
  4. Ngay trước khi bắt đầu pha GREEN của một hướng: gọi `sensor.read(dir)` để lấy `SensorReading` đưa vào Controller tính thời lượng đèn xanh.
- **`Controller`**: **KHÔNG cần biết hàm `update()` của sensor**, không parse chuỗi console, chỉ nhận `Event` sự kiện và nhận `SensorReading` đã được kiểm tra hợp lệ để chuyển state FSM.

## 6. Biên dịch và kiểm thử

Biên dịch bằng PowerShell:

```powershell
.\build.ps1
.\build\traffic_tests.exe
```

Test tự động trong `tests/test_main.cpp` bao gồm:
- Phân loại mật độ `LOW`, `MEDIUM`, `HIGH`.
- Bắt lỗi số xe âm (`valid = false`).
- Bắt lỗi chuỗi chữ `"abc"`.
- Buffer bàn phím: gõ `E`, `W`, ` `, `4` chưa ra event; gõ tiếp `Enter` mới ra `SENSOR_UPDATE(EW, 4)`.
- Phân biệt độc lập giữa `E` + Enter và `EW 4` + Enter.
