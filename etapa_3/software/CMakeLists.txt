# Projeto ESP-IDF: CMake organiza a compilação, mas não executa no ESP.
# IDF_PATH deve apontar para a instalação do SDK (preparada por build_flash.ps1).
cmake_minimum_required(VERSION 3.16)
# Carrega as regras do SDK antes de declarar o projeto.
include($ENV{IDF_PATH}/tools/cmake/project.cmake)
# Define o nome usado nos artefatos .elf/.bin produzidos em build/.
project(cookie_express_idf)
