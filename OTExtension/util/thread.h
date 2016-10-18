// thread.h by sgchoi@cs.umd.edu

#ifndef __THREAD_H__BY_SGCHOI  
#define __THREAD_H__BY_SGCHOI 

#include "typedefs.h"

class CThread {
public:
	CThread() {
		m_bRunning = FALSE;
	}
	virtual ~CThread(){}

public:
  BOOL Start() {
  	ThreadMain();
		return true;
	}

  BOOL Wait() {
  	return true;
  }

	BOOL Kill() {
		return true;
	}

protected:
	virtual void ThreadMain() = 0;

protected:
 	BOOL		m_bRunning;
};

#endif //__THREAD_H__BY_SGCHOI


