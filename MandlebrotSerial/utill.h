#pragma once
#include <chrono>
#include <string>
#include <iostream>

class MeasureExecution
{
public:
    MeasureExecution() :_start(std::chrono::high_resolution_clock::now())
    {
    }

    MeasureExecution(std::string&& text) :
        _start(std::chrono::high_resolution_clock::now()),
        _prefixText(text)
    {

    }

    ~MeasureExecution()
    {
        auto end = std::chrono::high_resolution_clock::now();

        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - _start).count();
        _prefixText += "  ";
        std::cout << "\n\n" << _prefixText << duration << " ms \n\n";
    }

private:
    std::chrono::high_resolution_clock::time_point _start;
    std::string _prefixText{ "EXECUTION time  " };
};