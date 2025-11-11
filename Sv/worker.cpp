#include "testServer.h"
#include "worker.h"
#include "support.h"

Worker::Worker()
{
  return;
}

Worker::~Worker()
{
  int retC = close(this->socClient_fd);
  check_err::check(string(__func__) + " close(this->socClient_fd)", retC, -1);
  return;
}

void Worker::set_p_Sv(Sv *in_p_Sv)
{
  this->p_Sv = in_p_Sv;
  return;
}

void Worker::set_socClient_fd(int in_socClient_fd)
{
  this->socClient_fd = in_socClient_fd;
  return;
}

void Worker::set_socPair1_fd(int in_socPair1_fd)
{
  this->socPair1_fd = in_socPair1_fd;
  return;
}

void *Worker::entryPoint_Client(void *vp)
{
  try
  {
    Worker *p_this = (Worker *)vp;
    cout << "Entry Point Client" << endl;
    // p_this->createTh_Sv();
    // sv용 쓰레드 생성

    while (1)
    {
      p_this->entryPoint_loop_Client();
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

void Worker::entryPoint_loop_Client()
{
  while (this->connectStat)
  {
    this->recv_send_All_Client();
  }
  return;
}

void Worker::recv_send_All_Client()
{
  int bufSize = 4;
  int tmp_bufSize = 0;
  int bufRecvSize = 0;

  while (bufSize > tmp_bufSize)
  {
    int ret_recvSize = recv(this->socClient_fd, (void *)((char *)&bufRecvSize + tmp_bufSize), bufSize - tmp_bufSize, 0);
    tmp_bufSize += ret_recvSize;

    if (ret_recvSize == 0)
    {
      cout << "Disconnected Client" << endl;
      this->set_connectStat(false);
      this->p_Sv->remove_SocClient_fd(this->socClient_fd);
      return;
    }
  }

  int tmp_bufRecvSize = 0;
  char *bufRecv = new char[bufRecvSize + 4];
  while (bufRecvSize > tmp_bufRecvSize)
  {
    int ret_recv = recv(this->socClient_fd, (void *)(bufRecv + tmp_bufRecvSize), bufRecvSize - tmp_bufRecvSize, 0);
    tmp_bufRecvSize += ret_recv;
  }

  string bufRecv_str(bufRecv, bufRecvSize);
  cout << __func__ << endl;
  this->sendServer(bufRecv_str);

  delete[] bufRecv;
  return;
}

void Worker::sendServer(string &bufRecv_str)
{
  int bufRecvSize = bufRecv_str.size();
  int tmp_bufRecvSize = 0;

  while (4 > tmp_bufRecvSize)
  {
    int ret_sendSize = send(this->socPair1_fd, ((char *)&bufRecvSize + tmp_bufRecvSize), 4 - tmp_bufRecvSize, 0);
    tmp_bufRecvSize += ret_sendSize;
  }

  tmp_bufRecvSize = 0;
  while (bufRecvSize > tmp_bufRecvSize)
  {
    int ret_sendSize = send(this->socPair1_fd, (bufRecv_str.c_str() + tmp_bufRecvSize), bufRecvSize - tmp_bufRecvSize, 0);
    tmp_bufRecvSize += ret_sendSize;
  }

  cout << __func__ << endl;
  return;
}

void Worker::createTh_Sv()
{
  pthread_t tid;
  int ret_pthC = pthread_create(&tid, nullptr, entryPoint_Sv, (void *)this);
  check_err::check_r(string(__func__) + " pthread_create", ret_pthC, 0);
  // recv용 쓰레드 생성

  int ret_pthD = pthread_detach(tid);
  check_err::check_r(string(__func__) + " pthread_detach", ret_pthD, 0);
  // 자원 자동회수 설정
  return;
}

void *Worker::entryPoint_Sv(void *vp) // static
{
  try
  {
    Worker *p_this = (Worker *)vp;
    cout << "Entry Point Server" << endl;

    while (1)
    {
      p_this->entryPoint_loop_Sv();
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

void Worker::entryPoint_loop_Sv()
{
  while (this->connectStat)
  {
    sleep(1);
    // this->recv_send_All_Sv();
  }

  return;
}

void Worker::set_connectStat(bool stat)
{
  this->connectStat = stat;
  return;
}

/*
void Worker::recv_send_All_Sv()
{
  int bufSize = 4;
  int tmp_bufSize = 0;
  int bufRecvSize = 0;

  while (bufSize > tmp_bufSize)
  {
    int ret_recvSize = recv(this->socPair1_fd, (void *)((char *)&bufRecvSize + tmp_bufSize), bufSize - tmp_bufSize, 0);
    tmp_bufSize += ret_recvSize;

    if (ret_recvSize == 0)
    {
      cout << "Disconnected Server" << endl;
      return;
    }
  }

  int tmp_bufRecvSize = 0;
  char *bufRecv = new char[bufRecvSize + 4];
  while (bufRecvSize > tmp_bufRecvSize)
  {
    int ret_recv = recv(this->socPair1_fd, (void *)(bufRecv + tmp_bufRecvSize), bufRecvSize - tmp_bufRecvSize, 0);
    tmp_bufRecvSize += ret_recv;
  }

  string bufRecv_str(bufRecv, bufRecvSize);

  cout << __func__ << endl;
  this->sendClient(bufRecv_str);

  delete[] bufRecv;
  return;
}
// 읽는 쪽도 어느 소켓에서 읽을지를 구별해야댐
// 즉 멀티쓰레드 돌려야함

void Worker::sendClient(string &bufRecv_str)
{
  int bufRecvSize = bufRecv_str.size();
  int tmp_bufRecvSize = 0;

  while (4 > tmp_bufRecvSize)
  {
    int ret_sendSize = send(this->socClient_fd, ((char *)&bufRecvSize + tmp_bufRecvSize), 4 - tmp_bufRecvSize, 0);
    tmp_bufRecvSize += ret_sendSize;
  }

  tmp_bufRecvSize = 0;

  while (bufRecvSize > tmp_bufRecvSize)
  {
    int ret_sendSize = send(this->socClient_fd, (bufRecv_str.c_str() + tmp_bufRecvSize), bufRecvSize - tmp_bufRecvSize, 0);
    tmp_bufRecvSize += ret_sendSize;
  }

  cout << bufRecv_str << endl;
  cout << __func__ << endl;
  return;
}
  */