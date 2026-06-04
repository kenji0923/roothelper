find_package(Doxygen)

if (DOXYGEN_FOUND)
    set (DOXYGEN_SOURCE_BROWSER YES)
    set (DOXYGEN_INLINE_SOURCES YES)
    set (EXTRACT_ALL YES)
    set(DOXYGEN_USE_MDFILE_AS_MAINPAGE "${PROJECT_SOURCE_DIR}/document/API_REFERENCE.md")
    doxygen_add_docs(doc
        "${PROJECT_SOURCE_DIR}"
        COMMENT "Generate manual pages"
    )
endif()
