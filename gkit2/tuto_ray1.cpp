// squelette de lancer de rayons

#include <cstdlib>
#include <time.h>

#include "Geometry.h"
#include "Transform.h"

#include "Triangle.h"

#include "Mesh.h"
#include "Image.h"

#include "MeshIO.h"
#include "ImageIO.h"


// objet
gk::Mesh *mesh= NULL;

// representation d'une source de lumiere
struct Source
{
    gk::Triangle triangle;
    gk::Color emission;
    
    Source( const gk::Triangle& t, const gk::Color& e ) : triangle(t), emission(e) {}
};

// ensemble de sources de lumieres
std::vector<Source> sources;

// recuperer les sources de lumiere du mesh : triangles associee a une matiere qui emet de la lumiere, material.emission != 0
int build_sources( const gk::Mesh *mesh )
{
    for(int i= 0; i < mesh->triangleCount(); i++)
    {
        // recuepre la matiere associee a chaque triangle de l'objet
        const gk::MeshMaterial& material= mesh->triangleMaterial(i);
       
        if(gk::Color(material.emission).isBlack() == false)
            // inserer la source de lumiere dans l'ensemble.
            sources.push_back( Source(mesh->triangle(i), gk::Color(material.emission)) );
    }
    
    printf("%lu sources.\n", sources.size());
    return sources.size();
}


// ensemble de triangles
std::vector<gk::Triangle> triangles;

// recuperer les triangles du mesh
int build_triangles( const gk::Mesh *mesh )
{
    for(int i= 0; i < mesh->triangleCount(); i++)
        triangles.push_back( mesh->triangle(i) );
    
    printf("%lu triangles.\n", triangles.size());
    return triangles.size();
}


// calcule l'intersection d'un rayon et de tous les triangles
bool intersect( const gk::Ray& ray, gk::Hit& hit )
{
    for(unsigned int i= 0; i < triangles.size(); i++)
    {
	float t, u, v;
	if(triangles[i].Intersect(ray, hit.t, t, u, v))
	{
            hit.t= t;
            hit.u= u;
            hit.v= v;
            hit.p= ray(t);      // evalue la positon du point d'intersection sur le rayon
            hit.object_id= i;	// permet de retrouver toutes les infos associees au triangle
	}
    }

    return (hit.object_id != -1);
}

float oneFloat( )
{
    return drand48();
}


gk::Color direct( const gk::Point& p, const gk::Normal& n, const gk::MeshMaterial& material )
{
    gk::Color color;
    
    // a completer
    //~ { ... }
    
    return color;
}

gk::Color indirect( const gk::Point& p, const gk::Normal& n, const gk::MeshMaterial& material )
{
    gk::Color color;
    
    // a completer
    //~ { ... }
    
    return color;
}

int main( )
{
    srand48( time(NULL) );
    
    // charger un objet
    mesh= gk::MeshIO::readOBJ("geometry.obj");
    if(mesh == NULL) return 1;

    build_sources(mesh);        // recupere les sources de lumiere
    build_triangles(mesh);      // recupere les triangles
   
    // creer une image resultat
    gk::Image *image= gk::createImage(512, 512);
   
    // definir les transformations
    gk::Transform model;
    
    /*
        geometry
            translate x y z
            -221.766296 232.837692 575.962341 
            rotate y x
            -378.000000 -7.000000
    */
    
    gk::Transform view= (gk::Translate( gk::Vector(-221.f, 232.f, 575.f) ) * gk::RotateX(-7.f) * gk::RotateY(-378.f)).inverse();
    gk::Transform projection= gk::Perspective(50.f, 1.f, 1.f, 1000.f);  // projection perspective
    gk::Transform viewport= gk::Viewport(image->width, image->height);      // transformation adaptee a la resolution de l'image resultat
    
    // compose les transformations utiles
    gk::Transform vpv= viewport * projection * view;
    
    // parcours tous les pixels de l'image
    for(int y= 0; y < image->height; y++)
    {
        for(int x= 0; x < image->width; x++)
        {
            // generer le rayon pour le pixel (x,y) dans le repere de l'image
            gk::Point origine(x +.5f, y + .5f, -1.f);    // sur le plan near
            gk::Point extremite(x +.5f, y + .5f, 1.f);    // sur le plan far
           
            // transformer le rayon dans le repere de la scene
            gk::Point o= { ... }; // transformation de origine
            gk::Point e= { ... }; // transformation de extremite
            
            gk::Ray ray(o, e);  // construire le rayon
            gk::Hit hit(ray);   // preparer l'intersection
            
            gk::Color color;    // couleur du pixel.
           
            // calculer l'intersection du rayon avec les triangles
            if(intersect(ray, hit))
            {
                // si une intersection existe... hit_object_id= indice du triangle
                // recuperer les informations sur la matiere associee au point d'intersection
                gk::Point p= hit.p;
                gk::Normal normal= mesh->triangle(hit.object_id).normal();
                gk::MeshMaterial material= mesh->triangleMaterial(hit.object_id);

                // couleur "aleatoire", eventuellement
                //~ material.diffuse_color= gk::Color(material.diffuse_color) * gk::Color(1.f - float(hit.object_id % 100) / 99.f, float(hit.object_id % 10) / 9.f, float(hit.object_id % 1000) / 999.f);       
                
                // calculer l'energie reflechie par le point vers la camera
                // etape 1 : eclairage direct
                color= color + direct(p, normal, material);
               
                // etape 2 : eclairage indirect
                color= color + indirect(p, normal, material);
            }
           
            // ecrire la couleur dans l'image
            image->setPixel(x, y, gk::Color(color.r, color.g, color.b, 1.0f));
        }
    }
   
    // enregistrer l'image
    gk::ImageIO::writeImage("render.hdr", image);
    delete image;
    
    return 0;
}
