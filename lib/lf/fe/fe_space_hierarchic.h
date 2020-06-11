/**
 * @file
 * @brief Lagrangian finite elements of arbitrary polynomials degree
 * @author Tobias Rohner
 * @date   May 20202
 * @copyright MIT License
 */

#ifndef LF_USCALFE_FE_SPACE_HP_H_
#define LF_USCALFE_FE_SPACE_HP_H_

#include <lf/mesh/utils/all_codim_mesh_data_set.h>
#include "hierarchic_fe.h"
#include "scalar_fe_space.h"

namespace lf::fe {

/**
 * @brief Lagrangian Finite Element Space of arbitrary degree
 */
template <typename SCALAR>
class FeSpaceHierarchic : public ScalarFESpace<SCALAR> {
 public:
  using Scalar = SCALAR;

  FeSpaceHierarchic() = delete;
  FeSpaceHierarchic(const FeSpaceHierarchic &) = delete;
  FeSpaceHierarchic(FeSpaceHierarchic &&) noexcept = default;
  FeSpaceHierarchic &operator=(const FeSpaceHierarchic &) = delete;
  FeSpaceHierarchic &operator=(FeSpaceHierarchic &&) noexcept = default;

  /**
   * @brief Constructor: Sets up the dof handler
   * @param mesh_p A shared pointer to the underlying mesh (immutable)
   * @param N The polynomial degree of the Finite Element Space
   */
  explicit FeSpaceHierarchic(
      const std::shared_ptr<const lf::mesh::Mesh> &mesh_p, unsigned N)
      : ScalarFESpace<SCALAR>(mesh_p), ref_el_(mesh_p, nullptr), degree_(N) {
    init();
  }

  using ScalarFESpace<SCALAR>::Mesh;

  /** @brief access to associated local-to-global map
   * @return a reference to the lf::assemble::DofHandler object (immutable)
   */
  [[nodiscard]] const lf::assemble::DofHandler &LocGlobMap() const override {
    LF_VERIFY_MSG(Mesh() != nullptr, "No valid FE space object: no mesh");
    LF_VERIFY_MSG(dofh_p_ != nullptr,
                  "No valid FE space object: no dof handler");
    return *dofh_p_;
  }

  /** @brief access to shape function layout for cells
   * @copydoc SclarFESpace::ShapeFunctionLayout(const lf::mesh::Entity&)
   */
  [[nodiscard]] std::shared_ptr<const ScalarReferenceFiniteElement<SCALAR>>
  ShapeFunctionLayout(const lf::mesh::Entity &entity) const override {
    return ref_el_(entity);
  }

  /** @brief number of _interior_ shape functions associated to entities of
   * various types
   */
  [[nodiscard]] size_type NumRefShapeFunctions(
      const lf::mesh::Entity &entity) const override {
    return ShapeFunctionLayout(entity)->NumRefShapeFunctions();
  }

  ~FeSpaceHierarchic() override = default;

 private:
  // R.H.: Should this really be shared pointers ?
  lf::mesh::utils::AllCodimMeshDataSet<
      std::shared_ptr<const lf::fe::ScalarReferenceFiniteElement<SCALAR>>>
      ref_el_;
  std::unique_ptr<lf::assemble::UniformFEDofHandler> dofh_p_;
  unsigned degree_;

  // R.H. Detailed comment needed for this function
  void init() {
    // Initialize all shape function layouts for nodes
    size_type num_rsf_node = 1;
    for (auto entity : Mesh()->Entities(2)) {
      ref_el_(*entity) = std::make_shared<FeHierarchicPoint<SCALAR>>(degree_);
    }
    // Initialize all shape function layouts for the edges
    size_type num_rsf_edge = 0;
    for (auto entity : Mesh()->Entities(1)) {
      ref_el_(*entity) = std::make_shared<FeHierarchicSegment<SCALAR>>(
          degree_, entity->RelativeOrientations());
      num_rsf_edge = ref_el_(*entity)->NumRefShapeFunctions(0);
    }
    // Initialize all shape function layouts for the cells
    size_type num_rsf_tria = 0;
    size_type num_rsf_quad = 0;
    for (auto entity : Mesh()->Entities(0)) {
      switch (entity->RefEl()) {
        case lf::base::RefEl::kTria():
          ref_el_(*entity) = std::make_shared<FeHierarchicTria<SCALAR>>(
              degree_, entity->RelativeOrientations());
          num_rsf_tria = ref_el_(*entity)->NumRefShapeFunctions(0);
          break;
        case lf::base::RefEl::kQuad():
          ref_el_(*entity) = std::make_shared<FeHierarchicQuad<SCALAR>>(
              degree_, entity->RelativeOrientations());
          num_rsf_quad = ref_el_(*entity)->NumRefShapeFunctions(0);
          break;
        default:
          LF_VERIFY_MSG(false, "Illegal entity type");
      }
    }
    // Initialize the dofhandler
    lf::assemble::UniformFEDofHandler::dof_map_t rsf_layout{
        {lf::base::RefEl::kPoint(), num_rsf_node},
        {lf::base::RefEl::kSegment(), num_rsf_edge},
        {lf::base::RefEl::kTria(), num_rsf_tria},
        {lf::base::RefEl::kQuad(), num_rsf_quad}};
    dofh_p_ =
        std::make_unique<lf::assemble::UniformFEDofHandler>(Mesh(), rsf_layout);
  }
};

}  // end namespace lf::fe

#endif  // LF_USCALFE_FE_SPACE_HP_H_