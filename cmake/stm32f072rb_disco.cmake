# Set branch names for the sub modules dependencies
set(GITHUB_BRANCH_STM32F072RB_DISCO "HEAD" CACHE STRING "git SHA or board")

cmake_print_variables(GITHUB_BRANCH_STM32F072RB_DISCO)
FetchContent_Declare(
    stm32f072rb_disco                             # Recommendation: Stick close to the original name.
    GIT_REPOSITORY  git@github.com:kodezine/STM32F072RB-DISCO.git
    GIT_TAG         ${GITHUB_BRANCH_STM32F072RB_DISCO}
)

# prefetch the hal library during project configuration
FetchContent_GetProperties(stm32f072rb_disco)

if(NOT stm32f072rb_disco_POPULATED)
    FetchContent_Populate(stm32f072rb_disco)
endif()
add_subdirectory(${stm32f072rb_disco_SOURCE_DIR})
