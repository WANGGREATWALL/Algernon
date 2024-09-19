#ifndef __XFILE_H__
#define __XFILE_H__

#include <string>
#include <vector>
#include "xbuffer.h"


namespace file {

    bool exists(const std::string filename);

    bool isDirectory(const std::string& dir);

    int createDirectory(const std::string& dir);


    class XFile {
    public:
        static int loadFileToBuffer(const std::string& filename, memory::XBuffer<char>& buffer);
        static int saveBufferToFile(const memory::XBuffer<char>& buffer, const std::string& filename);
        static int saveBufferToFile(const std::string& content, const std::string& filename);
    };


    class XFilelistMaker {
    public:
        /**
         * @return name with folder
         */
        static std::vector<std::string> getFullListIn(const std::string& folder);
        static std::vector<std::string> getFilelistFilteredByRegexIn(const std::string& folder, const std::string& regex);
    };


    struct XSizeImage {
        uint32_t width;
        uint32_t height;
    };

    class XFilenameMaker {
    public:
        static std::string getFolder(const std::string& filename);

        static std::string eliminatePath(const std::string& filename);
        static std::string eliminatePathAndFormat(const std::string& filename);
        static std::string eliminatePathAndBackpartOfFirstFoundSize(const std::string& filename);
        static std::string eliminatePathAndBackpartOfLastFoundSize(const std::string& filename);

        static XSizeImage getFirstFoundImageSize(const std::string& filename);
        static XSizeImage getLastFoundImageSize(const std::string& filename);

        static std::string getFirstMatchedPartByRegex(const std::string& filename, const std::string& regex);
        static std::string getLastMatchedPartByRegex(const std::string& filename, const std::string& regex);
    };

} // namespace file


#endif // __XFILE_H__