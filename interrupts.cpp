/**
 *
 * @file interrupts.cpp
 * @authors Marwa Diab 
 *
 */

#include<interrupts.hpp>

int main(int argc, char** argv) {

    //vectors is a C++ std::vector of strings that contain the address of the ISR
    //delays  is a C++ std::vector of ints that contain the delays of each device
    //the index of these elemens is the device number, starting from 0
    auto [vectors, delays] = parse_args(argc, argv);
    std::ifstream input_file(argv[1]);

    std::string trace;      //!< string to store single line of trace file
    std::string execution;  //!< string to accumulate the execution output

    /******************ADD YOUR VARIABLES HERE*************************/
    int current_time = 0;
    const int context_save_time = 10;
    const int isr_time = 40; 
    
    auto add = [&](int start, int dur, const std::string& msg) {
        execution += std::to_string(start) + ", " + std::to_string(dur) + ", " + msg + "\n";
    };

    /******************************************************************/
    //parse each line of the input trace file
     while(std::getline(input_file, trace)) {
        auto [activity, duration_intr] = parse_trace(trace);
         
        /******************ADD YOUR SIMULATION CODE HERE*************************/
        if (activity == "CPU") {
            add(current_time, duration_intr, "CPU Burst");
            current_time += duration_intr;
        }
        else if (activity == "SYSCALL") {
            int dev = duration_intr;
            if (dev < 0 || dev >= (int)delays.size() || dev >= (int)vectors.size()) {
                std::cerr << "Invalid device: " << dev << "\n";
                continue;
            }

            // standard interrupt entry
            auto pre = intr_boilerplate(current_time, dev, context_save_time, vectors);
            execution += pre.first;
            current_time = pre.second;

            // ISR 
            add(current_time, isr_time, "SYSCALL: run the ISR (device driver)");
            current_time += isr_time;

            // transfer from device to memory
            add(current_time, 40, "transfer data from device to memory");
            current_time += 40;

            int check_err = delays[dev] - (isr_time + 40);
            if (check_err < 0) check_err = 0;
            add(current_time, check_err, "check for errors");
            current_time += check_err;

            // restore context + IRET 
            add(current_time, context_save_time, "context restored");
            current_time += context_save_time;

            add(current_time, 1, "IRET");
            current_time += 1;
        }
        else if (activity == "END_IO") {
            int dev = duration_intr;
            if (dev < 0 || dev >= (int)delays.size() || dev >= (int)vectors.size()) {
                std::cerr << "Invalid device: " << dev << "\n";
                continue;
            }

            // standard interrupt entry
            auto pre = intr_boilerplate(current_time, dev, context_save_time, vectors);
            execution += pre.first;
            current_time = pre.second;

            // ISR work to service completion
            add(current_time, isr_time, "ENDIO: run the ISR (device driver)");
            current_time += isr_time;

            // device status/cleanup to match average delay
            int check_status = delays[dev] - isr_time;
            if (check_status < 0) check_status = 0;
            add(current_time, check_status, "check device status");
            current_time += check_status;

            // restore context + IRET 
            add(current_time, context_save_time, "context restored");
            current_time += context_save_time;

            add(current_time, 1, "IRET");
            current_time += 1;
        }
        /************************************************************************/
    }

    input_file.close();

    write_output(execution);

    return 0;
}
