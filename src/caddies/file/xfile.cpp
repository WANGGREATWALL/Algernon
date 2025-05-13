#include <fstream>
#include "xfile.h"
#include "logger.h"
#include "xregex.h"

#ifdef __unix__
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>
#else
#include <windows.h>
#endif


namespace file {

    bool exists(const std::string filename)
    {
        std::ifstream f(filename);
        return f ? true : false;
    }

    bool isDirectory(const std::string& dir)
    {
#ifdef __unix__
        struct stat path_stat;
        if (stat(dir.c_str(), &path_stat) == -1) {
            return false;
        }
        return S_ISDIR(path_stat.st_mode);
#else
        DWORD fileAttr = GetFileAttributes(dir.c_str());
        return (fileAttr != INVALID_FILE_ATTRIBUTES) && (fileAttr & FILE_ATTRIBUTE_DIRECTORY);
#endif
    }

    int createDirectory(const std::string& dir)
    {
#ifdef __unix__
        mode_t mode = 0755; // permission setting
        if (mkdir(dir.c_str(), mode) == 0) {
            return NO_ERROR;
        } else if (errno == EEXIST) {
            return NO_ERROR;
        }
#else
        if (CreateDirectory(dir.c_str(), NULL) || GetLastError() == ERROR_ALREADY_EXISTS) {
            return NO_ERROR;
        }
#endif
        return ERROR_NOT_SUPPORTED;
    }


    int XFile::loadFileToBuffer(const std::string& filename, memory::XBuffer<char>& buffer)
    {
        ASSERTER_WITH_RET(exists(filename), ERROR_FILE_NOT_FOUND);

        std::ifstream f(filename, std::ios::binary);

        f.seekg(0, std::ios::end);
        auto size = f.tellg();
        buffer = memory::XBuffer<char>(size);

        f.seekg(0);
        f.read(buffer.get(), size);
        f.close();

        return NO_ERROR;
    }

    int XFile::saveBufferToFile(const memory::XBuffer<char>& buffer, const std::string& filename)
    {
        ASSERTER_WITH_RET(buffer.sizeByByte() > 0, ERROR_INVALID_PARAMETER);

        std::ofstream file(filename, std::fstream::out | std::fstream::binary);
        ASSERTER_WITH_RET(file.is_open(), ERROR_OPEN_FAILED);

        file.write(buffer.get(), buffer.sizeByByte());
        file.close();

        return NO_ERROR;
    }

    int XFile::saveBufferToFile(const std::string& content, const std::string& filename)
    {
        ASSERTER_WITH_RET(!content.empty(), ERROR_INVALID_PARAMETER);

        std::ofstream file(filename, std::fstream::out | std::fstream::binary);
        ASSERTER_WITH_RET(file.is_open(), ERROR_OPEN_FAILED);

        file.write(content.data(), content.size());
        file.close();

        return NO_ERROR;
    }


    std::vector<std::string> XFilelistMaker::getFullListIn(const std::string& folder)
    {
        std::vector<std::string> list;
#ifdef __unix__
        DIR* dir;
        struct dirent* ent;

        if ((dir = opendir(folder.c_str())) != NULL) {
            while ((ent = readdir(dir)) != NULL) {
                if (ent->d_type == DT_REG) {
                    list.emplace_back(ent->d_name);
                }
            }
            closedir(dir);
        }
#else
        std::wstring wFolder(folder.size(), L' ');
        std::copy(folder.begin(), folder.end(), wFolder.begin());

        WIN32_FIND_DATA findFileData;
        HANDLE hFind = FindFirstFile((folder + "\\*").c_str(), &findFileData);

        if (hFind == INVALID_HANDLE_VALUE) {
            return list;
        }

        do {
            if (!(findFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
                list.emplace_back(findFileData.cFileName);
            }
        } while (FindNextFile(hFind, &findFileData) != 0);

        FindClose(hFind);
#endif

        for (auto& name : list) {
            std::string path = folder;
            if (folder.back() != '/')
                path += "/";
            name = path + name;
        }

        return list;
    }

    std::vector<std::string> XFilelistMaker::getFilelistFilteredByRegexIn(const std::string& folder, const std::string& regex)
    {
        std::vector<std::string> listFull = getFullListIn(folder);
        std::vector<std::string> filtered;

        for (auto name : listFull) {
            if (re::matchRegexInString(name, regex)) {
                filtered.push_back(name);
            }
        }

        return filtered;
    }

    std::string XFilenameMaker::getFolder(const std::string& filename)
    {
        size_t pos = filename.rfind('/'); // find last '/'

        std::string folder = filename.substr(0, pos);
        if (folder.size() == 0) {
            folder = "./";
        } else if (folder.back() != '/') {
            folder += '/';
        }

        return folder;
    }

    std::string XFilenameMaker::eliminatePath(const std::string& filename)
    {
        size_t pos = filename.rfind('/'); // find last '/'
        std::string name = filename.substr(pos + 1);
        return name;
    }

    std::string XFilenameMaker::eliminatePathAndFormat(const std::string& filename)
    {
        std::string nameWithFormat = eliminatePath(filename);
        std::string nameWithoutFormat = nameWithFormat.substr(0, nameWithFormat.rfind('.')); // eliminate last '.' and rest
        return nameWithoutFormat;
    }

    std::string XFilenameMaker::eliminatePathAndBackpartOfFirstFoundSize(const std::string& filename)
    {
        std::string nameWithoutPath = eliminatePath(filename);

        std::string str = re::getFirstMatchInString(nameWithoutPath, "_[0-9]{1,5}x[0-9]{1,5}");
        if (str.empty()) {
            LOGGER_W("failed to find size info in filename(%s)\n", filename.c_str());
            return eliminatePathAndFormat(filename);
        }

        return nameWithoutPath.substr(0, nameWithoutPath.find(str));
    }

    std::string XFilenameMaker::eliminatePathAndBackpartOfLastFoundSize(const std::string& filename)
    {
        std::string nameWithoutPath = eliminatePath(filename);

        std::string str = re::getLastMatchInString(nameWithoutPath, "_[0-9]{1,5}x[0-9]{1,5}");
        if (str.empty()) {
            LOGGER_W("failed to find size info in filename(%s)\n", filename.c_str());
            return eliminatePathAndFormat(filename);
        }

        return nameWithoutPath.substr(0, nameWithoutPath.rfind(str));
    }

    XSizeImage XFilenameMaker::getFirstFoundImageSize(const std::string& filename)
    {
        std::string nameWithoutPath = eliminatePath(filename);

        std::string str = re::getFirstMatchInString(nameWithoutPath, "[0-9]{1,5}x[0-9]{1,5}");
        if (str.empty()) {
            LOGGER_W("failed to find size info in filename(%s)\n", filename.c_str());
            return XSizeImage{0, 0};
        }

        uint32_t width = std::stoi(re::getFirstMatchInString(str, "[0-9]{1,5}"));
        uint32_t height = std::stoi(re::getLastMatchInString(str, "[0-9]{1,5}"));

        return XSizeImage{width, height};
    }

    XSizeImage XFilenameMaker::getLastFoundImageSize(const std::string& filename)
    {
        std::string nameWithoutPath = eliminatePath(filename);

        std::string str = re::getLastMatchInString(nameWithoutPath, "[0-9]{1,5}x[0-9]{1,5}");
        if (str.empty()) {
            LOGGER_W("failed to find size info in filename(%s)\n", filename.c_str());
            return XSizeImage{0, 0};
        }
        
        uint32_t width = std::stoi(re::getFirstMatchInString(str, "[0-9]{1,5}"));
        uint32_t height = std::stoi(re::getLastMatchInString(str, "[0-9]{1,5}"));

        return XSizeImage{width, height};
    }

    std::string XFilenameMaker::getFirstMatchedPartByRegex(const std::string& filename, const std::string& regex)
    {
        return re::getFirstMatchInString(filename, regex);
    }

    std::string XFilenameMaker::getLastMatchedPartByRegex(const std::string& filename, const std::string& regex)
    {
        return re::getLastMatchInString(filename, regex);
    }

} // namespace file