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


#ifndef __JUST_GARBLE_H__
#define __JUST_GARBLE_H__

#include <vector>
#include <unordered_map>

#include "dkcipher.h"
#include "common.h"

using namespace std;

typedef struct {
  long value, id;
  block label, label0, label1;
} Wire;

typedef struct {
  long type, id; 
  Wire *input0, *input1, *output;
} Gate;

typedef struct {
  long input0, input1, output; int id, type;
} GarbledGate;

typedef struct {
  block table[2];
} GarbledTable;

typedef struct {
  int n, m, q, r;
  long id;
  block globalKey;

  Wire* wires;
  GarbledGate *garbledGates;
  int *outputs;

  GarbledTable *garbledTable;
  block *fixedLabels;
  int nAndGates;

  vector<pair<int, int>> fixedWireIndices;
  block fixedWiresSeed;
} GarbledCircuit;

typedef struct {
  int m;
  block *outputLabels;
  long id;
} GarbledOutput;

typedef struct {
  long wireIndex, gateIndex, tableIndex;
} GarblingContext;


typedef block* InputLabels;
typedef block* ExtractedLabels;
typedef block* OutputMap;


/*
 * The following are the functions involved in creating, garbling, and 
 * evaluating circuits. Most of the data-structures involved are defined
 * above, and the rest are in other header files.
 */


// Start and finish building a circuit. In between these two steps, gates
// and sub-circuits can be added in. See AESFullTest and LargeCircuitTest
// for examples. Note that the data-structures involved are GarbledCircuit
// and GarblingContext, although these steps do not involve any garbling.
// The reason for this is efficiency. Typically, garbleCircuit is called
// right after finishBuilding. So, using a GarbledCircuit data-structure
// here means that there is no need to create and initialize a new 
// data-structure just before calling garbleCircuit.
void startBuilding(GarbledCircuit *gc, GarblingContext *garblingContext);
void finishBuilding(GarbledCircuit *garbledCircuit, GarblingContext *garblingContext, int *outputs);

// Create memory for an empty circuit of the specified size.
void createEmptyGarbledCircuit(GarbledCircuit *garbledCircuit, int n, int m, int q, int r);

//Garble the circuit described in garbledCircuit. For efficiency reasons,
//we use the garbledCircuit data-structure for representing the input 
//circuit and the garbled output. The garbling process is non-destructive and 
//only affects the garbledTable member of the GarbledCircuit data-structure.
//In other words, the same GarbledCircuit object can be reused multiple times,
//to create multiple garbled circuit copies, 
//by just switching the garbledTable field to a new one. Also, the garbledTable
//field is the only one that should be sent over the network in the case of an 
//MPC-type application, as the topology is expected to be avaiable to the 
//receiver, and the gate-types are to be hidden away.
//The inputLabels field is expected to contain 2n fresh input labels, obtained
//by calling createInputLabels. The outputMap is expected to be a 2m-block sized
//empty array.
void createInputLabels(InputLabels inputLabels, int n);
void createInputLabels(InputLabels inputLabels, int n, block R);
void garbleCircuit(GarbledCircuit *garbledCircuit, InputLabels inputLabels,
    OutputMap outputMap);

//Evaluate a garbled circuit, using n input labels in the Extracted Labels
//to return m output labels. The garbled circuit might be generated either in 
//one piece, as the result of running garbleCircuit, or may be pieced together,
// by building the circuit (startBuilding ... finishBuilding), and adding 
// garbledTable from another source, say, a network transmission.
void evaluate(GarbledCircuit *garbledCircuit, ExtractedLabels extractedLabels,
    OutputMap outputMap);

// A simple function that selects n input labels from 2n labels, using the 
// inputBits array where each element is a bit.
void extractLabels(ExtractedLabels extractedLabels, InputLabels inputLabels,
    uint8_t* inputBits, int n);

// A simple function that takes 2m output labels, m labels from evaluate, 
// and returns a m bit output by matching the labels. If one or more of the
// m evaluated labels donot match either of the two corresponding output labels,
// then the function flags an error.
void mapOutputs(OutputMap outputMap, OutputMap extractedMap, int *outputVals,
    int m);

#include "garble.h"
#include "util.h"

#endif
