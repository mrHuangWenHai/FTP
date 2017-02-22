#include<sys/socket.h>
#include<netinet/in.h>
#include <arpa/inet.h>
#include <iostream>
#include <string>

//登陆服务器代码
bool Login(int client,char *name,char*pass)
{
    //发送USER
    char buf[1000];
    std::string USER("USER ");
    USER+=name;
    send(client, USER.c_str(), USER.size(), 0);
    memset(buf, 0, 1000);
    if(recv(client, buf, 1000, 0) > 0)
    {
        std::string b(buf);
        int x = atoi((b.substr(0,b.find(' '))).c_str());
        if(x != 331)
        {
            std::cout<<"账户名不正确"<<std::endl;
            return false;
        }
        std::cout<<b<<std::endl;
    }
    else
    {
        perror("name:");
    }

    //发送密码
    std::string PASS("PASS ");
    PASS+=pass;
    send(client, PASS.c_str(), PASS.size(),0);
    if(recv(client, buf, 1000, 0) > 0)
    {
        std::string b(buf);
        int x = atoi((b.substr(0,b.find(' '))).c_str());
        if(x != 230)
        {
            std::cout<<"密码不正确"<<std::endl;
            return false;
        }
        std::cout<<b<<std::endl;
    }
    else
    {
        perror("pass:");
    }
    return true;
}


int main(int argc, const char * argv[]) {

    
    int client = socket(AF_INET, SOCK_STREAM, 0);
    char buf[1000];
    char name[50];
    char pass[50];
    std::cout<<"请输入用户名和密码"<<std::endl;
    std::cin>>name>>pass;
    getchar();
   // std::cin.ignore(INT_MAX, '\n');
    if(client < 0)
    {
        perror("client");
        return 0;
    }
    //链接服务器代码
    sockaddr_in addr;
    addr.sin_port = 20;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    if(connect(client, (sockaddr*)&addr, sizeof(addr)) < 0)
    {
        perror("connect:");
        return 0;
    }
    if(recv(client, buf, 1000, 0) > 0)
    {
        std::string b(buf);
        int x = atoi((b.substr(0,b.find(' '))).c_str());
        if(x != 220)
        {
            std::cout<<"服务器拒绝服务"<<std::endl;
            return 0;
        }
        std::cout<<b<<std::endl;
        //发送用户名和密码
        if(!Login(client,name,pass)) return 0;
    }
    else
    {
        perror("recv:");
    }
    //告诉 FTP 服务器，数据通道服务器地址
    std::string PORT("PORT 127,0,0,1,66,66\r\n");
    send(client, PORT.c_str(), PORT.size(), 0);
    std::string bufString;
    int i = 0;
    while(recv(client, buf, 1000, 0) > 0)
    {
        
        std::cout<<buf<<"请输入你的命令："<<std::endl;
        bufString.clear();
        getline(std::cin,bufString);
        if(bufString == "QUIT ")
        {
            std::cout<<"服务结束"<<std::endl;
            break;
        }
        bufString+="\r\n";
        send(client, bufString.c_str(), bufString.size(), 0);
        memset(buf, 0, 1000);
        i++;
    }
  

    return 0;
}
