# Smart Traffic Light Controller Simulator

## Trạng thái

Giai đoạn 0: chốt môi trường và workflow. Chưa tạo codebase.

## Môi trường chuẩn

| Hạng mục | Quy định |
|---|---|
| Hệ điều hành | Windows 10/11 x64 |
| Ngôn ngữ | C++17 |
| Compiler | w64devkit v2.9.1 x64, GCC 16.2 |
| Build/test | Dùng script chung trong repo |

### Cách cài compiler (làm một lần trên mỗi máy)

1. Mở trang [w64devkit v2.9.1](https://github.com/skeeto/w64devkit/releases/tag/v2.9.1).
2. Kéo xuống **Assets**, tải gói **Windows x64** có tên bắt đầu bằng `w64devkit` và kết thúc bằng `.exe`. Không tải `x86` hoặc `Source code`.
3. Đây là file tự giải nén. Chạy file vừa tải và chọn thư mục đích:

   `C:\tools\w64devkit-2.9.1`

   Nếu `C:\tools` chưa có, hãy tạo thư mục này trước. Sau khi giải nén, phải nhìn thấy file:

   `C:\tools\w64devkit-2.9.1\bin\g++.exe`

   Nếu file bị giải nén thành thư mục lồng nhau, di chuyển thư mục chứa `bin` về đúng đường dẫn trên. Không đặt compiler bên trong repo Git.
4. Mở PowerShell mới và chạy lệnh kiểm tra:

```powershell
& 'C:\tools\w64devkit-2.9.1\bin\g++.exe' --version
```

Kết quả phải hiển thị GCC `16.2.0`. IDE có thể khác nhau, nhưng mọi người phải build bằng compiler và script thống nhất. Không commit compiler, file build, `.exe`, log hoặc file sinh tự động.

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

## Gate trước khi tạo codebase

1. Cả 6 thành viên xác nhận compiler và chạy được lệnh kiểm tra trên.
2. Chạy thử chương trình C++17 nhỏ có `chrono`, `thread`, `fstream`.
3. Minh ghi kết quả vào `docs/ENVIRONMENT.md`.
4. Một thành viên khác clone repo và chạy lại thành công.

Sau khi đạt gate, nhóm mới tạo `include/`, `src/`, `tests/`, `config/` và bắt đầu code.
