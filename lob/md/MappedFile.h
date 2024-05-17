#pragma once

#include <boost/iostreams/device/mapped_file.hpp>

namespace md {

  class MappedFile {
   public:
      explicit MappedFile(std::string const& filename) : mFile(filename) {
        if(!mFile.is_open()) throw std::runtime_error("Could not load file");
      }
      
    private:
      boost::iostreams::mapped_file_source mFile;
  };
}

