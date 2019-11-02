CMAKE_MINIMUM_REQUIRED(VERSION 3.14)
PROJECT(Networking)

FILE(GLOB NETWORKING_EXTERNAL_SOURCES
)

SET(NETWORKING_SOURCES
	Source/pch.h
	Source/pch.cpp
    
    Include/Networking/Network.h
    Include/Networking/Networking.h
    Include/Networking/Connection.h
    Include/Networking/Client/WebSocketClient.h
    Include/Networking/Server/WebSocketServer.h
    
    Source/Networking/Network.cpp
    Source/Networking/Connection.cpp
    Source/Networking/ConnectionInternal.h
    Source/Networking/ConnectionInternal.cpp
    
    Source/Networking/Client/WebSocketClient.cpp
    Source/Networking/Server/WebSocketServer.cpp
)

ADD_LIBRARY(Networking STATIC ${NETWORKING_SOURCES} ${NETWORKING_EXTERNAL_SOURCES})

TARGET_INCLUDE_DIRECTORIES(Networking PUBLIC Include)
TARGET_INCLUDE_DIRECTORIES(Networking PRIVATE Source Include/Networking)

SET(CMAKE_INCLUDE_CURRENT_DIR ON)