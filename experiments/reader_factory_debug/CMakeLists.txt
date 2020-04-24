# cmake configuration file for compilation of LehrFEM++ test code for reading mesh

set(reader_factory_debug test_main.cc)

add_executable(experiments.reader_factory_debug.test ${reader_factory_debug})

target_link_libraries(experiments.reader_factory_debug.test
					Boost::filesystem
					Boost::program_options
					Boost::system
					Eigen3::Eigen
					lf.assemble
					lf.base
					lf.geometry
					lf.io
					lf.mesh
					lf.mesh.hybrid2d
					lf.mesh.test_utils
					lf.mesh.utils
					lf.refinement
					lf.uscalfe
					)

target_compile_features(experiments.reader_factory_debug.test PUBLIC cxx_std_17)



