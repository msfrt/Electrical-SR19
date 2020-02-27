#ifndef DONT_SMOKE_CIGARETTES
#define DONT_SMOKE_CIGARETTES

#include <StateCAN.h>
#include <FlexCAN_T4.h>

// format: (find parameter values in DBC)
// StateSignal MOD_sigName(bitl, signed, inv_factor, offset, min, max, secondary_val, timeout);

// outgoing signals
StateSignal DD_teensyTemp(16, true, 10, 0, -3276, 3276, 0);
StateSignal DD_boardTemp(16, true, 10, 0, -3276, 3276, 0);
StateSignal DD_requestDRS(16, true, 1, 0, 0, 5, 0);

// incoming signals
StateSignal M400_engineTemp(16, true, 10, 0, -3276, 3276, 0, 1000); // secondary val: 0, timeout: 1000ms
StateSignal M400_oilTemp(16, true, 10, 0, -3276, 3276, 0, 1000);
StateSignal M400_oilPressure(16, true, 10, 0, -3276, 3276, 0, 1000);
StateSignal M400_fuelPressure(16, true, 10, 0, -3276, 3276, 0, 1000);
StateSignal M400_rpm(16, true, 1, 0, -3276.8, 3276.7, 0, 1000); // secondary val: 0, timeout: 1000ms
StateSignal M400_batteryVoltage(16, true, 100, 0, -327.68, 327.67, 13.5, 1000); // secondary val: 0, timeout: 1000ms
StateSignal M400_gear(16, true, 1, 0, -3276, 3276, 0, 1000);
StateSignal M400_groundSpeed(16, true, 10, 0, -3276, 3276, 0, 1000);
StateSignal M400_groundSpeedLeft(16, true, 10, 0, -3276, 3276, 0, 1000);
StateSignal M400_groundSpeedRight(16, true, 10, 0, -3276, 3276, 0, 1000);
StateSignal M400_driveSpeedLeft(16, true, 10, 0, -3276, 3276, 0, 1000);
StateSignal M400_driveSpeedRight(16, true, 10, 0, -3276, 3276, 0, 1000);
StateSignal M400_fuelUsed(16, true, 1, 0, -3276.8, 3276.7, 0, 1000);
StateSignal M400_ignCutLevelTotal(16, true, 1, 0, 0, 256, 0, 50);
StateSignal M400_tcPowerReduction(16, true, 10, 0, 0, 75, 0, 50);

StateSignal MM5_Ax(16, true, -7849, 4.2, -5, 5, 0, 0);

StateSignal USER_brakeLightOverride(8, true, 1, 0, -5, 100, -1, 1000);
StateSignal USER_fanLeftOverride(8, true, 1, 0, -5, 100, -1, 1000);
StateSignal USER_fanRightOverride(8, true, 1, 0, -5, 100, -1, 1000);
StateSignal USER_wpOverride(8, true, 1, 0, -5, 100, -1, 1000);
StateSignal USER_driverSignal(8, true, 1, 0, -5, 100, 0, 1000);

StateSignal ATCCF_brakeBias(16, true, 10, 0, 0, 100, 0, 500);
StateSignal ATCCF_brakePressureF(16, true, 10, 0, -3276.8, 3276.7, 0, 500);
StateSignal ATCCF_brakePressureR(16, true, 10, 0, -3276.8, 3276.7, 0, 500);

StateSignal PDM_driverDisplayLEDs(8, true, 1, 0, -5, 100, 0, 1000);

StateSignal PDM_brakelightVoltAvg(16, true, 1000, 0, -32, 32, 0);

StateSignal PDM_boardTemp(16, true, 10, 0, -3276, 3276, 0);

StateSignal PDM_dataVoltAvg(16, true, 1000, 0, -32, 32, 0);
StateSignal PDM_dataVoltMax(16, true, 1000, 0, -32, 32, 0);
StateSignal PDM_dataVoltMin(16, true, 1000, 0, -32, 32, 0);

StateSignal PDM_fanLeftCurrentAvg(16, true, 100, 0, -327, 327, 0);
StateSignal PDM_fanLeftCurrentMax(16, true, 100, 0, -327, 327, 0);
StateSignal PDM_fanLeftCurrentMin(16, true, 100, 0, -327, 327, 0);
StateSignal PDM_fanLeftPWM(8, false, 1, 0, 0, 255, 0);
StateSignal PDM_fanLeftVoltAvg(16, true, 1000, 0, -32, 32, 0);
StateSignal PDM_fanLeftVoltMax(16, true, 1000, 0, -32, 32, 0);
StateSignal PDM_fanLeftVoltMin(16, true, 1000, 0, -32, 32, 0);

StateSignal PDM_fanRightCurrentAvg(16, true, 100, 0, -327, 327, 0);
StateSignal PDM_fanRightCurrentMax(16, true, 100, 0, -327, 327, 0);
StateSignal PDM_fanRightCurrentMin(16, true, 100, 0, -327, 327, 0);
StateSignal PDM_fanRightPWM(8, false, 1, 0, 0, 255, 0);
StateSignal PDM_fanRightVoltAvg(16, true, 1000, 0, -32, 32, 0);
StateSignal PDM_fanRightVoltMax(16, true, 1000, 0, -32, 32, 0);
StateSignal PDM_fanRightVoltMin(16, true, 1000, 0, -32, 32, 0);

StateSignal PDM_fuelCurrentAvg(16, true, 100, 0, -327, 327, 0);
StateSignal PDM_fuelCurrentMax(16, true, 100, 0, -327, 327, 0);
StateSignal PDM_fuelCurrentMin(16, true, 100, 0, -327, 327, 0);

StateSignal PDM_fuelVoltAvg(16, true, 1000, 0, -32, 32, 0);
StateSignal PDM_fuelVoltMax(16, true, 1000, 0, -32, 32, 0);
StateSignal PDM_fuelVoltMin(16, true, 1000, 0, -32, 32, 0);

StateSignal PDM_keepAliveVoltAvg(16, true, 1000, 0, -32, 32, 0);
StateSignal PDM_keepAliveVoltMax(16, true, 1000, 0, -32, 32, 0);
StateSignal PDM_keepAliveVoltMin(16, true, 1000, 0, -32, 32, 0);

StateSignal PDM_mainVoltAvg(16, true, 1000, 0, -32, 32, 0);
StateSignal PDM_mainVoltMax(16, true, 1000, 0, -32, 32, 0);
StateSignal PDM_mainVoltMin(16, true, 1000, 0, -32, 32, 0);

StateSignal PDM_pdmCurrentAvg(16, true, 100, 0, -327, 327, 0);
StateSignal PDM_pdmCurrentMax(16, true, 100, 0, -327, 327, 0);
StateSignal PDM_pdmCurrentMin(16, true, 100, 0, -327, 327, 0);

StateSignal PDM_pdmVoltAvg(16, true, 1000, 0, -32, 32, 0);
StateSignal PDM_pdmVoltMax(16, true, 1000, 0, -32, 32, 0);
StateSignal PDM_pdmVoltMin(16, true, 1000, 0, -32, 32, 0);

StateSignal PDM_starterRelayVoltAvg(16, true, 1000, 0, -32, 32, 0);

StateSignal PDM_wpCurrentAvg(16, true, 100, 0, -327, 327, 0);
StateSignal PDM_wpCurrentMax(16, true, 100, 0, -327, 327, 0);
StateSignal PDM_wpCurrentMin(16, true, 100, 0, -327, 327, 0);
StateSignal PDM_wpPWM(8, false, 1, 0, 0, 255, 0);
StateSignal PDM_wpVoltAvg(16, true, 1000, 0, -32, 32, 0);
StateSignal PDM_wpVoltMax(16, true, 1000, 0, -32, 32, 0);
StateSignal PDM_wpVoltMin(16, true, 1000, 0, -32, 32, 0);

StateSignal PDM_teensyTemp(16, true, 10, 0, -3276, 3276, 0);

#endif
