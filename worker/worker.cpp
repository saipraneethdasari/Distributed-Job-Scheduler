#include <iostream>
#include <thread>
#include <winsock2.h>

#pragma comment(lib, "ws2_32.lib")

#include "../common/protocol.h"

using namespace std;

// -------- HEARTBEAT --------
void sendHeartbeat(int sock) {
    while (true) {
        string hb = string(HEARTBEAT_MSG) + "\n";
        send(sock, hb.c_str(), hb.size(), 0);
        Sleep(2000);
    }
}

int main() {
    WSADATA wsa;
    WSAStartup(MAKEWORD(2,2), &wsa);

    cout << "Connecting to scheduler...\n";

    int sock = socket(AF_INET, SOCK_STREAM, 0);

    sockaddr_in serv_addr{};
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(8080);
    serv_addr.sin_addr.s_addr = inet_addr("127.0.0.1");

    if (connect(sock, (sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
        cout << "Connection failed\n";
        return -1;
    }

    cout << "Connected to scheduler\n";

    thread hb(sendHeartbeat, sock);
    hb.detach();

    char buffer[1024];

    while (true) {
        memset(buffer, 0, sizeof(buffer));

        int valread = recv(sock, buffer, sizeof(buffer), 0);

        if (valread <= 0) break;

        string msg(buffer, valread);

        if (msg.find(JOB_MSG) != string::npos) {
            int id, duration;

            sscanf(msg.c_str(), "JOB %d %d", &id, &duration);

            cout << "Executing Job " << id << endl;

            Sleep(duration * 1000);

            bool fail = rand()%10 < 3;

            string response = fail ? string(FAILED_MSG) : string(DONE_MSG);
            response += " " + to_string(id);

            send(sock, response.c_str(), response.size(), 0);
        }
    }

    closesocket(sock);
    WSACleanup();
}