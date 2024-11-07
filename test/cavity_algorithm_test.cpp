#include <gpolylla/algo.h>
#include <gpolylla/io.h>
#include <gtest/gtest.h>
#include <unistd.h>

namespace gpolylla {
class CavityAlgorithmTest: public ::testing::Test {
protected:
  CavityAlgorithmTest(): algo(), reader() {}
  CavityAlgorithm algo;
  TetgenReader reader;

  void testAll(const std::function<bool(TetraMesh, PolyMesh)>& testFun) {
    std::string data[] = {"minimal", "socket.1", "1000points.1", "3D_100.1"};
    for (const auto& d : data) {
      std::string path(DATA_DIR);
      reader.setBasename(path.append(d));
      TetraMesh mesh = reader.build();
      PolyMesh pmesh = algo(mesh);
      ASSERT_TRUE(testFun(mesh, pmesh)) << "Failed for " << d << "\n";
    }
  }
};


//TEST_F(CavityAlgorithmTest, ContainsAllVertices) {
//  testAll([&](TetraMesh mesh, PolyMesh pmesh) -> bool {
//    std::vector<bool> exists(mesh.vertices.size(), false);
//    for (const auto& poly: pmesh.cells) {
//      for (int vi : poly.vertices) exists[vi] = true;
//    }
//
//    bool flag = true;
//    for (int i = 0; i < exists.size(); i++) {
//      // ASSERT_TRUE(exists[i]) << "Vertex " <<  i << " doesn't exists" << "\n";
//      flag = flag && exists[i];
//    }
//    return flag;
//  });
//}

TEST_F(CavityAlgorithmTest, Foo) {
  std::string path(DATA_DIR);
  reader.setBasename(path.append("3D_100.1"));
  auto mesh = reader.build();
  auto pMes = algo(mesh);

}

}
