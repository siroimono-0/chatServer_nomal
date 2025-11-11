#include "testClient.h"
#include "support.h"

using namespace std;

Client *Client::sp_this = nullptr;

Client::Client()
{
  this->sp_this = this;
  return;
}

Client::~Client()
{
  close(this->socClient_fd);
}

void Client::set_life(bool in_life)
{
  this->life = in_life;
  return;
}

void Client::setSoc()
{
  int socfd = socket(AF_INET, SOCK_STREAM, 0);
  check_err::check(string(__func__) + " socfd", socfd, -1);
  this->socClient_fd = socfd;
  // 소켓 생성

  this->st_socClient.sin_family = AF_INET;
  this->st_socClient.sin_port = htons(7777);

  int ret_inet_pton = inet_pton(AF_INET, "127.0.0.1", &this->st_socClient.sin_addr);
  if (ret_inet_pton != 1)
  {
    check_err::check(string(__func__) + " ret_inet_pton", ret_inet_pton, ret_inet_pton);
  }

  socklen_t socLen = sizeof(this->st_socClient);
  int ret_connect = connect(this->socClient_fd, (struct sockaddr *)&this->st_socClient, socLen);
  check_err::check(string(__func__) + " ret_connect", ret_connect, -1);
  // 소켓 서버쪽에 연결

  this->createTh_recv();
  // recv용 쓰레드 생성

  while (this->life)
  {
    this->main_loop();
  }
}

void Client::main_loop()
{
  string tmp_s;
  getline(cin, tmp_s);

  if (tmp_s == "end")
  {
    shutdown(this->socClient_fd, SHUT_RDWR);
    this->set_life(false);
    cout << "Exit Client" << "\n";
    return;
  }

  string bufGet("C ");
  bufGet += tmp_s;

  int bufSize = bufGet.size();
  int tmp_bufSize = 0;
  while (4 > tmp_bufSize)
  {
    int ret_sendSize = send(this->socClient_fd, (void *)((char *)&bufSize + tmp_bufSize), 4 - tmp_bufSize, 0);
    tmp_bufSize += ret_sendSize;
  }

  tmp_bufSize = 0;
  while (bufSize > tmp_bufSize)
  {
    int ret_sendSize = send(this->socClient_fd, (bufGet.c_str() + tmp_bufSize), bufSize - tmp_bufSize, 0);
    tmp_bufSize += ret_sendSize;
  }

  cout << ">> Client send <<" << endl;
  return;
}

//====================recv Thread=============================================
//====================recv Thread=============================================
void Client::createTh_recv()
{
  pthread_t tid;
  int ret_pthC = pthread_create(&tid, nullptr, entryPoint, (void *)this);
  check_err::check_r(string(__func__) + " pthread_create", ret_pthC, 0);
  // recv용 쓰레드 생성

  int ret_pthD = pthread_detach(tid);
  check_err::check_r(string(__func__) + " pthread_detach", ret_pthD, 0);
  // recv용 쓰레드 자원 자동회수 설정

  return;
}

void *Client::entryPoint(void *vp)
{
  try
  {
    Client *p_this = (Client *)vp;
    cout << "Entry Point" << endl;

    while (1)
    {
      p_this->entryPoint_loop();
    }
  }
  catch (Exception err)
  {
    int err_code = err.get_err_code();
    string err_name = err.get_err_name();
    string name = err.get_name();
    printf("ERR\nname     :: %s\nerr type :: %s\nerr code :: %d\n",
           name.c_str(), err_name.c_str(), err_code);
    return nullptr;
  }
  return nullptr;
}

void Client::entryPoint_loop()
{
  while (this->life)
  {
    this->recvAll();
  }
  return;
}

void Client::recvAll()
{
  int bufSize = 4;
  int tmp_bufSize = 0;
  int bufRecvSize = 0;

  while (bufSize > tmp_bufSize)
  {
    int ret_recvSize = recv(this->socClient_fd, (void *)((char *)&bufRecvSize + tmp_bufSize), bufSize - tmp_bufSize, 0);

    if (ret_recvSize == 0)
    {
      return;
    }

    tmp_bufSize += ret_recvSize;
  }

  int tmp_bufRecvSize = 0;
  char *bufRecv = new char[bufRecvSize + 4];
  while (bufRecvSize > tmp_bufRecvSize)
  {
    int ret_recv = recv(this->socClient_fd, (void *)(bufRecv + tmp_bufRecvSize), bufRecvSize - tmp_bufRecvSize, 0);
    tmp_bufRecvSize += ret_recv;
  }
  string bufRecv_str(bufRecv, bufRecvSize);

  cout << ">> recv << " << endl;
  cout << bufRecv_str << endl;
  cout << ">> recv << " << endl;
  delete[] bufRecv;

  return;
}
//====================recv Thread=============================================
//====================recv Thread=============================================
