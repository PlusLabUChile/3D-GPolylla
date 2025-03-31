#include <gpolylla/algo.h>
#include <gpolylla/io.h>

int main(int argc, char const* argv[]) {
  if (argc != 4) {
    std::cerr << "Usage: " << argv[0] << " <algo> <input_file> <output_file>"
              << std::endl;
    return 1;
  }

  gpolylla::TetgenReader reader;
  reader.setBasename(argv[2]);
  gpolylla::TetraMesh mesh = reader.build();

  gpolylla::PolyMesh ans;
  if (std::string(argv[1]) == "face") {
    gpolylla::FaceAlgorithm algo;
    ans = algo(mesh);
  } else if (std::string(argv[1]) == "cavity") {
    gpolylla::CavityAlgorithm algo;
    ans = algo(mesh);
  } else {
    std::cerr << "Invalid algorithm: " << argv[1] << std::endl;
    return 1;
  }

  gpolylla::VisFWriter writer;
  writer.setBasename(argv[3]);
  writer.write(ans);

  return 0;
}
