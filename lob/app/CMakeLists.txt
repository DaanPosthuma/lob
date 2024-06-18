add_executable(app main.cpp StrategyDiagnostics.cpp)
target_link_libraries(app PRIVATE lob md STDEXEC::stdexec nlohmann_json::nlohmann_json)
