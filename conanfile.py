from conan import ConanFile
from conan.tools.cmake import cmake_layout


class CompressorRecipe(ConanFile):
    settings = ("os", "compiler", "build_type", "arch")
    generators = "CMakeToolchain", "CMakeDeps"
    options = {"shared": [True, False], "fPIC": [True, False]}
    default_options = {"shared": False, "fPIC": True}

    def requirements(self):
        self.requires("boost/1.84.0")
        self.requires("pybind11/2.12.0")
        self.requires("nlohmann_json/3.11.3")

    def build_requirements(self):
        self.tool_requires("cmake/3.29.2")
        self.test_requires("gtest/1.14.0")
        
    def layout(self):
        cmake_layout(self)
        
