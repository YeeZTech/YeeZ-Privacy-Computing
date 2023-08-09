add_library(t_org_info_reader SHARED t_org_info_reader.cpp)
target_link_libraries(t_org_info_reader pthread ff_net)
target_include_directories(t_org_info_reader PRIVATE
  "$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>"
  "$<BUILD_INTERFACE:${FF_PATH}/include>"
  )
target_link_directories(t_org_info_reader PRIVATE
  "$<BUILD_INTERFACE:${FF_PATH}/lib>")
install(TARGETS t_org_info_reader DESTINATION lib)

add_library(t_tax_reader SHARED t_tax_reader.cpp)
target_link_libraries(t_tax_reader pthread ff_net)
target_include_directories(t_tax_reader PRIVATE
  "$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>"
  "$<BUILD_INTERFACE:${FF_PATH}/include>"
  )
target_link_directories(t_tax_reader PRIVATE
  "$<BUILD_INTERFACE:${FF_PATH}/lib>")
install(TARGETS t_tax_reader DESTINATION lib)
