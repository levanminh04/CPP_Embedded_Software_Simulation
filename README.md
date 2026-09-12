# Smart Traffic Light Controller Simulator

## Trạng thái

Giai đoạn 1: đã có codebase tối thiểu và chương trình Hello World. Chưa triển khai logic đèn giao thông.

## Môi trường chuẩn

| Hạng mục | Quy định |
|---|---|
| Hệ điều hành | Windows 10/11 x64 |
| Ngôn ngữ | C++17 |
| Compiler ưu tiên | `g++` C++17 có sẵn trên máy |
| Build/test | `build.ps1` và `run.ps1` |

### Kiểm tra compiler hiện có

Mở PowerShell và chạy:

```powershell
g++ --version
where.exe g++
```

Nếu lệnh trả về phiên bản GCC và đường dẫn `g++.exe`, có thể chạy project ngay. Máy Minh đang dùng `C:\MinGW64\bin\g++.exe` (GCC 15.1.0). Không dùng `C:\MinGW\bin\g++.exe` (GCC 6.3.0).

### Khi nào cần w64devkit?

Chỉ cài w64devkit nếu `g++ --version` báo không tìm thấy lệnh hoặc `build.ps1` không build được. Tải bản [w64devkit v2.9.1](https://github.com/skeeto/w64devkit/releases/tag/v2.9.1), chọn gói Windows x64 trong **Assets**, giải nén ngoài repo, ví dụ:

```powershell
& 'C:\tools\w64devkit-2.9.1\bin\g++.exe' --version
```

Compiler không được commit vào Git. IDE có thể khác nhau, nhưng phải build bằng C++17.

## Chạy Hello World

```powershell
git clone https://github.com/levanminh04/CPP_Embedded_Software_Simulation.git
cd CPP_Embedded_Software_Simulation
.\build.ps1
.\run.ps1
.\run.ps1 -Test
```

Mỗi thành viên cần clone repo và chạy đủ ba lệnh trên trước khi bắt đầu task. Nếu gặp lỗi, gửi nguyên văn kết quả `g++ --version`, `where.exe g++` và lỗi build cho Minh.

## Cấu trúc hiện tại

| File | Tác dụng |
|---|---|
| `src/main.cpp` | Chương trình Hello World |
| `tests/test_main.cpp` | Smoke test tối thiểu |
| `build.ps1` | Biên dịch chương trình và test vào `build/` |
| `run.ps1` | Tự build nếu cần, sau đó chạy chương trình hoặc test |

## Workflow Git

- `main` là nhánh ổn định; không push trực tiếp.
- Mỗi thành viên làm trên branch riêng, ví dụ `feature/loi-input`.
- Mỗi task tạo một Pull Request; tác giả không tự approve.
- Minh review/merge sau khi CI hoặc test local đạt.

```powershell
git clone https://github.com/levanminh04/CPP_Embedded_Software_Simulation.git
git switch -c feature/<ten>-<task>
git add .
git commit -m "feat: <mo-ta-ngan>"
git push -u origin feature/<ten>-<task>
```

## Phân công

| Thành viên | Phụ trách | Hạn |
|---|---|---|
| Phạm Thị Ngân | Tài liệu, rule, test case, sơ đồ, demo | 13/9 |
| Lê Văn Minh | Repo, môi trường, build/run, README, review/merge | 13/9 |
| Trần Bá Lợi | Input/Sensor, validate input | 17/9 |
| Phùng Trung Kiên | Logic Controller | 17/9 |
| Trần Đức Trung | Actuator/state/display | 17/9 |
| Nguyễn Thanh Phong | Timer, logging, error handling | 17/9 |

## Gate trước khi chia task

1. Cả 6 thành viên clone repo và chạy được Hello World cùng smoke test.
2. Ai không có `g++` hoặc build lỗi mới cài w64devkit.
3. Minh ghi các compiler đã xác nhận vào `docs/ENVIRONMENT.md` ở bước tiếp theo.
