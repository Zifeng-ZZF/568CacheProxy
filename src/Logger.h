#ifndef LOGGER_FILE_H
#define LOGGER_FILE_H

#include <fstream>
#include <mutex>
#include <string>
#include "SocketException.h"

class Logger{ 
private:
    std::ofstream stream;
    std::mutex mtx_lock;

public:

    Logger(){}

    ~Logger(){}

    /**
     * Open the file input stream
    */
    void openFileStream(const std::string & filename);

    /**
     * Write content into file
    */
    void writeFile(std::string & content);

    /**
     * Close file stream.
    */
    void closeFile();
};

#endif
