#pragma once
#include "pch.h"
#include "Image.h"

using namespace NCL;

Image::Image(Image&& other) noexcept
    : data(std::exchange(other.data, nullptr)),
    width(std::exchange(other.width, 0)),
    height(std::exchange(other.height, 0)),
    channels(std::exchange(other.channels, 0)) {
}

Image& Image::operator=(Image&& other) noexcept {
    if (this != &other) {
        this->Free();
        std::swap(data, other.data);
        std::swap(width, other.width);
        std::swap(height, other.height);
        std::swap(channels, other.channels);
    }
    return *this;
}

