#pragma once

#include <bits/stdc++.h>
//==========================================================
#include <iostream>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h> // struct sockaddr_in
#include <arpa/inet.h>  // inet_addr, htons
#include <unistd.h>     // close()
#include <stdio.h>      // perror()
#include <arpa/inet.h>
#include <netinet/in.h> // IPv4용 sockaddr_in

using namespace std;

class RAII_fd;
class Client
{
  static Client *sp_this;

public:
  Client();
  ~Client();

  void setSoc();
  //======================================
  static void *entryPoint(void *vp);
  void createTh_recv();

  void entryPoint_loop();
  void main_loop();

  void recvAll();

  void set_life(bool in_life);

private:
  bool life = true;
  struct sockaddr_in st_socClient = {};
  int socClient_fd;
};