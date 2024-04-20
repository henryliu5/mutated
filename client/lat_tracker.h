#pragma once

#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <algorithm>
#include <chrono>
#include <iomanip>


struct Measurement{
    long lat_ns;
    double timestamp;
};

template <typename T>
class CircularVector{

public:
    CircularVector(int max_size){
        this->max_size = max_size;
        this->size = 0;
        this->head = 0;
        this->tail = 0;
        this->data = std::vector<T>(max_size);
    }

    void push(T value){
        if(size == max_size){
            throw std::runtime_error("Cannot push to full circular vector");
            head = (head + 1) % max_size;
        } else{
            size++;
        }
        data[tail] = value;
        tail = (tail + 1) % max_size;
    }

    T get(int index){
        return data[(head + index) % max_size];
    }

    T pop(){
        if(size == 0){
            throw std::runtime_error("Cannot pop from empty circular vector");
        }
        T value = data[head];
        head = (head + 1) % max_size;
        size--;
        return value;
    }

    int size;

private:
    int max_size;
    int head;
    int tail;
    std::vector<T> data;
};

/**
 * Rolling window of latencies, reports some percentile.
 * Outputs to a file.
*/
class LatencyTracker{
    // Carefully choose max size to ensure that the number of observations won't overflow
    const double WINDOW_TIME = 0.1;
    const int MAX_SIZE = 200000;

public:
LatencyTracker(double percentile, std::string output_file) : 
        percentile(percentile), 
        output_file(output_file), 
        last_write(0),
        window_seconds(WINDOW_TIME),
        measurements(MAX_SIZE){
    if(percentile < 0 || percentile > 1){
        throw std::runtime_error("Percentile must be between 0 and 1");
    }
    // Set fixed-point notation
    std::cerr << std::fixed;

    // Set precision to 2 decimal places
    std::cerr << std::setprecision(2);
}

/**
 * Add a new measurement to the tracker. 
 * The percentile file is updated when this method is invoked, so 
 * the value only updates when new values are pushed.
*/
void add_measurement(long lat_ns){
    Measurement m;
    m.lat_ns = lat_ns;
    // get current timestamp in seconds
    m.timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();

    // Remove old measurements
    while(measurements.size > 0){
        Measurement oldest = measurements.get(0);
        if(oldest.timestamp < m.timestamp - window_seconds * 1000){
            measurements.pop();
        } else{
            break;
        }
    }

    measurements.push(m);

    if(last_write == 0){
        // Write only after accruing one window of data
        last_write = m.timestamp;
    }

    if(m.timestamp >= last_write + window_seconds * 1000){
        std::cerr << "writing percentile to file at time: " << m.timestamp << std::endl;
        write_percentile();
        last_write = m.timestamp;
    }
}

protected:
    void write_percentile(){
        long percentile = get_percentile();
        std::ofstream file(output_file);
        if (!file.is_open()){
            throw std::runtime_error("Could not open file " + output_file + " for writing");
            return;
        }
        file << percentile;
        std::cerr << "wrote percentile: " << percentile << std::endl;
        file.close();
    }

    // Can be optimized but whatever for now
    long get_percentile(){
        std::vector<long> values;
        for(int i = 0; i < measurements.size; i++){
            values.push_back(measurements.get(i).lat_ns);
        }

        std::sort(values.begin(), values.end());
        int index = (int)(percentile * values.size());
        return values[index];
    }

    CircularVector<Measurement> measurements;
    double window_seconds;
    double percentile;
    double last_write;
    std::string output_file;
};