IF (WEBKIT_USE_DATABASE)
    LIST (APPEND ICON_SRC
        loader/icon/IconDatabase.cpp
    	loader/icon/IconLoader.cpp
        loader/icon/IconRecord.cpp
        loader/icon/PageURLRecord.cpp
	loader/icon/IconFetcher.cpp
    )
ELSE (WEBKIT_USE_DATABASE)
    LIST (APPEND ICON_SRC
        loader/icon/IconDatabaseNone.cpp
    	loader/icon/IconLoader.cpp
    )
ENDIF (WEBKIT_USE_DATABASE)
