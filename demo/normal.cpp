//
// Created by Vicente González on 11-12-24.
//
#include <string>
#include "gpolylla/io.h"
#include "gpolylla/utils.h"
#include "polyscope/polyscope.h"
#include "polyscope/volume_mesh.h"

int main() {
    using std::string;

    string inputMeshFile = string(DATA_DIR) + string("1000points");

    gpolylla::TetgenReader reader;
    reader.setBasename(inputMeshFile);
    gpolylla::TetraMesh mesh = reader.build();


    polyscope::init();

    auto *psMesh = polyscope::registerTetMesh("my mesh", mesh.vertices, mesh.tetras);


    for (int ti = 0; ti < mesh.tetras.size(); ++ti) {
        std::vector<Eigen::Vector3d> normals(mesh.vertices.size());
        const auto &t = mesh.tetras[ti];
        for (int i = 0; i < t.faces.size(); i++) {
            int fi = t.faces[i];
            const auto &f = mesh.faces[fi];
            auto n = -normal(mesh.vertices[f[0]], mesh.vertices[f[1]], mesh.vertices[f[2]]);
            normals[t.vertices[i]] = n;
        }
        psMesh->addVertexVectorQuantity(string("normals of ") + std::to_string(ti), normals);
    }


    polyscope::show();
}