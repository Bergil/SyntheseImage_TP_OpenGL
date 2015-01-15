
// pipeline 2d "attributs" / interpolation barycentrique

#include "Vec.h"
#include "Geometry.h"
#include "Triangle.h"
#include "Image.h"
#include "ImageIO.h"

struct edge
{
    float a, b, c;
    
    edge( const gk::Point& pa, const gk::Point& pb )
    {
        a= -(pb.y - pa.y);
        b= pb.x - pa.x;
        c= -pa.x * a - pa.y * b;
    }
    
    float side( const gk::Point& p ) const { return a*p.x + b*p.y + c; }
};

int main( )
{
    // image resultat
    gk::Image *image= gk::createImage(512, 512);
    
    // sommets du triangle abc
    gk::Point a(0, 0); gk::Color color_a(1, 0, 0);
    gk::Point b(100, 50); gk::Color color_b(0, 1, 0);
    gk::Point c(50, 150); gk::Color color_c(0, 0, 1);
    
    // transforme les sommets du triangle
    gk::Transform t= gk::Translate( gk::Vector(256, 256) ) * gk::RotateZ(35.f);
    a= t(a); 
    b= t(b); 
    c= t(c);
    
    // assemble le triangle
    gk::Triangle triangle( a, b, c );
    
    // calcule les arete du triangle
    edge ab(triangle.a, triangle.b);
    edge bc(triangle.b, triangle.c);
    edge ca(triangle.c, triangle.a);
    // calcule la normalisation 
    float weight= 1.f / (2.f * triangle.area());
    
    // calcule la boite englobante du triangle.
    // pas necessaire, mais plus rapide que de tester tous les pixels de l'image
    gk::BBox screen( gk::Point(0, 0), gk::Point(image->width, image->height) );
    gk::BBox pixels= gk::Intersection(triangle.bbox(), screen);
    
    // dessine le triangle, teste l'inclusion de chaque pixel dans le triangle
    for(int y= pixels.pMin.y; y <= pixels.pMax.y; y++)
    for(int x= pixels.pMin.x; x <= pixels.pMax.x; x++)
    {
        gk::Point p(x, y);
        bool inside= (ab.side(p) > 0.f) && (bc.side(p) > 0.f) && (ca.side(p) > 0.f);
        if(inside)
        {
            // interpole la couleur des sommets du triangle
            gk::Color color= color_a * ab.side(p) * weight + color_b * bc.side(p) * weight + color_c * ca.side(p) * weight;
            image->setPixel(x, y, color);
        }
    }
    
    // enregistre l'image
    gk::ImageIO::writeImage("out.png", image);
    delete image;
    
    return 0;
}
