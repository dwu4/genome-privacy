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

static void RunProtocol(CSocket* socket, byte* input, void* args) {
  uint32_t nElems = ((BasicIntersectionArgs*) args)->nElems;

  uint32_t nServerInputWires = nElems;
  uint32_t nInputWires = 2 * nServerInputWires;

  GarbledCircuit circuit;
  CreateBasicIntersectionCircuit(circuit, nElems);

  if (!RunServerProtocol(socket, circuit, input, nInputWires, nServerInputWires)) {
    ServerLog("protocol execution failed");
  }
}

int main(int argc, char** argv) {
  if (argc < 3) {
    cout << "usage: ./BasicIntersectionServer input nElems [port]" << endl;
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
    ServerLog("unable to read from input file");
    return 1;
  }

  ServerLog("finished reading input");

  BasicIntersectionArgs args(nElems);
  StartServer(port, input, &args, RunProtocol);

  delete[] input;

  return 0;
}
