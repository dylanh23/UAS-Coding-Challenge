#define WIN32_LEAN_AND_MEAN

#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <string>
#include <QMessageBox>
#include <winsock2.h>
#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>

// Link with ws2_32.lib
#pragma comment(lib, "Ws2_32.lib")

#define DEFAULT_BUFLEN 512

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
}

MainWindow::~MainWindow()
{
    delete ui;
}

inline bool isInteger(const std::string & s)
{
    if(s.empty() || ((!isdigit(s[0])))) return false ;

    char * p ;
    strtol(s.c_str(), &p, 10) ;

    return (*p == 0) ;
}

void errorMessage(const QString & s)
{
    QMessageBox messageBox;
    messageBox.critical(0,"Error",s);
    messageBox.setFixedSize(500,200);
}

void MainWindow::on_pushButton_clicked()
{
    const char * ip;// = INADDR_NONE;
    std::string tempIP = ui->textEdit->toPlainText().toLocal8Bit().constData();
    if (inet_addr(tempIP.c_str()) == INADDR_NONE) {
        errorMessage("IP address is malformed!");
        ui->textEdit->setText("");
        return;
    } else {
        ip = tempIP.c_str();
    }

    const char * port;
    std::string tempPort = ui->textEdit_2->toPlainText().toLocal8Bit().constData();
    if (isInteger(tempPort)) {
        port = tempPort.c_str();
    } else {
        errorMessage("Port must be a number!");
        ui->textEdit_2->setText("");
        return;
    }

    std::string message = ui->textEdit_3->toPlainText().toLocal8Bit().constData();
    if (message.length() == 0) {
        errorMessage("Message is blank!");
        ui->textEdit_3->setText("");
        return;
    }
    int iResult;
    WSADATA wsaData;

    SOCKET ConnectSocket = INVALID_SOCKET;
    struct sockaddr_in clientService;

    int recvbuflen = DEFAULT_BUFLEN;
    char *sendbuf = new char[message.length() + 1];
    std::strcpy(sendbuf, message.c_str());
    char recvbuf[DEFAULT_BUFLEN] = "";

    //initialize winsock
    iResult = WSAStartup(MAKEWORD(2,2), &wsaData);
    if (iResult != NO_ERROR) {
        errorMessage("Winsock startup failed! " + iResult);
        return;
    }

    //create socket
    ConnectSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (ConnectSocket == INVALID_SOCKET) {
       WSACleanup();
       errorMessage("Create socket failed! " + WSAGetLastError());
       return;
    }

    clientService.sin_family = AF_INET;
    clientService.sin_addr.s_addr = inet_addr(ip);
    clientService.sin_port = htons((unsigned short) strtoul(port, NULL, 0));

    iResult = _WINSOCK2API_::connect( ConnectSocket, (SOCKADDR*) &clientService, sizeof(clientService) );
    if (iResult == SOCKET_ERROR) {
        closesocket(ConnectSocket);
        WSACleanup();
        errorMessage("Connect socket failed! " + WSAGetLastError());
        return;
    }

    iResult =  send( ConnectSocket, sendbuf, (int)strlen(sendbuf), 0 );
    if (iResult == SOCKET_ERROR) {
        closesocket(ConnectSocket);
        WSACleanup();
        errorMessage("Create socket failed! " + WSAGetLastError());
        return;
    }

    printf("Bytes Sent: %d\n", iResult);
    iResult = shutdown(ConnectSocket, SD_SEND);
    if (iResult == SOCKET_ERROR) {
        closesocket(ConnectSocket);
        WSACleanup();
        errorMessage("Shutdown failed! " + WSAGetLastError());
        return;
    }

    // close the socket
    iResult = closesocket(ConnectSocket);
    if (iResult == SOCKET_ERROR) {
        WSACleanup();
        errorMessage("Close failed! " + WSAGetLastError());
        return;
    }

    WSACleanup();
    return;
}
