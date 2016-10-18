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

#include "common.h"

#include <sstream>

static const uint64_t MAX_OT_BATCH = 15000000;
static const int TIMEOUT_MS = 10000;

void CreateArgMaxCircuit(GarbledCircuit& circuit, int nElems, int nBits) {
  uint32_t nInputWires = nElems * 2 * nBits;
  uint32_t nOutputWires = nElems + nBits;

  GarblingContext garblingContext;

  int* inputs = new int[nInputWires];
  countToN(inputs, nInputWires);
  int* outputs = new int[nOutputWires];

  // rough estimates for number of gates and wires in circuit
  uint32_t nGates = 15 * nElems * nBits;
  uint32_t nWires = 18 * nElems * nBits;

  createEmptyGarbledCircuit(&circuit, nInputWires, nOutputWires, nGates, nWires);
  startBuilding(&circuit, &garblingContext);
  ARGMAXVecSharedCircuit(&circuit, &garblingContext, nBits, nInputWires, inputs, outputs);
  finishBuilding(&circuit, &garblingContext, outputs);

  delete[] inputs;
  delete[] outputs;
}

void CreateBasicIntersectionCircuit(GarbledCircuit& circuit, int nElems) {
  uint32_t nInputWires = nElems * 2;
  uint32_t nOutputWires = nElems;

  GarblingContext garblingContext;

  int* inputs = new int[nInputWires];
  countToN(inputs, nInputWires);
  int* outputs = new int[nOutputWires];

  // one AND gate for each element (3 wires each : 2 input wires, 1 output wire)
  uint32_t nGates = nElems;
  uint32_t nWires = 3 * nElems;

  createEmptyGarbledCircuit(&circuit, nInputWires, nOutputWires, nGates, nWires);
  startBuilding(&circuit, &garblingContext);
  ANDCircuit(&circuit, &garblingContext, nInputWires, inputs, outputs);
  finishBuilding(&circuit, &garblingContext, outputs);

  delete[] inputs;
  delete[] outputs;
}

void CreateSetDiffCircuit(GarbledCircuit& circuit, int nElems, int nBits) {
  uint32_t nInputWires = nElems * (2 * nBits + 2);
  uint32_t nOutputWires = nElems;

  GarblingContext garblingContext;

  int* inputs = new int[nInputWires];
  countToN(inputs, nInputWires);
  int* outputs = new int[nOutputWires];

  // number of gates and wires in circuit
  uint32_t nGates = (8 * nBits - 1) * nElems;
  uint32_t nWires = (11 * nBits + 2) * nElems + 1;

  createEmptyGarbledCircuit(&circuit, nInputWires, nOutputWires, nGates, nWires);
  startBuilding(&circuit, &garblingContext);
  SetDiffVecSharedCircuit(&circuit, &garblingContext, nBits, nInputWires, inputs, outputs);
  finishBuilding(&circuit, &garblingContext, outputs);

  delete[] inputs;
  delete[] outputs;
}

bool Listen(CSocket* socket, int port) {
  if (!socket->Socket()) {
    return false;
  }

  if (!socket->Bind((uint16_t) port)) {
    return false;
  }

  if (!socket->Listen()) {
    return false;
  }

  CSocket sock;
  if (!socket->Accept(sock)) {
    return false;
  }

  socket->AttachFrom(sock);
  sock.Detach();
}

bool Connect(CSocket* socket, const char* address, int port) {
  for (int i = 0; i < RETRY_CONNECT; i++) {
    if (!socket->Socket()) {
      return false;
    }
    if (socket->Connect(address, (uint16_t) port, TIMEOUT_MS)) {
      return true;
    }
    SleepMiliSec(20);
  }

  return false;
}

void StartServer(int port, byte* input, void* args,
                 void (*RunProtocol)(CSocket*, byte*, void*)) {
  CSocket* socket = new CSocket();
  if (Listen(socket, port)) {
    ServerLog("accepted connection from client");
  } else {
    stringstream ss;
    ss << "failed to listen for client connections on port " << port;
    ServerLog(ss.str());
    return;
  }

  RunProtocol(socket, input, args);
  socket->Close();

  delete socket;
}

void StartClient(const char *address, int port, byte* input, void* args,
                 void (*RunProtocol)(CSocket*, byte*, void*)) {
  CSocket *socket = new CSocket();
  if (Connect(socket, address, port)) {
    ClientLog("successfully connected to server");
  } else {
    stringstream ss;
    ss << "unable to connect to port " << port << " on address" << address;
    ClientLog(ss.str());
    return;
  }

  RunProtocol(socket, input, args);

  socket->Close();

  delete socket;
}

void CreateChoiceVec(CBitVector& choices, byte* input, uint64_t len) {
  choices.Create(len);
  choices.Reset();

  uint8_t* choicesBuf = choices.GetArr();

  uint8_t val = 0;
  for (int i = 0; i < (len + 7) / 8; i++) {
    uint8_t val = 0;
    for (int j = 0; j < 8; j++) {
      if (8*i + j >= len) {
        break;
      }
      if (input[8*i + j] == 1) {
        val |= (1 << j);
      }
    }
    choicesBuf[i] = val;
  }
}

uint32_t RunServerProtocol(CSocket* socket, GarbledCircuit& circuit, byte* input, 
                           uint32_t nInputWires, uint32_t nServerInputWires) {
  uint32_t nClientInputWires = nInputWires - nServerInputWires;

  // run server OT protocol
  CBitVector zeroLabelsVec;
  CBitVector oneLabelsVec;
  CBitVector delta;

  block* zeroLabels = new block[nClientInputWires];
  block* oneLabels  = new block[nClientInputWires];

  // choose a random offset (last bit of offset is 1 for point-and-permute)
  // between the 0-labels and 1-labels to support free XORs
  byte offset[128];
  if (!GetRandomSeed(offset, 128)) {
    ServerLog("OT failed");
    return 0;
  }

  int ctr = 0;
  delta.Create(128, (byte*) offset, ctr);
  block* tmp = (block*)(delta.GetArr());
  block mask = makeBlock((uint64_t) 0, (uint64_t) 1);
  *tmp |= mask;

  // run server OT (in batches)
  OTServer otServer;
  otServer.InitOTSender(socket);

  uint64_t batchesNeeded = (nClientInputWires + MAX_OT_BATCH - 1) / MAX_OT_BATCH;
  for (int i = 0; i < batchesNeeded; i++) {
    uint64_t batchSize = MAX_OT_BATCH;
    if (i == batchesNeeded - 1) {
      batchSize = nClientInputWires % MAX_OT_BATCH;
      if (batchSize == 0) {
        batchSize = MAX_OT_BATCH;
      }
    }

    otServer.ObliviouslySend(zeroLabelsVec, oneLabelsVec, delta, batchSize);
    memcpy(zeroLabels + i * MAX_OT_BATCH, zeroLabelsVec.GetArr(), batchSize * sizeof(block));
    memcpy(oneLabels + i * MAX_OT_BATCH, oneLabelsVec.GetArr(), batchSize * sizeof(block));

    zeroLabelsVec.delCBitVector();
    oneLabelsVec.delCBitVector();
  }

  cout << "OT bytes sent: " << socket->GetBytesSent() << endl;
  cout << "OT bytes received: " << socket->GetBytesReceived() << endl;
  ServerLog("finished OT for input wires");
  
  // garbled circuit evaluation
  InputLabels allInputLabels = new block[2*nInputWires];
  OutputMap outputMap = new block[2 * circuit.m];

  for (int i = 0; i < nClientInputWires; i++) {
    allInputLabels[2*i] = zeroLabels[i];
    allInputLabels[2*i + 1] = oneLabels[i];
  }

  block R = zeroLabels[0] ^ oneLabels[0];
  createInputLabels(allInputLabels + nClientInputWires * 2, nServerInputWires, R);
  garbleCircuit(&circuit, allInputLabels, outputMap);

  InputLabels inputLabels = new block[nServerInputWires];
  extractLabels(inputLabels, allInputLabels + 2 * nClientInputWires, input, nServerInputWires);

  // send garbled circuit and garbled inputs
  socket->SendLarge((byte*) inputLabels, nServerInputWires * sizeof(block));
  socket->SendLarge((byte*) outputMap, 2 * circuit.m * sizeof(block));
  socket->SendLarge((byte*) circuit.garbledTable, circuit.q * sizeof(GarbledTable));
  socket->Send(&circuit.nAndGates, sizeof(circuit.nAndGates));
  socket->Send(&circuit.fixedWiresSeed, sizeof(circuit.fixedWiresSeed));
  socket->Send(&circuit.globalKey, sizeof(circuit.globalKey));

  cout << endl << "bytes sent: " << socket->GetBytesSent() << endl;
  cout << "bytes received: " << socket->GetBytesReceived() << endl;

  uint32_t finished = 0;
  socket->Receive(&finished, sizeof(finished));

  delete[] zeroLabels;
  delete[] oneLabels;
  delete[] allInputLabels;
  delete[] outputMap;
  delete[] inputLabels;

  return finished;
}

void RunClientProtocol(CSocket* socket, GarbledCircuit& circuit, byte* input, int* outputVals,
                       uint32_t nInputWires, uint32_t nClientInputWires, uint32_t nOutputWires) {
  uint32_t nServerInputWires = nInputWires - nClientInputWires;

  block* inputLabels = new block[nInputWires];

  OTClient otClient;
  otClient.InitOTClient(socket);

  uint64_t batchesNeeded = (nClientInputWires + MAX_OT_BATCH - 1) / MAX_OT_BATCH;
  for (int i = 0; i < batchesNeeded; i++) {
    uint64_t batchSize = MAX_OT_BATCH;
    if (i == batchesNeeded - 1) {
      batchSize = nClientInputWires % MAX_OT_BATCH;
      if (batchSize == 0) {
        batchSize = MAX_OT_BATCH;
      }
    }

    CBitVector choices;
    CreateChoiceVec(choices, input + i * MAX_OT_BATCH, batchSize);

    otClient.ObliviouslyReceive((byte*) (inputLabels + i * MAX_OT_BATCH), choices, batchSize);
  }

  ClientLog("finished OT for input wires");
  cout << "OT bytes sent: " << socket->GetBytesSent() << endl;
  cout << "OT bytes received: " << socket->GetBytesReceived() << endl;

   // prepare garbled circuit
  block *outputMap = new block[2 * nOutputWires];
  block *computedOutputMap = new block[nOutputWires];

  socket->ReceiveLarge((byte*) (inputLabels + nClientInputWires), nClientInputWires * sizeof(block));
  socket->ReceiveLarge((byte*) outputMap, 2 * nOutputWires * sizeof(block));
  socket->ReceiveLarge((byte*) circuit.garbledTable, circuit.q * sizeof(GarbledTable));
  socket->Receive(&circuit.nAndGates, sizeof(circuit.nAndGates));
  socket->Receive(&circuit.fixedWiresSeed, sizeof(circuit.fixedWiresSeed));
  socket->Receive(&circuit.globalKey, sizeof(circuit.globalKey));

  // garbled circuit evaluation  
  evaluate(&circuit, inputLabels, computedOutputMap);
  mapOutputs(outputMap, computedOutputMap, outputVals, nOutputWires);

  uint32_t finished = 1;
  socket->Send(&finished, sizeof(finished));

  delete[] inputLabels;
  delete[] outputMap;
  delete[] computedOutputMap;
}

void PrintStatistics(CSocket* socket, GarbledCircuit& circuit, double timeElapsed) {
  cout << "Number of gates:     " << circuit.q << endl;
  cout << "Number of AND gates: " << circuit.nAndGates << endl;
  cout << "Number of wires:     " << circuit.r << endl << endl;

  cout << "bytes sent: " << socket->GetBytesSent() << endl;
  cout << "bytes received: " << socket->GetBytesReceived() << endl;

  cout << "total bytes: " << (socket->GetBytesSent() + socket->GetBytesReceived()) << endl << endl;

  cout << "network communication time: " << socket->GetNetworkTime() << endl;
  cout << "non-network execution time: " << (timeElapsed - socket->GetNetworkTime()) << endl;
  cout << "total protocol execution time: " << timeElapsed << endl;
}

bool ReadInputFile(byte* buf, const char* filename, int len) {
  FILE* f = fopen(filename, "r");
  if (f == NULL) {
    return false;
  }

  if (fread(buf, 1, len, f) != len) {
    return false;
  }

  fclose(f);

  for (int i = 0; i < len; i++) {
    if (buf[i] == '0') {
      buf[i] = 0;
    } else {
      buf[i] = 1;
    }
  }

  return true;
}
