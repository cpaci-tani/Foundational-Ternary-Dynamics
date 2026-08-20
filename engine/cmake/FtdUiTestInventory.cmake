if(NOT DEFINED CTEST_COMMAND OR NOT DEFINED BUILD_DIR OR NOT DEFINED CONFIG)
    message(FATAL_ERROR "CTEST_COMMAND, BUILD_DIR, and CONFIG are required")
endif()

execute_process(
    COMMAND "${CTEST_COMMAND}" -N -C "${CONFIG}" -L "^ui$"
    WORKING_DIRECTORY "${BUILD_DIR}"
    RESULT_VARIABLE inventory_rc
    OUTPUT_VARIABLE inventory_out
    ERROR_VARIABLE inventory_err
)
if(NOT inventory_rc EQUAL 0)
    message(FATAL_ERROR "ctest inventory failed: ${inventory_err}")
endif()
if(NOT inventory_out MATCHES "ui_observer_neutrality_cpu")
    message(FATAL_ERROR "ui label is missing ui_observer_neutrality_cpu:\n${inventory_out}")
endif()
if(NOT inventory_out MATCHES "ui_command_queue")
    message(FATAL_ERROR "ui label is missing ui_command_queue:\n${inventory_out}")
endif()
if(NOT inventory_out MATCHES "ui_snapshot_publisher")
    message(FATAL_ERROR "ui label is missing ui_snapshot_publisher:\n${inventory_out}")
endif()
if(NOT inventory_out MATCHES "ui_command_boundary")
    message(FATAL_ERROR "ui label is missing ui_command_boundary:\n${inventory_out}")
endif()
if(NOT inventory_out MATCHES "ui_journal_replay")
    message(FATAL_ERROR "ui label is missing ui_journal_replay:\n${inventory_out}")
endif()
if(NOT inventory_out MATCHES "Total Tests: ([0-9]+)")
    message(FATAL_ERROR "could not parse UI test count:\n${inventory_out}")
endif()
set(ui_count "${CMAKE_MATCH_1}")
if(ui_count LESS 5)
    message(FATAL_ERROR "UI test inventory is vacuous")
endif()
message(STATUS "UI test inventory: ${ui_count}")
