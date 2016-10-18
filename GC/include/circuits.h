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

#ifndef __CIRCUITS_H__
#define __CIRCUITS_H__

#include "justGarble.h"

void ADDCircuit(GarbledCircuit *garbledCircuit, GarblingContext *garblingContext, int n, int* inputs, int* outputs);
void EQCircuit(GarbledCircuit *garbledCircuit, GarblingContext *garblingContext, int n, int *inputs, int *outputs);
void ANDCircuit(GarbledCircuit *garbledCircuit, GarblingContext *garblingContext, int n, int *inputs, int *outputs);
void CMPCircuit(GarbledCircuit *garbledCircuit, GarblingContext *garblingContext, int n, int *inputs, int *outputs);
void MUXCircuit(GarbledCircuit *garbledCircuit, GarblingContext *garblingContext, int n, int *inputs, int *outputs);
void MAXCircuit(GarbledCircuit *garbledCircuit, GarblingContext *garblingContext, int n, int *inputs, int *outputs);
void MAXVecCircuit(GarbledCircuit *garbledCircuit, GarblingContext *garblingContext, int k, int n, int *inputs, int *outputs);
void ARGMAXVecCircuit(GarbledCircuit *garbledCircuit, GarblingContext *garblingContext, int k, int n, int *inputs, int *outputs);
void ARGMAXVecSharedCircuit(GarbledCircuit *garbledCircuit, GarblingContext *garblingContext, int k, int n, int *inputs, int *outputs);
void SetDiffVecSharedCircuit(GarbledCircuit *garbledCircuit, GarblingContext *garblingContext, int k, int n, int *inputs, int *outputs);

#endif
