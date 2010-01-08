#include "SDF.h"
#include <Logging/Logger.h>
#include <LevelSet/CPUStrategy.h>

namespace OpenEngine {
namespace LevelSet {

void Compare( Tex<Vector<2,float> > &g, Vector<2,float> &p, unsigned int x, unsigned int y, int offsetx, int offsety ) {
	Vector<2,float> other = g(x+offsetx,y+offsety);
    other[0] += offsetx;
    other[1] += offsety;

	if (other.GetLengthSquared() < p.GetLengthSquared()) {
        p = other;
	}
}



void GenerateSDF(Tex<Vector<2,float> >& g, int width, int height ) {
    // Pass 0
    for (int y=0;y<height;y++) {
        for (int x=0;x<width;x++) {
            Vector<2,float> p = g(x, y );
			if(x>0)
				Compare( g, p, x, y, -1,  0 );
			if(y>0)
				Compare( g, p, x, y,  0, -1 );
			if(x>0 && y>0)
				Compare( g, p, x, y, -1, -1 );
			if(x<width-1 && y>0)
				Compare( g, p, x, y,  1, -1 );
            g( x, y) =  p;
        }

        for (int x=width-1;x>=0;x--) {
            Vector<2,float> p = g( x, y );
			if(x<width-1)
				Compare( g, p, x, y, 1, 0 );
            g( x, y) =( p );
        }
    }

    // Pass 1
    for (int y=height-1;y>=0;y--) {
        for (int x=width-1;x>=0;x--) {
            Vector<2,float> p = g( x, y );
			if(x<width-1)
				Compare( g, p, x, y,  1,  0 );
			if(y<height-1)
				Compare( g, p, x, y,  0,  1 );
			if(x>0 && y<height-1)
				Compare( g, p, x, y, -1,  1 );
			if(x<width-1 && y<height-1)
				Compare( g, p, x, y,  1,  1 );
            g( x, y) = ( p );
        }

        for (int x=0;x<width;x++) {
            Vector<2,float> p = g( x, y );
			if(x>0)
				Compare( g, p, x, y, -1, 0 );
            g( x, y) =( p );
        }
    }
}


// ^- This will go away soon
SDF::SDF(unsigned int w, unsigned int h, Strategy* s) 
    : strategy(s)
    , width(w)
    , height(h)
    , phiTexture(EmptyTextureResource::Create(width,height,8))
    , outputTexture(EmptyTextureResource::Create(width,height,8))
    , gradientTexture(EmptyTextureResource::Create(width,height,24))
    , phi(width,height)
    , gradient(width,height)    
{
    if (s == NULL)
        strategy = new CPUStrategy();
    phiTexture->Load();
    outputTexture->Load();
    gradientTexture->Load();
     
    // BuildSDF();
    // BuildGradient();

}

SDF::SDF(ITextureResourcePtr input, Strategy* s) 
    : strategy(s)
    , inputTexture(input)
    , width(inputTexture->GetWidth())
    , height(inputTexture->GetHeight())
    , phiTexture(EmptyTextureResource::Create(width,height,8))
    , outputTexture(EmptyTextureResource::Create(width,height,8))
    , gradientTexture(EmptyTextureResource::Create(width,height,24))
    , phi(width,height)
    , gradient(width,height)
{
    


    phiTexture->Load();
    outputTexture->Load();
    gradientTexture->Load();
     
    BuildSDF();
    BuildGradient();
    Reinitialize(30);
}

void SDF::BuildSDF() {
    //Tex<Vector<2,float> > gridInner;
    //Tex<Vector<2,float> > gridOuter;
    
    ITextureResourcePtr in = inputTexture;
    

    const unsigned int Y = in->GetHeight();
    const unsigned int X = in->GetWidth();
    const unsigned char* bw = in->GetData();
    const unsigned int depth = in->GetDepth()/8;


    //Tex<float> pphi(X,Y);

    //Grid* gridInner = new Grid(X,Y);
    //Grid* gridOuter = new Grid(X,Y);

    Tex<Vector<2,float> > gridInner(X,Y);
    Tex<Vector<2,float> > gridOuter(X,Y);

    for (unsigned int y=0; y<Y; y++) {
		for (unsigned int x=0; x<X; x++) {
            unsigned int gray = 0;
            for (unsigned int i=0;i<depth;i++)
                gray += bw[y*X*depth+x*depth+i]; // dummy copy

            gray = (gray > 256)?255:0;

            if(!gray){
				(gridInner)(x,y)[0] = 0;
				(gridInner)(x,y)[1] = 0;
				(gridOuter)(x,y)[0] = X;
				(gridOuter)(x,y)[1] = Y;
			} else {
				(gridInner)(x,y)[0] = X;
				(gridInner)(x,y)[1] = Y;
				(gridOuter)(x,y)[0] = 0;
				(gridOuter)(x,y)[1] = 0;
			}
		}
	}

    GenerateSDF(gridInner,X,Y);
	GenerateSDF(gridOuter,X,Y);

	int dist1 = 0, dist2 = 0, dist = 0;
	for (unsigned int y=0; y<Y; y++) {
		for (unsigned int x=0; x<X; x++) {
			dist1 = (int)( sqrt( (double)gridInner(x,y).GetLengthSquared() ) );
            dist2 = (int)( sqrt( (double)gridOuter(x,y).GetLengthSquared() ) );
            dist = -dist2 + dist1;

			//result[y][x] = dist;
            phi(x,y) = dist;

            if (dist == 0.0f)
                logger.info << "WTF" << logger.end;
		}
	}

    phi.ToTexture(phiTexture);
    updateQueue.Put(phiTexture);


    SDFToTexture(phi,outputTexture);
    updateQueue.Put(outputTexture);
}


void SDF::BuildGradient() {   
    const unsigned int Y = height;
    const unsigned int X = width;

    float dx = 1;
    float dy = 1;
    float cdX, cdY;
    for (unsigned int x=0; x<X; x++)
        for (unsigned int y=0; y<Y; y++) {
      
            //lower left corner
            if (x == 0 && y == 0) {
                cdX = -(phi(x, y) - phi(x+1, y)) / dx;
                cdY = -(phi(x, y) - phi(x, y+1)) / dy;

            } 
            //lower right corner
            else if (x == X - 1 && y == 0) {
                cdX = (phi(x, y) - phi(x-1, y)) / dx;
                cdY = -(phi(x, y) - phi(x, y+1)) / dy;
            }
            //upper left corner
            else if (x == 0 && y == Y - 1) {
                cdX = -(phi(x, y) - phi(x+1, y)) / dx;
                cdY = (phi(x, y) - phi(x, y-1)) / dy;

            }      
            //upper right corner
            else if (x == X - 1 && y == Y - 1) {
                cdX = (phi(x, y) - phi(x-1, y)) / dx;
                cdY = (phi(x, y) - phi(x, y-1)) / dy;

            }

            // upper border
            else if (y == 0 && (x > 0 && x < X - 1)) {
                cdX = -(phi(x-1, y) - phi(x+1, y)) / 2 * dx;
                cdY = -(phi(x, y)   - phi(x, y+1)) / dy;

            }       
            // lower border
            else if (y == Y - 1 && (x > 0 && x < X - 1)) {
                cdX = -(phi(x-1, y) - phi(x+1, y)) / 2 * dx;
                cdY = (phi(x, y)   - phi(x, y-1)) / dy;

            }
            // left border
            else if (x == 0 && (y > 0 && y < Y - 1)) {
                cdX = -(phi(x, y)   - phi(x+1, y)) / dx;
                cdY = -(phi(x, y-1) - phi(x, y+1)) / 2 * dy;

            }
            // right border
            else if (x == X - 1 && (y > 0 && y < Y - 1)) {
                cdX = (phi(x, y)   - phi(x-1, y)) / dx;
                cdY = -(phi(x, y-1) - phi(x, y+1)) / 2 * dy;

            }
            // Normal case
            else {
	
                // central differences
                cdX = -(phi(x-1, y) - phi(x+1, y)) / 2 * dx;
                cdY = -(phi(x, y-1) - phi(x, y+1)) / 2 * dy;
            }

            
            Vector<2, float> g(cdX, cdY);
            // if (g.IsZero()) 
            //     g = Vector<2,float>(0,1);
                
            // g.Normalize();
            gradient(x,y) = g;

        }
    gradient.ToTexture(gradientTexture);
    updateQueue.Put(gradientTexture);

}


Vector<2, float> SDF::Gradient(unsigned int i, unsigned int j) {
    
    return gradient(i,j);       
}

// // Fortegns funktion fra bogen.
// int SDF::S(unsigned int x, unsigned int y) {
//     float phi = phi0(x,y);

//     //7.5
//     //return (phi > 0 ? 1 : -1);

//     Vector<2,float> grad = gradient(x,y);
//     //7.6
//     return phi / sqrt(phi*phi + grad.GetLengthSquared());
// }


void SDF::Reinitialize(unsigned int iterations) {
    strategy->Reinitialize(this,iterations);

    BuildGradient();

    // // debug pring to console
    // unsigned int multip = 15;
    // unsigned int xras = 2*multip, yras = multip;
    // float lengthSum = 0.0, maxGL = 1.0, minGL = 1.0;
    // for (unsigned int y=0; y<height; y++)
    //     for (unsigned int x=0; x<width; x++) {
    //         if (x > width/2-xras && x < width/2+xras) continue;
    //         if (y > height/2-yras && y < height/2-yras) continue;

    //         Vector<2,float> g = gradient(x,y);
    //         float gL = g.GetLength();
    //             if (gL > maxGL) {
    //                 maxGL = gL;
    //                 /*
    //                   logger.info << "max hit on x,y=" << x << "," << y
    //                             << " value=" << gL << logger.end;
    //                 */
    //             }
    //             else if (gL < minGL) {
    //                 minGL = gL;
    //                 /*
    //                 logger.info << "min hit on x,y=" << x << "," << y
    //                             << " value=" << gL << logger.end;
    //                 */
    //             }
    //         lengthSum += gradient(x,y).GetLength() - 1;
    //         if (isnan(g.GetLength()))
    //             logger.info << g << logger.end;
    //     }
    // // logger.info << "gradient length sum:" << lengthSum << logger.end;
    // // logger.info << "gradient length min:" << minGL << logger.end;
    // // logger.info << "gradient length max:" << maxGL << logger.end;

    SDFToTexture(phi,outputTexture);
    updateQueue.Put(outputTexture);

}

void SDF::SDFToTexture(Tex<float> p, EmptyTextureResourcePtr t) {
    for (unsigned int x=0;x<p.GetWidth();x++) {
        for (unsigned int y=0;y<p.GetHeight();y++) {
            //logger.info << p(x,y) << logger.end;
            if (p(x,y) < 0 )
                (*t)(x,y,0) = 0;
            else 
                (*t)(x,y,0) = -1;
        }
    }
}

Tex<float> SDF::GetPhi() {
    //return phi;

    Tex<float> cp(width,height);
    cp = phi;
    return cp;
}

void SDF::SetPhi(Tex<float> phiT) {
    phi = phiT;

    phi.ToTexture(phiTexture);
    updateQueue.Put(phiTexture);

    SDFToTexture(phi,outputTexture);
    updateQueue.Put(outputTexture);
}


void SDF::Refresh() {
    
    

    while(!updateQueue.IsEmpty()) {
        EmptyTextureResourcePtr t = updateQueue.Get();
        t->RebindTexture();
    }

}

}
}
