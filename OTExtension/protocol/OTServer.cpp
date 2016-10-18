/*
 * This code is based off the OT extension code from here:
 * https://github.com/encryptogroup/OTExtension.
 *
 */

#include "OTServer.h"

BOOL OTServer::Listen(const char* addr, int port) {
  CSocket newSock;
 
  if (!sock->Socket()) {
    goto listen_failure;
  }
  if (!sock->Bind(port, addr))
    goto listen_failure;
  if (!sock->Listen())
    goto listen_failure;

  if (!sock->Accept(newSock)) {
    cerr << "Error in accept" << endl;
    goto listen_failure;
  }

  sock->AttachFrom(newSock);
  newSock.Detach();

  return TRUE;

listen_failure:
  cout << "Listen failed" << endl;
  return FALSE;
}

BOOL OTServer::PrecomputeNaorPinkasSender() {
  int nSndVals = 2;
  BYTE* pBuf = new BYTE[NUM_EXECS_NAOR_PINKAS * SHA1_BYTES];

  int log_nVals = (int) ceil(log(nSndVals)/log(2));

  U.Create(NUM_EXECS_NAOR_PINKAS * log_nVals, m_aSeed, m_nCounter);

  bot->Receiver(nSndVals, NUM_EXECS_NAOR_PINKAS, U, *sock, pBuf);

  // Key expansion
  BYTE* pBufIdx = pBuf;

  // HF calls for the Naor Pinkas protocol
  for(int i = 0; i < NUM_EXECS_NAOR_PINKAS; i++) {
    memcpy(vKeySeeds + i * AES_KEY_BYTES, pBufIdx, AES_KEY_BYTES);
    pBufIdx += SHA1_BYTES;
  }
  delete[] pBuf;

  return TRUE;
}

void OTServer::InitOTSender(CSocket* socket) {
  Init();
  sock = socket;
}

void OTServer::InitOTSender(const char* addr, int port) {
  Init();

  if (sock == NULL) {
    sock = new CSocket();
    isHeap = true;
  }

  // Server listen
  cout << "[server] Listening for connections on " << addr
     << " on port " << port << endl;
  Listen(addr, port);
}

BOOL OTServer::ObliviouslySend(CBitVector& X1, CBitVector& X2, CBitVector& delta, uint64_t numOTs) {
  int nSndVals = 2;
  vKeySeeds = new BYTE[AES_KEY_BYTES*NUM_EXECS_NAOR_PINKAS];

  PrecomputeNaorPinkasSender();
  OTExtensionSender* sender = new OTExtensionSender(nSndVals, sock, U, vKeySeeds);

  int bitlength = 128;
  MaskingFunction* maskFn = new XORMasking(bitlength);

  CBitVector deltaReplicated;
  deltaReplicated.Create(numOTs * bitlength);
  for (int i = 0; i < numOTs; i++) {
    deltaReplicated.SetBits(delta.GetArr(), i*bitlength, bitlength);
  }

  X1.Create(numOTs * bitlength);
  X2.Create(numOTs * bitlength);

  bool success = sender->send(numOTs, bitlength, X1, X2, deltaReplicated, C_OT, 1, maskFn);

  delete[] vKeySeeds;
  delete sender;
  delete maskFn;

  return success;
}
