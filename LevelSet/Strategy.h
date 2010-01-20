// 
// -------------------------------------------------------------------
// Copyright (C) 2007 OpenEngine.dk (See AUTHORS) 
// 
// This program is free software; It is covered by the GNU General 
// Public License version 2 or any later version. 
// See the GNU General Public License for more details (see LICENSE). 
//--------------------------------------------------------------------

#ifndef _LEVEL_SET_STRATEGY_H_
#define _LEVEL_SET_STRATEGY_H_

#include <string>
#include <typeinfo>

namespace OpenEngine {
namespace LevelSet {

class SDF; // Forward declaration

class Strategy {
    
public:
    virtual void Reinitialize(SDF* sdf, unsigned int iterations) =0;
    virtual void BuildGradient(SDF* sdf) =0;

    std::string ToString() const {
        return typeid(*this).name();
    }
};


} // NS LevelSet
} // NS OpenEngine

#endif
