# Set branch names for the sub modules dependencies
set(GITHUB_BRANCH_HAL "HEAD" CACHE STRING "git SHA or branch name for HAL")

cmake_print_variables(GITHUB_BRANCH_HAL)
FetchContent_Declare(
    hal                             # Recommendation: Stick close to the original name.
    GIT_REPOSITORY  git@github.com:kodezine/STM32F072RB-DISCO.git
    GIT_TAG         ${GITHUB_BRANCH_HAL}
)

# prefetch the hal library during project configuration
FetchContent_GetProperties(hal)

if(NOT hal_POPULATED)
    FetchContent_Populate(hal)
endif()

add_subdirectory(${hal_SOURCE_DIR})
