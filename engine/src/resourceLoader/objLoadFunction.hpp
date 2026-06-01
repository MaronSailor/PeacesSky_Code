#include <tiny_obj_loader.h>

#include "../renderer/components/vertex.hpp"
#include "meshData.hpp"

#include <iostream>

struct TriAngle
{
    unsigned int cornerA;
    unsigned int cornerB;
    unsigned int cornerC;
};

struct TripleKey
{
    int vetexIndex, textureIndex, normalIndex;
    bool operator==(const TripleKey& o) const { return vetexIndex == o.vetexIndex && textureIndex == o.textureIndex && normalIndex == o.normalIndex; }
};

struct TripleKeyHash {
    size_t operator()(TripleKey const& tripleKey) const noexcept
    {
        uint64_t a = (uint32_t)tripleKey.vetexIndex;
        uint64_t b = (uint32_t)tripleKey.textureIndex;
        uint64_t c = (uint32_t)tripleKey.normalIndex;
        return (size_t)((a << 42) ^ (b << 21) ^ c);
    }
};

void loadToBuffers(const tinyobj::attrib_t& attrib, const std::vector<tinyobj::shape_t>& shapes,
    std::vector<VertexData>& vertexBuffer, std::vector<unsigned int>& IndexBuffer)
{
    std::vector<TriAngle> triangles;
    std::unordered_map<TripleKey, unsigned int, TripleKeyHash> seen;

    const int vertexCount = (int)(attrib.vertices.size() / 3);
    const int texcoordCount = (int)(attrib.texcoords.size() / 2);
    const int normalCount = (int)(attrib.normals.size() / 3);

    for (const auto& shape : shapes)
    {
        const auto& shapeIndices = shape.mesh.indices;
        for (size_t i = 0; i + 2 < shapeIndices.size(); i += 3)
        {
            TriAngle triangle{};
            for (int corner = 0; corner < 3; ++corner)
            {
                const tinyobj::index_t& idx = shapeIndices[i + corner];

                int vertexIndex = idx.vertex_index >= 0 ? idx.vertex_index : vertexCount + idx.vertex_index;
                int textureIndex = idx.texcoord_index >= 0 ? idx.texcoord_index : -1;
                int normalIndex = idx.normal_index >= 0 ? idx.normal_index : -1;

                if (vertexIndex < 0 || vertexIndex >= vertexCount) vertexIndex = 0;
                if (textureIndex < -1 || textureIndex >= texcoordCount) textureIndex = -1;
                if (normalIndex < -1 || normalIndex >= normalCount) normalIndex = -1;

                TripleKey key{ vertexIndex, textureIndex, normalIndex };

                auto iterator = seen.find(key);
                if (iterator != seen.end())
                {
                    unsigned int existing = iterator->second;
                    if (corner == 0) triangle.cornerA = existing;
                    else if (corner == 1) triangle.cornerB = existing;
                    else triangle.cornerC = existing;
                }
                else
                {
                    VertexData vertex{};
                    vertex.position = Vec3{
                        attrib.vertices[3 * vertexIndex + 0],
                        attrib.vertices[3 * vertexIndex + 1],
                        attrib.vertices[3 * vertexIndex + 2]
                    };

                    if (textureIndex >= 0)
                    {
                        vertex.texCoords = Vec2{
                            attrib.texcoords[2 * textureIndex + 0],
                            attrib.texcoords[2 * textureIndex + 1]
                        };
                    }
                    else
                    {
                        vertex.texCoords = Vec2{ 0.0f, 0.0f };
                    }

                    if (normalIndex >= 0)
                    {
                        vertex.normals = Vec3{
                            attrib.normals[3 * normalIndex + 0],
                            attrib.normals[3 * normalIndex + 1],
                            attrib.normals[3 * normalIndex + 2]
                        };
                    }
                    else
                    {
                        vertex.normals = Vec3{ 0.0f, 0.0f, 0.0f }; // or compute later
                    }

                    unsigned int newIndex = (unsigned int)vertexBuffer.size();
                    vertexBuffer.push_back(vertex);
                    seen.emplace(key, newIndex);

                    if (corner == 0) triangle.cornerA = newIndex;
                    else if (corner == 1) triangle.cornerB = newIndex;
                    else triangle.cornerC = newIndex;
                }
            }
            triangles.push_back(triangle);
        }
    }

    IndexBuffer.clear();
    IndexBuffer.reserve(triangles.size() * 3);
    for (const auto& t : triangles)
    {
        IndexBuffer.push_back(t.cornerA);
        IndexBuffer.push_back(t.cornerB);
        IndexBuffer.push_back(t.cornerC);
    }
}

bool loadObjToMeshData(MeshData& meshData, const char* filename, const char* basepath = NULL,
    bool triangulate = true)
{
    tinyobj::attrib_t attrib;
    std::vector<tinyobj::shape_t> shapes;
    std::vector<tinyobj::material_t> materials;
    std::string warn;
    std::string err;

    bool isLoaded = tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, filename,
        basepath, triangulate);

    if (!isLoaded)
    {
        std::cout << "cant load obj file\n";
        return false;
    }

    loadToBuffers(attrib, shapes, meshData.vertexBufferData, meshData.indexBufferData);
    return true;
}