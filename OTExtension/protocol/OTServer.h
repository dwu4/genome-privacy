/*
 * This code is based off the OT extension code from here:
 * https://github.com/encryptogroup/OTExtension.
 *
 */

#ifndef _OT_SERVER_H_
#define _OT_SERVER_H_

#include "../util/typedefs.h"
#include "../util/socket.h"
#include "../ot/naor-pinkas.h"
#include "../ot/asharov-lindell.h"
#include "../ot/ot-extension.h"
#include "../util/cbitvector.h"
#include "../ot/xormasking.h"

#include <iostream>
#include <vector>

#include "OTParty.h"

using namespace std;

class OTServer : public OTParty {
  public:
    OTServer() { }
    ~OTServer() {
        U.delCBitVector();
    }
    void InitOTSender(CSocket* socket);
    void InitOTSender(const char* addr, int port);

    BOOL ObliviouslySend(CBitVector& X1, CBitVector& X2, CBitVector& delta, uint64_t numOTs);
  private:
    BOOL Listen(const char* addr, int port);
    BOOL PrecomputeNaorPinkasSender();

    // Naor-Pinkas OT
    CBitVector U;
    BYTE *vKeySeeds;
    BYTE *vKeySeedMtx;
};

#endif
