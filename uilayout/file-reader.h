//
// Created by baifeng on 2022/1/11.
//

#ifndef SDL2_UI_FILE_READER_H
#define SDL2_UI_FILE_READER_H

#include <memory>
#include <string>
#include <map>

namespace ui {

    class FileReader;
    class FileData final {
        friend class FileReader;
    public:
        FileData(uint32_t size);
        ~FileData();
    public:
        unsigned char* data() const;
        uint32_t size() const;
        std::string const& name() const;
        bool empty() const;
    private:
        void resize(uint32_t size);
        void free();
    private:
        uint32_t _size;
        unsigned char* _buffer;
        std::string _name;
    };

    class FileReader {
    public:
        typedef std::shared_ptr<FileData> Data;
    public:
        virtual ~FileReader() {}
        virtual Data getData(std::string const& name) = 0;
    };

    class DiskFileReader : public FileReader {
    public:
        Data getData(std::string const& name) override;
    };
}

#endif //SDL2_UI_FILE_READER_H
