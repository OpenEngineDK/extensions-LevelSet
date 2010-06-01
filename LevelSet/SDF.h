#ifndef _LS_SDF_H_
#define _LS_SDF_H_

#include <Resources/ITexture2D.h>
#include <Math/Vector.h>
#include <Resources/Tex.h>
#include <Core/LockedQueue.h>

#include <LevelSet/Strategy.h>

using namespace OpenEngine::Resources;
using namespace OpenEngine::Math;
    
namespace OpenEngine {
namespace LevelSet {
    
/**
 * Short description.
 *
 * @class SDF SDF.h ts/LSD/SDF.h
 */

class SDF {
    friend class Strategy;
private:   
    Strategy* strategy;    
    ITextureResourcePtr inputTexture;


    unsigned int width,height;

    EmptyTextureResourcePtr phiTexture;   
    EmptyTextureResourcePtr outputTexture;
    EmptyTextureResourcePtr gradientTexture;

    Tex<float> phi;
    Tex<Vector<2,float> > gradient;

    void BuildSDF();
    void BuildGradient();
    
    void SDFToTexture(Tex<float> p, EmptyTextureResourcePtr t);
    //int S(unsigned int x, unsigned int y);

    LockedQueue<EmptyTextureResourcePtr> updateQueue;

public:
    SDF(ITextureResourcePtr,Strategy* s = NULL);
    SDF(unsigned int,unsigned int,Strategy* s = NULL);
    
    void Reinitialize(unsigned int);

    // Returns the isosurface?
    //ITextureResourcePtr GetInputTexture() {return inputTexture;}
    EmptyTextureResourcePtr GetOutputTexture() {return outputTexture;} 
    EmptyTextureResourcePtr GetPhiTexture() {return phiTexture;}
    EmptyTextureResourcePtr GetGradientTexture() {return gradientTexture;}

    Tex<float> GetPhi();
    void SetPhi(Tex<float>);
    
    
    Tex<Vector<2,float> > GetGradient();
    void SetGradient(Tex<Vector<2,float> >);
    


     unsigned int GetWidth() {return width;}
    unsigned int GetHeight() {return height;}

    float operator()(unsigned int x, unsigned int y) {return phi(x,y);}

    Vector<2,float> Gradient(unsigned int, unsigned int);

    void Refresh();

};


}
}
#endif
