/*******************************************************************************
GPU OPTIMIZED MONTE CARLO (GOMC) 2.31
Copyright (C) 2018  GOMC Group
A copy of the GNU General Public License can be found in the COPYRIGHT.txt
along with this program, also can be found at <http://www.gnu.org/licenses/>.
********************************************************************************/
#ifndef SIM_EVENT_FREQUENCY_H
#define SIM_EVENT_FREQUENCY_H

#include "BasicTypes.h" //for ulong
#include "ConfigSetup.h" //for event frequencies from config file.

struct SimEventFrequency {
// GJS
  ulong total, perAdjust, tillEquil, pCalcFreq, perExchange;
// GJS
  bool pressureCalc;

  void Init(config_setup::Step const& s)
  {
    total = s.total;
    perAdjust = s.adjustment;
// GJS
    perExchange = s.exchange;
// GJS
    tillEquil = s.equil;
    pCalcFreq = s.pressureCalcFreq;
    pressureCalc = s.pressureCalc;
  }
};

#endif /*SIM_EVENT_FREQUENCY_H*/
