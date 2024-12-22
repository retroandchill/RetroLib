import os

from conan.tools.build import check_min_cppstd
from conan.tools.cmake import CMakeDeps, cmake_layout
from conan.tools.files import copy
from conan.tools.layout import basic_layout
from conans.model.conan_file import ConanFile


class RetroLibConan(ConanFile):
    name = "retrolib"
    license = "MIT"
    url = "https://github.com/retro-cpp/retrolib"
    package_type = "header-library"
    description = "An extension to the standard library for C++20"
    topics = ("range", "range-library", "utility", "iterator", "header-only")
    settings = "os", "compiler", "build_type", "arch"
    options = {
        "with_tests": [True, False],
    }
    default_options = {
        "with_tests": False
    }
    exports_sources = "RetroLib/*", "CMakeLists.txt", "cmake/*"

    @property
    def min_cpp_std(self):
        return "20"

    def layout(self):
        if self.options.with_tests:
            cmake_layout(self)
        else:
            basic_layout(self, src_folder="RetroLib")

    def build_requirements(self):
        if self.options.with_tests:
            self.test_requires("catch2/3.7.1")

    def generate(self):
        if self.options.with_tests:
            deps = CMakeDeps(self)
            deps.generate()

    def package_id(self):
        self.info.clear()

    def validate(self):
        if self.settings.compiler.get_safe("cppstd"):
            check_min_cppstd(self, self.min_cpp_std)

    def package(self):
        print(self.source_folder)
        copy(self, "*", dst=os.path.join(self.package_folder, "include"), src=os.path.join(self.source_folder, "include"))

    def package_info(self):
        self.cpp_info.libdirs = []
        self.cpp_info.bindirs = []
        self.cpp_info.defines = ["RETROLIB_WITH_MODULES=0"]