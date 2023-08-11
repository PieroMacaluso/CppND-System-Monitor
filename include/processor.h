#ifndef PROCESSOR_H
#define PROCESSOR_H

class Processor {
 public:
  float Utilization();

 private:
  float lastIdleCalc = 0.0;
  float lastNonIdleCalc = 0.0;
};

#endif