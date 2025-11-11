#include "testServer.h"
#include "support.h"
#include "worker.h"

using namespace std;

Sv *Sv::sp_Sv = nullptr;

Sv::Sv()
{
  this->sp_Sv = this;
}

Sv::~Sv()
{
  int ret_c1 = close(this->socSv_fd);
  check_err::check(string(__func__) + " close(this->socSv_fd)", ret_c1, -1);

  int ret_pair0 = close(this->socPair[0]);
  check_err::check(string(__func__) + " close(this->socPair[0])", ret_pair0, -1);
  int ret_pair1 = close(this->socPair[1]);
  check_err::check(string(__func__) + " close(this->socPair[1])", ret_pair1, -1);
}

void Sv::set_life(bool in_life)
{
  this->life = (in_life);
  return;
}

void Sv::SIGINThandler(int sig)
{
  shutdown(sp_Sv->socSv_fd, SHUT_RDWR);
  sp_Sv->set_life(false);
  (void)sig;
  return;
}

void Sv::setServer()
{
  signal(SIGINT, SIGINThandler);
  int ret_soc = socketpair(AF_UNIX, SOCK_STREAM, 0, this->socPair);
  check_err::check(string(__func__) + " socketpair", ret_soc, -1);

  this->socSv_fd = socket(AF_INET, SOCK_STREAM, 0);

  // 소켓 생성

  int opt = 1;
  setsockopt(socSv_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
  // 소켓 재접속 옵션 설정

  this->st_soc.sin_family = AF_INET;
  this->st_soc.sin_port = htons(7777);
  // 포트 설정

  int ret_inet_pton = inet_pton(AF_INET, "0.0.0.0", &this->st_soc.sin_addr);
  if (ret_inet_pton != 1)
  {
    check_err::check(string(__func__) + " ret_inet_pton", ret_inet_pton, ret_inet_pton);
  }
  else
  {
    check_err::check(string(__func__) + " ret_inet_pton", ret_inet_pton, -1);
  }
  // 주소 설정 -> 전체

  int ret_bind = bind(this->socSv_fd, (struct sockaddr *)&this->st_soc, sizeof(this->st_soc));
  check_err::check(string(__func__) + " ret_bind", ret_bind, -1);
  // 설정 소켓 st 바인드 (서버 소켓)

  int ret_listen = listen(this->socSv_fd, 5);
  check_err::check(string(__func__) + " ret_listen ", ret_listen, -1);
  // 클라이언트 소켓 입장 대기

  this->createTh_recvSend();
  // 에코용 쓰레드 생성

  while (1)
  {
    struct sockaddr_in st_socClient = {};
    socklen_t st_len = sizeof(st_socClient);
    int socClient_fd = accept(this->socSv_fd, (struct sockaddr *)&st_socClient, &st_len);
    if (socClient_fd == -1 && (errno == 4 || errno == 22))
    {
      cout << "Server close" << endl;
      break;
    }
    else
    {
      check_err::check(string(__func__) + " accept", socClient_fd, -1);
    }

    if (this->life == false)
    {
      return;
    }

    vec_socClient_fd.push_back(socClient_fd);
    // 클라 입장시 소켓 생성

    Worker *wk = new Worker;
    wk->set_p_Sv(this);
    wk->set_socClient_fd(socClient_fd);
    wk->set_socPair1_fd(this->socPair[1]);
    wk->set_connectStat(true);
    // 워커 객체 생성 및 기본값 설정

    this->createTh_work(wk);
    // 워커용 쓰레드 생성
    // entryPoint는 워커 객체에 정의
  }

  return;
}

void Sv::createTh_work(Worker *wk)
{
  pthread_t tid;
  int ret_pthCre = pthread_create(&tid, nullptr, wk->entryPoint_Client, (void *)wk);
  check_err::check_r(string(__func__) + " pthread_create", ret_pthCre, 0);
  // 통신할 쓰레드 생성

  int ret_pthDe = pthread_detach(tid);
  check_err::check_r(string(__func__) + " pthread_detach", ret_pthDe, 0);
  // 통신할 쓰레드 자원 알아서 회수
}

void Sv::createTh_recvSend()
{
  pthread_t tid;
  int ret_pthC = pthread_create(&tid, nullptr, entryPoint, (void *)this);
  check_err::check_r(string(__func__) + " pthread_create", ret_pthC, 0);

  int ret_pthD = pthread_detach(tid);
  check_err::check_r(string(__func__) + " pthread_detach", ret_pthD, 0);
}

void *Sv::entryPoint(void *vp)
{
  try
  {
    Sv *p_this = (Sv *)vp;
    cout << "Entry Point_ServerEcho" << endl;

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

void Sv::entryPoint_loop()
{
  while (this->life)
  {
    this->recv_send_All();
  }
  return;
}

void Sv::recv_send_All()
{
  int bufSize = 4;
  int tmp_bufSize = 0;
  int bufRecvSize = 0;

  while (bufSize > tmp_bufSize)
  {
    int ret_recvSize = recv(this->socPair[0], (void *)((char *)&bufRecvSize + tmp_bufSize), bufSize - tmp_bufSize, 0);
    tmp_bufSize += ret_recvSize;
  }

  int tmp_bufRecvSize = 0;
  char *bufRecv = new char[bufRecvSize + 4];
  while (bufRecvSize > tmp_bufRecvSize)
  {
    int ret_recv = recv(this->socPair[0], (void *)(bufRecv + tmp_bufRecvSize), bufRecvSize - tmp_bufRecvSize, 0);
    tmp_bufRecvSize += ret_recv;
  }

  string bufRecv_str(bufRecv, bufRecvSize);
  cout << bufRecv_str << endl;

  for (int i = 0; i < (int)this->vec_socClient_fd.size(); i++)
  {
    bufRecvSize = bufRecv_str.size();
    tmp_bufRecvSize = 0;

    while (4 > tmp_bufRecvSize)
    {
      int ret_sendSize = send(this->vec_socClient_fd[i], ((char *)&bufRecvSize + tmp_bufRecvSize), 4 - tmp_bufRecvSize, 0);
      tmp_bufRecvSize += ret_sendSize;
    }

    tmp_bufRecvSize = 0;

    while (bufRecvSize > tmp_bufRecvSize)
    {
      int ret_sendSize = send(this->vec_socClient_fd[i], (bufRecv_str.c_str() + tmp_bufRecvSize), bufRecvSize - tmp_bufRecvSize, 0);
      tmp_bufRecvSize += ret_sendSize;
    }
  }

  cout << __func__ << endl;
  delete[] bufRecv;
  return;
}