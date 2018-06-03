CMAKE_MINIMUM_REQUIRED(VERSION 3.10)
PROJECT(RuleConverter)

SET(CMAKE_CXX_STANDARD 17)

SET(SOURCE_FILES
	main.cpp
	Symbol/Symbol.cpp
	Symbol/Symbol.h
	Node/Node.cpp
	Node/Node.h
	NonterminalHelper/NonterminalHelper.cpp
	NonterminalHelper/NonterminalHelper.h
	RuleRightSide/RuleRightSide.cpp
	RuleRightSide/RuleRightSide.h)

ADD_EXECUTABLE(RuleConverter ${SOURCE_FILES})

IF (NOT TARGET InputLibrary)
	ADD_SUBDIRECTORY(InputLibrary)
ENDIF ()

IF (NOT TARGET TokenLibrary)
	ADD_SUBDIRECTORY(TokenLibrary)
ENDIF ()

TARGET_LINK_LIBRARIES(RuleConverter InputLibrary TokenLibrary)
