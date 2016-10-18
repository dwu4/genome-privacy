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

#include <iostream>
#include <sstream>
#include <stdint.h>

#include "OTExtension/protocol/OTClient.h"

#include "common.h"

using namespace std;

static void RunProtocol(CSocket* socket, byte* input, void* args) {
  clock_t startTime = clock();

  uint32_t nElems = ((BasicIntersectionArgs*) args)->nElems;

  uint32_t nClientInputWires = nElems;
  uint32_t nInputWires = 2 * nClientInputWires;
  uint32_t nOutputWires = nElems;

  GarbledCircuit circuit;
  CreateBasicIntersectionCircuit(circuit, nElems);

  int* outputVals = new int[nOutputWires];
  RunClientProtocol(socket, circuit, input, outputVals, nInputWires, nClientInputWires, nOutputWires);

  double timeElapsed = (double)(clock() - startTime) / CLOCKS_PER_SEC;

  cout << endl << "output: ";
  for (int i = 0; i < nElems; i++) {
    if (outputVals[i] == 1) {
      cout << i << " ";
    }
  }
  cout << endl << endl;

  cout << "Number of elements:  " << nElems << endl;
  PrintStatistics(socket, circuit, timeElapsed);
}

int main(int argc, const char** argv) {
  if (argc < 3) {
    cout << "usage: ./BasicIntersectionClient input nElems [port]" << endl;
    return 1;
  }

  const char* inputFile = argv[1];
  const uint32_t nElems = atoi(argv[2]);
  uint16_t port = 8100;
  if (argc > 3) {
    port = atoi(argv[3]);
  }

  byte* input = new byte[nElems];
  if (!ReadInputFile(input, inputFile, nElems)) {
    ClientLog("unable to read from input file");
    return 1;
  }
  ClientLog("finished reading input");

  BasicIntersectionArgs args(nElems);
  StartClient("127.0.0.1", port, input, &args, RunProtocol);

  delete[] input;

  return 0;
}
