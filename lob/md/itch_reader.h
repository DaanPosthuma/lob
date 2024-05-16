#pragma once

#include <memory>
#include <string>

namespace itch_reader {

  class FileReader {
   public:
      FileReader(std::string const& filename);
      char currentMessageType() const;
      bool next();
      ~FileReader();

    private:
      class Impl;
      std::unique_ptr<Impl> mImpl;
  };

  void read(std::string const& filename);
}

