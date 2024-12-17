#include <gpolylla/io.h>

#include <fstream>
#include <iostream>
#include <vector>

namespace gpolylla {
    using std::cerr;
    using std::endl;
    using std::ofstream;
    using std::string;
    using std::vector;

    void VisFWriter::setBasename(const string &basename) {
        this->basename = basename;
    }

    void VisFWriter::write(const PolyMesh &mesh) {
        ofstream file(basename + ".visf");
        if (!file.is_open()) {
            cerr << "Unable to create file: " << basename << ".visf" << endl;
        }

        // primer valor -> formato del archivo
        // 0 -> big endian
        // 1 -> little endian
        // 2 -> ascii

        // segundo valor -> tipo de malla
        // 0 -> nube de puntos
        // 1 -> malla de poligonos
        // 2 -> malla de poliedros
        file << 2 << " " << 2 << endl;

        // cantidad de puntos y puntos
        file << mesh.vertices.size() << endl;
        for (const auto &v: mesh.vertices) {
            file << v.x << " " << v.y << " " << v.z << endl;
        }

        // cantidad de poligonos y poligonos
        file << mesh.faces.size() << endl;
        for (const auto &f: mesh.faces) {
            file << f.vertices.size();
            for (int vi: f.vertices) {
                file << " " << vi;
            }
            file << endl;
        }

        // relacion de vecindad entre poligonos
        file << 0 << endl;
        // numero de poliedros y poliedros (basado en poligonos)
        file << mesh.cells.size() << endl;
        // polyhedra are saved based on faces
        for (const auto &p: mesh.cells) {
            file << p.faces.size();
            for (int fi: p.faces) {
                file << " " << fi;
            }
            file << endl;
        }
    }

}  // namespace gpolylla
