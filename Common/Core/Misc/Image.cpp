#pragma once
#include "pch.h"
#include "Image.h"
#define STB_IMAGE_IMPLEMENTATION

#include "./stb/stb_image.h"


using namespace NCL;

Image::Image(const std::filesystem::path& path, int desiredChannels) {
    LoadInternal(path.string().c_str(), desiredChannels, false);
}

Image::Image(Image&& other) noexcept
    : data(std::move(other.data)),
    width(std::exchange(other.width, 0)),
    height(std::exchange(other.height, 0)),
    channels(std::exchange(other.channels, 0)) {
    other.Nullify();
}

Image& Image::operator=(Image&& other) noexcept {
    if (this != &other) {
        this->Free();
        data = std::move(other.data); 
        width = std::exchange(other.width, 0);
        height = std::exchange(other.height, 0);
        channels = std::exchange(other.channels, 0);
        other.Nullify();
    }
    return *this;
}

void Image::LoadInternal(const char* filename, int desiredChannels, bool loadAsFloat) {
    int w = 0, h = 0, c = 0;
    if (loadAsFloat) {
        data = stbi_loadf(filename, &w, &h, &c, desiredChannels);  //4 forces this to always be rgba!
    }
    else {
        data = stbi_load(filename, &w, &h, &c, desiredChannels);
    }
    if (!IsValid()) {
        LOG_ERROR("Failed to load image from file");
    }
    width = w;
    height = h;
    channels = (desiredChannels > 0 ? desiredChannels : c);
}

void Image::Nullify() noexcept {
    std::visit([](auto*& ptr) {
        ptr = nullptr;
    }, data);
}