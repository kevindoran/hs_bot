project(hypersonic)

set(HEADER_FILES
        Position.h
        InputParser.h
        OnlineMedian.h
        AnnealingBot.h
        Mechanics.h
        Bot.h
        Board.h)


set(SOURCE_FILES
        board.cpp
        Position.cpp
        Mechanics.cpp
        main.cpp)


add_library(hypersonic STATIC ${SOURCE_FILES} ${HEADER_FILES})

