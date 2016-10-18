/*
 * This code is based off the OT extension code from here:
 * https://github.com/encryptogroup/OTExtension.
 *
 */

#ifndef _OT_CLIENT_H
#define _OT_CLIENT_H

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

class OTClient : public OTParty {
  public:
    OTClient() { }
    void InitOTClient(CSocket* socket);
    void InitOTClient(const char* addr, int port);

    BOOL ObliviouslyReceive(BYTE* msgBuf, CBitVector& choices, uint64_t nInputs);
  private:
    BOOL Connect(const char* addr, int port);
    BOOL PrecomputeNaorPinkasClient();

    // Naor-Pinkas OT
    OTExtensionReceiver *receiver;
    CBitVector U;
    BYTE *vKeySeeds;
    BYTE *vKeySeedMtx;
};

#endif
