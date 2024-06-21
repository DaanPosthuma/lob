add_library(strategies StrategyDiagnostics.cpp)
target_link_libraries(strategies PRIVATE md lob nlohmann_json::nlohmann_json)
