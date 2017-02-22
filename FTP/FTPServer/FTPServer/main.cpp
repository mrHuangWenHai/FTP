
#include <iostream>
#include<fstream>
#include <sys/socket.h>
#include <errno.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include<string>
#include <map>
#include <pthread.h>
#include <sys/stat.h>
#include <dirent.h>
//#include <unistd.h>


//c 语言与 c++库的冲突，放在 A 的命名空间便于引用
namespace A {
    //防止 c++变异器对函数名的特殊处理，防止链接失败
    extern "C" {
        extern int accept(int, struct sockaddr * __restrict, socklen_t * __restrict);
        extern int pthread_create(pthread_t * __restrict, const pthread_attr_t * __restrict,
                           void *(*)(void *), void * __restrict);
    }
}

std::map<std::string,std::string>user;
pthread_t ntid;

//命令结构体
struct Arg{
   const char*path;
   const char* cmd;
};

//初始化服务器
int initSocket()
{
    int so = socket(AF_INET, SOCK_STREAM, 0);
    if(so == -1)
    {
        perror("socket:");
        return -1;
    }
    sockaddr_in addr;
    addr.sin_port = 20;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    socklen_t len = sizeof(addr);
    //监听20端口
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

// 获取数据服务器的 IP 和端口
void initDataAddr(std::string str,sockaddr_in &dataAddr)
{
    const char *a = str.c_str();
    int j = 0;
    int begin = 0;
    int end = 0;
    int port = 0;
    char b[2];
    b[1] = 0;
    for(int i = 0; a[i]!= 0; i++)
    {
        if(a[i] == ',')
        {
            j++;
            if(j == 4)
            {
                begin = i;
            }
        }
        
        if(a[i] == '\r')
            end = i;
        
        if(begin != end && end == 0 && a[i]!=',')
        {
            b[0] = a[i];
            port = port*10+atoi(b);
        }
    }
    dataAddr.sin_addr.s_addr = inet_addr(str.substr(0,begin).c_str());
    dataAddr.sin_port = port;
    dataAddr.sin_family = AF_INET;
    
}

//获得特定路径先的目录结构
std::string getList(const char *dir)
{
    std::string list;
    DIR *dp;
    struct dirent *entry;   //目录项相关信息
    struct stat statbuf;
    if((dp = opendir(dir)) == NULL) {
        fprintf(stderr,"cannot open directory: %s\n", dir);
        return list;
    }
    chdir(dir);
    while((entry = readdir(dp)) != NULL) {
        lstat(entry->d_name,&statbuf);//获取每个目录项的 stat 结构
        if(S_ISDIR(statbuf.st_mode)) {
            
            if(strcmp(".",entry->d_name) == 0 ||
               strcmp("..",entry->d_name) == 0)
                continue;
            list = list+entry->d_name+",";
        }
        else list = list+entry->d_name+",";
        
    }
    chdir("..");
    closedir(dp);
    return list;
}

//这个是链接数据服务器的函数
void* startFTPClient(void*arg)
{
    int client = socket(AF_INET, SOCK_STREAM, 0);
    char buf[1000];
    Arg* param = (Arg*)arg;
    std::cout<<"path :  "<<param->path;
    if(client < 0)
    {
        perror("client");
        return 0;
    }
    sockaddr_in addr;
    addr.sin_port = 6666;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    if(connect(client, (sockaddr*)&addr, sizeof(addr)) < 0)
    {
        perror("connect:");
        return 0;
    }
    if(recv(client, buf, 1000, 0) > 0)
    {
        std::cout<<buf<<std::endl;
    }
    else
    {
        std::cout<<"数据服务器连接失败"<<std::endl;
        return 0;
    }
    
    std::string cmd(param->cmd);
    if(send(client, cmd.c_str(), cmd.size(), 0) < 0)
    {
        std::cout<<"传送命令出错"<<std::endl;
        return ((void*)0);
    }

    if(cmd == "LIST")
    {
        std::string list = getList("/Users/huangwenhai/Desktop/计算机网络/4/FTPServer/WorkSpace");
        std::cout<<list<<std::endl;
        send(client, list.c_str(), list.size(), 0);
        std::cout<<"传输成功"<<std::endl;
    }
    
    if(cmd == "RETR")
    {
        std::string path(param->path);
        std::string P("/Users/huangwenhai/Desktop/计算机网络/4/FTPServer/WorkSpace/");
        P+=path;
        std::ifstream fin(P);
        std::string  s;
        std::cout<<path<<"   dd111  "<<std::endl;
        if(send(client, path.c_str(), path.size(), 0) < 0)
        {
            std::cout<<"传文件路径出错"<<std::endl;
            return ((void*)0);
        }
        
        while (getline(fin,s))
        {
            std::cout<<"begin";
            std::cout <<  "Read from file: "  << s << std::endl;
            s+="\n";
            send(client, s.c_str(), s.size(), 0);
            
        }
        std::cout<<"文件传输完成"<<std::endl;
    }
    
    close(client);
    return ((void*)0);
}

//检查用户登录，记账户名与密码是否正确
bool check(char success[],int clf)
{
    if(recv(clf, success,1000, 0) < 0)
    {
        std::cout<<"user 传输出错"<<std::endl;
        return false;
    }
    std::string checkName(success);
    std::string name = checkName.substr(checkName.find(' ')+1,checkName.size());
    if(user.count(name) == 1)
    {
        std::string word("331 User name okay, need password");
        send(clf, word.c_str(), word.size(), 0);
    }
    memset(success, 0,1000);
    
    if(recv(clf, success,1000, 0) < 0)
    {
        std::cout<<"pass 传输出错"<<std::endl;
        return false;
    }
    std::string checkPass(success);
    std::string pass = checkPass.substr(checkPass.find(' ')+1,checkPass.size());
    if(user[name] == pass)
    {
        std::string word("230 User logged in, proceed");
        send(clf, word.c_str(), word.size(), 0);
    }
    

    
    return true;
}

int main(int argc, const char * argv[]) {
    int so = initSocket();
    int clf;
    sockaddr_in addr;
    sockaddr_in dataAddr;
    unsigned size = sizeof(addr);
    char success[1000];
    memset(success, 0, sizeof(success));
    std::cout<<"服务器开启成功"<<std::endl;
    user["huang"] = "123";
    
    while (1)
    {
        
        if((clf = A::accept(so,(sockaddr*)&addr, &size)) > 0)
        {
            std::string buf("220 Server FTP ready");
            int a;
            send(clf, buf.c_str(), buf.size(), 0);
            if(!check(success,clf))
            {
                return 0;
            }
            while ((a=recv(clf, success, sizeof(success), 0) > 0))
            {
                std::string buf(success);
                memset(success, 0, sizeof(success));
                std::cout<<"from "<<buf<<std::endl;
                std::string cmd = buf.substr(0,buf.find(' '));
                if(cmd == "PORT")
                {
                    initDataAddr(buf.substr(buf.find(' '),buf.size()),dataAddr);
                    std::string replay("227 Entering Passive Mode");
                    replay += buf.substr(buf.find(' '),buf.size());
                    send(clf, replay.c_str(), replay.size(), 0);
                    memset(success, 0, 1000);
                    continue;
                }
                
                if(cmd == "LIST")
                {
                    std::string path;
                    if(buf.size() == 7)
                    {
                        path = "/Users/huangwenhai/Desktop/计算机网络/4/FTPServer/WorkSpace";
                    }
                    else
                        path = buf.substr(buf.find(' ')+1,buf.size());
                    Arg arg;
                    arg.path = (char*)"/Users/huangwenhai/Desktop/计算机网络/4/FTPServer/WorkSpace";
                    arg.cmd = (char*)"LIST";
                    //开启一个新线程，链接数据服务器，发送数据
                    A::pthread_create(&ntid, NULL, startFTPClient, &arg);
                    std::string re("150 Opening ASCII mode data\r\n");
                    send(clf, re.c_str(), re.size(), 0);
                    
                    continue;
                }
                
                if(cmd == "RETR")
                {
                    std::string target(buf.substr(buf.find(' ')+1,buf.size()));
                    std::string f =target.substr(0,target.find('\r'));
                    std::cout<<"file"<<f<<" 111"<<buf<<"222"<<target<<std::endl;
                    Arg arg;
                    arg.cmd = "RETR";
                    arg.path = f.c_str();
                    A::pthread_create(&ntid, NULL, startFTPClient, &arg);
                    std::string re("150 Openint ASCII mode data connection for "+buf.substr(buf.find(' ')+1,buf.size()));
                    send(clf, re.c_str(), re.size(), 0);
                    continue;
                }
                
                if(cmd == "QUIT")
                    break;
             
                
                send(clf, success, sizeof(success), 0);
            }
        }
    }
    return 0;
}


