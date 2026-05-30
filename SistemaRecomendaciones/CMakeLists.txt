cmake_minimum_required(VERSION 3.21)

project(ApiManagerQt LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt6 REQUIRED COMPONENTS Widgets Network)

qt_add_executable(ApiManagerQt
    main.cpp
    DataManager.h
    DataManager.cpp
    ApiSteamBiblioteca.h
    ApiSteamBiblioteca.cpp
    ApiSteamJuegos.h
    ApiSteamJuegos.cpp
    ApiYoutube.h
    ApiYoutube.cpp
    ApiPeliculas.h
    ApiPeliculas.cpp
    ApiLibros.h
    ApiLibros.cpp
    ApiChatGpt.h
    ApiChatGpt.cpp
    SolicitudChatGpt.h
    SolicitudChatGpt.cpp
    Libro.h
    Pelicula.h
    Video.h
    Juego.h
    PaqueteRecomendaciones.h
    Recomendador.h
    Recomendador.cpp
    MainWidget.h
    MainWidget.cpp
)

target_link_libraries(ApiManagerQt PRIVATE Qt6::Widgets Qt6::Network)

if(WIN32)
    set_target_properties(ApiManagerQt PROPERTIES WIN32_EXECUTABLE TRUE)
endif()

qt_finalize_executable(ApiManagerQt)
