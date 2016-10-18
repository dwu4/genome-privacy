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
#include "include/justGarble.h"

static int genericGate(GarbledCircuit *garbledCircuit, GarblingContext *garblingContext, int input0, int input1, int output, int type) {
  garbledCircuit->wires[output].id = output;

  GarbledGate *garbledGate = &(garbledCircuit->garbledGates[garblingContext->gateIndex]);

  garbledGate->id = garblingContext->gateIndex;
  garbledGate->type = type;
  garbledGate->input0 = input0;
  garbledGate->input1 = input1;
  garbledGate->output = output;

  garblingContext->gateIndex++;
  garblingContext->tableIndex++;

  return garbledGate->id;
}

int ANDGate(GarbledCircuit *garbledCircuit, GarblingContext *garblingContext, int input0, int input1, int output) {
  return genericGate(garbledCircuit, garblingContext, input0, input1, output, ANDGATE);
}

int ORGate(GarbledCircuit *garbledCircuit, GarblingContext *garblingContext, int input0, int input1, int output) {
  return genericGate(garbledCircuit, garblingContext, input0, input1, output, ORGATE);
}

int NOTGate(GarbledCircuit *garbledCircuit, GarblingContext *garblingContext, int input, int output) {
  return genericGate(garbledCircuit, garblingContext, 0, input, output, NOTGATE);
}

int XORGate(GarbledCircuit *garbledCircuit, GarblingContext *garblingContext,
            int input0, int input1, int output) {
  garbledCircuit->wires[output].id = output;

  GarbledGate *garbledGate = &(garbledCircuit->garbledGates[garblingContext->gateIndex]);

  garbledGate->id = XOR_ID;
  garbledGate->type = XORGATE;
  
  garbledGate->input0 = input0;
  garbledGate->input1 = input1;
  garbledGate->output = output;

  garblingContext->gateIndex++;

  return 0;
}

static int fixedWire(GarbledCircuit *garbledCircuit, GarblingContext *garblingContext, int id) {
  int ind = getNextWire(garblingContext);

  garbledCircuit->fixedWireIndices.push_back(pair<int, int>(ind, id));
  garbledCircuit->wires[ind].id = ind;

  return ind;
}

int fixedZeroWire(GarbledCircuit *garbledCircuit, GarblingContext *garblingContext) {
  return fixedWire(garbledCircuit, garblingContext, FIXED_ZERO_GATE);
}

int fixedOneWire(GarbledCircuit *garbledCircuit, GarblingContext *garblingContext) {
  return fixedWire(garbledCircuit, garblingContext, FIXED_ONE_GATE);
}
