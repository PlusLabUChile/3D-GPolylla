#include <gpolylla/polylla.h>

#include <fstream>
#include <unordered_set>

#include "logger.h"

using namespace Polylla;
using namespace std;

class FileNotFoundError : public std::runtime_error {
  using std::runtime_error::runtime_error;
};

vector<Vertex> readNode(const string& file) {
  vector<Vertex> verts;

  ifstream nodeStream(file);
  if (!nodeStream.is_open()) {
    throw FileNotFoundError("Cannot open node file: " + file);
  }

  string line;
  getline(nodeStream, line);
  istringstream headerStream(line);
  int numVertices;
  headerStream >> numVertices;
  verts.reserve(numVertices);

  while (getline(nodeStream, line)) {
    istringstream lineStream(line);
    string token;
    lineStream >> token;
    if (token == "#") continue;  // Skip comments
    float x, y, z;
    lineStream >> x >> y >> z;
    verts.emplace_back(x, y, z);
  }
  nodeStream.close();
  return verts;
}

vector<Tetrahedron> readEle(const string& file) {
  vector<Tetrahedron> cells;
  ifstream eleStream(file);
  if (!eleStream.is_open()) {
    throw FileNotFoundError("Cannot open element file: " + file);
  }

  string line;
  getline(eleStream, line);
  istringstream headerStream(line);
  int numCells;
  headerStream >> numCells;
  cells.reserve(numCells);
  while (getline(eleStream, line)) {
    istringstream lineStream(line);
    string token;
    lineStream >> token;
    if (token == "#") continue;  // Skip comments
    int v0, v1, v2, v3;
    lineStream >> v0 >> v1 >> v2 >> v3;
    cells.emplace_back(v0, v1, v2, v3);
  }
  eleStream.close();
  return cells;
}

vector<Face> readFace(const string& file) {
  vector<Face> faces;
  ifstream faceStream(file);
  if (!faceStream.is_open()) {
    throw FileNotFoundError("Cannot open face file: " + file);
  }

  string line;
  getline(faceStream, line);
  istringstream headerStream(line);
  int numFaces;
  headerStream >> numFaces;
  faces.reserve(numFaces);

  while (getline(faceStream, line)) {
    istringstream lineStream(line);
    string token;
    lineStream >> token;
    if (token == "#") continue;  // Skip comments
    Face f;
    lineStream >> f.vertices[0] >> f.vertices[1] >> f.vertices[2];
    faces.push_back(f);
  }
  faceStream.close();
  return faces;
}

Mesh TetgenReader::readMesh() {
  Mesh m;
  m.vertices = readNode(this->meshName + ".node");
  m.cells = readEle(this->meshName + ".ele");

  try {
    m.faces = readFace(this->meshName + ".face");
  } catch (const FileNotFoundError& e) {
    log("No face file found, generating faces from tetrahedra", INFO);
  }

  return m;
}