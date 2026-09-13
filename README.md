# Smart Traffic Light Controller Simulator

Ứng dụng C++17 chạy trên console, mô phỏng bộ điều khiển đèn giao thông theo luồng phần mềm nhúng:

```text
Input/Sensor -> Controller -> Output mô phỏng LED -> Display/Logger
```

## Môi trường

- Windows 10/11 x64.
- `g++` hỗ trợ C++17.
- PowerShell để chạy script build/run.

Kiểm tra compiler đang có:

```powershell
g++ --version
where.exe g++
```

Nếu hai lệnh trên nhận ra `g++`, không cần cài thêm compiler.

### Cài w64devkit khi chưa có compiler

1. Mở [w64devkit v2.9.1](https://github.com/skeeto/w64devkit/releases/tag/v2.9.1).
2. Trong **Assets**, tải `w64devkit-x64-2.9.1.7z.exe`.
3. Tạo `C:\tools`, chạy file vừa tải và giải nén để có:

```text
C:\tools\w64devkit\bin\g++.exe
```

4. Kiểm tra và build:

```powershell
& 'C:\tools\w64devkit\bin\g++.exe' --version
.\build.ps1 -Compiler 'C:\tools\w64devkit\bin\g++.exe'
```

Compiler đặt ngoài project và không commit lên Git.

## Build và chạy

```powershell
git clone https://github.com/levanminh04/CPP_Embedded_Software_Simulation.git
cd CPP_Embedded_Software_Simulation
.\build.ps1
.\run.ps1
```

Test chạy riêng bằng `.\run.ps1 -Test`, không tham gia luồng vận hành bình thường.

## Kiến trúc

Project dùng **super loop** kết hợp **Finite State Machine (FSM)**:

```text
main()
  -> Application
       -> Config: đọc thời lượng và ngưỡng
       -> Input::poll(): đọc ký tự, tạo Event
       -> Sensor: kiểm tra và lưu số xe NS/EW
       -> Timer: báo khi đủ 1 nhịp
       -> Controller: xử lý Event/tick, quyết định TrafficState
       -> Output: đổi TrafficState thành tín hiệu LED giả lập
       -> Display: vẽ trạng thái lên console
       -> Logger: ghi chuyển state, request, emergency và lỗi
```

`Application` điều phối các module. Input, Timer, Output và Display không tự quyết định state tiếp theo.

### Event và tick

`Event` là việc xảy ra từ bên ngoài:

```text
P     -> PEDESTRIAN_REQUEST
E     -> EMERGENCY_TOGGLE
NS 20 -> SENSOR_UPDATE(NS, 20)
EW 4  -> SENSOR_UPDATE(EW, 4)
Q     -> QUIT
```

`tick()` là một giây mô phỏng vừa trôi qua. Với `TICK_MS=1000`:

```text
đủ 1.000 ms thật -> gọi tick()
t=18, remaining=7 -> t=19, remaining=6
```

`steady_clock` chỉ đo khoảng thời gian, không phải ngày giờ `dd/mm/yyyy hh:mm:ss`.

## Nhập lệnh và sensor

`Input::poll()` kiểm tra bàn phím rồi trả về ngay, không đứng chờ Enter. Nếu vòng lặp nghỉ `10 ms`, bàn phím được kiểm tra khoảng `100 lần/giây`, trong khi Timer vẫn tiếp tục countdown.

Các ký tự được giữ trong buffer. Input chỉ phân tích khi nhấn Enter:

```text
P + Enter     -> yêu cầu người đi bộ
E + Enter     -> bật/tắt Emergency
Q + Enter     -> thoát chương trình
NS 20 + Enter -> cập nhật NS thành 20 xe
EW 4 + Enter  -> cập nhật EW thành 4 xe
```

Ví dụ nhập `NS 20`:

```text
N       -> buffer "N"
S       -> buffer "NS"
2, 0    -> buffer "NS 20"
Enter   -> tạo SENSOR_UPDATE(NS, 20)
```

Nhập `NS` rồi Enter tạo `INVALID_INPUT` vì thiếu số xe. Nó không thể bị nhầm với `P` vì parser so sánh toàn bộ buffer sau khi Enter. `Esc` xóa buffer; `Backspace` xóa một ký tự.

Input kiểm tra cú pháp, Sensor kiểm tra giá trị:

```text
NS ABC -> INVALID_INPUT
NS -5  -> SENSOR_ERROR
NS 20  -> hợp lệ, mật độ HIGH
```

Giá trị sensor mới chỉ áp dụng cho lượt GREEN kế tiếp. Ví dụ NS đang GREEN còn `7 giây`, nhập `NS 20` không thay đổi lượt hiện tại; lượt NS tiếp theo chạy `25 giây`.

## State, Output và Snapshot

`TrafficState` là quyết định nghiệp vụ. `LightOutput` là tín hiệu dành cho các LED giả lập:

```text
TrafficState = NS_GREEN
        |
        v
NS LED         = GREEN
EW LED         = RED
Pedestrian LED = RED
```

Output chỉ ánh xạ state sang LED. Display đổi giá trị LED thành chữ trên console.

`SystemSnapshot` là bản chụp chỉ đọc của Controller:

```text
simulationSecond    = 18
state               = NS_GREEN
remainingSeconds    = 7
nsVehicleCount      = 20
ewVehicleCount      = 4
pedestrianRequested = false
emergencyPending    = false
```

Application phối hợp dữ liệu:

```cpp
const SystemSnapshot snapshot = controller.snapshot();
const LightOutput lights = output.fromState(snapshot.state);
display.show(snapshot, lights);
```

Display không nhận quyền gọi `tick()`, xử lý Event hoặc thay đổi FSM.

## Console dự kiến

Màn hình được ghi đè tại cùng một vị trí, không in thêm một dòng sau mỗi giây:

```text
===== SMART TRAFFIC CONTROLLER =====
Simulation Time      : 18 s
State                : NS_GREEN
NS Vehicle LED       : GREEN
EW Vehicle LED       : RED
Pedestrian LED       : RED
Remaining Time       : 7 s
Pedestrian Req.      : NO
Emergency            : OFF
Traffic NS / EW      : 20 / 4
Command              : NS 2_
====================================
Nhập lệnh rồi nhấn Enter
P = Pedestrian request | E = Emergency ON/OFF | Q = Quit
NS 20 = NS sensor     | EW 4 = EW sensor
```

Output mô phỏng LED giao thông; Display chỉ trình bày các LED đó trên console.

## Cấu hình mặc định

```ini
TICK_MS=1000
GREEN_LOW_TIME=15
GREEN_MEDIUM_TIME=20
GREEN_HIGH_TIME=25
YELLOW_TIME=3
ALL_RED_TIME=2
PED_WALK_TIME=10
PED_WARNING_TIME=5
DENSITY_LOW_MAX=5
DENSITY_MEDIUM_MAX=15
```

```text
0-5 xe  -> LOW    -> GREEN 15 giây
6-15 xe -> MEDIUM -> GREEN 20 giây
>=16 xe -> HIGH   -> GREEN 25 giây
```

Sau khi luồng tương tác ổn định, có thể bổ sung nguồn Event để chạy demo tự động. Nguồn mới phải dùng cùng `Event` và `Controller::apply()`, không tạo FSM thứ hai.

## Workflow Git

- Không push trực tiếp lên `main`.
- Mỗi task dùng một branch riêng, ví dụ `feature/input-sensor`.
- Mỗi branch tạo một Pull Request; tác giả không tự approve.
- Chỉ merge khi build và test đạt.

```powershell
git switch -c feature/<module>
git add <file-thuoc-module>
git commit -m "feat: <mo-ta-ngan>"
git push -u origin feature/<module>
```

Không dùng `git add .` khi workspace có file chưa hoàn thiện của module khác.
