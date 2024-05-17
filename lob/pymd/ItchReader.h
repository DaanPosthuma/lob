#pragma once

#include <md/MappedFile.h>

#include <array>
#include <functional>
#include <map>
#include <string>
#include <variant>

namespace pymd {

class ItchReader {
  using DictType = std::map<std::string, std::variant<std::string, int>>;
  using Callback = std::function<void(DictType const&)>;

 public:
  explicit ItchReader(std::string const& filename) : mFile(filename) {}
  void onMessage(char messageType, Callback const& callback) { mCallbacks[messageType] = callback; }
  int read(int num) const;

 private:
  md::MappedFile mFile;
  std::array<Callback, 256> mCallbacks;
};

}  // namespace pymd
