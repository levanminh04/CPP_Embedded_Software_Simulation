# Smart Traffic Light Controller Simulator

## Trạng thái dự án

Đây là repository của nhóm cho đề tài 2 trong Mock Project C++ mô phỏng phần mềm nhúng cơ bản.

Ngày khởi tạo repository: 12/09/2026.

Giai đoạn hiện tại: **Giai đoạn 0 — chốt môi trường và workflow**.

Chưa tạo `include/`, `src/`, `tests/` hoặc code ứng dụng trong giai đoạn này. Nhóm chỉ bắt đầu codebase sau khi sáu thành viên xác nhận cùng build được chương trình kiểm tra môi trường theo quy định bên dưới.

## Phạm vi nghiệp vụ đã thống nhất

Hệ thống mô phỏng một Controller chung cho một ngã tư gồm:

- NS (North/South) và EW (East/West) là hai luồng xe xung đột.
- Chu kỳ bình thường: `NS_GREEN → NS_YELLOW → ALL_RED → EW_GREEN → EW_YELLOW → ALL_RED → NS_GREEN`.
- `P` tạo một pedestrian request; request chỉ được phục vụ sau chuyển trạng thái an toàn.
- Khi pedestrian WALK/WARNING, cả NS và EW phải RED.
- `E` có ưu tiên cao nhất; Emergency phải đưa tất cả đèn về RED qua clearance phù hợp.
- Traffic density của NS và EW được mô phỏng riêng và chỉ quyết định thời lượng GREEN khi bắt đầu lượt đó.
- Mọi transition quan trọng, pedestrian request, Emergency và lỗi phải được ghi log.

Các quy tắc chi tiết lấy từ tài liệu đề bài và tài liệu định hướng của nhóm. Khi codebase bắt đầu, `docs/Requirement.md`, `docs/Design.md` và state diagram sẽ là nguồn sự thật cho implementation.

## Môi trường build chính thức

Môi trường chuẩn của nhóm được chốt như sau:

| Thành phần | Quy ước chính thức |
|---|---|
| Hệ điều hành kiểm tra chính | Windows 10/11 64-bit |
| Ngôn ngữ | C++17 |
| Compiler | w64devkit **v2.9.1**, gói x64 |
| GCC trong compiler chuẩn | GCC 16.2 theo release v2.9.1 |
| Build | Một script build của repository, dùng cùng flags cho mọi thành viên |
| Test | Một lệnh test chung, chạy trước khi mở Pull Request |
| Encoding | UTF-8 |
| Thư viện ứng dụng | C++ Standard Library; không dùng database, network, GUI hoặc phần cứng |

> Tài liệu phát hành chính thức: https://github.com/skeeto/w64devkit/releases/tag/v2.9.1

### Vì sao chọn compiler portable?

Portable chỉ là cách phân phối compiler: tải đúng archive, giải nén và dùng, không cần mỗi máy tự cài một bản MinGW khác nhau. Nó giúp nhóm giảm khác biệt phiên bản trong thời gian 10 ngày. IDE không cần giống nhau; thành viên có thể dùng VS Code, Code::Blocks hoặc IDE khác, miễn lệnh build chính thức của repository chạy được.

Compiler nằm **ngoài Git repository**. Không commit thư mục compiler, file `.exe` của compiler hoặc object file vào Git. Repository chỉ lưu source, script build, cấu hình và tài liệu. Nếu cần gửi bản chạy demo, nhóm tạo gói release riêng sau khi code hoàn tất.

### Cách mỗi thành viên xác nhận môi trường

1. Tải đúng bản `w64devkit-x64-2.9.1.7z.exe` từ trang release ở trên.
2. Giải nén vào một thư mục cá nhân, ví dụ `C:\tools\w64devkit-2.9.1`.
3. Mở terminal mới và kiểm tra:

```powershell
& 'C:\tools\w64devkit-2.9.1\bin\g++.exe' --version
```

Kết quả phải hiển thị GCC 16.2.x.

4. Gửi ảnh hoặc copy kết quả lệnh cho Minh. Minh ghi nhận vào bảng kiểm tra môi trường.
5. Khi codebase có script build, luôn gọi script đó với compiler chuẩn. Không dùng nút Run riêng của IDE để kết luận project đã build đúng.

Nếu đường dẫn trên máy khác, chỉ thay đường dẫn local. Không sửa source để chứa đường dẫn riêng của một máy.

### Điều kiện chốt môi trường

Giai đoạn 0 chỉ hoàn tất khi:

- Sáu thành viên xác nhận OS, IDE và compiler local.
- Sáu thành viên hiển thị đúng GCC 16.2.x.
- Một chương trình kiểm tra nhỏ dùng C++17, `std::chrono`, `std::this_thread::sleep_for` và `fstream` build/chạy được trên sáu máy.
- Minh tạo biên bản xác nhận trong `docs/ENVIRONMENT.md`.
- Reviewer khác Minh clone repository và chạy lại kiểm tra từ đầu.

Nếu một thành viên chưa cài được compiler, người đó vẫn có thể đọc tài liệu và chuẩn bị task, nhưng chưa nhận task code cần merge vào nhánh chính.

## Cấu trúc repository sau khi chốt môi trường

Sau khi Giai đoạn 0 được xác nhận, nhóm mới tạo cấu trúc codebase:

```text
traffic-light-simulator/
├── include/       # Header: interface và kiểu dữ liệu
├── src/           # Implementation .cpp
├── tests/         # Unit test và integration test
├── config/        # File cấu hình mẫu
├── docs/          # Requirement, design, diagram, test evidence
├── logs/          # Log chạy demo; không commit log tạm
├── main.cpp       # Điểm khởi chạy, không chứa business logic
├── build.ps1      # Lệnh build chính thức
├── run.ps1        # Lệnh chạy/test tiện dụng
├── README.md
└── .gitignore
```

## Workflow Git bắt buộc

`main` là nhánh ổn định. **Không thành viên nào được push thẳng vào `main`, kể cả trưởng nhóm.** Mọi thay đổi phải đi qua Pull Request.

### Nhánh

Mỗi thành viên dùng một nhánh riêng cho từng task:

```text
feature/input-event
feature/controller-normal-cycle
feature/output-display
feature/timing-logger
test/controller-cases
docs/requirement-state-diagram
fix/<mo-ta-ngan>
```

Không dùng một nhánh chung cho nhiều thành viên. Tên nhánh phải nói rõ mục đích, không đặt tên như `test`, `code-moi` hoặc `final`.

### Quy trình một task

```text
Task có mã và điều kiện hoàn thành
        ↓
git switch main; git pull
        ↓
Tạo nhánh riêng từ main mới nhất
        ↓
Code + test phần mình
        ↓
Tự build bằng môi trường chuẩn
        ↓
Commit thay đổi logic có ý nghĩa
        ↓
Push branch và mở Pull Request
        ↓
Reviewer đọc diff, chạy test và phản hồi
        ↓
Tác giả sửa review nếu có
        ↓
Minh kiểm tra trạng thái tích hợp và merge vào main
        ↓
Chạy lại build/test trên main
```

### Quy tắc Pull Request

- Một Pull Request có một mục đích chính.
- Mỗi PR phải ghi task/requirement, file thay đổi, cách kiểm tra và kết quả.
- Tác giả không tự approve PR của mình.
- Cần ít nhất một reviewer ngoài tác giả; thay đổi FSM hoặc interface dùng chung cần thêm Minh.
- Không merge nếu build/test liên quan thất bại.
- Nếu PR đổi state, event, config hoặc output, phải cập nhật tài liệu và test liên quan.
- Trước khi merge, tác giả cập nhật branch của mình từ `main` và chạy lại test.
- Không commit mật khẩu, token, đường dẫn cá nhân, compiler binary, `build/`, file object hoặc log tạm.

### Phân công và reviewer

| Thành viên | Phạm vi chính | Reviewer chính | Deadline đầu ra |
|---|---|---|---|
| Phạm Thị Ngân | Requirement, rule, test case, demo scenario, block/state diagram, test cuối | Trần Đức Trung; các chủ module xác nhận phần liên quan | hết 13/09 cho bản thiết kế đầu; cập nhật đến ngày 10 |
| **Lê Văn Minh** | Repository, phân quyền PR, skeleton, môi trường build, README, build/run, review và merge | Nguyễn Thanh Phong; Phùng Trung Kiên nếu ảnh hưởng Controller | hết 13/09 cho Giai đoạn 0 và skeleton sau khi môi trường được chốt |
| Trần Bá Lợi | Sensor/event giả lập và validate input | Phùng Trung Kiên | tối 17/09 |
| Phùng Trung Kiên | Logic Controller/FSM | Phạm Thị Ngân + Minh | tối 17/09 |
| Trần Đức Trung | Actuator/state/display | Trần Bá Lợi | tối 17/09 |
| Nguyễn Thanh Phong | Timer, logging, error handling | Minh | tối 17/09 |

Ngân có thể viết test case và sơ đồ trước khi code tồn tại. Test cuối chỉ được kết luận sau khi code tích hợp. Minh là người điều phối merge, không phải người viết thay module của các thành viên.

### Kiểm soát file dùng chung

Các file sau cần Minh điều phối vì nhiều module phụ thuộc:

- kiểu dữ liệu dùng chung (`Types.h`);
- giao diện giữa Input, Controller, Output, Config và Logger;
- file build và entry point;
- tài liệu thiết kế quyết định state/event.

Chủ module có quyền triển khai `.cpp` và test của mình. Muốn đổi header hoặc quy tắc nghiệp vụ phải mở task/PR riêng, mô tả module bị ảnh hưởng và chờ reviewer xác nhận.

## Tiêu chí nghiệm thu

Nhóm đối chiếu theo trọng số của đề:

| Tiêu chí | Bằng chứng cần có |
|---|---|
| Đúng chức năng — 30% | TL-01..TL-09, normal/pedestrian/emergency demo |
| C++ và cấu trúc — 20% | Header/source tách rõ, class/enum/vector/array, main điều phối |
| Tư duy embedded — 15% | Sensor/Event → Controller → Actuator/Output → Display/Logger; FSM và timing |
| Testing/robustness — 15% | Boundary, sensor/config/input error, invariant và log |
| Teamwork/Git — 10% | Branch riêng, PR, review, commit có ý nghĩa, không push thẳng main |
| Tài liệu/defense — 10% | Requirement, block/state diagram, test evidence, README; mọi thành viên trả lời được |

## Các bước ngay sau khi README được merge

1. Minh mở issue `ENV-01` và gửi README này vào nhóm.
2. Sáu thành viên phản hồi OS, IDE, đường dẫn compiler và kết quả `g++ --version`.
3. Minh tạo biên bản `docs/ENVIRONMENT.md` sau khi đủ sáu xác nhận.
4. Cả nhóm build/chạy chương trình kiểm tra môi trường trên branch `chore/environment-check`.
5. Một reviewer độc lập kiểm tra rồi merge PR môi trường.
6. Chỉ sau bước 5 Minh tạo skeleton `include/`, `src/`, `tests/` và các task code.

README này là quy ước làm việc của nhóm, không thay thế tài liệu yêu cầu nghiệp vụ. Khi tài liệu nghiệp vụ và code mâu thuẫn, dừng merge, ghi rõ mâu thuẫn trong issue và thống nhất quyết định trước khi sửa.
