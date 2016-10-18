//socket.h (adapted)

#ifndef __SOCKET_H__BY_SGCHOI 
#define __SOCKET_H__BY_SGCHOI 

#include "typedefs.h"
#include <stdint.h>

class CSocket {

public:
  CSocket() { 
    m_hSock = INVALID_SOCKET;
    bytesSent = 0;
    bytesReceived = 0;
    networkTime = 0;
  }

  ~CSocket(){ }
  
  BOOL Socket() {
    BOOL success = false;
    BOOL bOptVal = true;
    int bOptLen = sizeof(BOOL);
    
    Close();

    success = (m_hSock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) != INVALID_SOCKET; 
    
    return success;
  }

  void Close() {
    if (m_hSock == INVALID_SOCKET) {
      return;
    }
    
    shutdown(m_hSock, SHUT_WR);
    close(m_hSock);
  
    m_hSock = INVALID_SOCKET; 
  } 

  void AttachFrom(CSocket& s) {
    m_hSock = s.m_hSock;
  }

  void Detach() {
    m_hSock = INVALID_SOCKET;
  }

public:
  string GetIP() {
    sockaddr_in addr;
    UINT addr_len = sizeof(addr);

    if (getsockname(m_hSock, (sockaddr *) &addr, (socklen_t *) &addr_len) < 0) return "";
    return inet_ntoa(addr.sin_addr);
  }


  USHORT GetPort() {
    sockaddr_in addr;
    UINT addr_len = sizeof(addr);

    if (getsockname(m_hSock, (sockaddr *) &addr, (socklen_t *) &addr_len) < 0) return 0;
    return ntohs(addr.sin_port);
  }
  
  BOOL Bind(USHORT nPort=0, string ip = "") {
    // Bind the socket to its port
    sockaddr_in sockAddr;
    memset(&sockAddr,0,sizeof(sockAddr));
    sockAddr.sin_family = AF_INET;

    if( ip != "" ) {
      int on = 1;
      setsockopt(m_hSock, SOL_SOCKET, SO_REUSEADDR, (const char*) &on, sizeof(on));

      sockAddr.sin_addr.s_addr = inet_addr(ip.c_str());

      if (sockAddr.sin_addr.s_addr == INADDR_NONE) {
        hostent* phost;
        phost = gethostbyname(ip.c_str());
        if (phost != NULL) {
          sockAddr.sin_addr.s_addr = ((in_addr*)phost->h_addr)->s_addr;
        }
        else {
          return FALSE;
        }
      }
    }
    else {
      sockAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    }
    
    sockAddr.sin_port = htons(nPort);

    return bind(m_hSock, (sockaddr *) &sockAddr, sizeof(sockaddr_in)) >= 0; 
  }

  BOOL Listen(int nQLen = 5) {
    return listen(m_hSock, nQLen) >= 0;
  } 

  BOOL Accept(CSocket& sock) {
    sock.m_hSock = accept(m_hSock, NULL, 0);
    if( sock.m_hSock == INVALID_SOCKET ) return FALSE;
 
    return TRUE;
  }
   
  BOOL Connect(string ip, USHORT port, LONG lTOSMilisec = -1) {
    sockaddr_in sockAddr;
    memset(&sockAddr,0,sizeof(sockAddr));
    sockAddr.sin_family = AF_INET;
    sockAddr.sin_addr.s_addr = inet_addr(ip.c_str());

    if (sockAddr.sin_addr.s_addr == INADDR_NONE) {
      hostent* lphost;
      lphost = gethostbyname(ip.c_str());
      if (lphost != NULL) {
        sockAddr.sin_addr.s_addr = ((in_addr*)lphost->h_addr)->s_addr;
      }
      else {
        return FALSE;
      }
    }

    sockAddr.sin_port = htons(port);
  
    timeval tv;
    socklen_t len;
    
    if (lTOSMilisec > 0) {
      tv.tv_sec = lTOSMilisec/1000;
      tv.tv_usec = (lTOSMilisec%1000)*1000;
  
      setsockopt(m_hSock, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
    }

    int ret = connect(m_hSock, (sockaddr*)&sockAddr, sizeof(sockAddr));
    
    if (ret >= 0 && lTOSMilisec > 0) {
      tv.tv_sec = 100000; 
      tv.tv_usec = 0;

      setsockopt(m_hSock, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
    }

    return ret >= 0;
  }

  int Receive(void* pBuf, int nLen, int nFlags = 0) {
    clock_t startTime = clock();

    bytesReceived += nLen;

    char* p = (char*) pBuf;
    int n = nLen;
    int ret = 0;
    while (n > 0) {
      ret = recv(m_hSock, p, n, 0);
      if (ret < 0) {
        if (errno == EAGAIN) {
          cout << "socket recv eror: EAGAIN" << endl;
          SleepMiliSec(200);
          continue;
        } else {
          cout << "socket recv error: " << errno << endl;
          networkTime += (clock() - startTime);
          return ret;
        }
      } else if (ret == 0) {
        networkTime += (clock() - startTime);
        return ret;
      }
      p += ret;
      n -= ret;
    }
    networkTime += (clock() - startTime);
    return nLen;
  }
 
  int Send(const void* pBuf, int nLen, int nFlags = 0) {
    clock_t startTime = clock();

    bytesSent += nLen;
    int ret = send(m_hSock, (char*)pBuf, nLen, nFlags);

    networkTime += (clock() - startTime);

    return ret;
  }

  static const int BLK_SIZE = 2147483647;

  void SendLarge(const uint8_t* pBuf, uint64_t len) {
    int nBatches = (len + BLK_SIZE - 1) / BLK_SIZE;
    for (int i = 0; i < nBatches; i++) {
      int batchSize = BLK_SIZE;
      if (i == nBatches - 1) {
        batchSize = len % BLK_SIZE;
        if (batchSize == 0) {
          batchSize = BLK_SIZE;
        }
      }
      Send(pBuf + i * BLK_SIZE, batchSize);
    }
  }

  void ReceiveLarge(uint8_t* pBuf, uint64_t len) {
    int nBatches = (len + BLK_SIZE - 1) / BLK_SIZE;
    for (int i = 0; i < nBatches; i++) {
      int batchSize = BLK_SIZE;
      if (i == nBatches - 1) {
        batchSize = len % BLK_SIZE;
        if (batchSize == 0) {
          batchSize = BLK_SIZE;
        }
      }
      Receive(pBuf + i * BLK_SIZE, batchSize);
    }
  }

  uint64_t GetBytesSent() {
    return bytesSent;
  }

  uint64_t GetBytesReceived() {
    return bytesReceived;
  }

  double GetNetworkTime() {
    return ((double) networkTime) / CLOCKS_PER_SEC;
  }

  void ResetStats() {
    bytesSent = 0;
    bytesReceived = 0;
  }
    
private:
  SOCKET  m_hSock;

  uint64_t bytesSent;
  uint64_t bytesReceived;

  clock_t networkTime;
};

#endif
