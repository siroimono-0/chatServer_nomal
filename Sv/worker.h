#pragma once
//===============================
#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <map>
//===============================
#include <unistd.h>
#include <errno.h>
#include <stdio.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <mqueue.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h> // struct sockaddr_in
#include <arpa/inet.h>  // inet_addr, htons
#include <unistd.h>     // close()
#include <stdio.h>      // perror()
#include <arpa/inet.h>
#include <netinet/in.h> // IPv4용 sockaddr_in
//===============================

class Sv;
class Exception;

class Worker
{
public:
  Worker();
  ~Worker();

  static void *entryPoint_Client(void *vp);
  static void *entryPoint_Sv(void *vp);
  //=====================================================
  void set_p_Sv(Sv *in_p_Sv);
  void set_socClient_fd(int in_socClient_fd);
  void set_socPair1_fd(int in_socPair1_fd);

  void entryPoint_loop_Client();
  void entryPoint_loop_Sv();

  void createTh_Sv();

  void recv_send_All_Client();
  // void recv_send_All_Sv();
  void sendServer(string &bufRecv_str);
  // void sendClient(string &bufRecv_str);

  void set_connectStat(bool stat);

private:
  Sv *p_Sv = nullptr;
  int socClient_fd;
  int socPair1_fd;
  bool connectStat = false;
};
