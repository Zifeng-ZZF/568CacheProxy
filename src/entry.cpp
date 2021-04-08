#include <iostream>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "Proxy.h"

using std::thread;

int main() {
    pid_t pid = fork();
    if (pid == -1) {
        cerr << "Error forking from dummy process";
        return EXIT_FAILURE;
    }
    else if (pid == 0) {
        // actual process & daemonized
        cout << "running as daemon ..... " << endl;
        if (daemon(0, 0) == -1) {
            cerr << "Daemo creation failed." << endl;
            return EXIT_FAILURE;
        }
        Proxy * proxy = new Proxy();
        proxy->run();
        delete proxy;
    }
    else {
        // run docker dummy
        cout << "Do nothing dummy." << endl;
        while (true) {}
    }
    return EXIT_SUCCESS;
}
