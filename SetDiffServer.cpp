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

#include "OTExtension/protocol/OTServer.h"

#include "common.h"

using namespace std;

static void RunProtocol(CSocket *socket, byte* input, void* args) {
  uint32_t nElems = ((SetDiffArgs*) args)->nElems;
  uint32_t nBits  = ((SetDiffArgs*) args)->nBits;

  uint64_t nServerInputWires = nElems * (nBits + 1);
  uint64_t nInputWires = 2 * nServerInputWires;

  GarbledCircuit circuit;
  CreateSetDiffCircuit(circuit, nElems, nBits);

  if (!RunServerProtocol(socket, circuit, input, nInputWires, nServerInputWires)) {
    ServerLog("protocol execution failed");
  }
}

int main(int argc, char **argv) {
  if (argc < 4) {
    cout << "usage: ./BasicIntersectionServer input nElems nBits [port]" << endl;
    return 1;
  }

  const char* inputFile = argv[1];
  const uint32_t nElems = atoi(argv[2]);
  const uint32_t nBits = atoi(argv[3]);
  uint16_t port = 8100;
  if (argc > 4) {
    port = atoi(argv[4]);
  }

  byte* input = new byte[nElems * (nBits + 1)];
  if (!ReadInputFile(input, inputFile, nElems * (nBits + 1))) {
    ServerLog("unable to read from input file");
    return 1;
  }

  ServerLog("finished reading input");
  
  SetDiffArgs args(nElems, nBits);
  StartServer(port, input, &args, RunProtocol);

  delete[] input;

  return 0;
}
