#ifndef INTERRUPTS_HPP_
#define INTERRUPTS_HPP_

#include<iostream>
#include<fstream>
#include<string>
#include<vector>
#include<random>
#include<utility>
#include<sstream>
#include<iomanip>

#include<stdio.h>

#define ADDR_BASE   0
#define VECTOR_SIZE 2
#define CPU_SPEED   100
#define MEM_LIMIT   1

/**
 * \brief Load vector table from file
 *
 * Reads a text file containing one vector address per line.
 * @param filename path to vector table file
 * @return vector of strings representing ISR addresses
 */
std::vector<std::string> load_vector_table(const std::string& filename) {
    std::vector<std::string> vectors;
    std::ifstream file(filename);
    std::string vector;
    while (std::getline(file, vector)) {
        if (!vector.empty()) vectors.push_back(vector);
    }
    return vectors;
}

/**
 * \brief Load device table from file
 *
 * Reads a text file containing one integer (delay) per line.
 * @param filename path to device table file
 * @return vector of integers representing delays per device
 */
std::vector<int> load_device_table(const std::string& filename) {
    std::vector<int> delays;
    std::ifstream file(filename);
    std::string line;
    while (std::getline(file, line)) {
        if (!line.empty()) delays.push_back(std::stoi(line));
    }
    return delays;
}

/**
 * \brief Helper to split a string by a delimiter
 */
std::vector<std::string> split_delim(std::string input, std::string delim) {
    std::vector<std::string> tokens;
    std::size_t pos = 0;
    std::string token;
    while ((pos = input.find(delim)) != std::string::npos) {
        token = input.substr(0, pos);
        tokens.push_back(token);
        input.erase(0, pos + delim.length());
    }
    tokens.push_back(input);
    return tokens;
}

/**
 * \brief Parse a line from the input trace file
 *
 * The line has the format: ACTIVITY, VALUE
 * where ACTIVITY isone of CPU / SYSCALL / END_IO, and VALUE is a duration or device number.
 */
std::tuple<std::string, int> parse_trace(std::string trace) {
    auto parts = split_delim(trace, ",");
    if (parts.size() < 2) {
        std::cerr << "Error: Malformed input line: " << trace << std::endl;
        return {"null", -1};
    }
    auto activity = parts[0];
    auto duration_intr = std::stoi(parts[1]);
    return {activity, duration_intr};
}

/**
 * \brief Standard interrupt boilerplate (entry)
 *
 * Generates the sequence of kernel mode switch, context save,
 * and loading the correct ISR address from the vector table.
 */
std::pair<std::string, int> intr_boilerplate(int current_time, int intr_num, int context_save_time, const std::vector<std::string>& vectors) {
    std::string execution = "";

    execution += std::to_string(current_time) + ", 1, switch to kernel mode\n";
    current_time++;

    execution += std::to_string(current_time) + ", " + std::to_string(context_save_time) + ", context saved\n";
    current_time += context_save_time;

    char vector_address_c[10];
    sprintf(vector_address_c, "0x%04X", (ADDR_BASE + (intr_num * VECTOR_SIZE)));
    std::string vector_address(vector_address_c);

    execution += std::to_string(current_time) + ", 1, find vector " + std::to_string(intr_num)
                + " in memory position " + vector_address + "\n";
    current_time++;

    execution += std::to_string(current_time) + ", 1, load address " + vectors.at(intr_num) + " into the PC\n";
    current_time++;

    return std::make_pair(execution, current_time);
}

/**
 * \brief Write simulation output to file
 *
 * Writes the full execution trace to the output directory
 */
void write_output(const std::string& execution, const std::string& filename = "output_files/execution.txt") {
    std::ofstream output_file(filename);
    if (output_file.is_open()) {
        output_file << execution;
        output_file.close();
        std::cout << "Output successfully written to " << filename << std::endl;
    } else {
        std::cerr << "Error opening " << filename << std::endl;
    }
}
#endif
