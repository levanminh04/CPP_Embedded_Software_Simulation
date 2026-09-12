# Yêu cầu hệ thống - Bộ điều khiển đèn giao thông thông minh

## 1. Thứ tự ưu tiên và phạm vi

Tài liệu này là nguồn chốt về hành vi nghiệp vụ của project.

1. `Mock_Project_CPP_Embedded_Software_Simulation.docx.pdf` là đề bài gốc của giảng viên. Các yêu cầu bắt buộc và ràng buộc kỹ thuật trong đề có ưu tiên cao nhất.
2. `Quy định và kịch bản vận hành Smart Traffic Light Controller.pdf` ghi lại các quy tắc vận hành và phần nâng cao mà nhóm đã chọn. Nội dung này không được mâu thuẫn với đề bài gốc.
3. Hành vi quan sát được dưới đây là cố định. Cách biểu diễn trong C++ được để mở: Controller có thể dùng nhiều giá trị `enum`, hoặc dùng pha + hướng + cờ chờ, miễn là giữ đúng các quy tắc này.

Chương trình là mô phỏng C++ chạy trên console. Hệ thống mô phỏng một giao lộ với hai hướng xe xung đột: Bắc-Nam (NS) và Đông-Tây (EW). Một Controller chung quyết định trạng thái toàn bộ giao lộ; NS và EW không chạy hai chu kỳ độc lập.

Ngoài phạm vi: phần cứng thật, vi điều khiển, GUI, cơ sở dữ liệu, mạng, Web API, camera, AI/ML, mô phỏng làn xe, mũi tên rẽ và sensor thật.

## 2. Luồng hệ thống bắt buộc

Chương trình phải theo luồng mô phỏng phần mềm nhúng mà đề bài yêu cầu:

`sensor/sự kiện giả lập -> controller -> actuator/đầu ra giả lập -> hiển thị và ghi log`

`main()` chỉ khởi tạo các thành phần và chạy vòng lặp điều khiển. Quyết định nghiệp vụ không được lặp lại trong phần nhập liệu, đầu ra, hiển thị hoặc `main()`.

Xem [Sơ đồ khối](Block_Diagram.png) và file nguồn có thể chỉnh sửa [Block_Diagram.dot](Block_Diagram.dot).

## 3. Đầu vào

| Mã | Đầu vào | Hành vi yêu cầu |
|---|---|---|
| IN-01 | Nhịp thời gian | Làm giảm countdown và kích hoạt chuyển pha theo thời gian. |
| IN-02 | `P` - yêu cầu người đi bộ | Ghi nhận một yêu cầu đang chờ. Nhiều lần `P` khi đã chờ chỉ tính là một yêu cầu, không tạo hàng đợi. |
| IN-03 | `E` - sự kiện khẩn cấp | Có ưu tiên vận hành cao nhất và yêu cầu hệ thống đi tới trạng thái khẩn cấp an toàn. |
| IN-04 | Sự kiện thoát khẩn cấp | Sự kiện console/kịch bản do nhóm quy định để thoát Emergency. Ký tự lệnh cụ thể sẽ chốt khi thiết kế phần nhập liệu. |
| IN-05 | Số xe NS/EW | Giá trị sensor giả lập, chỉ dùng để chọn thời lượng GREEN cho lượt kế tiếp của hướng đó. |
| IN-06 | Cấu hình | Các cặp khóa/giá trị cho thời lượng và ngưỡng mật độ; được nạp và kiểm tra trước khi chạy bình thường. |

Đầu vào có thể được sinh từ kịch bản hoặc nhập từ bàn phím. Mọi đầu vào phải được kiểm tra hợp lệ trước khi Controller xử lý.

## 4. Đầu ra và thông tin hiển thị

Ở mỗi chu kỳ điều khiển, console phải hiển thị:

- Đèn xe NS và đèn xe EW.
- Đèn người đi bộ.
- Tên pha/trạng thái hiện tại.
- Thời gian còn lại khi pha hiện tại có countdown.
- Trạng thái có/không có yêu cầu người đi bộ đang chờ.

Hệ thống ghi `logs/system.log` bằng `fstream`. Tối thiểu phải có log chuyển trạng thái, yêu cầu người đi bộ và sự kiện khẩn cấp. Theo phạm vi nhóm đã chọn, log cũng ghi việc phục vụ người đi bộ, mật độ/lần lấy mẫu, lỗi sensor, lỗi cấu hình và đầu vào không hợp lệ.

## 5. Hành vi nghiệp vụ bắt buộc

### REQ-01 - Chu kỳ đèn xe

Mỗi hướng xe có RED, GREEN và YELLOW. Ở chế độ bình thường, quyền đi luân phiên giữa NS và EW. GREEN có thời lượng hữu hạn và luôn chuyển sang YELLOW của chính hướng đó trước khi một hướng xung đột được cấp GREEN.

### REQ-02 - Khoảng an toàn ALL-RED

Trước khi cấp GREEN cho một hướng xung đột, Controller phải có khoảng ALL-RED. Khi ALL-RED, đèn NS, EW và người đi bộ đều RED.

### REQ-03 - Countdown và định thời

Mọi pha có thời lượng đều có countdown để hiển thị. Timing dùng `std::chrono` và `std::this_thread::sleep_for()` trong Application/vòng lặp điều khiển. Controller nhận nhịp thời gian; Controller không được chặn việc đọc bàn phím.

### REQ-04 - Yêu cầu người đi bộ và phục vụ an toàn

`P` không được làm đèn người đi bộ chuyển GREEN ngay lập tức. Yêu cầu chỉ được phục vụ ở điểm quyết định an toàn gần nhất, sau khi lượt xe hiện tại hoàn thành GREEN, YELLOW và clearance.

Phần người đi bộ là độc quyền:

- Trong WALK và WARNING, NS và EW đều RED.
- Yêu cầu đang chờ được xóa khi bắt đầu phục vụ, không phải khi kết thúc phục vụ.
- `P` mới trong pha người đi bộ tạo một yêu cầu cho lượt người đi bộ sau.
- Không được phục vụ liên tiếp nhiều pha người đi bộ khi chưa cho hướng xe kế tiếp một lượt GREEN.

### REQ-05 - Thời lượng GREEN theo mật độ

Phần nâng cao nhóm chọn cho phép quản lý số xe NS và EW riêng. Mật độ chỉ thay đổi thời lượng GREEN hữu hạn của hướng sắp bắt đầu; không thay đổi thứ tự luân phiên NS/EW, không bỏ ALL-RED, không ghi đè yêu cầu người đi bộ và không ghi đè Emergency.

Controller lấy mẫu sensor của một hướng ngay trước khi GREEN của hướng đó bắt đầu. Giá trị đổi trong lúc GREEN chỉ áp dụng ở lượt sau của chính hướng đó.

Các giá trị mặc định của nhóm phải đặt trong cấu hình hoặc hằng số tập trung, không hard-code rải rác:

| Thiết lập | Mặc định |
|---|---:|
| GREEN_LOW | 15 giây |
| GREEN_MEDIUM | 20 giây |
| GREEN_HIGH | 25 giây |
| YELLOW_TIME | 3 giây |
| ALL_RED_TIME | 2 giây |
| PED_WALK_TIME | 10 giây |
| PED_WARNING_TIME | 5 giây |
| Ngưỡng LOW | 0-5 xe |
| Ngưỡng MEDIUM | 6-15 xe |
| Ngưỡng HIGH | từ 16 xe |

### REQ-06 - Chế độ khẩn cấp

Emergency có ưu tiên cao hơn phục vụ người đi bộ, chu kỳ xe và tối ưu mật độ. Trạng thái khẩn cấp an toàn là:

`NS = RED, EW = RED, Pedestrian = RED`.

Controller phải giữ an toàn clearance, không chuyển thẳng đèn xe từ GREEN sang RED. Về nghiệp vụ: Emergency xuất hiện khi xe GREEN thì hệ thống đi qua YELLOW và ALL-RED trước khi vào Emergency; Emergency xuất hiện khi người đi bộ WALK thì hệ thống đi qua WARNING/clearance trước khi vào Emergency. Nếu đang ở pha clearance, Controller hoàn thành clearance liên quan rồi vào Emergency.

Khi nhận sự kiện thoát Emergency, Controller không khôi phục countdown cũ. Hệ thống khởi động lại một chu kỳ bình thường mới qua ALL-RED an toàn, rồi cấp lượt GREEN đầu tiên cho NS. Yêu cầu người đi bộ đang chờ vẫn được giữ tới khi có thể phục vụ an toàn.

### REQ-07 - Kiểm tra dữ liệu và xử lý bất thường

Project phải xử lý tối thiểu hai tình huống bất thường mà không crash:

1. Số xe âm hoặc không hợp lệ không được phân loại thành mật độ bình thường. Hệ thống ghi `SENSOR_ERROR`; timer đang chạy không đổi và dùng thời lượng fallback an toàn khi hướng đó bắt đầu GREEN ở lượt sau.
2. Cấu hình không hợp lệ, ví dụ thời lượng không dương hoặc thứ tự ngưỡng sai, bị từ chối. Hệ thống ghi `CONFIG_ERROR` và dùng toàn bộ cấu hình mặc định.

Lệnh từ bàn phím/kịch bản không hợp lệ phải được báo và ghi log lỗi; chúng không được làm sai trạng thái Controller.

## 6. Bất biến an toàn và thứ tự ưu tiên

Controller phải giữ các bất biến sau ở mọi thời điểm:

1. NS GREEN và EW GREEN không bao giờ cùng xảy ra.
2. Người đi bộ GREEN không bao giờ cùng xảy ra với xe GREEN.
3. Trong WALK hoặc WARNING, NS và EW đều RED.
4. Trong Emergency, tất cả đèn đều RED.
5. Xe GREEN có thời lượng hữu hạn và sau đó là YELLOW của chính hướng đó.
6. Yêu cầu người đi bộ đã chấp nhận không được tự mất.
7. Mật độ không được thay đổi thứ tự luân phiên hoặc thời lượng GREEN đang chạy.

Thứ tự ưu tiên khi ra quyết định là: an toàn, Emergency, hoàn tất clearance/chuyển pha, yêu cầu người đi bộ, luân phiên xe bình thường, rồi mới đến chọn thời lượng theo mật độ.

Xem [Sơ đồ trạng thái](State_Diagram.png) và file nguồn [State_Diagram.dot](State_Diagram.dot).

## 7. Kịch bản chấp nhận

| Mã | Kịch bản | Kết quả mong đợi |
|---|---|---|
| AT-01 | Chạy bình thường | Quyền đi luân phiên GREEN -> YELLOW -> ALL-RED; có countdown và log. |
| AT-02 | `P` trong lúc xe GREEN | Request được log và chỉ phục vụ ở điểm quyết định an toàn. |
| AT-03 | Người đi bộ WALK | Pedestrian GREEN trong khi hai hướng xe đều RED. |
| AT-04 | Nhấn `P` lặp lại | Nhiều lần nhấn khi đang chờ chỉ tạo một request, không tạo queue. |
| AT-05 | `P` trong pha người đi bộ | Request mới chờ đến sau ít nhất một lượt xe GREEN. |
| AT-06 | Emergency trong lúc xe GREEN | Hệ thống qua clearance, vào Emergency toàn RED và ghi log. |
| AT-07 | Thoát Emergency | Countdown cũ bị bỏ; chu kỳ khởi động lại an toàn từ NS. |
| AT-08 | Mật độ thay đổi | GREEN ở lượt kế tiếp đổi thời lượng, nhưng thứ tự NS/EW không đổi. |
| AT-09 | Sensor không hợp lệ | Có log lỗi; không phân loại mật độ sai và không crash. |
| AT-10 | Cấu hình không hợp lệ | Dùng cấu hình mặc định và ghi log lỗi. |

## 8. Ràng buộc kỹ thuật và sản phẩm nộp

- Ứng dụng C++ chạy console; không cần thư viện ngoài.
- Tách khai báo công khai trong `.h` và phần cài đặt trong `.cpp` khi module có hợp đồng công khai.
- Dùng tên class/hàm có nghĩa, `enum class` khi phù hợp và `array`/`vector` cơ bản khi cần.
- Thời lượng và ngưỡng nằm trong config/hằng số, không dùng magic number rải rác.
- Có hướng dẫn build/run, test case kèm kết quả, sơ đồ, log và commit có ý nghĩa từ mọi thành viên.
- Khi defense, nhóm phải giải thích được Controller quyết định state tiếp theo và output như thế nào.

## 9. Các lựa chọn còn để mở

Các nội dung sau cố ý chưa bị chốt trong requirements:

- Tên phần tử `enum class` và số lượng state nội bộ chính xác.
- Dùng nhiều ALL-RED state riêng hay dùng một phase ALL-RED kèm dữ liệu hướng đích/cờ chờ.
- Tên class và mức độ tách file, miễn trách nhiệm module rõ ràng.
- Cú pháp lệnh bàn phím và định dạng file kịch bản.
- Cách cài đặt parser cho `default.cfg`.

Mọi lựa chọn cài đặt phải giữ đúng requirements, bất biến an toàn, sơ đồ và kịch bản chấp nhận ở trên.
