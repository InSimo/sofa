/******************************************************************************
*       SOFA, Simulation Open-Framework Architecture, version 1.0 RC 1        *
*            (c) 2006-2021 MGH, INRIA, USTL, UJF, CNRS, InSimo                *
*                                                                             *
* This library is free software; you can redistribute it and/or modify it     *
* under the terms of the GNU Lesser General Public License as published by    *
* the Free Software Foundation; either version 2.1 of the License, or (at     *
* your option) any later version.                                             *
*                                                                             *
* This library is distributed in the hope that it will be useful, but WITHOUT *
* ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or       *
* FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License *
* for more details.                                                           *
*                                                                             *
* You should have received a copy of the GNU Lesser General Public License    *
* along with this library; if not, write to the Free Software Foundation,     *
* Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301 USA.          *
*******************************************************************************
*                               SOFA :: Modules                               *
*                                                                             *
* Authors: The SOFA Team and external contributors (see Authors.txt)          *
*                                                                             *
* Contact information: contact@sofa-framework.org                             *
******************************************************************************/
#include <sofa/core/ObjectFactory.h>
#include <SofaLoader/MeshObjLoader.h>
#include <sofa/core/visual/VisualParams.h>
#include <sofa/helper/system/SetDirectory.h>
#include <numeric>

namespace sofa
{

namespace component
{

namespace loader
{

using namespace sofa::defaulttype;
using namespace sofa::core::loader;

SOFA_DECL_CLASS(MeshObjLoader)

int MeshObjLoaderClass = core::RegisterObject("Specific mesh loader for Obj file format.")
        .add< MeshObjLoader >()
        ;



MeshObjLoader::MeshObjLoader()
    : MeshLoader()
    , d_handleSeams(initData(&d_handleSeams, (bool) false, "handleSeams", "Preserve UV and normal seams information (vertices with multiple UV and/or normals)"))
    , loadMaterial(initData(&loadMaterial, (bool) true, "loadMaterial", "Load the related MTL file or use a default one?"))
    , d_trianglesOnBorder(initData(&d_trianglesOnBorder, "trianglesOnBorder", "List of triangles on borders in case we have disonctinuity"))
    , d_trianglesIndices(initData(&d_trianglesIndices, "trianglesIndices", "List of triangles indices"))
    , faceType(MeshObjLoader::TRIANGLE)
    , materials(initData(&materials,"materials","List of materials") )
    , faceList(initData(&faceList,"faceList","List of face definitions.") )
    , texIndexList(initData(&texIndexList,"texcoordsIndex","Indices of textures coordinates used in faces definition."))
    , positionsList(initData(&positionsList,"positionsDefinition", "Vertex positions definition"))
    , texCoordsList(initData(&texCoordsList,"texcoordsDefinition", "Texture coordinates definition"))
    , normalsIndexList(initData(&normalsIndexList,"normalsIndex","List of normals of elements of the mesh loaded."))
    , normalsList(initData(&normalsList,"normalsDefinition","Normals definition"))
    , texCoords(initData(&texCoords,"texcoords","Texture coordinates of all faces, to be used as the parent data of a VisualModel texcoords data"))
    , texCoordsInFace(initData(&texCoordsInFace, "texCoordsInFace", "Texture coordinates sorted for each face, used mainly to detect discontinuities in the texture coordinates"))
    , computeMaterialFaces(initData(&computeMaterialFaces, false, "computeMaterialFaces", "True to activate export of Data instances containing list of face indices for each material"))
    , d_vertPosIdx      (initData   (&d_vertPosIdx, "vertPosIdx", "If vertices have multiple normals/texcoords stores vertices position indices"))
    , d_vertNormIdx     (initData   (&d_vertNormIdx, "vertNormIdx", "If vertices have multiple normals/texcoords stores vertices normal indices"))
    , d_pointsOnBorder(initData(&d_pointsOnBorder, "pointsOnBorder", "List of points on borders in case we have disonctinuity"))
{
    faceList.setGroup("OBJ");
    texIndexList.setGroup("OBJ");
    texCoordsList.setGroup("OBJ");
    normalsIndexList.setGroup("OBJ");
    normalsList.setGroup("OBJ");
    positionsList.setGroup("OBJ");
    //BUGFIX: data loaded from OBJ file should not be saved to XML
    faceList.setPersistent(false);
    texIndexList.setPersistent(false);
    texCoordsList.setPersistent(false);
    normalsIndexList.setPersistent(false);
    normalsList.setPersistent(false);
    positionsList.setPersistent(false);
    texCoords.setPersistent(false);
    texCoordsInFace.setPersistent(false);
    positions.setPersistent(false);
    normals.setPersistent(false);
    edges.setPersistent(false);
    triangles.setPersistent(false);
    quads.setPersistent(false);
    edgesGroups.setPersistent(false);
    trianglesGroups.setPersistent(false);
    quadsGroups.setPersistent(false);
    d_vertPosIdx.setPersistent(false);
    d_vertNormIdx.setPersistent(false);
    d_pointsOnBorder.setPersistent(false);
    d_trianglesOnBorder.setPersistent(false);
    d_trianglesIndices.setPersistent(false);
}


MeshObjLoader::~MeshObjLoader()
{

}


bool MeshObjLoader::load()
{
    sout << "Loading OBJ file: " << m_filename << sendl;

    bool fileRead = false;

    // -- Loading file
    const char* filename = m_filename.getFullPath().c_str();
    std::string fileContent;

    if (!sofa::helper::system::DataRepository.getFileContent(filename, fileContent))
    {
        serr << "Error: MeshObjLoader: Cannot read file '" << m_filename << "'." << sendl;
        return false;
    }
    
    std::istringstream filestream(fileContent);

    // -- Reading file
    fileRead = this->readOBJ (filestream,filename);

    return fileRead;
}


void MeshObjLoader::addGroup (const PrimitiveGroup& g)
{

    helper::vector< PrimitiveGroup>& my_edgesGroups = *(edgesGroups.beginEdit());
    helper::vector< PrimitiveGroup>& my_trianglesGroups = *(trianglesGroups.beginEdit());
    helper::vector< PrimitiveGroup>& my_quadsGroups = *(quadsGroups.beginEdit());

    switch (faceType)
    {
    case MeshObjLoader::EDGE:
        my_edgesGroups.push_back(g);
        break;
    case MeshObjLoader::TRIANGLE:
        my_trianglesGroups.push_back(g);
        break;
    case MeshObjLoader::QUAD:
        my_quadsGroups.push_back(g);
        break;
    default: break;
    }

    edgesGroups.endEdit();
    trianglesGroups.endEdit();
    quadsGroups.endEdit();
}

bool MeshObjLoader::readOBJ (std::istringstream& filestream, const char* filename)
{
    sout << "MeshObjLoader::readOBJ" << sendl;
    const bool handleSeams = d_handleSeams.getValue();
    helper::vector<sofa::defaulttype::Vector3>& my_positions = *(positionsList.beginEdit());
    helper::vector<sofa::defaulttype::Vector2>& my_texCoords = *(texCoordsList.beginEdit());
    helper::vector<sofa::defaulttype::Vector3>& my_normals   = *(normalsList.beginEdit());

    helper::vector<Material>& my_materials = *(materials.beginEdit());
    helper::SVector< helper::SVector <int> >& my_faceList = *(faceList.beginEdit() );
    helper::SVector< helper::SVector <int> >& my_normalsList = *(normalsIndexList.beginEdit());
    helper::SVector< helper::SVector <int> >& my_texturesList   = *(texIndexList.beginEdit());
    helper::vector<int> nodes, nIndices, tIndices;

    helper::vector<Edge >& my_edges = *(edges.beginEdit());
    helper::vector<Triangle >& my_triangles = *(triangles.beginEdit());
    helper::vector<Quad >& my_quads = *(quads.beginEdit());

    //BUGFIX: clear pre-existing data before loading the file
    my_positions.clear();
    my_texCoords.clear();
    my_normals.clear();
    my_materials.clear();
    my_faceList.clear();
    my_normalsList.clear();
    my_texturesList.clear();
    my_edges.clear();
    my_triangles.clear();
    my_quads.clear();
    edgesGroups.beginEdit()->clear(); edgesGroups.endEdit();
    trianglesGroups.beginEdit()->clear(); trianglesGroups.endEdit();
    quadsGroups.beginEdit()->clear(); quadsGroups.endEdit();

    int vtn[3];
    Vector3 result;
    helper::WriteAccessor<Data<helper::vector< PrimitiveGroup> > > my_faceGroups[NBFACETYPE] =
    {
        edgesGroups,
        trianglesGroups,
        quadsGroups
    };
    std::string curGroupName = "Default_Group";
    std::string curMaterialName;
    int curMaterialId = -1;
    int nbFaces[NBFACETYPE] = {0}; // number of edges, triangles, quads
    int groupF0[NBFACETYPE] = {0}; // first primitives indices in current group for edges, triangles, quads
    std::string line;
    while(std::getline(filestream,line) )
    {
        if (line.empty()) continue;
        std::istringstream values(line);
        std::string token;

        values >> token;
        if (token == "#")
        {
            /* comment */
        }
        else if (token == "v")
        {
            /* vertex */
            values >> result[0] >> result[1] >> result[2];
            my_positions.push_back(Vector3(result[0],result[1], result[2]));
        }
        else if (token == "vn")
        {
            /* normal */
            values >> result[0] >> result[1] >> result[2];
            my_normals.push_back(Vector3(result[0],result[1], result[2]));
        }
        else if (token == "vt")
        {
            /* texcoord */
            values >> result[0] >> result[1];
            my_texCoords.push_back(Vector2(result[0],result[1]));
        }
        else if ((token == "mtllib") && loadMaterial.getValue())
        {
            while (!values.eof())
            {
                std::string materialLibaryName;
                values >> materialLibaryName;
                std::string mtlfile = sofa::helper::system::SetDirectory::GetRelativeFromFile(materialLibaryName.c_str(), filename);
                this->readMTL(mtlfile.c_str(), my_materials);
            }
        }
        else if (token == "usemtl" || token == "g")
        {
            // end of current group
            //curGroup.nbp = nbf - curGroup.p0;
            for (int ft = 0; ft < NBFACETYPE; ++ft)
                if (nbFaces[ft] > groupF0[ft])
                {
                    my_faceGroups[ft].push_back(PrimitiveGroup(groupF0[ft], nbFaces[ft]-groupF0[ft], curMaterialName, curGroupName, curMaterialId));
                    groupF0[ft] = nbFaces[ft];
                }
            if (token == "usemtl")
            {
                values >> curMaterialName;
                curMaterialId = -1;
                helper::vector<Material>::iterator it = my_materials.begin();
                helper::vector<Material>::iterator itEnd = my_materials.end();
                for (; it != itEnd; ++it)
                {
                    if (it->name == curMaterialName)
                    {
                        // std::cout << "Using material "<<it->name<<std::endl;
                        (*it).activated = true;
                        if (!material.activated)
                            material = *it;
                        curMaterialId = it - my_materials.begin();
                        break;
                    }
                }
            }
            else if (token == "g")
            {
                curGroupName.clear();
                while (!values.eof())
                {
                    std::string g;
                    values >> g;
                    if (!curGroupName.empty())
                        curGroupName += " ";
                    curGroupName += g;
                }
            }
        }
        else if (token == "l" || token == "f")
        {
            /* face */
            nodes.clear();
            nIndices.clear();
            tIndices.clear();

            while (!values.eof())
            {
                std::string face;
                values >> face;
                if (face.empty()) continue;
                for (int j = 0; j < 3; j++)
                {
                    vtn[j] = -1;
                    std::string::size_type pos = face.find('/');
                    std::string tmp = face.substr(0, pos);
                    if (pos == std::string::npos)
                        face = "";
                    else
                    {
                        face = face.substr(pos + 1);
                    }

                    if (!tmp.empty())
                    {
                        vtn[j] = atoi(tmp.c_str());
                        if (vtn[j] >= 1)
                            vtn[j] -=1; // -1 because the numerotation begins at 1 and a vector begins at 0
                        else if (vtn[j] < 0)
                            vtn[j] += (j==0) ? my_positions.size() : (j==1) ? my_texCoords.size() : my_normals.size();
                        else
                        {
                            serr << "Invalid index " << tmp << sendl;
                            vtn[j] = -1;
                        }
                    }
                }

                nodes.push_back(vtn[0]);
                tIndices.push_back(vtn[1]);
                nIndices.push_back(vtn[2]);
            }

            my_faceList.push_back(nodes);
            my_normalsList.push_back(nIndices);
            my_texturesList.push_back(tIndices);

            if (nodes.size() == 2) // Edge
            {
                if (!handleSeams) // we have to wait for renumbering vertices if we handle seams
                {
                    addEdge(&my_edges, Edge(nodes[0], nodes[1]));
                }
                ++nbFaces[MeshObjLoader::EDGE];
                faceType = MeshObjLoader::EDGE;
            }
            else if (nodes.size()==4 && !this->triangulate.getValue()) // Quad
            {
                if (!handleSeams) // we have to wait for renumbering vertices if we handle seams
                {
                    addQuad(&my_quads, Quad(nodes[0], nodes[1], nodes[2], nodes[3]));
                }
                ++nbFaces[MeshObjLoader::QUAD];
                faceType = MeshObjLoader::QUAD;
            }
            else // Triangulate
            {
                if (!handleSeams) // we have to wait for renumbering vertices if we handle seams
                {
                    for (size_t j=2; j<nodes.size(); j++)
                        addTriangle(&my_triangles, Triangle(nodes[0], nodes[j-1], nodes[j]));
                }
                ++nbFaces[MeshObjLoader::TRIANGLE];
                faceType = MeshObjLoader::TRIANGLE;
            }

        }
        else
        {
            // std::cerr << "readObj : Unknown token for line " << line << std::endl;
        }
    }

    // end of current group
    for (size_t ft = 0; ft < NBFACETYPE; ++ft)
        if (nbFaces[ft] > groupF0[ft])
        {
            my_faceGroups[ft].push_back(PrimitiveGroup(groupF0[ft], nbFaces[ft]-groupF0[ft], curMaterialName, curGroupName, curMaterialId));
            groupF0[ft] = nbFaces[ft];
        }

    int nbVIn = (int)my_positions.size();
    // First we compute for each point how many pair of normal/texcoord indices are used
    // The map store the final index of each combinaison
    std::vector< std::map< std::pair<int,int>, int > > vertTexNormMap;
    vertTexNormMap.resize(nbVIn);
    for (size_t fi=0; fi<my_faceList.size(); ++fi)
    {
        const helper::SVector<int>& nodes = my_faceList[fi];
        const helper::SVector<int>& nIndices = my_normalsList[fi];
        const helper::SVector<int>& tIndices = my_texturesList[fi];
        for (size_t i = 0; i < nodes.size(); ++i)
        {
            unsigned int pi = nodes[i];
            unsigned int ni = nIndices[i];
            unsigned int ti = tIndices[i];
            vertTexNormMap[pi][std::make_pair(ti, ni)] = 0;
        }
    }

    const bool hasTexCoords = my_texCoords.size() > 0 ;
    const bool hasNormals = my_normals.size() > 0 ;

    if (!d_handleSeams.getValue())
    { // default mode, vertices are never duplicated, only one texcoord and normal is used per vertex
        helper::vector<sofa::defaulttype::Vector2>& vTexCoords = *texCoords.beginEdit();
        helper::vector<MapFaceTexCoord >& vTexCoordsInFace = *texCoordsInFace.beginEdit();
        helper::vector<sofa::defaulttype::Vector3>& vNormals   = *normals.beginEdit();
        helper::vector<sofa::defaulttype::Vector3>& vVertices  = *positions.beginEdit();
        vVertices = my_positions;
        size_t vertexCount = my_positions.size();
        vTexCoords.resize(hasTexCoords ? vertexCount : 0);
        vTexCoordsInFace.resize(vertexCount);
        vNormals.resize(hasNormals ? vertexCount : 0);
        for (size_t fi=0; fi<my_faceList.size(); ++fi)
        {
            const helper::SVector<int>& nodes = my_faceList[fi];
            const helper::SVector<int>& nIndices = my_normalsList[fi];
            const helper::SVector<int>& tIndices = my_texturesList[fi];

            for (size_t i = 0; i < tIndices.size(); ++i)
            {
                unsigned int ti = (unsigned int)tIndices[i];
                if (ti < my_texCoords.size())
                    vTexCoordsInFace[nodes[i]].setTexCoord(fi, my_texCoords[ti]);
            }

            for (size_t i = 0; i < nodes.size(); ++i)
            {
                unsigned int pi = (unsigned int)nodes[i];
                unsigned int ni = (unsigned int)nIndices[i];
                unsigned int ti = (unsigned int)tIndices[i];
                if (pi >= vertexCount) continue;
                if (ti < my_texCoords.size() && (vTexCoords[pi] == sofa::defaulttype::Vector2() ||
                                                 (my_texCoords[ti]-vTexCoords[pi])*sofa::defaulttype::Vector2(-1,1) > 0))
                {
                    vTexCoords[pi] = my_texCoords[ti];
                }
                if (ni < my_normals.size())
                    vNormals[pi] += my_normals[ni];
            }
        }
        for (size_t i=0; i<vNormals.size(); ++i)
        {
            vNormals[i].normalize();
        }
    }
    else
    { // handleSeam mode : vertices are duplicated in case they have different texcoords and/or normals
        // This code was initially in VisualModelImpl::setMesh()

        // Then we can compute how many vertices are created
        int nbVOut = 0;
        bool vsplit = false;
        for (int i = 0; i < nbVIn; i++)
        {
            int s = vertTexNormMap[i].size();
            nbVOut += s;
        }

        sout << nbVIn << " input positions, " << nbVOut << " final vertices." << sendl;

        if (nbVIn != nbVOut)
            vsplit = true;

        // Then we can create the final arrays
        helper::WriteAccessor<Data<helper::vector<sofa::defaulttype::Vector3>>> vertices2 = positions;
        helper::WriteAccessor<Data<helper::vector<sofa::defaulttype::Vector3>>> vnormals = normals;
        helper::WriteAccessor<Data<helper::vector<sofa::defaulttype::Vector2>>> vtexcoords = texCoords;
        helper::WriteAccessor<Data<helper::vector<MapFaceTexCoord>>> vTexCoordsInFace = texCoordsInFace;
        helper::WriteAccessor<Data<helper::vector<int>>> vertPosIdx = d_vertPosIdx;
        helper::WriteAccessor<Data<helper::vector<int>>> vertNormIdx = d_vertNormIdx;

        vertices2.resize(nbVOut);
        vtexcoords.resize(hasTexCoords ? nbVOut : 0);
        vTexCoordsInFace.resize(nbVOut);
        vnormals.resize(hasNormals ? nbVOut : 0);
        if (vsplit)
        {
            vertPosIdx.resize(nbVOut);
            vertNormIdx.resize(hasNormals ? nbVOut : 0);
        }

        int nbNOut = 0; /// Number of different normals
        for (int i = 0, j = 0; i < nbVIn; i++)
        {
            std::map<int, int> normMap;
            for (std::map<std::pair<int, int>, int>::iterator it = vertTexNormMap[i].begin();
                 it != vertTexNormMap[i].end(); ++it)
            {
                int t = it->first.first;
                int n = it->first.second;
                if ( n >= 0 && (unsigned)n < my_normals.size())
                    vnormals[j] = my_normals[n];
                if ( t >= 0 && (unsigned)t < my_texCoords.size())
                    vtexcoords[j] = my_texCoords[t];
                
                vertices2[j] = my_positions[i];
                if (vsplit)
                {
                    vertPosIdx[j] = i;
                    if (hasNormals)
                    {
                        if (normMap.count(n))
                            vertNormIdx[j] = normMap[n];
                        else
                        {
                            vertNormIdx[j] = nbNOut;
                            normMap[n] = nbNOut++;
                        }
                    }
                }
                it->second = j++;
            }
        }

        if( vsplit && nbNOut == nbVOut )
            vertNormIdx.resize(0);

        // Then we create the triangles and quads
        
        for (size_t fi=0; fi<my_faceList.size(); ++fi)
        {
            const helper::SVector<int>& verts = my_faceList[fi];
            const helper::SVector<int>& nIndices = my_normalsList[fi];
            const helper::SVector<int>& tIndices = my_texturesList[fi];
            std::vector<int> nodes;
            nodes.resize(verts.size());
            for (size_t i = 0; i < verts.size(); ++i)
            {
                unsigned int pi = verts[i];
                unsigned int ni = nIndices[i];
                unsigned int ti = tIndices[i];
                nodes[i] = vertTexNormMap[pi][std::make_pair(ti, ni)];
                if ((unsigned)nodes[i] >= (unsigned)nbVOut)
                {
                    serr << this->getName()<<" index "<<nodes[i]<<" out of range"<<sendl;
                    nodes[i] = 0;
                }
                if (ti < my_texCoords.size())
                    vTexCoordsInFace[nodes[i]].setTexCoord(fi, my_texCoords[ti]);
            }

            if (nodes.size() == 2) // Edge
            {
                addEdge(&my_edges, Edge(nodes[0], nodes[1]));
            }
            else if (nodes.size()==4 && !this->triangulate.getValue()) // Quad
            {
                addQuad(&my_quads, Quad(nodes[0], nodes[1], nodes[2], nodes[3]));
            }
            else // Triangulate
            {
                for (size_t j=2; j<nodes.size(); j++)
                    addTriangle(&my_triangles, Triangle(nodes[0], nodes[j-1], nodes[j]));
            }
        }
        for (size_t i=0; i<vnormals.size(); ++i)
        {
            vnormals[i].normalize();
        }
    }


    // create subset lists
    std::map< std::string, helper::vector<unsigned int> > materialFaces[NBFACETYPE];
    for (int ft = 0; ft < NBFACETYPE; ++ft)
    {
        for (size_t gi=0; gi<my_faceGroups[ft].size(); ++gi)
        {
            PrimitiveGroup g = my_faceGroups[ft][gi];
            helper::vector<unsigned int>& out = materialFaces[ft][g.materialName];
            for (int f=g.p0; f<g.p0+g.nbp; ++f)
                out.push_back(f);
        }
    }
    if (computeMaterialFaces.getValue())
    {
        for (int ft = 0; ft < NBFACETYPE; ++ft)
        {
            std::string fname;
            switch (faceType)
            {
            case MeshObjLoader::EDGE:     fname = "edge"; break;
            case MeshObjLoader::TRIANGLE: fname = "triangle"; break;
            case MeshObjLoader::QUAD:     fname = "quad"; break;
            default: break;
            }
            for (std::map< std::string, helper::vector<unsigned int> >::const_iterator it = materialFaces[ft].begin(), itend = materialFaces[ft].end(); it != itend; ++it)
            {
                std::string materialName = it->first;
                const helper::vector<unsigned>& faces = it->second;
                if (faces.empty()) continue;
                std::ostringstream oname;
                oname << "material_" << materialName << "_" << fname << "Indices";
                Data< helper::vector<unsigned int> >* dOut = new Data< helper::vector<unsigned int> >("list of face indices corresponding to a given material");
                dOut->setName(oname.str());

                this->addData(dOut);
                dOut->setGroup("Materials");
                dOut->setValue(faces);
                subsets_indices.push_back(dOut);
            }
        }
    }

    // compute list of point which are along an uv discontinuity
    helper::ReadAccessor< Data < sofa::helper::vector< MapFaceTexCoord > > > vTexCoordsInFace = texCoordsInFace;
    helper::WriteAccessor< Data < sofa::helper::vector< unsigned int > > > ptsOnBorder = d_pointsOnBorder;
    for (unsigned int i = 0; i < (unsigned int)nbVIn; i++)
    {
        if (vTexCoordsInFace[i].isDuplicated())
        {
            ptsOnBorder.push_back(i);
        }
    }

    // compute list of triangles which are along an uv discontinuity
    helper::WriteAccessor<Data<helper::vector<unsigned int>>> trianglesOnBorder = d_trianglesOnBorder;
    trianglesOnBorder.clear();
    if (!ptsOnBorder.empty())
    {
        for (std::size_t tid = 0; tid < my_triangles.size(); ++tid)
        {
            bool onBorder = false;
            for (std::size_t i = 0; i < 3; ++i)
            {
                if (std::find(ptsOnBorder.begin(), ptsOnBorder.end(), my_triangles[tid][i]) != ptsOnBorder.end())
                {
                    onBorder = true;
                }
            }
            if (onBorder)
            {
                trianglesOnBorder.push_back(tid);
            }
        }
    }

    helper::WriteAccessor<Data<helper::vector<unsigned int>>> trianglesIndices = d_trianglesIndices;
    trianglesIndices.resize(my_triangles.size());
    std::iota(trianglesIndices.begin(), trianglesIndices.end(), 0);

    edgesGroups.endEdit();
    trianglesGroups.endEdit();
    quadsGroups.endEdit();
    positions.endEdit();
    edges.endEdit();
    triangles.endEdit();
    quads.endEdit();
    normalsList.endEdit();
    normalsIndexList.endEdit();
    materials.endEdit();
    texIndexList.endEdit();
    texCoordsList.endEdit();
    texCoords.endEdit();
    faceList.endEdit();
    //vertices.endEdit();
    normals.endEdit();
    return true;
}



// -----------------------------------------------------
// readMTL: read a wavefront material library file
//
//    model - properly initialized GLMmodel structure
//    name  - name of the material library
// -----------------------------------------------------
bool MeshObjLoader::readMTL(const char* filename, helper::vector <Material>& materials)
{
    sout << "MeshObjLoader::readMTL" << sendl;

    FILE* file;
    char buf[128]; // Note: in the strings below, 127 is sizeof(buf)-1
    const char *single_string_format = "%127s"; // Better than "%s" for scanf
    const char *double_string_format = "%127s %127s"; // Better than "%s %s"

    file = fopen(filename, "r");
    Material *mat = NULL;
    if (!file);//serr << "readMTL() failed: can't open material file " << filename << sendl;
    else
    {
        /* now, read in the data */
        while (fscanf(file, single_string_format, buf) != EOF)
        {

            switch (buf[0])
            {
            case '#':
                /* comment */
                /* eat up rest of line */
                if ( fgets(buf, sizeof(buf), file) == NULL)
                {
                    if (feof (file) )
                        serr << "Error: MeshObjLoader: fgets function has encounter end of file. case #." << sendl;
                    else
                        serr << "Error: MeshObjLoader: fgets function has encounter an error. case #." << sendl;
                }
                break;
            case 'n':
                /* newmtl */
                if (mat != NULL)
                {
                    materials.push_back(*mat);
                    delete mat;
                    mat = NULL;
                }
                mat = new Material();
                if ( fgets(buf, sizeof(buf), file) == NULL)
                {
                    if (feof (file) )
                        serr << "Error: MeshObjLoader: fgets function has encounter end of file. case n." << sendl;
                    else
                        serr << "Error: MeshObjLoader: fgets function has encounter an error. case n." << sendl;
                }
                sscanf(buf, double_string_format, buf, buf);
                mat->name = buf;
                break;
            case 'N':
                switch (buf[1])
                {
                case 'i':
                {
                    float optical_density;
                    if ( fscanf(file, "%f", &optical_density) == EOF )
                        serr << "Error: MeshObjLoader: fscanf function has encounter an error. case N i." << sendl;
                    break;
                }
                case 's':
                    if (fscanf(file, "%f", &mat->shininess) == EOF )
                        serr << "Error: MeshObjLoader: fscanf function has encounter an error. case N s." << sendl;
                    // wavefront shininess is from [0, 1000], so scale for OpenGL
                    //mat->shininess /= 1000.0;
                    //mat->shininess *= 128.0;
                    mat->useShininess = true;
                    break;
                default:
                    /* eat up rest of line */
                    if ( fgets(buf, sizeof(buf), file) == NULL)
                    {
                        if (feof (file) )
                            serr << "Error: MeshObjLoader: fgets function has encounter end of file. case N." << sendl;
                        else
                            serr << "Error: MeshObjLoader: fgets function has encounter an error. case N." << sendl;
                    }
                    break;
                }
                break;
            case 'K':
                switch (buf[1])
                {
                case 'd':
                    if ( fscanf(file, "%f %f %f", &mat->diffuse[0], &mat->diffuse[1], &mat->diffuse[2]) == EOF)
                        serr << "Error: MeshObjLoader: fscanf function has encounter an error. case K d." << sendl;
                    mat->useDiffuse = true;
                    /*sout << mat->name << " diffuse = "<<mat->diffuse[0]<<' '<<mat->diffuse[1]<<'*/ /*'<<mat->diffuse[2]<<sendl;*/
                    break;
                case 's':
                    if ( fscanf(file, "%f %f %f", &mat->specular[0], &mat->specular[1], &mat->specular[2]) == EOF)
                        serr << "Error: MeshObjLoader: fscanf function has encounter an error. case K s." << sendl;
                    mat->useSpecular = true;
                    /*sout << mat->name << " specular = "<<mat->specular[0]<<' '<<mat->specular[1]<<'*/ /*'<<mat->specular[2]<<sendl;*/
                    break;
                case 'a':
                    if ( fscanf(file, "%f %f %f", &mat->ambient[0], &mat->ambient[1], &mat->ambient[2]) == EOF)
                        serr << "Error: MeshObjLoader: fscanf function has encounter an error. case K a." << sendl;
                    mat->useAmbient = true;
                    /*sout << mat->name << " ambient = "<<mat->ambient[0]<<' '<<mat->ambient[1]<<'*/ /*'<<mat->ambient[2]<<sendl;*/
                    break;
                default:
                    /* eat up rest of line */
                    if ( fgets(buf, sizeof(buf), file) == NULL)
                    {
                        if (feof (file) )
                            serr << "Error: MeshObjLoader: fgets function has encounter end of file. case K." << sendl;
                        else
                            serr << "Error: MeshObjLoader: fgets function has encounter an error. case K." << sendl;
                    }
                    break;
                }
                break;
            case 'd':
            case 'T':
                // transparency value
                if ( fscanf(file, "%f", &mat->diffuse[3]) == EOF)
                    serr << "Error: MeshObjLoader: fscanf function has encounter an error. case T i." << sendl;
                break;
            default:
                /* eat up rest of line */
                if ( fgets(buf, sizeof(buf), file) == NULL)
                {
                    if (feof (file) )
                        serr << "Error: MeshObjLoader: fgets function has encounter end of file. case default." << sendl;
                    else
                        serr << "Error: MeshObjLoader: fgets function has encounter an error. case default." << sendl;
                }
                break;
            }

        }
        fclose(file);
    }
    if (mat != NULL)
    {
        materials.push_back(*mat);
        delete mat;
        mat = NULL;
    }

    return true;
}



} // namespace loader

} // namespace component

} // namespace sofa

