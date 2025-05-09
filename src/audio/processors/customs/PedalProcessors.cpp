#include "PedalProcessors.hpp"
#include "../../../circuits/models/CapacitorModel.hpp"
#include "../../../circuits/models/DiodeModel.hpp"
#include "../../../circuits/models/ResistorModel.hpp"
#include "../../../circuits/models/VoltageSourceModel.hpp"
#include "../../../circuits/models/transistors/BJTs/NPNModel.hpp"

CircuitProcessor *PedalProcessors::FuzzProcessor() {
  int nodeCount = -1;
  int gnd = nodeCount++;
  int nodeIn = nodeCount++;
  int nodeB = nodeCount++;
  int nodeC = nodeCount++;
  int nodeE = nodeCount++;
  int nodeD = nodeCount++;
  int nodeF = nodeCount++;
  int nodePowerCap = nodeCount++;
  int nodePreClip = nodeCount++;
  int nodeLEDClip = nodeCount++;

  int outputNode = nodePreClip;

  CapacitorModel *c1 = new CapacitorModel(
      0.47e-6, nodeIn, nodeB); // Larger input capacitor for more bottom end
  CapacitorModel *c2 = new CapacitorModel(0.2e-6, nodeC, nodePreClip);
  ResistorModel *r1 = new ResistorModel(470e3, nodeB, nodeC); // Adjusted for more gain
  ResistorModel *r2 = new ResistorModel(200, nodeE, gnd); // Lower resistor for more gain
  ResistorModel *r3 = new ResistorModel(10e3, nodeC, nodePowerCap);
  ResistorModel *r4 = new ResistorModel(100e3, nodePreClip, gnd);
  ResistorModel *r5 = new ResistorModel(100e3, nodeD, gnd);
  VoltageSourceModel *v1 = new VoltageSourceModel(0, nodeIn, gnd);
  VoltageSourceModel *v2 = new VoltageSourceModel(9, nodePowerCap, gnd);
  NPNModel *t1 = new NPNModel(nodeB, nodeC, nodeE, "2N3904");
  DiodeModel *led1 = new DiodeModel(nodeD, nodeLEDClip, "1N5817"); // LED characteristics
  DiodeModel *led2 = new DiodeModel(nodeLEDClip, nodeF, "BAT41");
  DiodeModel *led3 = new DiodeModel(nodeF, nodeD, "BAT41");

  Circuit *c = new Circuit(nodeCount);
  c->addComponent(c1);
  c->addComponent(c2);
  c->addComponent(r1);
  c->addComponent(r2);
  c->addComponent(r3);
  c->addComponent(r4);
  c->addComponent(r5);
  c->addComponent(t1);
  c->addComponent(led1);
  c->addComponent(led2);
  c->addComponent(led3);
  int inputIndex = c->addComponent(v1);
  c->addComponent(v2);

  CircuitProcessor *cp = new CircuitProcessor(c);
  cp->setInput(inputIndex);
  cp->setOutput(outputNode);

  return cp;
}

CircuitProcessor *PedalProcessors::LowPassProcessor(double R, double C) {

  int nodeCount = -1;
  int gnd = nodeCount++;
  int nodeA = nodeCount++;
  int nodeB = nodeCount++;

  Circuit *c = new Circuit(nodeCount);
  VoltageSourceModel *v1 = new VoltageSourceModel(0.0, nodeA, gnd);
  ResistorModel *r1 = new ResistorModel(R, nodeB, gnd);
  CapacitorModel *c1 = new CapacitorModel(C, nodeA, nodeB);
  int inputIndex = c->addComponent(v1);
  c->addComponent(c1);
  c->addComponent(r1);

  CircuitProcessor *cp = new CircuitProcessor(c);
  cp->setInput(inputIndex);
  cp->setOutput(nodeA);

  return cp;
}
