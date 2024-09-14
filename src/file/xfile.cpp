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
            return ECODE_SUCCESS;
        } else if (errno == EEXIST) {
            return ECODE_SUCCESS;
        }
#else
        if (CreateDirectory(dir.c_str(), NULL) || GetLastError() == ERROR_ALREADY_EXISTS) {
            return ECODE_SUCCESS;
        }
#endif
        return ECODE_UNSUPPORTED;
    }


    XFile::XFile(const std::string filename)
    {
        ASSERTER(exists(filename));
        ASSERTER(reopen(filename) == ECODE_SUCCESS);
    }

    memory::XBuffer<char> XFile::getBuffer() const
    {
        return mData;
    }

    int XFile::reopen(const std::string filename)
    {
        ASSERTER_WITH_RET(exists(filename), ECODE_FILE_NOT_EXIST);
        std::ifstream f(filename, std::ios::binary);

        f.seekg(0, std::ios::end);
        mSize = f.tellg();
        mData = memory::XBuffer<char>(mSize);

        LOGGER_E("mSize = %lu\n", mSize);

        f.seekg(0);
        f.read(mData.get(), mSize);
        f.close();

        return ECODE_SUCCESS;
    }

    void XFile::clear()
    {
        mData.clear();
        mSize = 0;
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

        XRegex re(regex);
        for (auto name : listFull) {
            if (re.matchable(name)) {
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

        XRegex regex("_[0-9]{1,5}x[0-9]{1,5}");
        std::string str = regex.findFirstOneIn(nameWithoutPath);
        if (str.empty()) {
            LOGGER_W("failed to find size info in filename(%s)\n", filename.c_str());
            return eliminatePathAndFormat(filename);
        }

        return nameWithoutPath.substr(0, nameWithoutPath.find(str));
    }

    std::string XFilenameMaker::eliminatePathAndBackpartOfLastFoundSize(const std::string& filename)
    {
        std::string nameWithoutPath = eliminatePath(filename);

        XRegex regex("_[0-9]{1,5}x[0-9]{1,5}");
        std::string str = regex.findLastOneIn(nameWithoutPath);
        if (str.empty()) {
            LOGGER_W("failed to find size info in filename(%s)\n", filename.c_str());
            return eliminatePathAndFormat(filename);
        }

        return nameWithoutPath.substr(0, nameWithoutPath.rfind(str));
    }

    XSizeImage XFilenameMaker::getFirstFoundImageSize(const std::string& filename)
    {
        std::string nameWithoutPath = eliminatePath(filename);

        XRegex regex("[0-9]{1,5}x[0-9]{1,5}");
        std::string str = regex.findFirstOneIn(nameWithoutPath);
        if (str.empty()) {
            LOGGER_W("failed to find size info in filename(%s)\n", filename.c_str());
            return XSizeImage{0, 0};
        }

        regex.reset("[0-9]{1,5}");
        uint32_t width = std::stoi(regex.findFirstOneIn(str));
        uint32_t height = std::stoi(regex.findLastOneIn(str));

        return XSizeImage{width, height};
    }

    XSizeImage XFilenameMaker::getLastFoundImageSize(const std::string& filename)
    {
        std::string nameWithoutPath = eliminatePath(filename);

        XRegex regex("[0-9]{1,5}x[0-9]{1,5}");
        std::string str = regex.findLastOneIn(nameWithoutPath);
        if (str.empty()) {
            LOGGER_W("failed to find size info in filename(%s)\n", filename.c_str());
            return XSizeImage{0, 0};
        }

        regex.reset("[0-9]{1,5}");
        uint32_t width = std::stoi(regex.findFirstOneIn(str));
        uint32_t height = std::stoi(regex.findLastOneIn(str));

        return XSizeImage{width, height};
    }

    std::string XFilenameMaker::getFirstMatchedPartByRegex(const std::string& filename, const std::string& regex)
    {
        XRegex re(regex);
        return re.findFirstOneIn(filename);
    }

    std::string XFilenameMaker::getLastMatchedPartByRegex(const std::string& filename, const std::string& regex)
    {
        XRegex re(regex);
        return re.findLastOneIn(filename);
    }

} // namespace file