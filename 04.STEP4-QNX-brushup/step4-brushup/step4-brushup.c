/*
  使い方
./step4-brushup サーバアドレス < 転送ファイル名

*/

#include <netdb.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#define PORT 8000
#define BLOCK_SZ (4096) /* Sendでの１回の送信サイズ*/
int g_sockfd;

#define E_OK (0L)
#define E_NG (-1L)

int wrap_sockOpen(char *hostName)
{
    struct sockaddr_in serv_addr;
    struct hostent *server;
    int sockfd;

    // ソケットを作成する
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
    {
        perror("ERROR opening socket");
        return E_NG;
    }

    // サーバーのホスト名を取得する
    server = gethostbyname(hostName);
    if (server == NULL)
    {
        fprintf(stderr, "ERROR, no such host\n");
        return E_NG;
    }

    // 接続先アドレスを設定する
    memset((char *)&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    memcpy((char *)&serv_addr.sin_addr.s_addr, (char *)server->h_addr,
           server->h_length);
    serv_addr.sin_port = htons(PORT);

    // サーバーに接続する
    if (connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
    {
        perror("ERROR connecting");
        return E_NG;
    }
    return sockfd;
}
int wrap_sockTrans(int sockfd, char *buffer, int buffer_sz)
{
    int n;

    n = send(sockfd, buffer, buffer_sz, 0);
    if (n < 0)
    {
        perror("ERROR writing to socket");
        return E_NG;
    }
    return E_OK;
}
int wrap_sockClose(int sockfd)
{
    int retStatus;
    /* Zero for success, or -1 if an error occurs (errno is set). */
    retStatus = close(sockfd);
    return retStatus;
}

int main(int argc, char *argv[])
{
    char buffer[BLOCK_SZ];
    int n;
    int retStatus;

    if (argc < 2)
    {
        fprintf(stderr, "usage %s hostname \n", argv[0]);
        exit(0);
    }

    g_sockfd = wrap_sockOpen(argv[1]);
    fprintf(stderr, "Socketfd= %x \n", g_sockfd);
    if (g_sockfd < 0)
    {
        exit(0);
    }

    // データを送信する
    while (1)
    {
        memset(buffer, 0, BLOCK_SZ);
        n = read(0, buffer, BLOCK_SZ);
        if (n == 0)
        {
            break;
        }
        retStatus = wrap_sockTrans(g_sockfd, buffer, n);
        if (retStatus != E_OK)
        {
            perror("ERROR writing to socket");
            exit(0);
        }
    }

    (void)wrap_sockClose(g_sockfd);

    return 0;
}