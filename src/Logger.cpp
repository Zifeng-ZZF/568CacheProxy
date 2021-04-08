#include "Logger.h"
#include <iostream>
/**
 * Open the file input stream
*/
void Logger::openFileStream(const std::string & filename){
    try{
        stream.open(filename, std::ofstream::out | std::ofstream::app);
    }catch(const std::exception& e){
        throw SocketException("Error in openning the file: " + filename); 
    } 
    if (stream.is_open()) {
        std::cout << "[LOGGER] Open stream to " << filename << std::endl;
    }
    
}

/**
 * Write content into file
*/
void Logger::writeFile(std::string & content){
    if(stream.is_open() && mtx_lock.try_lock()){
        std::cout << "[LOGGER] Wrting content " << content << std::endl;
        stream << content;
        stream.flush();
        mtx_lock.unlock();
    }
}

/**
 * Close file stream.
*/
void Logger::closeFile(){
    try{
        stream.close();
    }catch(const std::exception& e){
        throw SocketException("Error in closing the file"); 
    } 
}
