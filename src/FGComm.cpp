/*
 * FGComm.cpp
 * Copyright (C) James Goppert 2010 <james.goppert@gmail.com>
 *
 * FGComm.cpp is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * FGComm.cpp is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "FGFDMExec.h"
#include "math/FGStateSpace.h"
#include "input_output/flightGearIO.h"

int main (int argc, char const* argv[])
{
    using namespace JSBSim;
    FGFDMExec fdm;
    fdm.LoadModel("../aircraft","../engine","../systems","c172p");

    // Turn on propulsion system
    fdm.GetPropulsion()->InitRunning(-1);

    // get propulsion pointer to determine type/ etc.
    FGEngine * engine0 = fdm.GetPropulsion()->GetEngine(0);
    FGThruster * thruster0 = engine0->GetThruster();

    bool variablePropPitch = false;

    FGStateSpace ss(fdm);
    ss.x.add(new FGStateSpace::Vt);
    ss.x.add(new FGStateSpace::Alpha);
    ss.x.add(new FGStateSpace::Theta);
    ss.x.add(new FGStateSpace::Q);

    if (thruster0->GetType()==FGThruster::ttPropeller)
    {
        ss.x.add(new FGStateSpace::Rpm);
        if (variablePropPitch) ss.x.add(new FGStateSpace::Pitch);
    }
    switch (engine0->GetType())
    {
    case FGEngine::etTurbine:
        ss.x.add(new FGStateSpace::N2);
        break;
    case FGEngine::etTurboprop:
        ss.x.add(new FGStateSpace::N1);
        break;
    default:
        break;
    }
    ss.x.add(new FGStateSpace::Beta);
    ss.x.add(new FGStateSpace::Phi);
    ss.x.add(new FGStateSpace::P);
    ss.x.add(new FGStateSpace::R);
    ss.x.add(new FGStateSpace::Alt);
    ss.x.add(new FGStateSpace::Psi);

    ss.x.add(new FGStateSpace::ThrottlePos);
    ss.x.add(new FGStateSpace::DaPos);
    ss.x.add(new FGStateSpace::DePos);
    ss.x.add(new FGStateSpace::DrPos);

    ss.u.add(new FGStateSpace::ThrottleCmd);
    ss.u.add(new FGStateSpace::DaCmd);
    ss.u.add(new FGStateSpace::DeCmd);
    ss.u.add(new FGStateSpace::DrCmd);

    // state feedback
    ss.y = ss.x;

    double x0[] =
        {   1.2000005040e+02,
            7.5858289225e-02,
            7.5858289225e-02,
            -5.7341647634e-06,
            1.8532553592e+03,
            -7.2931141482e-03,
            0.0000000000e+00,
            -4.1700389256e-08,
            -3.1694019426e-09,
            1.0000000000e+03,
            0.0000000000e+00,
            8.4822065863e-01,
            1.2929548335e-02,
            -2.6633415566e-02,
            -1.7816482429e-02
        };
    double u0[] =
        {   8.4822065863e-01,
            4.9396555243e-02,
            -5.4509651179e-02,
            -6.3812616150e-02
        };
    ss.x.set(x0),
    ss.u.set(u0);

    // stabilize propulsion
    fdm.GetPropulsion()->GetSteadyState();

    FGNetFDM netFdm;
    FGfdmSocket socket("localhost",5500,FGfdmSocket::ptUDP);
    while (1)
    {
        JSBSim2FlightGearNetFDM(fdm,netFdm,true);
        socket.Send((char *)(& netFdm), sizeof(netFdm));
        fdm.Run();
        //std::cout << "paused" << std::endl;
        //std::cin.get();
        usleep(1.0e6/150.0);
    }
    return 0;
}

// vim:ts=4:sw=4