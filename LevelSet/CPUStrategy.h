// 
// -------------------------------------------------------------------
// Copyright (C) 2007 OpenEngine.dk (See AUTHORS) 
// 
// This program is free software; It is covered by the GNU General 
// Public License version 2 or any later version. 
// See the GNU General Public License for more details (see LICENSE). 
//--------------------------------------------------------------------


#include <LevelSet/Strategy.h>

namespace OpenEngine {
namespace LevelSet {

class CPUStrategy : public Strategy {

public:
    CPUStrategy();
    void Reinitialize(SDF* sdf,unsigned int iterations); 
};

} // NS LevelSet
} // NS OpenEngine
