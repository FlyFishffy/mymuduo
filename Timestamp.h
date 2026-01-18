#pragma once

#include <iostream>
#include <string>

class Timestamp
{
public:
    Timestamp();
    // 禁止隐式转换
    explicit Timestamp(int64_t microSecondsSinceEpoch);
    static Timestamp now();
    std::string toString() const;
private:
    int64_t microSecondsSinceEpoch_;
};
