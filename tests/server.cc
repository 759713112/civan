#include <sys/types.h>          /* See NOTES */
#include <sys/socket.h>
#include <iostream>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <wait.h>

void recycle_child(int arg) {
    while (1) {
        int ret = waitpid(-1, NULL, WNOHANG);
        if (ret == -1) {
            break;
        } else if (ret == 0) {
            break;
        } else if (ret > 0) {
            std::cout<<"子进程" << ret << "已经被回收了" << std::endl;
        }
    }
    
}

int main () 
{

    struct sigaction act;
    act.sa_flags = 0;
    sigemptyset(&act.sa_mask);
    act.sa_handler = recycle_child;
    // 注册信号捕捉
    sigaction(SIGCHLD, &act, NULL);


    int listen_fd = socket(AF_INET, SOCK_STREAM, 0);
    int optval = 1;
    int r = setsockopt(listen_fd, SOL_SOCKET, SO_REUSEPORT, &optval, sizeof(optval));
    struct sockaddr_in addr; 
    addr.sin_family = AF_INET;
    inet_pton(AF_INET, "127.0.0.1", (void*)&addr.sin_addr.s_addr);
    std::cout<<addr.sin_addr.s_addr<<std::endl;
    addr.sin_port = htons(2223);
   
    int ret =  bind(listen_fd, (struct sockaddr*)&addr, sizeof(addr));
    if (ret == -1) {
        std::cout << "bind error" << std::endl;
    }

    ret = listen(listen_fd, 8);
    if (ret == -1) {
        std::cout << "listen error" << std::endl;
    }

    while (1) {
        struct sockaddr_in client_addr;

        socklen_t len = sizeof(client_addr);
        int connected_fd = accept(listen_fd, (struct sockaddr*)&client_addr, &len);
        if(connected_fd == -1) {
            //子进程退出触发信号处理  会导致accept不阻塞
            if(errno == EINTR) {
                continue;
            }
            perror("accept");
            exit(-1);
        }
        pid_t pid = fork();
        if (pid == 0) {
            //输出客户端信息
            char client_ip[16];
            inet_ntop(AF_INET, &client_addr.sin_addr.s_addr, client_ip, sizeof(client_addr));
            unsigned short client_port = ntohs(client_addr.sin_port);

            std::cout<< client_ip << "   " << client_port << std::endl;
            while (1) {
                
            }
            // char recv_buf[1024] = {0};
            // ret = read(connected_fd, recv_buf, 1024);
            // if (ret == -1) {
            //     std::cout<<"read_error" << std::endl;
            //     return -1;
            // } else if (ret == 0) {
            //     std::cout<<"connect close" << std::endl;
            //     return -1;
            // } else {
            //     std::cout<< "recv: " << recv_buf << std::endl;
            // }

            // const char* to_send = "server send";
            // ret = write(connected_fd, to_send, strlen(to_send));
            close(connected_fd);
            exit(0);    // 退出当前子进程
        }
    }

    close(listen_fd);
    return 0;
}