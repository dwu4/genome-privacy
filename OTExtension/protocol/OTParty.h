/*
 * This code is based off the OT extension code from here:
 * https://github.com/encryptogroup/OTExtension.
 *
 */

#ifndef _OT_PARTY_H_
#define _OT_PARTY_H_

#include "../util/typedefs.h"
#include "../util/socket.h"
#include "../ot/naor-pinkas.h"
#include "../ot/asharov-lindell.h"
#include "../ot/ot-extension.h"
#include "../util/cbitvector.h"
#include "../ot/xormasking.h"

#include <iostream>
#include <vector>

class OTParty {
  public:
    OTParty();
    ~OTParty();
  protected:
    BOOL Init();
    BOOL Cleanup();

    // Network Communication
    int m_nSecParam;
    bool m_bUseECC;
    int m_nBitLength;
    int m_nMod;

    // Kind of a hack...
    CSocket *sock;
    bool isHeap;

    int m_nNumOTThreads;

    // Naor-Pinkas OT
    BaseOT* bot;

    // SHA PRG
    BYTE        m_aSeed[SHA1_BYTES];
    int         m_nCounter;
    double      rndgentime;
};

#endif
