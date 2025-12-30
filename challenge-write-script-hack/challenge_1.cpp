// Thử thách 1: Script "Dò tìm đường dẫn ẩn" (Directory Brute Forcer)
// Đây là bài tập cơ bản nhất để làm quen với thư viện requests trong Python.
// Ngữ cảnh: Bạn có một website và muốn biết liệu nó có các trang ẩn như /admin, /config, hoặc /backup hay không.
// Nhiệm vụ:
// Tạo một file wordlist.txt chứa danh sách các từ khóa (admin, login, upload, v.v.).
// Viết script đọc từng dòng trong file đó.
// Gửi yêu cầu HTTP GET đến địa chỉ http://target.com/[từ-khóa].
// Nếu mã phản hồi (Status Code) là 200, hãy in ra màn hình: "Tìm thấy đường dẫn: [URL]".

#include <iostream>
#include <fstream>
#include <string>   
#include <cpr/cpr.h>

using namespace std;

int main() {
    string target_url = "http://example.com/"; 
    ifstream MyReadFile("wordlist.txt");
    string myText;

    if (!MyReadFile.is_open()) {
        cerr << "Lỗi: Không tìm thấy file wordlist.txt!" << endl;
        return 1;
    }

    cout << "--- Bat dau quet: " << target_url << " ---" << endl;

    while (getline(MyReadFile, myText)) {
        if (myText.empty()) continue;

        auto r = cpr::Get(cpr::Url{target_url + myText});

        if (r.status_code == 200) {
            cout << "[+] Tim thay: " << target_url << myText << " (Status: " << r.status_code << ")" << endl;
        } else if (r.status_code == 403) {
            cout << "[!] Cam truy cap: " << target_url << myText << " (Status: 403)" << endl;
        }
    }

    MyReadFile.close();
    cout << "--- Hoan thanh ---" << endl;
    return 0;
}


