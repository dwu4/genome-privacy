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
#include "include/justGarble.h"

static void ADD22Circuit(GarbledCircuit *garbledCircuit, GarblingContext *garblingContext, int *inputs, int *outputs) {
  int wire1 = getNextWire(garblingContext);
  int wire2 = getNextWire(garblingContext);

  XORGate(garbledCircuit, garblingContext, inputs[0], inputs[1], wire1);
  ANDGate(garbledCircuit, garblingContext, inputs[0], inputs[1], wire2);

  outputs[0] = wire1;
  outputs[1] = wire2;
}

static void ADD32Circuit(GarbledCircuit *garbledCircuit, GarblingContext *garblingContext, int *inputs, int *outputs) {
  int wire1 = getNextWire(garblingContext);
  int wire2 = getNextWire(garblingContext);
  int wire3 = getNextWire(garblingContext);
  int wire4 = getNextWire(garblingContext);
  int wire5 = getNextWire(garblingContext);

  XORGate(garbledCircuit, garblingContext, inputs[2], inputs[0], wire1);
  XORGate(garbledCircuit, garblingContext, inputs[1], inputs[0], wire2);
  XORGate(garbledCircuit, garblingContext, inputs[2], wire2, wire3);
  ANDGate(garbledCircuit, garblingContext, wire1, wire2, wire4);
  XORGate(garbledCircuit, garblingContext, inputs[0], wire4, wire5);

  outputs[0] = wire3;
  outputs[1] = wire5;
}

void ADDCircuit(GarbledCircuit *garbledCircuit, GarblingContext *garblingContext, int n, int *inputs, int *outputs) {
  int split = n / 2;
  int tempIn[3];
  int tempOut[2];

  tempIn[0] = inputs[0];
  tempIn[1] = inputs[split];
  ADD22Circuit(garbledCircuit, garblingContext, tempIn, tempOut);
  outputs[0] = tempOut[0];

  for (int i = 1; i < split; i++) {
    tempIn[2] = tempOut[1];
    tempIn[1] = inputs[split + i];
    tempIn[0] = inputs[i];
    ADD32Circuit(garbledCircuit, garblingContext, tempIn, tempOut);
    outputs[i] = tempOut[0];
  }
}

void EQCircuit(GarbledCircuit *garbledCircuit, GarblingContext *garblingContext, int n, int *inputs, int *outputs) {
  int split = n / 2;

  int tempOut = fixedOneWire(garbledCircuit, garblingContext);
  for (int i = 0; i < split; i++) {
    int wire1 = getNextWire(garblingContext);
    int wire2 = fixedOneWire(garbledCircuit, garblingContext);
    int wire3 = getNextWire(garblingContext);
    int wire4 = getNextWire(garblingContext);

    XORGate(garbledCircuit, garblingContext, inputs[i], inputs[split + i], wire1);
    XORGate(garbledCircuit, garblingContext, wire1, wire2, wire3);
    ANDGate(garbledCircuit, garblingContext, tempOut, wire3, wire4);
    tempOut = wire4;
  }
  outputs[0] = tempOut;
}

void ANDCircuit(GarbledCircuit *garbledCircuit, GarblingContext *garblingContext, int n, int *inputs, int *outputs) {
  int split = n / 2;
  for (int i = 0; i < split; i++) {
    outputs[i] = getNextWire(garblingContext);
    ANDGate(garbledCircuit, garblingContext, inputs[i], inputs[split + i], outputs[i]);
  }
}

static void CMP31Circuit(GarbledCircuit *garbledCircuit, GarblingContext *garblingContext, int *inputs, int *outputs) {
  int wire1 = getNextWire(garblingContext);
  int wire2 = getNextWire(garblingContext);
  int wire3 = getNextWire(garblingContext);
  int wire4 = getNextWire(garblingContext);

  XORGate(garbledCircuit, garblingContext, inputs[0], inputs[2], wire1);
  XORGate(garbledCircuit, garblingContext, inputs[1], inputs[2], wire2);
  ANDGate(garbledCircuit, garblingContext, wire1, wire2, wire3);
  XORGate(garbledCircuit, garblingContext, inputs[0], wire3, wire4);

  outputs[0] = wire4;
}

void CMPCircuit(GarbledCircuit *garbledCircuit, GarblingContext *garblingContext, int n, int *inputs, int *outputs) {
  int split = n / 2;
  int tempIn[3];
  int tempOut;

  tempIn[0] = inputs[0];
  tempIn[1] = inputs[split];
  tempIn[2] = fixedZeroWire(garbledCircuit, garblingContext);

  CMP31Circuit(garbledCircuit, garblingContext, tempIn, &tempOut);

  for (int i = 1; i < split; i++) {
    tempIn[2] = tempOut;
    tempIn[0] = inputs[i];
    tempIn[1] = inputs[split + i];

    CMP31Circuit(garbledCircuit, garblingContext, tempIn, &tempOut);
  }

  outputs[0] = tempOut;
}

void MUXCircuit(GarbledCircuit *garbledCircuit, GarblingContext *garblingContext, int n, int *inputs, int *outputs) {
  int split = (n - 1) / 2;
  int muxBit = inputs[n - 1];

  for (int i = 0; i < split; i++) {
    int wire1 = getNextWire(garblingContext);
    int wire2 = getNextWire(garblingContext);
    outputs[i] = getNextWire(garblingContext);

    XORGate(garbledCircuit, garblingContext, inputs[i], inputs[split + i], wire1);
    ANDGate(garbledCircuit, garblingContext, wire1, muxBit, wire2);
    XORGate(garbledCircuit, garblingContext, inputs[i], wire2, outputs[i]);
  }
}

void MAXCircuit(GarbledCircuit *garbledCircuit, GarblingContext *garblingContext, int n, int *inputs, int *outputs) {
  int nbits = n / 2;
  int *muxInput = new int[n + 1];
  memcpy(muxInput, inputs + nbits, nbits*sizeof(int));
  memcpy(muxInput + nbits, inputs, nbits*sizeof(int));

  CMPCircuit(garbledCircuit, garblingContext, n, inputs, &muxInput[n]);
  MUXCircuit(garbledCircuit, garblingContext, n + 1, muxInput, outputs);

  delete[] muxInput;
}

// Computes the maximum element in a vector of k-bit values
void MAXVecCircuit(GarbledCircuit *garbledCircuit, GarblingContext *garblingContext, int k, int n, int *inputs, int *outputs) {
  if (n == 0 || n % k != 0) {
    dbgs("input must be a non-empty vector of k-bit values");
    exit(1);
  }

  int nElems = n / k;
  if (nElems == 1) {
    memcpy(outputs, inputs, sizeof(int) * n);
  }

  int tempIn[2 * k];
  int tempOut[k];
  memcpy(tempIn, inputs, sizeof(int) * 2 * k);
  MAXCircuit(garbledCircuit, garblingContext, 2 * k, tempIn, tempOut);

  for (int i = 2; i < nElems; i++) {  
    memcpy(tempIn, tempOut, sizeof(int) * k);
    memcpy(tempIn + k, inputs + k * i, sizeof(int) * k);
    MAXCircuit(garbledCircuit, garblingContext, 2 * k, tempIn, tempOut);
  }

  memcpy(outputs, tempOut, sizeof(int) * k);
}

// Computes the max and all of the arg-maxes in a vector of k-bit values
// (represented as a bit-string)
void ARGMAXVecCircuit(GarbledCircuit *garbledCircuit, GarblingContext *garblingContext, int k, int n, int *inputs, int *outputs) {
  if (n == 0 || n % k != 0) {
    dbgs("input must be a non-empty vector of k-bit values");
    exit(1);
  }

  int nElems = n / k;
  int eqInputs[2*k];

  MAXVecCircuit(garbledCircuit, garblingContext, k, n, inputs, eqInputs + k);
  
  for (int i = 0; i < nElems; i++) {
    memcpy(eqInputs, inputs + k * i, sizeof(int) * k);
    EQCircuit(garbledCircuit, garblingContext, 2*k, eqInputs, &outputs[i]);
  }
  memcpy(outputs + nElems, eqInputs + k, sizeof(int) * k);
}

// Computes the max and all of the arg-maxes in an additively-shared vector of k-bit
// values (represented as a bit-string)
void ARGMAXVecSharedCircuit(GarbledCircuit *garbledCircuit, GarblingContext *garblingContext, int k, int n, int *inputs, int *outputs) {
  if (n == 0 || n % 2 != 0 || n % (2 * k) != 0) {
    dbgs("input must be two non-empty vectors of k-bit values");
    exit(1);
  }

  int *summedVector = new int[n / 2];
  int sharesVec[2*k];
  int nElems = n / (2 * k);
  int split = n / 2;
  for (int i = 0; i < nElems; i++) {
    memcpy(sharesVec, inputs + k*i, sizeof(int) * k);
    memcpy(sharesVec + k, inputs + split + k*i, sizeof(int) * k);
    ADDCircuit(garbledCircuit, garblingContext, 2*k, sharesVec, summedVector + k*i);  
  }
  ARGMAXVecCircuit(garbledCircuit, garblingContext, k, n / 2, summedVector, outputs);

  delete[] summedVector;
}

void SetDiffVecSharedCircuit(GarbledCircuit *garbledCircuit, GarblingContext *garblingContext, int k, int n, int *inputs, int *outputs) {
  if (n == 0 || n % 2 != 0 || n % (2 * k + 2) != 0) {
    dbgs("input must be two non-empty vectors of k-bit values");
    exit(1);
  }

  int nElems = n / (2 * k + 2);
  int *summedVec = new int[nElems * k];
  int sharesVec[2*k];
  int split = n / 2;
  for (int i = 0; i < nElems; i++) {
    memcpy(sharesVec, inputs + k*i, sizeof(int) * k);
    memcpy(sharesVec + k, inputs + split + k*i, sizeof(int) * k);
    ADDCircuit(garbledCircuit, garblingContext, 2*k, sharesVec, summedVec + k*i);
  }

  int *xorSharedVec = new int[nElems];
  for (int i = 0; i < nElems; i++) {
    xorSharedVec[i] = getNextWire(garblingContext);
    XORGate(garbledCircuit, garblingContext, inputs[nElems * k + i], inputs[split + nElems * k + i], xorSharedVec[i]);
  }

  int *threshVec = new int[nElems];
  
  int zeroWire = fixedZeroWire(garbledCircuit, garblingContext);
  int eqVec[2*k];
  for (int i = k; i < 2*k; i++) {
    eqVec[i] = zeroWire;
  }

  for (int i = 0; i < nElems; i++) {
    memcpy(eqVec, summedVec + k*i, sizeof(int) * k);
    EQCircuit(garbledCircuit, garblingContext, 2*k, eqVec, threshVec + i);
  }

  for (int i = 0; i < nElems; i++) {
    outputs[i] = getNextWire(garblingContext);
    ANDGate(garbledCircuit, garblingContext, threshVec[i], xorSharedVec[i], outputs[i]);
  }

  delete[] summedVec;
  delete[] xorSharedVec;
  delete[] threshVec;
}
