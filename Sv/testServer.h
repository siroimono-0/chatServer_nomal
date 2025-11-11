#pragma once
#include <bits/stdc++.h>
//=============================================================
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h> // struct sockaddr_in
#include <arpa/inet.h>  // inet_addr, htons
#include <unistd.h>     // close()
#include <stdio.h>      // perror()
#include <arpa/inet.h>
#include <netinet/in.h> // IPv4용 sockaddr_in
#include <sys/types.h>
#include <sys/socket.h>

//=============================================================
#define MAX_BUFFER_SIZE 1024000 // 1MB 제한

using namespace std;
class RAII_fd;
class Worker;

class Sv
{
  static Sv *sp_Sv;

public:
  Sv();
  ~Sv();

  void setServer();
  //==================================
  void createTh_work(Worker *wk);
  //==================================

  //==================================
  void createTh_recvSend();
  static void *entryPoint(void *vp);

  void entryPoint_loop();
  void recv_send_All();
  //==================================

  static void SIGINThandler(int sig);
  void set_life(bool in_life);

private:
  int socSv_fd;
  struct sockaddr_in st_soc = {};
  bool life = true;

  int socPair[2];

  vector<RAII_fd> vec_RAII_fd;
  vector<int> vec_socClient_fd;
};
