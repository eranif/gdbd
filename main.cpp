#include "Driver.hpp"
#include "CommandLineParser.hpp"
#include "dap/JsonRPC.hpp"
#include "dap/Process.hpp"
#include "dap/SocketBase.hpp"
#include "dap/SocketServer.hpp"
#include "dap/StringUtils.hpp"
#include "dap/dap.hpp"
#include <cstdio>
#include <cstdlib>
#include <iostream>

using namespace std;
// clang-format on

int main(int argc, char** argv)
{
    // Initialize the dap library
    
    CommandLineParser parser;
    parser.Parse(argc, argv);
    
    dap::Initialize();
    // Start the gdb process
    dap::Process* gdb = dap::ExecuteProcess(parser.GetGdb());
    
    if(!gdb) {
        cerr << "Unable to launch gdb: " << argc;
    }
    this_thread::sleep_for(chrono::milliseconds(100));
    size_t counter = 0;
    while(gdb->IsAlive() && (counter < 10)) {
        string o, e;
        if(gdb->Read(o, e)) {
            if(!o.empty()) {
                cout << o << endl;
            }
            if(!e.empty()) {
                cerr << e << endl;
            }
            continue;
        }
        this_thread::sleep_for(chrono::milliseconds(10));
        gdb->Write("help");
        ++counter;
    }

    delete gdb;
    // We are done
    return 0;
}
