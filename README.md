# Input Sensor Module

Module này cung cấp dữ liệu đầu vào cho Smart Traffic Light Controller Simulator.

```text
Lệnh bàn phím -> EventSimulator -> Event
                                -> Controller

Số xe -> TrafficDensitySensor -> SensorReading -> Controller
```

Module chỉ nhận và kiểm tra dữ liệu. Nó **không** điều khiển FSM, không đổi đèn, không giữ trạng thái Emergency/Pedestrian và không tự ghi log.

## Cấu trúc file

```text
include/traffic/Types.h              Kiểu dữ liệu dùng chung của project
include/traffic/Config.h             Cấu hình và ngưỡng mật độ
include/traffic/InputValidator.h     Kiểm tra chuỗi số xe
include/traffic/Sensor.h             TrafficDensitySensor
include/traffic/EventSimulator.h     Chuyển lệnh thành Event
src/input_sensor/Config.cpp          Cài đặt Config
src/input_sensor/InputValidator.cpp  Cài đặt validate input
src/input_sensor/Sensor.cpp          Cài đặt sensor và phân loại mật độ
src/input_sensor/EventSimulator.cpp  Cài đặt parser lệnh
```

## Contract dữ liệu

Các kiểu này nằm trong namespace `traffic` và được Controller dùng trực tiếp.

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

`SensorReading` có ý nghĩa:

- `vehicleCount`: số xe của một hướng.
- `density`: mật độ do Sensor phân loại.
- `valid`: `false` khi số xe không hợp lệ, ví dụ `-5`.
- Khi `valid == false`, `density` là `LOW` làm fallback an toàn; Controller không được xem đó là mật độ LOW hợp lệ và nên ghi `SENSOR_ERROR`.

Các kiểu `Direction`, `EventType` và `Event` hiện có trong `Types.h` cũng là một phần contract với Controller.

## Config

```cpp
Config config = Config::defaults();
```

Giá trị mặc định:

| Thành phần | Giá trị |
|---|---:|
| `tickMs` | 1000 |
| `greenLowTime` | 15 |
| `greenMediumTime` | 20 |
| `greenHighTime` | 25 |
| `yellowTime` | 3 |
| `allRedTime` | 2 |
| `pedestrianWalkTime` | 10 |
| `pedestrianWarningTime` | 5 |
| `densityLowMax` | 5 |
| `densityMediumMax` | 15 |

`Config::isValid()` trả về `false` nếu:

- Một thời lượng nhỏ hơn hoặc bằng `0`.
- `densityLowMax` nhỏ hơn `0`.
- `densityMediumMax` không lớn hơn `densityLowMax`.

Module hiện chỉ cung cấp cấu hình mặc định và kiểm tra cấu hình. Việc đọc `config/default.cfg` chưa thuộc module này.

## InputValidator

```cpp
bool parseVehicleCount(const std::string& text, int& vehicleCount);
```

Hàm trả về `true` nếu toàn bộ chuỗi là một số nguyên và ghi kết quả vào `vehicleCount`.

| Input | Kết quả |
|---|---|
| `"20"` | `true`, `vehicleCount = 20` |
| `"-5"` | `true`, `vehicleCount = -5` |
| `"abc"` | `false` |
| `"20 abc"` | `false` |
| `"20.5"` | `false` |

`-5` được parse thành công vì đây là lỗi **giá trị**. Sensor mới là nơi quyết định số âm không hợp lệ. Ngược lại, `abc` là lỗi **cú pháp**, nên InputValidator từ chối ngay.

## TrafficDensitySensor

```cpp
TrafficDensitySensor sensor(Config::defaults());

SensorReading reading = sensor.update(Direction::NS, 20);
SensorReading current = sensor.read(Direction::NS);
```

### Hàm

| Hàm | Chức năng |
|---|---|
| `update(direction, vehicleCount)` | Kiểm tra, phân loại và lưu reading mới cho NS hoặc EW |
| `read(direction)` | Đọc reading đang lưu của NS hoặc EW |

### Quy tắc phân loại

| Số xe | `density` | `valid` |
|---:|---|---|
| Nhỏ hơn `0` | `LOW` fallback | `false` |
| `0..5` | `LOW` | `true` |
| `6..15` | `MEDIUM` | `true` |
| Từ `16` | `HIGH` | `true` |

Các ngưỡng lấy từ `Config`, không viết trực tiếp trong Controller.

### Quy tắc thời điểm đọc

Controller cập nhật sensor khi nhận `SENSOR_UPDATE`, nhưng chỉ lấy `read(direction)` ngay trước khi bắt đầu GREEN của hướng đó.

Nếu nhận số xe mới khi GREEN đang chạy, timer hiện tại không đổi. Giá trị mới chỉ ảnh hưởng lượt GREEN tiếp theo của cùng hướng.

## EventSimulator

```cpp
EventSimulator events;
Event event = events.parseLine("NS 20");
```

`parseLine()` chỉ phân tích lệnh và trả về `Event`; nó không xử lý nghiệp vụ.

| Lệnh | Event |
|---|---|
| `P` | `PEDESTRIAN_REQUEST` |
| `E` | `EMERGENCY_TOGGLE` |
| `Q` | `QUIT` |
| `NS 20` | `SENSOR_UPDATE`, hướng `NS`, số xe `20` |
| `EW 4` | `SENSOR_UPDATE`, hướng `EW`, số xe `4` |
| `NS abc` | `INVALID_INPUT` |
| `NS` | `INVALID_INPUT` |
| `NS 20 extra` | `INVALID_INPUT` |
| Lệnh khác | `INVALID_INPUT` |

Số âm như `NS -5` có cú pháp hợp lệ nên tạo `SENSOR_UPDATE`. Controller chuyển event đó cho Sensor; Sensor trả về `valid == false` để Controller/Logger xử lý `SENSOR_ERROR`.

## Cách Controller tích hợp

```cpp
Config config = Config::defaults();
TrafficDensitySensor sensor(config);
EventSimulator events;

Event event = events.parseLine(inputLine);

if (event.type == EventType::SENSOR_UPDATE) {
    SensorReading reading = sensor.update(event.direction, event.vehicleCount);

    if (!reading.valid) {
        // Ghi SENSOR_ERROR.
        // Không thay đổi timer GREEN hiện tại.
    }
}
```

Khi bắt đầu GREEN, Controller lấy mật độ đã lưu:

```cpp
SensorReading reading = sensor.read(Direction::NS);
int greenTime = config.greenLowTime;

if (reading.valid && reading.density == TrafficDensity::MEDIUM) {
    greenTime = config.greenMediumTime;
} else if (reading.valid && reading.density == TrafficDensity::HIGH) {
    greenTime = config.greenHighTime;
}
```

Controller vẫn là nơi quyết định state, timer, thứ tự NS/EW và xử lý các flag `pedestrianRequested`, `emergencyPending`.

## Kiểm thử

Build và chạy test từ PowerShell:

```powershell
.\build.ps1
.\build\traffic_tests.exe
```

Kết quả mong đợi:

```text
Build OK.
Input sensor tests passed.
```

Test trong `tests/test_main.cpp` bao phủ:

- Density LOW, MEDIUM, HIGH.
- Số xe âm trả `valid == false`.
- Chuỗi không phải số bị từ chối.
- Lệnh `P`, `E`, `NS abc`, `EW 4`.
