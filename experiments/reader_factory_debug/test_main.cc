#include <cstdlib>
#include <iostream>
#include <string>

#include <boost/filesystem.hpp>

#include <lf/assemble/assemble.h>
#include <lf/io/io.h>
#include <lf/mesh/hybrid2d/hybrid2d.h>
#include <lf/mesh/utils/utils.h>
#include <lf/uscalfe/uscalfe.h>

int main(int /*argc*/, char ** /*argv*/){

  /* Obtain mesh */
  auto mesh_factory = std::make_unique<lf::mesh::hybrid2d::MeshFactory>(2);
  boost::filesystem::path here = __FILE__;
  auto mesh_file = (here.parent_path() /"/meshes/earth_refined.msh").string();
  const lf::io::GmshReader reader(std::move(mesh_factory), mesh_file);
  std::shared_ptr<const lf::mesh::Mesh> mesh_p = reader.mesh();
  /* Finite Element Space */
  auto fe_space = std::make_shared<lf::uscalfe::FeSpaceLagrangeO1<double>>(mesh_p);
  /* Dofhandler */
  const lf::assemble::DofHandler &dofh{fe_space->LocGlobMap()};
  const lf::uscalfe::size_type N_dofs(dofh.NumDofs());

  std::cout << "Ndofs" << N_dofs << std::endl;

  return 0;

}
