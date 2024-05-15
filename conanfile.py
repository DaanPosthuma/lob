from conan import ConanFile
from conan.tools.cmake import cmake_layout


class CompressorRecipe(ConanFile):
    settings = ("os", "compiler", "build_type", "arch")
    generators = "CMakeToolchain", "CMakeDeps"

    def requirements(self):
        #self.requires("rapidjson/cci.20211112")
        self.requires("boost/1.84.0")
        #self.requires("openssl/1.1.1q")
        pass

    def build_requirements(self):
        self.tool_requires("cmake/3.29.2")
        self.test_requires("gtest/1.14.0")
        
    def layout(self):
        cmake_layout(self)
        