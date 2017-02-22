#include <iostream>
#include <sys/socket.h>
#include <errno.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fstream>
namespace A {
    extern "C" {
        extern int accept(int, struct sockaddr * __restrict, socklen_t * __restrict);
    }
}
//初始客户端服务器
int initSocket()
{
    int so = socket(AF_INET, SOCK_STREAM, 0);
    if(so == -1)
    {
        perror("socket:");
        return -1;
    }
    sockaddr_in addr;
    addr.sin_port = 6666;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    socklen_t len = sizeof(addr);
    if(bind(so, (sockaddr*)&addr, len) < 0)
    {
        perror("bind:");
        return -1;
    }
    
    if(listen(so, 100) < 0)
    {
        perror("listen:");
    }
    
    return so;
}

int main(int argc, const char * argv[]) {
    
    int so = initSocket();
    int clf;
    sockaddr_in addr;
    unsigned size = sizeof(addr);
    char success[1000] = "链接数据服务器成功";
    std::string re(success);
    std::cout<<"数据服务器开启成功\n";
    //接受客户端的链接
        while((clf = A::accept(so,(sockaddr*)&addr, &size)) > 0)
        {
            send(clf, re.c_str(), re.size(), 0);
            memset(success, 0, 1000);
            std::string cmd;
            //接受客户端所要进行的操作
            if(recv(clf, success, sizeof(success), 0) > 0)
            {
                cmd = success;
            }
            else
            {
                std::cout<<"命令出错"<<std::endl;
            }
            if(cmd == "LIST")
            {
                if(recv(clf, success, sizeof(success), 0) > 0)
                {
                    std::cout<<"FTP服务器列表\n";
                    std::string result(success);
                    for(int i = 0; i < result.size(); i++)
                    {
                        if(success[i] == ',') std::cout<<"\n";
                        else
                        {
                            std::cout<<success[i];
                        }
                    }
                }
                else
                {
                    std::cout<<"数据传输失败"<<std::endl;
                }
            }
            
            if(cmd == "RETR")
            {
                if(recv(clf, success, sizeof(success), 0) < 0)
                {
                    std::cout<<"接受文件路径出错"<<std::endl;
                }
                //客户端存放文件的地址
                std::string path("/Users/huangwenhai/Desktop/计算机网络/4/FTPClient/WorkSpace/");
                path+=success;
                std::cout<<path;
                std::ofstream of(path.c_str());
                while (recv(clf, success, sizeof(success), 0) > 0)
                {
                    std::string data(success);
                    if(data == "end")
                    {
                        break;
                    }
                    of << data<<std::endl;
                }
            }
            memset(success, 0, 1000);
            close(clf);
     }
    std::cout<<std::endl;
    return 0;
}
