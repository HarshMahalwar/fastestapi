from conan import ConanFile
from conan.tools.cmake import CMakeDeps, CMakeToolchain, cmake_layout
from conan.tools.files import copy, load
from conan.tools.scm import Git
import os


class FastestAPIConan(ConanFile):
    name = "fastestapi"
    description = "A lightweight C++ REST framework inspired by Python's FastAPI"
    license = "MIT"
    url = "https://github.com/HarshMahalwar/fastestapi"
    homepage = "https://github.com/HarshMahalwar/fastestapi"
    topics = ("rest", "api", "http", "framework", "fastapi", "cpp17")

    package_type = "header-library"
    settings = "os", "compiler", "build_type", "arch"
    options = {
        "with_redis": [True, False],
    }
    default_options = {
        "with_redis": True,
    }

    exports_sources = "CMakeLists.txt", "include/*"
    no_copy_source = True

    def set_version(self):
        if not self.version:
            git = Git(self)
            tag = git.run("describe --tags --abbrev=0").strip()
            self.version = tag.lstrip("v")

    def requirements(self):
        self.requires("cpp-httplib/0.18.3") 
        self.requires("nlohmann_json/3.11.3")
        self.requires("sqlite3/3.45.0")      
        if self.options.with_redis:
            self.requires("poco/1.13.3")

    def layout(self):
        cmake_layout(self)

    def generate(self):
        deps = CMakeDeps(self)
        deps.generate()
        tc = CMakeToolchain(self)
        tc.generate()

    def package(self):
        copy(self, "*.hpp", src=os.path.join(self.source_folder, "include"),
             dst=os.path.join(self.package_folder, "include"))

    def package_info(self):
        self.cpp_info.bindirs = []
        self.cpp_info.libdirs = []
        self.cpp_info.includedirs = ["include"]
        self.cpp_info.set_property("cmake_file_name", "fastestapi")
        self.cpp_info.set_property("cmake_target_name", "fastestapi::fastestapi")

        self.cpp_info.requires = [
            "cpp-httplib::cpp-httplib",
            "nlohmann_json::nlohmann_json",
            "sqlite3::sqlite3",
        ]
        if self.options.with_redis:
            self.cpp_info.requires.extend([
                "poco::poco_redis"
            ])
