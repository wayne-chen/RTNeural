#ifndef DENSE_H_INCLUDED
#define DENSE_H_INCLUDED

#include <algorithm>
#include <numeric>
#include <vector>

#if defined(USE_EIGEN)
#include "dense_eigen.h"
#elif defined(USE_XSIMD)
#include "dense_xsimd.h"
#elif defined(USE_ACCELERATE)
#include "dense_accelerate.h"
#else
#include "../Layer.h"

namespace RTNeural
{

template <typename T>
class Dense1
{
public:
    Dense1(size_t in_size)
        : in_size(in_size)
    {
        weights = new T[in_size];
    }

    ~Dense1() { delete[] weights; }

    inline T forward(const T* input)
    {
        return std::inner_product(weights, weights + in_size, input, (T)0) + bias;
    }

    void setWeights(const T* newWeights)
    {
        for(size_t i = 0; i < in_size; ++i)
            weights[i] = newWeights[i];
    }

    void setBias(T b) { bias = b; }

    T getWeight(size_t i) const noexcept { return weights[i]; }

    T getBias() const noexcept { return bias; }

private:
    const size_t in_size;
    T bias;

    T* weights;
};

template <typename T>
class Dense : public Layer<T>
{
public:
    Dense(size_t in_size, size_t out_size)
        : Layer<T>(in_size, out_size)
    {
        subLayers = new Dense1<T>*[out_size];
        for(size_t i = 0; i < out_size; ++i)
            subLayers[i] = new Dense1<T>(in_size);
    }

    Dense(std::initializer_list<size_t> sizes)
        : Dense(*sizes.begin(), *(sizes.begin() + 1))
    {
    }

    Dense(const Dense& other)
        : Dense(other.in_size, other.out_size)
    {
    }

    Dense& operator=(const Dense& other)
    {
        return *this = Dense(other);
    }

    virtual ~Dense()
    {
        for(size_t i = 0; i < Layer<T>::out_size; ++i)
            delete subLayers[i];

        delete[] subLayers;
    }

    inline void forward(const T* input, T* out) override
    {
        for(size_t i = 0; i < Layer<T>::out_size; ++i)
            out[i] = subLayers[i]->forward(input);
    }

    void setWeights(const std::vector<std::vector<T>>& newWeights)
    {
        for(size_t i = 0; i < Layer<T>::out_size; ++i)
            subLayers[i]->setWeights(newWeights[i].data());
    }

    void setWeights(T** newWeights)
    {
        for(size_t i = 0; i < Layer<T>::out_size; ++i)
            subLayers[i]->setWeights(newWeights[i]);
    }

    void setBias(T* b)
    {
        for(size_t i = 0; i < Layer<T>::out_size; ++i)
            subLayers[i]->setBias(b[i]);
    }

    T getWeight(size_t i, size_t k) const noexcept
    {
        return subLayers[i]->getWeight(k);
    }

    T getBias(size_t i) const noexcept { return subLayers[i]->getBias(); }

private:
    Dense1<T>** subLayers;
};

} // namespace RTNeural

#endif // USE_EIGEN

#endif // DENSE_H_INCLUDED
