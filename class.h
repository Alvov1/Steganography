//
// Created by Alexander on 1/8/2021.
//

#include <vector>
#include <cmath>
#ifndef P5_CLASS_H
#define P5_CLASS_H

class numberWithBites {
public:
    numberWithBites(){
        this->number = 0;
        this->given = 0;
        this->got = 0;

        for(int i = 0; i < this->size; i++)
            this->array.push_back(0);
    }

    /* Changes size of this object in bites. */
    void resize(const unsigned newSize = 0){
        this->size = newSize;
        this->array.clear();
        for(int i = 0; i < newSize; i++)
            this->array.push_back(0);
    }

    /* Sets values of this object relatively to NUM. */
    int setValue(unsigned num) {
        if (num >= pow(2, this->size))
            return -1;
        this->number = num;
        this->given = 0;
        this->got = 0;
        for (int i = int(this->size - 1); i >= 0; i--) {
            this->array[i] = num % 2;
            num /= 2;
        }
        return 0;
    }

    unsigned giveNumber() const{
        return this->number;
    }

    /* Puts BITES to this->array and counts new this->number. */
    void putBites(const std::vector <unsigned>& bites, const unsigned N){
        for(int i = 0; i < N; i++)
            this->array[this->size - 1 - i - got] = bites[i];

        unsigned sum = 0;
        unsigned power = 1;
        for(int i = int(this->size - 1); i >= 0; i--){
            unsigned temp = this->array[i];
            sum += temp * power;
            power *= 2;
        }
        this->got += N;
        this->number = sum;
    }

    /* Returns pointer to vector with last N bites from this->array. */
    std::vector <unsigned> giveBites(const unsigned N){
        std::vector <unsigned> newArray;
        newArray.reserve(N);
        for(int i = 0; i < N; i++)
            newArray.push_back(this->array[this->size - 1 - i - given]);

        this->given += N;
        return newArray;
    }

    ~numberWithBites(){
        this->given = 0;
        this->got = 0;
        this->number = 0;
        this->array.clear();
    }

private:
    /* Size of the stored number int bites. */
    unsigned size = 8;
    /* Stored number. */
    unsigned number;
    /* Number of bites given to change. */
    unsigned given;
    /* Number of bites have been changed. */
    unsigned got;
    /* Binary values of the number. */
    std::vector <unsigned> array;

};

#endif //P5_CLASS_H
