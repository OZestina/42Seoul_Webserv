#pragma once
#ifndef INPUTBUFFER_HPP
#define INPUTBUFFER_HPP

#include <string>

using namespace std;

class InputBuffer : public string
{
    public:
        InputBuffer();
        ~InputBuffer();

        void            reset();
        void            reset(size_t index);
        char*           getCharPointer();
        void            updateIndex();
        void            updateIndex(size_t mIndex);

        size_t          getIndex();

    private:
        size_t  mIndex;
};

#endif // INPUTBUFFER_HPP
