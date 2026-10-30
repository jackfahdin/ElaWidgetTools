foreach(test_name IN ITEMS appbar controls navigation resources)
    add_executable(tst_${test_name} tst_${test_name}.cpp)
    target_link_libraries(tst_${test_name} PRIVATE Ela::WidgetTools Qt6::Test)
    if(MSVC)
        target_compile_options(tst_${test_name} PRIVATE /utf-8)
        # 测试程序与组件 DLL 位于不同目录，复制实际配置对应的依赖。
        add_custom_command(
            TARGET tst_${test_name}
            POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_RUNTIME_DLLS:tst_${test_name}>
                    $<TARGET_FILE_DIR:tst_${test_name}>
            COMMAND_EXPAND_LISTS)
    endif()
    add_test(NAME ${test_name} COMMAND tst_${test_name})
    set_tests_properties(${test_name} PROPERTIES TIMEOUT 90 LABELS "qt;widgets")
endforeach()
