#include <md/MappedFile.h>
#include <pybind11/functional.h>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include <atomic>
#include <chrono>
#include <functional>
#include <print>
#include <ranges>
#include <string>
#include <iostream>
#include <thread>
#include <unordered_map>

namespace {

using namespace std::chrono_literals;

class DataSource {
 public:
  DataSource() = default;

  virtual int pull() = 0;

  ~DataSource() = default;
};

class Strategy {
 public:
  Strategy() = default;

  virtual void run(DataSource& dataSource) = 0;
  virtual ~Strategy() = default;
};

class FileStreamInput : public DataSource {
 public:
  FileStreamInput(std::string const& filename) : mFilename(filename) {}

  virtual int pull() override {
    std::println("FileStreamInput {}: pull", mFilename);
    return 0;
  }

 private:
  std::string mFilename;
};

class RingBuffer {
 public:
  RingBuffer(int id) : mId(id) {}

  class Producer {
   public:
    Producer(RingBuffer& buffer) : mBuffer(buffer) {}

    void push(int x) {
      mBuffer.push(x);
    };

   private:
    RingBuffer& mBuffer;
  };

  class Consumer : public DataSource {
   public:
    Consumer(RingBuffer& buffer) : mBuffer(buffer) {}

    virtual int pull() override {
      std::println("Consumer: pull ({})", mReadLoc);
      return mBuffer.pull(mReadLoc);
    }

   private:
    RingBuffer& mBuffer;
    size_t mReadLoc = 0;
  };

  Producer& getProducer() {
    return mProducer;
  }

  Consumer createConsumer() {
    return {*this};
  }

 private:
  void
  push(int x) {
    std::println("RingBuffer {}: push", mId);
    mData.push_back(x);
  }

  int pull(size_t& loc) const {
    std::println("RingBuffer {}: pull({})", mId, loc);
    if (loc < mData.size()) {
      return mData[loc++];
    } else {
      return -1;
    }
  }

  std::vector<int> mData;
  Producer mProducer = {*this};
  int mId;
};

class ConcreteStrategy : public Strategy {
 public:
  ConcreteStrategy(std::string const& identifier) : mIdentifier(identifier) {}

  virtual void run(DataSource& dataSource) override {
    std::println("Strategy {}: run", mIdentifier);
    for (auto i : std::views::iota(0, 10)) {
      std::println("Strategy {}: pull", mIdentifier);
      dataSource.pull();
    }
    std::println("Strategy {}: run done", mIdentifier);
  }

 private:
  std::string mIdentifier;
};

class LoopRunner;

class AtomicBool {
 public:
  AtomicBool(bool value) : mValue(value) {}
  AtomicBool(AtomicBool const&) = delete;

  void set(bool value) {
    mValue = value;
  }

  friend class LoopRunner;

 private:
  std::atomic<bool> mValue;
};

class LoopRunner {
 public:
  LoopRunner(AtomicBool const& b) {
    mThread = std::jthread([&running = b.mValue]() {
      while (running.load()) {
        std::this_thread::sleep_for(1ms);
      }
      std::cout << "Runner done" << std::endl;
    });
  }

 private:
  std::jthread mThread;
};

void RunSingleThreaded(DataSource& data, Strategy& strategy) {
  auto item = data.pull();
}

}  // namespace

namespace py = pybind11;

PYBIND11_MODULE(pymd, m) {
  m.doc() = "trading simulator";

  py::class_<DataSource>(m, "DataSource")
      .def("pull", &DataSource::pull);

  py::class_<Strategy>(m, "Strategy")
      .def("run", &Strategy::run);

  py::class_<FileStreamInput, DataSource>(m, "FileStreamInput")
      .def(py::init<std::string>());

  py::class_<RingBuffer>(m, "RingBuffer")
      .def(py::init<int>())
      .def("getProducer", &RingBuffer::getProducer)
      .def("createConsumer", &RingBuffer::createConsumer);

  py::class_<RingBuffer::Producer>(m, "Producer")
      .def("push", &RingBuffer::Producer::push);

  py::class_<RingBuffer::Consumer, DataSource>(m, "Consumer");

  py::class_<ConcreteStrategy, Strategy>(m, "ConcreteStrategy")
      .def(py::init<std::string>());

  py::class_<AtomicBool>(m, "AtomicBool")
      .def(py::init<bool>())
      .def("set", &AtomicBool::set);

  py::class_<LoopRunner>(m, "LoopRunner")
      .def(py::init<AtomicBool&>());
}
