#include <gpolylla/io.h>
#include <gtest/gtest.h>

#include "gpolylla/mesh.h"
#include "gpolylla/algo.h"

namespace gpolylla {
    class VisFTest : public ::testing::Test {
    protected:
        VisFWriter writer;
    };

    TEST_F(VisFTest, WriteMinimalTest) {
        PolyMesh m;

        m.vertices = {
                Vertex(0.0, 0.0, 0.0), Vertex(0.0, 0.0, 1.0), Vertex(1.0, 0.0, 1.0),
                Vertex(1.0, 0.0, 0.0), Vertex(0.0, 1.0, 0.0), Vertex(0.0, 1.0, 1.0),
                Vertex(1.0, 1.0, 1.0), Vertex(1.0, 1.0, 0.0),
        };

        m.faces = {
                Face(0, 1, 2), Face(2, 3, 0), Face(2, 3, 7), Face(7, 6, 2),
                Face(2, 1, 5), Face(5, 6, 2), Face(5, 6, 7), Face(7, 4, 5),
                Face(0, 4, 7), Face(7, 3, 0), Face(0, 1, 5), Face(5, 4, 0),
        };

        m.cells = {Polyhedron({0, 1, 2, 3, 4, 5, 6, 7})};

        writer.setBasename(DATA_DIR "test");
        writer.write(m);
    }

    TEST_F(VisFTest, TransformFormatTest) {
        TetgenReader reader;
        reader.setBasename(DATA_DIR "socket");
        auto mesh = reader.build();
        CavityAlgorithm algo((CavityAlgorithm::Criteria()));
        PolyMesh pmesh = algo(mesh);
        writer.setBasename(DATA_DIR "socket");
        writer.write(pmesh);
    }
}  // namespace gpolylla
