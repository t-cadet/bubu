#pragma once

#include <cassert>
#include <filesystem>
#include <string_view>
#include <string>
#include <vector>
namespace fs = std::filesystem;

enum TargetKind : char {
  Executable = 0,
  StaticLibrary,
  SharedLibrary,
};

struct Target {
  std::string_view name;
  TargetKind kind;
  std::vector<std::filesystem::path> sources;
  std::vector<std::filesystem::path> includePaths;
  std::vector<std::filesystem::path> libraryPaths;
  std::vector<Target> dependencies;
  std::filesystem::path projectDirectory;

  std::vector<std::filesystem::path> getIncludePathsRecursive() const {
    std::vector<std::filesystem::path> paths = includePaths;
    for (auto const& dependency : dependencies) {
      for (auto const& path : dependency.getIncludePathsRecursive()) {
        paths.push_back(dependency.projectDirectory / path);
      }
    }
    return paths;
  }
};

#include <iostream>
#include <vector>
#include <string>
#include <sstream>
#include <cstdio>
#include <memory>
#include <stdexcept>
#include <array>

int cmd(std::vector<std::string> const& args, std::string directory) {
    // std::cerr << "directory=" << directory << "\n";
    auto currentPathBackup = fs::current_path();
    // std::cerr << "currentPathBackup=" << currentPathBackup << "\n";
    fs::current_path(directory);

    // Construct the args string from the vector
    std::ostringstream commandStream;
    for (const auto& part : args) {
        commandStream << part << " ";
    }
    std::string command = commandStream.str();
    std::cerr << "+ " << command << "\n";

    // Open a pipe to the args
    std::array<char, 4096> buffer;
    std::string result;
    std::string error;

    // Use popen to execute the args
    std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(command.c_str(), "r"), pclose);
    if (!pipe) {
        throw std::runtime_error("popen() failed!");
    }

    // Read the output from the args
    while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) {
        result += buffer.data();
    }

    // Check the exit status of the args
    int exitCode = pclose(pipe.release());

    // Print the output and error
    // std::cout << "Output:\n" << result << std::endl;
    // if (exitCode != 0) {
    //     std::cerr << "Error executing args. Exit code: " << exitCode << std::endl;
    // }
    fs::current_path(currentPathBackup);

    return exitCode;
}

const char* OUTPUT = ".build";
const char* CC = "clang++";
struct Compiler {
  // TODO: fields

  bool build(Target const& target) {
    fs::path output = fs::absolute(fs::path(OUTPUT));

    assert(!target.sources.empty());
    bool ok = true;
    // TODO: add target to the set of targets whose build started

    // build dependencies
    for (auto const& dependency : target.dependencies) {
      build(dependency);
    }

    // compile sources
    for (auto const& source : target.sources) {
      std::vector<std::string> args = {CC};
      // TODO: mkdir output
      args.push_back("-o");
      args.push_back((fs::absolute(fs::path(OUTPUT)) / std::filesystem::path(source).replace_extension(".o")).string());
      for (auto const& includePath : target.getIncludePathsRecursive()) {
        args.push_back("-I");
        args.push_back(includePath.string());
      }
      args.push_back("-c");
      for (auto const& source : target.sources) {
        args.push_back(source);
      }
      ok &= cmd(args, target.projectDirectory);
    }
    // if (!ok) return false;

    // link
    switch (target.kind) {
      case Executable: {
          std::vector<std::string> args = {CC};
          // TODO: mkdir output
          args.push_back("-o");
          args.push_back((fs::absolute(fs::path(OUTPUT)) / target.name).string());
          for (auto const& source : target.sources) {
            args.push_back((output / std::filesystem::path(source)).replace_extension(".o").string());
          }
          args.push_back("-L");
          args.push_back(output.string());
          for (auto const& libraryPath : target.libraryPaths) {
            args.push_back("-L");
            args.push_back(libraryPath.string());
          }
          for (auto const& dependency : target.dependencies) {
            args.push_back("-l");
            args.push_back(std::string(dependency.name));
          }
          ok &= cmd(args, target.projectDirectory);
      } break;
      case StaticLibrary: {
          std::vector<std::string> args = {"ar", "rcs"};
          // TODO: mkdir output
          args.push_back((fs::absolute(fs::path(OUTPUT)) / (std::string("lib") + std::string(target.name) + ".a")).string());
          for (auto const& source : target.sources) {
            args.push_back((output / fs::path(source).replace_extension(".o")).string());
          }
          ok &= cmd(args, target.projectDirectory);
      } break;
      case SharedLibrary: {
          // TODO
      } break;
    }

    // TODO: clean .o
    return ok;
  }
};

#define Target() Target { .projectDirectory = fs::absolute(fs::path(__FILE__)).parent_path() }
