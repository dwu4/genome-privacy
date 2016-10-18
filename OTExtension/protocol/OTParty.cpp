/*
 * This code is based off the OT extension code from here:
 * https://github.com/encryptogroup/OTExtension.
 *
 */

#include "OTParty.h"

OTParty::OTParty() {
  sock = NULL;
  isHeap = false;
}

OTParty::~OTParty() {
  Cleanup();
  if (isHeap) {
   delete sock;
  }
}

BOOL OTParty::Init() {
  const char* m_nSeed = "437398417012387813714564100";

  // Initialize parameters
  m_nSecParam = 283;
  m_bUseECC = true;
  m_nNumOTThreads = 1;
  m_nCounter = 0;

  // Invoke the random oracle to obtain a seed
  SHA_CTX sha;
  OTEXT_HASH_INIT(&sha);
  OTEXT_HASH_UPDATE(&sha, (BYTE*) m_nSeed, sizeof(m_nSeed));
  OTEXT_HASH_FINAL(&sha, m_aSeed);

  bot = new NaorPinkas(m_nSecParam, m_aSeed, m_bUseECC);

  return TRUE;
}

BOOL OTParty::Cleanup() {
  sock->Close();
  delete bot;

  return true;
}
