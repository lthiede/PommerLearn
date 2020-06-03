#include <iostream>

#include "runner.h"
#include "ipc_manager.h"

int main() {
    FileBasedIPCManager ipcManager("data.zr");
    Runner runner;

    runner.generateSupervisedTrainingData(&ipcManager, 500, 10);
    ipcManager.flush();

    return 0;
}
