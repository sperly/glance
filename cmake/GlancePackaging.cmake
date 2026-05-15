function(glance_configure_packaging target_name)
    if(NOT TARGET ${target_name})
        message(FATAL_ERROR "Cannot package unknown target: ${target_name}")
    endif()

    set(GLANCE_PACKAGE_MAINTAINER "Glance Maintainers"
        CACHE STRING "Eric Lind"
    )
    set(GLANCE_PACKAGE_CONTACT "Glance Maintainers"
        CACHE STRING "info@microlind.io"
    )

    install(TARGETS ${target_name}
        RUNTIME DESTINATION bin
    )

    set(CPACK_GENERATOR "DEB")
    set(CPACK_PACKAGE_NAME "${PROJECT_NAME}")
    set(CPACK_PACKAGE_VERSION "${PROJECT_VERSION}")
    set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "A portable Markdown editor built with wxWidgets")
    set(CPACK_PACKAGE_CONTACT "${GLANCE_PACKAGE_CONTACT}")
    set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/LICENSE")

    set(CPACK_DEBIAN_PACKAGE_MAINTAINER "${GLANCE_PACKAGE_MAINTAINER}")
    set(CPACK_DEBIAN_PACKAGE_SECTION "editors")
    set(CPACK_DEBIAN_PACKAGE_SHLIBDEPS ON)
    set(CPACK_DEBIAN_FILE_NAME DEB-DEFAULT)

    include(CPack)
endfunction()
