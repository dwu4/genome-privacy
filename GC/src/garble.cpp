/*
 * Much of this code is taken from JustGarble
 * (https://github.com/irdan/justGarble). Our implementation is a
 * slimmed-down version of JustGarble and supports the half-gates
 * optimization (https://eprint.iacr.org/2014/756).
 *
 * JustGarble is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * JustGarble is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with JustGarble.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "include/garble.h"
#include "include/common.h"
#include "include/gates.h"
#include "include/util.h"
#include "include/dkcipher.h"
#include "include/aes.h"
#include "include/justGarble.h"
#include <malloc.h>
#include <stdbool.h>
#include <time.h>
#include <wmmintrin.h>

static unsigned long currentId;

// Compute AES in place. out is a block and sched is a pointer to an 
// expanded AES key.
#define inPlaceAES(out, sched) {int jx; out = _mm_xor_si128(out, sched[0]);\
                                for (jx = 1; jx < 10; jx++)\
                                  out = _mm_aesenc_si128(out, sched[jx]);\
                                out = _mm_aesenclast_si128(out, sched[jx]);}

static block __current_rand_index;
static GC_AES_KEY __rand_aes_key;

#define getRandContext() ((__m128i *) (__rand_aes_key.rd_key));
#define randAESBlock(out,sched) {__current_rand_index++; *out = __current_rand_index; inPlaceAES(*out,sched);}

int already_initialized = 0;
void seedRandom() {
  if (!already_initialized) {
    uint32_t seed[4];

    FILE* f = fopen("/dev/urandom", "r");
    if (f == NULL) {
      dbgs("unable to seed PRG");
    }

    if (fread(seed, 1, sizeof(seed), f) != sizeof(seed)) {
      dbgs("unable to seed PRG");
    }

    fclose(f);

    already_initialized = 1;
    __current_rand_index = zero_block();
    block cur_seed = _mm_set_epi32(seed[0], seed[1], seed[2], seed[3]);
    GC_AES_set_encrypt_key((unsigned char *) &cur_seed, 128, &__rand_aes_key);
  }
}

block randomBlock() {
  block out;
  const __m128i *sched = ((__m128i *) (__rand_aes_key.rd_key));
  randAESBlock(&out, sched);
  return out;
}

int getNextId() {
  currentId++;
  return currentId;
}

int getFreshId() {
  currentId = 0;
  return currentId;
}

int getNextWire(GarblingContext *garblingContext) {
  int i = garblingContext->wireIndex;
  garblingContext->wireIndex++;
  return i;
}

// n :       number of input wires
// m :       number of output wires
// q :       number of gates
// r :       number of wires
// r_fixed : number of fixed wires
void createEmptyGarbledCircuit(GarbledCircuit *garbledCircuit, int n, int m, int q, int r) {
  memset(garbledCircuit, 0, sizeof(GarbledCircuit));
  garbledCircuit->id = getNextId();

  garbledCircuit->garbledGates = (GarbledGate*)  memalign(128, sizeof(GarbledGate) * q);
  garbledCircuit->garbledTable = (GarbledTable*) memalign(128, sizeof(GarbledTable) * q);
  garbledCircuit->wires = (Wire*) memalign(128, sizeof(Wire) * r);
  garbledCircuit->outputs = (int*) memalign(128, sizeof(int) * m);

  if (garbledCircuit->garbledGates == NULL ||
      garbledCircuit->garbledGates == NULL || 
      garbledCircuit->wires        == NULL ||
      garbledCircuit->outputs      == NULL) {
    dbgs("Memory allocation error");
    exit(1);
  }

  memset(garbledCircuit->garbledGates, 0, sizeof(GarbledGate) * q);
  memset(garbledCircuit->garbledTable, 0, sizeof(GarbledTable) * q);
  memset(garbledCircuit->wires, 0, sizeof(Wire) * r);
  memset(garbledCircuit->outputs, 0, sizeof(int) * m);

  garbledCircuit->m = m;
  garbledCircuit->n = n;
  garbledCircuit->q = q;
  garbledCircuit->r = r;
}

void removeGarbledCircuit(GarbledCircuit *garbledCircuit) {
  free(garbledCircuit->wires);
  free(garbledCircuit->garbledGates);
  free(garbledCircuit->outputs);
  free(garbledCircuit->garbledTable);
}

void startBuilding(GarbledCircuit *garbledCircuit, GarblingContext *garblingContext) {
  memset(garblingContext, 0, sizeof(GarblingContext));
  garblingContext->wireIndex = garbledCircuit->n;
}

void finishBuilding(GarbledCircuit *garbledCircuit, GarblingContext *garblingContext, int *outputs) {
  garbledCircuit->q = garblingContext->gateIndex;
  garbledCircuit->r = garblingContext->wireIndex;
  memcpy(garbledCircuit->outputs, outputs, garbledCircuit->m * sizeof(int));
}

void extractLabels(ExtractedLabels extractedLabels, InputLabels inputLabels,
                   uint8_t *inputBits, int n) {
  for (int i = 0; i < n; i++) {
    if (inputBits[i]) {
      extractedLabels[i] = inputLabels[2 * i + 1];
    } else {
      extractedLabels[i] = inputLabels[2 * i];
    }
  }
}

void createInputLabels(InputLabels inputLabels, int n) {
  seedRandom();
  block R = randomBlock();
  block mask = makeBlock((uint64_t) 0, (uint64_t) 1);
  R |= mask;

  createInputLabels(inputLabels, n, R);
}

void createInputLabels(InputLabels inputLabels, int n, block R) {
  seedRandom();
  block *rand_context = getRandContext();
  for (int i = 0; i < 2*n; i += 2) {
    randAESBlock(&inputLabels[i], rand_context);
    inputLabels[i + 1] = xorBlocks(R, inputLabels[i]);
  }
}

void garbleCircuit(GarbledCircuit *garbledCircuit, InputLabels inputLabels, OutputMap outputMap) {
  seedRandom();
  garbledCircuit->id = getFreshId();

  for(int i = 0; i < 2*garbledCircuit->n; i += 2) {
    garbledCircuit->wires[i/2].id = i / 2;
    garbledCircuit->wires[i/2].label0 = inputLabels[i];
    garbledCircuit->wires[i/2].label1 = inputLabels[i+1];
  }
  block R = xorBlocks(garbledCircuit->wires[0].label0, garbledCircuit->wires[0].label1);

  // initialize fixed wires from a PRG (built from AES)
  garbledCircuit->fixedWiresSeed = randomBlock();
  DKCipherContext fixedWireCipherContext;
  DKCipherInit(&(garbledCircuit->fixedWiresSeed), &fixedWireCipherContext);

  block *fixedLabels = new block[garbledCircuit->fixedWireIndices.size()];
  for (uint32_t i = 0; i < garbledCircuit->fixedWireIndices.size(); i++) {
    fixedLabels[i] = makeBlock((long) i, (long) 0);
  }
  GC_AES_ecb_encrypt_blks(fixedLabels, garbledCircuit->fixedWireIndices.size(), &(fixedWireCipherContext.K));

  for (uint32_t i = 0; i < garbledCircuit->fixedWireIndices.size(); i++) {
    const pair<int, int> w = garbledCircuit->fixedWireIndices[i];

    if (w.second == FIXED_ZERO_GATE) {
      garbledCircuit->wires[w.first].label0 = fixedLabels[i];
      garbledCircuit->wires[w.first].label1 = xorBlocks(fixedLabels[i], R);
    } else if (w.second == FIXED_ONE_GATE) {
      garbledCircuit->wires[w.first].label1 = fixedLabels[i];
      garbledCircuit->wires[w.first].label0 = xorBlocks(fixedLabels[i], R);
    }
  }
  delete[] fixedLabels;

  // garble each gate of circuit
  garbledCircuit->globalKey = randomBlock();
  GarbledTable *garbledTable = garbledCircuit->garbledTable;

  DKCipherContext dkCipherContext;
  DKCipherInit(&(garbledCircuit->globalKey), &dkCipherContext);
  int tableIndex = 0;

  for(int i = 0; i < garbledCircuit->q; i++) {
    GarbledGate *garbledGate = &(garbledCircuit->garbledGates[i]);

    int input0 = garbledGate->input0;
    int input1 = garbledGate->input1;
    int output = garbledGate->output;

    if (garbledGate->type == XORGATE) {
      garbledCircuit->wires[output].label0 = xorBlocks(garbledCircuit->wires[input0].label0, garbledCircuit->wires[input1].label0);
      garbledCircuit->wires[output].label1 = xorBlocks(garbledCircuit->wires[input0].label1, garbledCircuit->wires[input1].label0);
      continue;
    } else if (garbledGate->type != ANDGATE) {
      dbgs("currently only support AND and XOR gates");
      exit(1);
    }

    int lsb0 = getLSB(garbledCircuit->wires[input0].label0);
    int lsb1 = getLSB(garbledCircuit->wires[input1].label0);

    block tweak0 = makeBlock((long) 2*i, (long) 0);
    block tweak1 = makeBlock((long) 2*i + 1, (long) 0);

    block hashInputs[4];
    hashInputs[0] = xorBlocks(DOUBLE(garbledCircuit->wires[input0].label0), tweak0);
    hashInputs[1] = xorBlocks(DOUBLE(garbledCircuit->wires[input0].label1), tweak0);

    hashInputs[2] = xorBlocks(DOUBLE(garbledCircuit->wires[input1].label0), tweak1);
    hashInputs[3] = xorBlocks(DOUBLE(garbledCircuit->wires[input1].label1), tweak1);

    block hashValues[4];
    memcpy(hashValues, hashInputs, sizeof(hashValues));
    GC_AES_ecb_encrypt_blks(hashValues, 4, &(dkCipherContext.K));
    for (int j = 0; j < 4; j++) {
      hashValues[j] = xorBlocks(hashValues[j], hashInputs[j]);
    }

    // first half gate
    block WG = hashValues[0];
    block TG = xorBlocks(hashValues[0], hashValues[1]);
    if (lsb1 == 1) {
      TG = xorBlocks(TG, R);
    }
    if (lsb0 == 1) {  
      WG = xorBlocks(WG, TG);
    }

    // second half gate
    block WE = hashValues[2];
    block TE = xorBlocks(hashValues[2], hashValues[3]);
    if (lsb1 == 1) {
      WE = xorBlocks(WE, TE);
    }
    TE = xorBlocks(TE, garbledCircuit->wires[input0].label0);

    block newToken = xorBlocks(WG, WE);

    garbledCircuit->wires[garbledGate->output].label0 = newToken;
    garbledCircuit->wires[garbledGate->output].label1 = xorBlocks(R, newToken);

    garbledTable[tableIndex].table[0] = TG;
    garbledTable[tableIndex].table[1] = TE;

    tableIndex++;
  }

  for(int i = 0; i < garbledCircuit->m; i++) {
    outputMap[2*i]   = garbledCircuit->wires[garbledCircuit->outputs[i]].label0;
    outputMap[2*i+1] = garbledCircuit->wires[garbledCircuit->outputs[i]].label1;
  }

  garbledCircuit->nAndGates = tableIndex;
}

int blockEqual(block a, block b) {
  long *ap = (long*) &a;
  long *bp = (long*) &b;
  if ((ap[0] == bp[0]) && (ap[1] == bp[1])) {
    return 1;
  } else {
    return 0;
  }
}

void mapOutputs(OutputMap outputMap, OutputMap outputMap2, int *vals, int m) {
  for (int i = 0; i < m; i++) {
    if (blockEqual(outputMap2[i], outputMap[2 * i])) {
      vals[i] = 0;
    } else if (blockEqual(outputMap2[i], outputMap[2 * i + 1])) {
      vals[i] = 1;
    } else {
      vals[i] = -1;
      printf("MAP FAILED %d\n", i);
    }
  }
}
