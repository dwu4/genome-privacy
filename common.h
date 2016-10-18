/*
 * Copyright (c) 2016, David J. Wu
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef __COMMON_H__
#define __COMMON_H__

#include "GC/include/justGarble.h"
#include "GC/include/circuits.h"

#include "OTExtension/protocol/OTClient.h"
#include "OTExtension/protocol/OTServer.h"
#include "OTExtension/util/cbitvector.h"

#include <iomanip>
#include <iostream>
#include <openssl/sha.h>

#define makeBlock(X,Y) _mm_set_epi64((__m64)(X), (__m64)(Y))

using namespace std;

typedef unsigned char byte;

struct ArgMaxArgs {
  uint32_t nElems;
  uint32_t nBits;

  ArgMaxArgs(uint32_t nElems, uint32_t nBits) : nElems(nElems), nBits(nBits) { }
};

struct BasicIntersectionArgs {
  uint32_t nElems;

  BasicIntersectionArgs(uint32_t nElems) : nElems(nElems) { }
};

struct SetDiffArgs {
  uint32_t nElems;
  uint32_t nBits;

  SetDiffArgs(uint32_t nElems, uint32_t nBits) : nElems(nElems), nBits(nBits) { }
};

void CreateArgMaxCircuit(GarbledCircuit& circuit, int nElems, int nBits);
void CreateBasicIntersectionCircuit(GarbledCircuit& circuit, int nElems);
void CreateSetDiffCircuit(GarbledCircuit& circuit, int nElems, int nBits);

bool Listen(CSocket* socket, int port);
bool Connect(CSocket* socket, const char* address, int port);
void StartServer(int port, byte* input, void* args,
                 void (*RunProtocol)(CSocket*, byte*, void*));
void StartClient(const char *address, int port, byte* input, void* params,
                 void (*RunProtocol)(CSocket*, byte*, void*));

void CreateChoiceVec(CBitVector& choices, byte* input, uint64_t len);
uint32_t RunServerProtocol(CSocket* socket, GarbledCircuit& circuit, byte* input, uint32_t nInputWires,
                           uint32_t nServerInputWires);
void RunClientProtocol(CSocket* socket, GarbledCircuit& circuit, byte* input, int* outputVals,
                       uint32_t nInputWires, uint32_t nClientInputWires, uint32_t nOutputWires);
void PrintStatistics(CSocket* socket, GarbledCircuit& circuit, double timeElapsed);

static void PrintBlock(block& b) {
  cout << setw(24) << *((uint64_t*) &b) << " " << setw(24) << *(((uint64_t*) &b) + 1);
}

static void SHA_256(uint8_t* dst, uint8_t* src, uint64_t len) {
  SHA256_CTX ctx;
  SHA256_Init(&ctx);
  SHA256_Update(&ctx, src, len);
  SHA256_Final(dst, &ctx);
}

static void PrintSHA(byte* buf, uint64_t len) {
  uint8_t hashBuf[32];
  SHA_256(hashBuf, buf, len);
  for (int i = 0; i < 32; i++) {
    cout << setw(2) << setfill('0') << (hex) << ((unsigned int) hashBuf[i]);
  }
  cout << endl << (dec);
}

static bool BlockEqual(block& a, block& b) {
  long* ap = (long*) &a;
  long* bp = (long*) &b;

  return ((ap[0] == bp[0]) && (ap[1] == bp[1]));
}

static inline void Log(const string& type, const string& msg) {
  cout << "[" << type << "] " << msg << endl;
}

static inline void ServerLog(const string& msg) {
    Log("server", msg);
}

static inline void ClientLog(const string& msg) {
    Log("client", msg);
}

static inline bool GetRandomSeed(byte* buf, uint64_t len) {
  FILE* f = fopen("/dev/urandom", "r");
  if (f == NULL) {
    return false;
  }

  if (fread(buf, 1, len, f) != len) {
    return false;
  }

  fclose(f);
  return true;
}

bool ReadInputFile(byte* buf, const char* filename, int len);

#endif
