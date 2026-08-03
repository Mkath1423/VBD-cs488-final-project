
#include <vector>
#include <filesystem>

struct TriangleMeshFileData {    
    std::vector<float> positions;
    std::vector<float> normals;
    std::vector<float> uvs;
    std::vector<unsigned int> faces;
};

bool load_obj_file(const std::filesystem::path &objFile, TriangleMeshFileData &mesh);

struct TetGenFileData {    
    std::vector<float> positions;
    std::vector<unsigned int> elements;
    std::vector<unsigned int> faces;
    int element_size;
};

bool load_tet_gen_file(
    const std::filesystem::path &nodePath,
    const std::filesystem::path &elementPath,
    const std::filesystem::path &facePath, 
    TetGenFileData &mesh);
