//
// Copyright (C) 2017 Christoph Sommer <sommer@ccs-labs.org>
//
// Documentation for these modules is at http://veins.car2x.org/
//
// SPDX-License-Identifier: GPL-2.0-or-later
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//

package finalproject.simulations.veins5Final;

import org.car2x.veins.nodes.RSU;
import org.car2x.veins.nodes.Scenario;
import org.car2x.veins.modules.mobility.traci.TraCIVehicleInserter;
import org.car2x.veins.modules.mobility.traci.TraCIScenarioManagerLaunchd;

simple VehicleSpawner {
    parameters:
        int numVehicles @default(7);
        double spawnTime @default(0.1s);
        @display("i=block/app");
        double targetTravelTime @unit(s) = default(30s);  // make it configurable
        
    gates:
        // none
}

network RSUExampleScenario extends Scenario
{
    submodules:
        spawner: VehicleSpawner {
            @display("p=200,100");
        }
        rsu[1]: RSU {
            @display("p=150,140;i=veins/sign/yellowdiamond;is=vs");
        }
}
