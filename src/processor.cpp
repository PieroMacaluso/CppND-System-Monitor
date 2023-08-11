#include "processor.h"

#include "linux_parser.h"

// Return the aggregate CPU utilization
float Processor::Utilization() {
  float idleCalc = LinuxParser::IdleJiffies();
  float nonIdleCalc = LinuxParser::ActiveJiffies();

  float lastTotal = lastIdleCalc + lastNonIdleCalc;
  float total = idleCalc + nonIdleCalc;

  float totalD = total - lastTotal;
  float idleD = idleCalc - lastIdleCalc;

  lastIdleCalc = idleCalc;
  lastNonIdleCalc = nonIdleCalc;

  return (totalD - idleD) / totalD;
}