#include <Arduino.h>
#include <unity.h>
#include "../src/telemetry/telemetry_manager.h" // Sửa lại đường dẫn nếu cần

TelemetryManager telemetry;

// Viết 1 cái test case kiểm tra đóng gói JSON
void test_telemetry_json_output(void) {
    // 1. Tạo dữ liệu giả (xe đang nghiêng 15.5 độ)
    TelemetryPayload fakeData;
    fakeData.angle = 15.5;
    fakeData.gyro = 0.0;
    fakeData.pidOutput = 10.0;
    fakeData.pwmLeft = 100;
    fakeData.pwmRight = 100;
    fakeData.rpmLeft = 30;
    fakeData.rpmRight = 30;
    fakeData.targetAngle = 0.0;

    // 2. Nạp vào manager
    telemetry.update(fakeData);

    // 3. Lấy chuỗi JSON ra
    String json = telemetry.buildTelemetryJSON();

    // 4. Kiểm tra xem trong JSON có chứa góc 15.5 không
    // Nếu có thì test PASS, không có thì FAILED
    TEST_ASSERT_TRUE(json.indexOf("15.5") >= 0);
}

void setup() {
    // Chờ 2 giây để bật Serial Monitor
    delay(2000); 
    UNITY_BEGIN(); // Khởi động framework test
    
    // Chạy cái test case vừa viết
    RUN_TEST(test_telemetry_json_output); 
    
    UNITY_END();   // Kết thúc
}

void loop() {
    // Để trống
}