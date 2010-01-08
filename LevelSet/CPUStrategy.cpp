// 
// -------------------------------------------------------------------
// Copyright (C) 2007 OpenEngine.dk (See AUTHORS) 
// 
// This program is free software; It is covered by the GNU General 
// Public License version 2 or any later version. 
// See the GNU General Public License for more details (see LICENSE). 
//--------------------------------------------------------------------

#include "CPUStrategy.h"
#include <Resources/Tex.h>
#include <LevelSet/SDF.h>

namespace OpenEngine {
namespace LevelSet {


CPUStrategy::CPUStrategy() {

}

void CPUStrategy::Reinitialize(SDF* sdf,unsigned int iterations) {
//----------------------------
    Tex<float> phi = sdf->GetPhi();
    Tex<float> phi0 = sdf->GetPhi();
    Tex<float> phin = sdf->GetPhi();

    unsigned int width=sdf->GetWidth(),
        height=sdf->GetHeight();


    //const unsigned int numIterations = 100;
    for (unsigned int i = 0; i<iterations; i++) {
        for (unsigned int x = 0; x < width; x++)
            for (unsigned int y = 0; y < height; y++) {                                
                float xy = phi(x, y);
                
                float phiXPlus = 0.0f;
                float phiXMinus = 0.0f;
                float phiYPlus = 0.0f;
                float phiYMinus = 0.0f;        	
                if (x != width-1) phiXPlus  = (phi(x+1, y) - xy);
                if (x != 0)       phiXMinus = (xy - phi(x-1, y));
           
                if (y !=height-1) phiYPlus  = (phi(x, y+1) - xy);
                if (y != 0)       phiYMinus = (xy - phi(x, y-1));
        	
                float dXSquared = 0;
                float dYSquared = 0;
                float a = phi0(x,y);
                if (a > 0) {
                    // formula 6.3 page 58
                    float max = std::max(phiXMinus, 0.0f);
                    float min = std::min(phiXPlus, 0.0f);
                    dXSquared = std::max(max*max, min*min);
                    
                    max = std::max(phiYMinus, 0.0f);
                    min = std::min(phiYPlus, 0.0f);
                    dYSquared = std::max(max*max, min*min);
                } else {
                    // formula 6.4 page 58
                    float max = std::max(phiXPlus, 0.0f);
                    float min = std::min(phiXMinus, 0.0f);
                    dXSquared = std::max(max*max, min*min);
                    
                    max = std::max(phiYPlus, 0.0f);
                    min = std::min(phiYMinus, 0.0f);
                    dYSquared = std::max(max*max, min*min);        				
                }
        			
                float normSquared = dXSquared + dYSquared;           
                float norm = sqrt(normSquared);

                // Using the S(phi) sign formula 7.6 on page 67
                //float sign = phi(x,y) / sqrt(phi(x,y)*phi(x,y) + normSquared);
                float sign = phi0(x,y) / sqrt(phi0(x,y)*phi0(x,y) + 1);
                float t = 0.3; // A stabil CFL condition
                phin(x,y) = phi(x,y) - sign*(norm - 1)*t;
                
                // Testing
                //phin(x,y) = phiYPlus;

            }

        for (unsigned int y=0; y<height ; y++)
            for (unsigned int x=0; x<width; x++)
                phi(x,y) = phin(x,y);

    }
    sdf->SetPhi(phi);

}


} // NS LevelSet
} // NS OpenEngine
