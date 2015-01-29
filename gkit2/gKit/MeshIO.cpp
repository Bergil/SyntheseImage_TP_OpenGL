
#include <cstdio>
#include <vector> 
#include <map> 

#include "Logger.h"

#include "IOFileSystem.h"

#include "MeshIO.h"
#include "Geometry.h"

#include "Mesh.h"
#include "QuadMesh.h"
#include "MeshData.h"


namespace gk {

namespace MeshIO {

//! analyse un fichier .obj compose de quads exclusivement
QuadMesh *readQuadOBJ( const std::string& filename )
{
    MeshData data;
    if(readOBJ(filename, data) < 0)
        return NULL;
    
    QuadMesh *mesh= new QuadMesh;
    mesh->filename= filename;
    
    if(data.faces.empty())
    {
        // renvoie les donnes brutes, sans essayer de construire une indexation lineaire.
        mesh->positions.swap(data.positions);
        
        printf("  %lu positions, %lu texcoords, %lu normals\n",  
            mesh->positions.size(), mesh->texcoords.size(), mesh->normals.size());
        printf("done.\n");
        return mesh;
    }
    
    unsigned int triangles, quads, others;
    faceCount(data.faces, triangles, quads, others);
    printf("  %lu faces: %u quad, %u triangles\n", data.faces.size(), quads, triangles);
    if(data.faces.size() != quads)
    {
        delete mesh;
        return NULL;
    }

    buildFaces(data);
    
    mesh->box= data.box;
    mesh->positions.swap(data.positions);
    mesh->texcoords.swap(data.texcoords);
    mesh->normals.swap(data.normals);
    mesh->faces.swap(data.faces);
    
    //! \todo materials, groups
    mesh->materials.assign(mesh->faces.size(), 0);      // tous les quads utilisent la matiere 0
    mesh->groups.push_back( MeshGroup(data.materials[0], 0) );
    
    return mesh;
}    


//! analyse un fichier .obj et construit un objet Mesh.
Mesh *readOBJ( const std::string& filename ) 
{
    MeshData data;
    if(readOBJ(filename, data) < 0)
        return NULL;
    
    Mesh *mesh= new Mesh;
    mesh->filename= filename;
    
    if(data.faces.empty())
    {
        // renvoie les donnes brutes, sans essayer de construire une indexation lineaire.
        mesh->positions.swap(data.positions);
        
        printf("  %lu positions, %lu texcoords, %lu normals\n",  
            mesh->positions.size(), mesh->texcoords.size(), mesh->normals.size());
        printf("done.\n");
        return mesh;
    }
    
    // triangule le maillage, si necessaire
    unsigned int triangles, quads, others;
    faceCount(data.faces, triangles, quads, others);
    printf("  %lu faces: %u quads, %u triangles\n", data.faces.size(), quads, triangles);
    if(data.faces.size() != triangles)
        triangulate(data.faces);
    
    //! \todo eventuellement recalculer les normales moyennes des sommets avant de lineariser les buffers...
    
    // construit l'index buffer avec indexation unique
    // et re ordonne les attributs
    std::map<Vertex, int> remap;
    mesh->indices.reserve(data.faces.size() * 3);
    
    // identifie les triplets uniques d'attributs pour construire une indexation "unique" / vertex buffer lineaire.
    int material_id= -1;
    for(unsigned int i= 0; i < data.faces.size(); i++)
    for(unsigned int k= 0; k < data.faces[i].n; k++)
    {
        assert(data.faces[i].n == 3);
        if(data.faces[i].vertex[k].position == -1)
        {
            printf("loading mesh '%s'... failed.\n  invalid structure.\n", filename.c_str());
            delete mesh;
            return NULL;
        }
        
        std::pair< std::map<Vertex, int>::iterator, bool > found= 
            remap.insert( std::make_pair(data.faces[i].vertex[k], remap.size()) );
        mesh->indices.push_back(found.first->second);

        // cree les groupes de faces utilisant la meme matiere
        //! \todo fusionner les groupes de faces utilisant la meme matiere.
        if(found.first->first.material != material_id)
        {
            material_id= found.first->first.material;
            mesh->groups.push_back( MeshGroup(data.materials[material_id], i*3) );      // vertex index
        }
        
        mesh->groups.back().end= 3*(i +1);
    }
    
    //~ for(unsigned int i= 0; i < mesh->groups.size(); i++)
        //~ printf("group %s, begin %d end %d\n  diffuse '%s'\n  specular '%s'\n", 
            //~ mesh->groups[i].material.name.c_str(), mesh->groups[i].begin, mesh->groups[i].end,
            //~ mesh->groups[i].material.diffuse_texture.c_str(), mesh->groups[i].material.specular_texture.c_str());
    
    // affecte les matieres aux triangles 
    mesh->materials.resize(data.faces.size(), -1);
    for(unsigned int i= 0; i < mesh->groups.size(); i++)
    {
        unsigned int begin= mesh->groups[i].begin / 3;
        unsigned int end= mesh->groups[i].end / 3;
        
        for(unsigned int k= begin; k < end; k++)
            mesh->materials[k]= i;
    }
    
    mesh->box= data.box;
    // pre-alloue les buffers
    mesh->positions.resize(remap.size());
    if(data.has_texcoord) mesh->texcoords.resize(remap.size());
    if(data.has_normal) mesh->normals.resize(remap.size());
    
    // reordonne les attributs
    unsigned int mesh_index= 0;
    for(unsigned int i= 0; i < data.faces.size(); i++)
    for(unsigned int k= 0; k < data.faces[i].n; k++, mesh_index++)
    {
        const int index= mesh->indices[mesh_index];
        mesh->positions[index]= data.positions[data.faces[i].vertex[k].position];
        
        if(data.has_texcoord) // && data.faces[i].vertices[k].texcoord != -1)
            mesh->texcoords[index]= data.texcoords[data.faces[i].vertex[k].texcoord];
        
        if(data.has_normal) // && data.faces[i].vertices[k].normal != -1)
            mesh->normals[index]= data.normals[data.faces[i].vertex[k].normal];
    }
    
    printf("  %lu positions, %lu texcoords, %lu normals, %lu triangles\n",  
        mesh->positions.size(), mesh->texcoords.size(), mesh->normals.size(), 
        mesh->indices.size() / 3);
    printf("  %lu material groups\n", mesh->groups.size());
    printf("done.\n");
    return mesh;
}


//! calcule les normales moyenne par sommet.
int buildNormals( Mesh *mesh )
{
    if(mesh == NULL)
        return -1;

    mesh->normals.assign(mesh->positions.size(), Vec3());
    
    //! \todo virer les positions dupliquees avant de calculer les normales moyennes...
    const unsigned int triangles= mesh->indices.size() / 3u;
    for(unsigned int i= 0; i < triangles; i++)
    {
        const unsigned int ia= mesh->indices[3u*i];
        const unsigned int ib= mesh->indices[3u*i +1u];
        const unsigned int ic= mesh->indices[3u*i +2u];
        
        const Point a(mesh->positions[ia]);
        const Point b(mesh->positions[ib]);
        const Point c(mesh->positions[ic]);
        
        // attention aux triangles degeneres avec des aretes de longueur nulles
        Vector ab(a, b);
        Vector ac(a, c);
        Vector n= Cross(ab, ac);
        mesh->normals[ia]= Vector(mesh->normals[ia]) + n;
        mesh->normals[ib]= Vector(mesh->normals[ib]) + n;
        mesh->normals[ic]= Vector(mesh->normals[ic]) + n;
    }
    
    for(unsigned int i= 0; i < mesh->normals.size(); i++)
        mesh->normals[i]= Normalize( Vector(mesh->normals[i]) );
    
    return 0;
}

//! calcule les tangentes / bitangentes moyennes des sommets.
//! cf. http://www.terathon.com/code/tangent.html
//! cf. http://www.crytek.com/download/Triangle_mesh_tangent_space_calculation.pdf
int buildTangents( Mesh *mesh, std::vector<Vec3>& tangents, std::vector<Vec3>& bitangents )
{
    if(mesh == NULL) return -1;
    
    if(mesh->normals.size() != mesh->positions.size())
        if(buildNormals(mesh) < 0) return -1;
    
    tangents.assign(mesh->positions.size(), Vec3());
    bitangents.assign(mesh->positions.size(), Vec3());
    
    const unsigned int triangles= mesh->indices.size() / 3u;
    for(unsigned int i= 0; i < triangles; i++)
    {
        const PTNTriangle& triangle= mesh->ptntriangle(i);
        Vector e1= triangle.b - triangle.a;
        Vector e2= triangle.c - triangle.a;
        
        Vector st1= triangle.tb - triangle.ta;
        Vector st2= triangle.tc - triangle.ta;
        
        float r= 1.0f / (st1.x * st2.y - st2.x * st1.y);
        Vector s( 
            (st2.t * e1.x - st1.t * e2.x) * r, 
            (st2.t * e1.y - st1.t * e2.y) * r, 
            (st2.t * e1.z - st1.t * e2.z) * r);
        Vector t( 
            (st1.s * e2.x - st2.s * e1.x) * r, 
            (st1.s * e2.y - st2.s * e1.y) * r, 
            (st1.s * e2.z - st2.s * e1.z) * r);
        
        const unsigned int ia= mesh->indices[3u*i];
        const unsigned int ib= mesh->indices[3u*i +1u];
        const unsigned int ic= mesh->indices[3u*i +2u];
        tangents[ia]= Vector(tangents[ia]) + s;
        tangents[ib]= Vector(tangents[ib]) + s;
        tangents[ic]= Vector(tangents[ic]) + s;
        
        bitangents[ia]= Vector(bitangents[ia]) + t;
        bitangents[ib]= Vector(bitangents[ib]) + t;
        bitangents[ic]= Vector(bitangents[ic]) + t;
    }
    
    // normalisation.
    for(unsigned int i= 0; i < mesh->positions.size(); i++)
    {
        tangents[i]= Normalize( Vector(tangents[i]) );
        bitangents[i]= Normalize( Vector(bitangents[i]) );
    }
    
    return 0;
}

int buildOrthoTangents( Mesh *mesh, std::vector<Vec4>& tangents )
{
    std::vector<Vec3> tangents3;
    std::vector<Vec3> bitangents3;
    if(buildTangents(mesh, tangents3, bitangents3) < 0) return -1;
    
    // reconstruit un repere ortho norme.
    // cf. http://www.terathon.com/code/tangent.html
    tangents.assign(mesh->positions.size(), Vec4());
    for(unsigned int i= 0; i < mesh->positions.size(); i++)
    {
        Vector n= Vector(mesh->normals[i]);
        Vector t= Vector(tangents3[i]);
        Vector b= Vector(bitangents3[i]);
        
        float w= Dot(Cross(n, t), b) < 0.0f ? -1.0f : 1.0f;
        tangents[i]= Vec4(Normalize(t - n * Dot(n, t)), w);
        
        // b = tangents[i].w * cross(n, tangents[i].xyz)
    }
    
    return 0;
}

}       // namespace

}       // namespace

