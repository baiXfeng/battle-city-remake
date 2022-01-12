//
// Created by baifeng on 2022/1/11.
//

#include "file-reader.h"
#include <stdio.h>
#include <string.h>

namespace ui {

    FileData::FileData(uint32_t size):_buffer(nullptr), _size(0) {
        resize(size);
    }

    FileData::~FileData() {
        free();
        _name.clear();
    }

    unsigned char* FileData::data() const {
        return _buffer;
    }

    uint32_t FileData::size() const {
        return _size;
    }

    std::string const& FileData::name() const {
        return _name;
    }

    bool FileData::empty() const {
        return _buffer == nullptr;
    }

    void FileData::resize(uint32_t size) {
        free();
        _size = size;
        _buffer = (unsigned char*)malloc(size);
        memset(_buffer, 0, size);
    }

    void FileData::free() {
        if (_buffer) {
            ::free(_buffer);
            _buffer = nullptr;
        }
        _size = 0;
    }

    DiskFileReader::Data DiskFileReader::getData(std::string const& name) {
        FILE* fp = fopen(name.c_str(), "rb");
        if (fp == NULL) {
            return Data();
        }

        fseek(fp, 0, SEEK_END);

        auto size = (uint32_t)ftell(fp);
        Data ret(new FileData(size));

        fseek(fp, 0, SEEK_SET);
        fread(ret->data(), size, 1, fp);
        fclose(fp);

        (std::string&)ret->name() = name;
        return ret;
    }
}