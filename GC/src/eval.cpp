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

#include "include/common.h"
#include "include/garble.h"
#include "include/gates.h"
#include "include/util.h"
#include "include/dkcipher.h"
#include "include/aes.h"
#include "include/justGarble.h"

#include <malloc.h>
#include <wmmintrin.h>

void evaluate(GarbledCircuit *garbledCircuit, ExtractedLabels extractedLabels,
              OutputMap outputMap) {

  GarbledTable *garbledTable = garbledCircuit->garbledTable;
  
  // set input wires
  for (int i = 0; i < garbledCircuit->n; i++) {
    garbledCircuit->wires[i].label = extractedLabels[i];
  }

  // set fixed wires
  DKCipherContext fixedWireCipherContext;
  DKCipherInit(&(garbledCircuit->fixedWiresSeed), &fixedWireCipherContext);

  block *fixedLabels = new block[garbledCircuit->fixedWireIndices.size()];
  for (uint32_t i = 0; i < garbledCircuit->fixedWireIndices.size(); i++) {
    fixedLabels[i] = makeBlock((long) i, (long) 0);
  }
  GC_AES_ecb_encrypt_blks(fixedLabels, garbledCircuit->fixedWireIndices.size(), &(fixedWireCipherContext.K));
  for (uint32_t i = 0; i < garbledCircuit->fixedWireIndices.size(); i++) {
    const pair<int, int> w = garbledCircuit->fixedWireIndices[i];
    garbledCircuit->wires[w.first].label = fixedLabels[i];
  }
  delete[] fixedLabels;

  // evaluate each gate of circuit
  int tableIndex = 0;

  DKCipherContext dkCipherContext;
  DKCipherInit(&(garbledCircuit->globalKey), &dkCipherContext);

  for (int i = 0; i < garbledCircuit->q; i++) {
    GarbledGate *garbledGate = &(garbledCircuit->garbledGates[i]);

    if (garbledGate->type == XORGATE) {
      garbledCircuit->wires[garbledGate->output].label =
            xorBlocks(garbledCircuit->wires[garbledGate->input0].label, 
                      garbledCircuit->wires[garbledGate->input1].label);
    } else {
      int lsb0 = getLSB(garbledCircuit->wires[garbledGate->input0].label);
      int lsb1 = getLSB(garbledCircuit->wires[garbledGate->input1].label);

      block tweak0 = makeBlock((long) 2*i, (long) 0);
      block tweak1 = makeBlock((long) 2*i + 1, (long) 0);

      block hashInputs[2];    
      hashInputs[0] = xorBlocks(DOUBLE(garbledCircuit->wires[garbledGate->input0].label), tweak0);
      hashInputs[1] = xorBlocks(DOUBLE(garbledCircuit->wires[garbledGate->input1].label), tweak1);

      block hashValues[2];
      memcpy(hashValues, hashInputs, sizeof(hashValues));
      GC_AES_ecb_encrypt_blks(hashValues, 2, &(dkCipherContext.K));
      for (int j = 0; j < 2; j++) {
        hashValues[j] = xorBlocks(hashValues[j], hashInputs[j]);
      }

      block WG = hashValues[0];
      if (lsb0 == 1) {
        WG = xorBlocks(WG, garbledTable[tableIndex].table[0]);
      }

      block WE = hashValues[1];
      if (lsb1 == 1) {
        WE = xorBlocks(WE, xorBlocks(garbledTable[tableIndex].table[1], garbledCircuit->wires[garbledGate->input0].label));     
      }

      garbledCircuit->wires[garbledGate->output].label = xorBlocks(WG, WE);

      tableIndex++;
    }
  }

  for (int i = 0; i < garbledCircuit->m; i++) {
    outputMap[i] = garbledCircuit->wires[garbledCircuit->outputs[i]].label;
  }
}
