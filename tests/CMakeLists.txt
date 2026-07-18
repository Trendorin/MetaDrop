qt_add_executable(metadrop_tests
    CoreTests.cpp
)

target_link_libraries(metadrop_tests PRIVATE
    metadrop_core
    Qt6::Test
)
metadrop_set_project_warnings(metadrop_tests)
metadrop_enable_sanitizers(metadrop_tests)

add_test(NAME metadrop-core COMMAND metadrop_tests)
set_tests_properties(metadrop-core PROPERTIES TIMEOUT 30)
