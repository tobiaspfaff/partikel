#ifndef SIM_SEMILAGRANGE_HPP
#define SIM_SEMILAGRANGE_HPP

#include "sim/grid.hpp"

void semiLagrangeSelfAdvect(GridMac2f& velSrc, GridMac2f& temp, float dt, float h);

#endif