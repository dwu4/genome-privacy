/*
 * This code is based off the OT extension code from here:
 * https://github.com/encryptogroup/OTExtension.
 *
 */

#include "OTClient.h"

BOOL OTClient::Connect(const char* addr, int port) {
  BOOL bFail = FALSE;
  LONG lTO = CONNECT_TIMEO_MILISEC;

  for (int i = 0; i < RETRY_CONNECT; i++) {
    if (!sock->Socket()) {
      cout << "Socket failure: ";
      goto connect_failure;
    }

    if (sock->Connect(addr, port, lTO)) {
      return true;
    }

    SleepMiliSec(20);
    if (i + 1 == RETRY_CONNECT) {
      goto server_not_available;
    }
  }

server_not_available:
  cout << "server not available" << endl;
connect_failure:
  cout << "connection failed" << endl;
  return FALSE;
}

BOOL OTClient::PrecomputeNaorPinkasClient() {
  int nSndVals = 2;

  // Execute NP receiver routine and obtain the key
  BYTE* pBuf = new BYTE[SHA1_BYTES * NUM_EXECS_NAOR_PINKAS * nSndVals];

  // N-P sender: send: C0 (=g^r), C1, C2, C3
  bot->Sender(nSndVals, NUM_EXECS_NAOR_PINKAS, *sock, pBuf);

  //Key expansion
  BYTE* pBufIdx = pBuf;
  for(int i = 0; i < NUM_EXECS_NAOR_PINKAS * nSndVals; i++) {
    memcpy(vKeySeedMtx + i * AES_KEY_BYTES, pBufIdx, AES_KEY_BYTES);
    pBufIdx += SHA1_BYTES;
  }

  delete[] pBuf;

  return TRUE;
}

void OTClient::InitOTClient(CSocket* socket) {
  Init();
  sock = socket;
}

void OTClient::InitOTClient(const char* addr, int port) {
  Init();

  if (sock == NULL) {
    sock = new CSocket();
    isHeap = true;
  }

  Connect(addr, port);
}

BOOL OTClient::ObliviouslyReceive(BYTE* labels, CBitVector& choices, uint64_t nInputs) {
  int nSndVals = 2;
  vKeySeedMtx = (BYTE*) malloc(AES_KEY_BYTES*NUM_EXECS_NAOR_PINKAS * nSndVals);

  PrecomputeNaorPinkasClient();
  receiver = new OTExtensionReceiver(nSndVals, sock, vKeySeedMtx, m_aSeed);

  int bitlength = 128;

  MaskingFunction* maskFn = new XORMasking(bitlength);

  // Prepare choice bits and OT response vector
  CBitVector ret;
  ret.Create(nInputs * bitlength);

  // Engage in OT with the server
  if (!receiver->receive(nInputs, bitlength, choices, ret, C_OT, m_nNumOTThreads, maskFn)) {
    return FALSE;
  }

  memcpy(labels, ret.GetArr(), bitlength / 8 * nInputs);

  free(vKeySeedMtx);
  delete maskFn;
  delete receiver;

  return TRUE;
}
