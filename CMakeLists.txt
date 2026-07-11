cmake_minimum_required(VERSION 3.20)
project(GraphiteStudio LANGUAGES CXX C)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Configure FetchContent
include(FetchContent)

# 1. Fetch GLAD 2 (OpenGL Loader Generator)
FetchContent_Declare(
    glad
    GIT_REPOSITORY https://github.com/Dav1dde/glad.git
    GIT_TAG        v2.0.8
    SOURCE_SUBDIR  cmake
)
FetchContent_MakeAvailable(glad)

# Generate glad target using glad2 generator function
glad_add_library(glad_gl_46 API gl:core=4.6)
target_include_directories(glad_gl_46 PUBLIC ${CMAKE_CURRENT_BINARY_DIR}/gladsources/glad_gl_46/include)


# 2. Fetch GLFW
FetchContent_Declare(
    glfw
    GIT_REPOSITORY https://github.com/glfw/glfw.git
    GIT_TAG        3.4
)
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(GLFW_INSTALL OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(glfw)

# 3. Fetch Dear ImGui (Docking Branch)
FetchContent_Declare(
    imgui
    GIT_REPOSITORY https://github.com/ocornut/imgui.git
    GIT_TAG        docking
)
FetchContent_MakeAvailable(imgui)

# ImGui does not provide its own CMakeLists.txt. We define it manually.
FetchContent_GetProperties(imgui)
if(NOT imgui_POPULATED)
    FetchContent_Populate(imgui)
endif()

set(IMGUI_SOURCES
    ${imgui_SOURCE_DIR}/imgui.cpp
    ${imgui_SOURCE_DIR}/imgui_draw.cpp
    ${imgui_SOURCE_DIR}/imgui_widgets.cpp
    ${imgui_SOURCE_DIR}/imgui_tables.cpp
    ${imgui_SOURCE_DIR}/imgui_demo.cpp
    ${imgui_SOURCE_DIR}/backends/imgui_impl_glfw.cpp
    ${imgui_SOURCE_DIR}/backends/imgui_impl_opengl3.cpp
)

add_library(imgui STATIC ${IMGUI_SOURCES})
target_include_directories(imgui PUBLIC 
    ${imgui_SOURCE_DIR}
    ${imgui_SOURCE_DIR}/backends
)
# ImGui links to GLFW and the generated glad library
target_link_libraries(imgui PUBLIC glfw glad_gl_46)

# 4. Main Executable
add_executable(GraphiteStudio 
    src/main.cpp
    src/core/Image.cpp
    src/core/Layer.cpp
    src/core/LayerStack.cpp
    src/core/Compositor.cpp
    src/core/HistoryManager.cpp
    src/core/DocumentCommands.cpp
    src/core/BrushEngine.cpp
    src/core/Document.cpp
    src/core/Application.cpp
    src/core/GLWrappers.cpp
    src/platform/FileDialogs.cpp
    src/gui/CanvasView.cpp
    src/gui/Theme.cpp
    src/gui/Workspace.cpp
    src/gui/Toolbar.cpp
    src/gui/StatusBar.cpp
    src/gui/PropertiesPanel.cpp
    src/gui/LayersPanel.cpp
    src/gui/OptionsBar.cpp
    src/gui/HistoryPanel.cpp
)

target_include_directories(GraphiteStudio PRIVATE 
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(GraphiteStudio PRIVATE imgui glad_gl_46 glfw)

# Windows Specific Link Options
if(WIN32)
    # Ensure OpenGL and Comdlg32 standard libraries are linked
    target_link_libraries(GraphiteStudio PRIVATE Opengl32 comdlg32 Shell32)
endif()
