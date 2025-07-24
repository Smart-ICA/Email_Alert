cmake_minimum_required(VERSION 3.20)
project(email_alert_plugin VERSION 1.0.0 LANGUAGES CXX)

# Définir le type de compilation
if(CMAKE_BUILD_TYPE STREQUAL "")
  set(CMAKE_BUILD_TYPE "Release" CACHE STRING "Choose the type of build." FORCE)
endif()

set(CMAKE_BUILD_TYPE Debug)

# Définir le répertoire source
set(SRC_DIR ${CMAKE_CURRENT_LIST_DIR}/src)

# Utilisation du standard C++17
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Définir le répertoire include
include_directories(BEFORE ${CMAKE_SOURCE_DIR}/include)

# Dépendances via FetchContent
include(FetchContent)

# Déclaration des dépendances

# JSON for Modern C++ (nlohmann/json)
FetchContent_Declare(json
  GIT_REPOSITORY https://github.com/nlohmann/json.git
  GIT_TAG        v3.11.3
  GIT_SHALLOW    TRUE
)

# MADS Plugin (pbosetti/mads_plugin)
FetchContent_Declare(plugin
  GIT_REPOSITORY https://github.com/pbosetti/mads_plugin.git
  GIT_TAG        HEAD
  GIT_SHALLOW    TRUE
)

# Télécharge les dépendances
FetchContent_MakeAvailable(json plugin)

# Inclure les répertoires des dépendances
include_directories(${json_SOURCE_DIR}/include)
include_directories(${plugin_SOURCE_DIR}/src)

# Macro pour ajouter un plugin
macro(add_plugin name)
  set(multiValueArgs LIBS SRCS)
  cmake_parse_arguments(plugin "" "" "${multiValueArgs}" ${ARGN})

  add_library(${name} SHARED ${SRC_DIR}/${name}.cpp ${plugin_SRCS})
  target_link_libraries(${name} PRIVATE pugg ${plugin_LIBS})
  set_target_properties(${name} PROPERTIES PREFIX "")
  set_target_properties(${name} PROPERTIES SUFFIX ".plugin")
  list(APPEND TARGET_LIST ${name})
endmacro()

# Ajout du plugin email_alert
add_plugin(email_alert)


# Paramètres de build spécifiques à chaque plateforme
if (APPLE)
  set(CMAKE_INSTALL_RPATH "@executable_path/../lib")
  include_directories(/opt/homebrew/include)
  link_directories(/opt/homebrew/lib)
else()
  set(CMAKE_INSTALL_RPATH "\$ORIGIN/../lib;/usr/local/lib")
endif()

# Définir les définitions de compilation
add_definitions(-DINSTALL_PREFIX="${CMAKE_INSTALL_PREFIX}")

# Instructions d'installation
if(APPLE)
  install(TARGETS ${TARGET_LIST}
    LIBRARY DESTINATION lib
    RUNTIME DESTINATION lib
    ARCHIVE DESTINATION lib
  )
else()
  install(TARGETS ${TARGET_LIST}
    LIBRARY DESTINATION lib
    RUNTIME DESTINATION bin
    ARCHIVE DESTINATION lib
  )
endif()
