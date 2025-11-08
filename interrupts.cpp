/**
 *
 * @file interrupts.cpp
 * @authors Marwa Diab 
 *
 */

#include<interrupts.hpp>

// Load vector addresses from file
std::vector<std::string> load_vectors(const std::string& filename) {
    std::ifstream file(filename);
    std::vector<std::string> vectors;
    std::string address;
    while (file >> address) vectors.push_back(address);
    return vectors;
}

// Load device delays from file
std::vector<int> load_delays(const std::string& filename) {
    std::ifstream file(filename);
    std::vector<int> delays;
    int val;
    while (file >> val) delays.push_back(val);
    return delays;
}

// Parse a single trace line (activity, device number or duration)
std::pair<std::string, int> parse_trace(const std::string& line) {
    std::string type;
    int val = 0;
    std::stringstream ss(line);
    ss >> type >> val;
    return { type, val };
}

int main(int argc, char** argv) {

    // If no argument, use default external files
    std::string vector_file = "vector_table.txt";
    std::string device_file = "device_table.txt";
    std::string input_file_name = "input_files/program1.txt";
    std::string output_file_name = "output_files/execution.txt";

    // Load external tables
    auto vectors = load_vectors(vector_file);
    auto delays  = load_delays(device_file);

    std::ifstream input_file(input_file_name);
    std::string trace;
    std::string execution;

    int current_time = 0;
    const int context_save_time = 10;
    const int isr_time = 40;

    auto add = [&](int start, int dur, const std::string& msg) {
        execution += std::to_string(start) + ", " + std::to_string(dur) + ", " + msg + "\n";
    };

    while (std::getline(input_file, trace)) {
        if (trace.empty()) continue;
        auto [activity, duration_intr] = parse_trace(trace);

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

            add(current_time, context_save_time, "Context save (SYSCALL)");
            current_time += context_save_time;

            add(current_time, isr_time, "SYSCALL: run ISR " + vectors[dev]);
            current_time += isr_time;

            add(current_time, 40, "Transfer data from device to memory");
            current_time += 40;

            int check_err = delays[dev] - (isr_time + 40);
            if (check_err < 0) check_err = 0;
            add(current_time, check_err, "Check for errors");
            current_time += check_err;

            add(current_time, context_save_time, "Context restored");
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

            add(current_time, context_save_time, "Context save (END_IO)");
            current_time += context_save_time;

            add(current_time, isr_time, "END_IO: run ISR " + vectors[dev]);
            current_time += isr_time;

            int check_status = delays[dev] - isr_time;
            if (check_status < 0) check_status = 0;
            add(current_time, check_status, "Check device status");
            current_time += check_status;

            add(current_time, context_save_time, "Context restored");
            current_time += context_save_time;

            add(current_time, 1, "IRET");
            current_time += 1;
        }
        else {
            add(current_time, 0, "Unknown activity: " + activity);
        }
    }

    input_file.close();

    std::ofstream output_file(output_file_name);
    output_file << execution;
    output_file.close();

    std::cout << "Execution finished. Output written to " << output_file_name << std::endl;
    return 0;
}
