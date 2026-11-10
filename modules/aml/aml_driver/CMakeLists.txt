ADD_EXECUTABLE(aml_driver)
ADD_DEPENDENCIES(aml_driver aml)
TARGET_SOURCES(aml_driver PRIVATE driver.c)
TARGET_LINK_LIBRARIES(aml_driver aml)
