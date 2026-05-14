function(glance_configure_webview target_name)
    find_library(wxWidgets_WEBVIEW_LIBRARY
        NAMES
            wx_gtk3u_webview-3.2
            wx_gtk3u_webview-3.0
            wx_osx_cocoau_webview-3.2
            wxmsw32u_webview
    )

    if(wxWidgets_WEBVIEW_LIBRARY)
        target_link_libraries(${target_name} ${wxWidgets_WEBVIEW_LIBRARY})
        target_compile_definitions(${target_name} PRIVATE GLANCE_USE_WEBVIEW)
        message(STATUS "Using wxWebView for Markdown preview: ${wxWidgets_WEBVIEW_LIBRARY}")
    else()
        message(STATUS "wxWebView library not found; using wxHtmlWindow fallback for Markdown preview")
    endif()
endfunction()
