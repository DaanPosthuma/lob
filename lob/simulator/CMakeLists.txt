add_library(simulator StrategyDiagnostics.cpp functions.cpp)
target_link_libraries(simulator PUBLIC md PRIVATE lob nlohmann_json::nlohmann_json)
