#ifndef __FILE_SYSTEM_H__
#define __FILE_SYSTEM_H__

#if defined(__linux__)
#define OS_LINUX
#if defined(__ANDROID__)
#define OS_ANDROID
#endif
#elif defined(_WIN64)
#define OS_WINDOWS
#define OS_WIN64
#elif defined(_WIN32)
#define OS_WINDOWS
#define OS_WIN32
#else
#error "Operating system currently not supported!"
#endif

#if (defined(_MSVC_LANG) && _MSVC_LANG >= 201703L)
#if _MSVC_LANG == 201703L
#define CPP_RUNNING_17
#else
#define CPP_RUNNING_20
#endif
#elif (defined(__cplusplus) && __cplusplus >= 201703L)
#if __cplusplus == 201703L
#define CPP_RUNNING_17
#else
#define CPP_RUNNING_20
#endif
#endif

#include <string>
#include <system_error>
#include <chrono>
#include <memory>
#include <stack>
#include <vector>

namespace filesystem {

template <typename CharType>
class PathHelperBase {
public:
    using value_type = CharType;
#ifdef OS_WINDOWS
    static constexpr value_type separator = '\\';
#else
    static constexpr value_type separator = '/';
#endif
};

#if __cplusplus < 201703L
template <typename CharType>
constexpr CharType PathHelperBase<CharType>::separator;
#endif

class Path : private PathHelperBase<std::string::value_type> {
public:
    using PathHelperBase<std::string::value_type>::value_type;
    using StringType = std::basic_string<value_type>;
    using PathHelperBase<value_type>::separator;

    // The path format in which the constructor argument is given.
    enum Format {
        generic_format,  //< The generic format, internally used by
                         //< ghc::filesystem::path with slashes
        native_format,   //< The format native to the current platform this code
                         //< is build for
        auto_format,     //< Try to auto-detect the format, fallback to native
    };

    template <class T>
    struct _is_basic_string : std::false_type {};

    template <class CharT, class Traits, class Alloc>
    struct _is_basic_string<std::basic_string<CharT, Traits, Alloc>> : std::true_type {};

    template <class CharT>
    struct _is_basic_string<std::basic_string<CharT, std::char_traits<CharT>, std::allocator<CharT>>> : std::true_type {};

    template <typename T1, typename T2 = void>
    using path_type = typename std::enable_if<!std::is_same<path, T1>::value, path>::type;

    template <typename T>
    using PathFromString = typename std::enable_if<_is_basic_string<T>::value
        || std::is_same<char const*, typename std::decay<T>::type>::value
        || std::is_same<char*, typename std::decay<T>::type>::value
        || std::is_same<char16_t const*, typename std::decay<T>::type>::value
        || std::is_same<char16_t*, typename std::decay<T>::type>::value
        || std::is_same<char32_t const*, typename std::decay<T>::type>::value
        || std::is_same<char32_t*, typename std::decay<T>::type>::value
        || std::is_same<wchar_t const*, typename std::decay<T>::type>::value
        || std::is_same<wchar*, typename std::decay<T>::type>::value, Path>::type;
    
    template <typename T>
    using PathTypeEcharT = typename std::enable_if<std::is_same<T, char>::value
        || std::is_same<T, char16_t>::value
        || std::is_same<T, char32_t>::value
        || std::is_same<T, wchar_t>::value, Path>::type;

    // constructors and destructor
    Path() noexcept;
    Path(const Path& path);
    Path(Path&& path) noexcept;
    Path(StringType&& source, Format fmt=auto_format);

    template <class Source, typename=PathFromString<Source>>
    Path(const Source& source, Format fmt=auto_format);

    template <class InputIterator>
    Path(InputIterator first, InputIterator last, Format fmt=auto_format);

    ~Path();

    // assignments
    Path& operator=(const Path& path);
    Path& operator=(Path&& path) noexcept;
    Path& operator=(StringType&& source);
    Path& assign(StringType&& source);
    template <class Source>
    Path& operator=(const Source& source);
    template <class Source>
    Path& assign(const Source& source);
    template <class InputIterator>
    Path& assign(InputIterator first, InputIterator last);

    // appends
    Path& operator/=(const Path& path);
    template <class Source>
    Path& operator/=(const Source& source);
    template <class Source>
    Path& append(const Source& source);
    template <class InputInterator>
    Path& append(InputInterator first, InputInterator last);

    // concatenation
    Path& operator+=(const Path& path);
    Path& operator+=(const StringType& path);
    Path& operator+=(const value_type* x);
    Path& operator+=(value_type x);
    template <class Source>
    PathFromString<Source>& operator+=(const Source& x);
    template <class EcharT>
    PathTypeEcharT<EcharT>& operator+=(EcharT x);
    template <class Source>
    Path& concat(const Source& x);
    template <class InputIterator>
    Path& concat(InputIterator first, InputIterator last);

    // modifiers
    void clear() noexcept;
    Path& make_preferred();
    Path& remove_filename();
    Path& replace_filename(const Path& replacement);
    Path& replace_extension(const Path& replacement=Path());
    void swap(Path& rhs) noexcept;

    // native format observers
    const StringType& native() const noexcept;
    const value_type* c_str() const noexcept;
    operator StringType() const;
    template <class EcharT, class traits = std::char_traits<EcharT>, class Allocator = std::allocator<EcharT>>
    std::basic_string<EcharT, traits, Allocator> string(const Allocator& a = Allocator()) const;
    std::string string() const;
    std::wstring wstring() const;
    std::string u8string() const;

    // compare
    int compare(const Path& p) const noexcept;
    int compare(const StringType& s) const;
    int compare(const value_type* s) const;

    // decomposition
    Path root_name() const;
    Path root_directory() const;
    Path root_path() const;
    Path relative_path() const;
    Path parent_path() const;
    Path filename() const;
    Path stem() const;
    Path extension() const;

    // query
    bool empty() const noexcept;
    bool has_root_name() const;
    bool has_root_directory() const;
    bool has_root_path() const;
    bool has_relative_path() const;
    bool has_parent_path() const;
    bool has_filename() const;
    bool has_stem() const;
    bool has_extension() const;
    bool is_absolute() const;
    bool is_relative() const;

    // generation
    Path lexically_normal() const;
    Path lexically_relative(const Path& base) const;
    Path lexically_proximate(const Path& base) const;

    // iterators
    class iterator;
    using const_iterator = iterator;
    iterator begin() const;
    iterator end() const;

private:
    using impl_value_type = value_type;
    using impl_string_type = std::basic_string<impl_value_type>;

    friend class directory_iterator;

    void append_name(const value_type* name);

    static constexpr impl_value_type generic_separator = '/';

    template <typename InputIterator>
    class RangeInputIterator
    {
    public:
        typedef InputIterator iterator;
        typedef InputIterator const_iterator;
        typedef typename InputIterator::difference_type difference_type;

        RangeInputIterator(const InputIterator& first, const InputIterator& last)
            : _first(first)
            , _last(last)
        {
        }

        InputIterator begin() const { return _first; }
        InputIterator end() const { return _last; }

    private:
        InputIterator _first;
        InputIterator _last;
    };

    friend void swap(Path& lhs, Path& rhs) noexcept;
    friend size_t hash_value(const Path& p) noexcept;
    friend Path canonical(const Path& p, std::error_code& ec);
    friend bool create_directories(const Path& p, std::error_code& ec) noexcept;
    StringType::size_type root_name_length() const noexcept;
    void postprocess_path_with_format(Format fmt);
    void check_long_path();
    impl_string_type _path;
#ifdef OS_WINDOWS
    void handle_prefixes();
    // friend bool detail::has_executable_extension(const Path& p);
#ifdef WIN_AUTO_PREFIX_LONG_PATH
    StringType::size_type _prefixLength{0};
#else   // WIN_AUTO_PREFIX_LONG_PATH
    static const StringType::size_type _prefixLength{0};
#endif  // WIN_AUTO_PREFIX_LONG_PATH
#else
    static const StringType::size_type _prefixLength{0};
#endif
};

// Path non-member functions
void swap(Path& lhs, Path& rhs) noexcept;
size_t hash_value(const Path& p) noexcept;
bool operator==(const Path& lhs, const Path& rhs) noexcept;
bool operator!=(const Path& lhs, const Path& rhs) noexcept;
bool operator<(const Path& lhs, const Path& rhs) noexcept;
bool operator<=(const Path& lhs, const Path& rhs) noexcept;
bool operator>(const Path& lhs, const Path& rhs) noexcept;
bool operator>=(const Path& lhs, const Path& rhs) noexcept;
Path operator/(const Path& lhs, const Path& rhs);

// Path inserter and extractor
template <class charT, class traits>
std::basic_ostream<charT, traits>& operator<<(std::basic_ostream<charT, traits>& os, const Path& p);

template <class charT, class traits>
std::basic_istream<charT, traits>& operator>>(std::basic_istream<charT, traits>& is, Path& p);

// Path factory functions
template <class Source, typename = Path::PathFromString<Source>>
#if defined(__cpp_lib_char8_t) && !defined(FILESYSTEM_ENFORCE_CPP17_API)
[[deprecated("use filesystem::Path::Path() with std::u8string instead")]]
#endif
Path u8path(const Source& source);

template <class InputIterator>
#if defined(__cpp_lib_char8_t) && !defined(FILESYSTEM_ENFORCE_CPP17_API)
[[deprecated("use filesystem::Path::Path() with std::u8string instead")]]
#endif
Path u8path(InputIterator first, InputIterator last);

// class filesystem_error
class filesystem_error : public std::system_error
{
public:
    filesystem_error(const std::string& what_arg, std::error_code ec);
    filesystem_error(const std::string& what_arg, const Path& p1, std::error_code ec);
    filesystem_error(const std::string& what_arg, const Path& p1, const Path& p2, std::error_code ec);
    const Path& path1() const noexcept;
    const Path& path2() const noexcept;
    const char* what() const noexcept override;

private:
    std::string _what_arg;
    std::error_code _ec;
    Path _p1, _p2;
};

class Path::iterator
{
public:
    using value_type = const Path;
    using difference_type = std::ptrdiff_t;
    using pointer = const Path*;
    using reference = const Path&;
    using iterator_category = std::bidirectional_iterator_tag;

    iterator();
    iterator(const Path& p, const impl_string_type::const_iterator& pos);
    iterator& operator++();
    iterator operator++(int);
    iterator& operator--();
    iterator operator--(int);
    bool operator==(const iterator& other) const;
    bool operator!=(const iterator& other) const;
    reference operator*() const;
    pointer operator->() const;

private:
    friend class Path;
    impl_string_type::const_iterator increment(const impl_string_type::const_iterator& pos) const;
    impl_string_type::const_iterator decrement(const impl_string_type::const_iterator& pos) const;
    void updateCurrent();

    impl_string_type::const_iterator _first;
    impl_string_type::const_iterator _last;
    impl_string_type::const_iterator _prefix;
    impl_string_type::const_iterator _root;
    impl_string_type::const_iterator _iter;
    Path _current;
};

struct SpaceInfo
{
    uintmax_t capacity;
    uintmax_t free;
    uintmax_t available;
};

enum class FileType
{
    none,
    not_found,
    regular,
    directory,
    symlink,
    block,
    character,
    fifo,
    socket,
    unknown,
};

enum class Perms : uint16_t
{
    none = 0,

    owner_read = 0400,
    owner_write = 0200,
    owner_exec = 0100,
    owner_all = 0700,

    group_read = 040,
    group_write = 020,
    group_exec = 010,
    group_all = 070,

    others_read = 04,
    others_write = 02,
    others_exec = 01,
    others_all = 07,

    all = 0777,
    set_uid = 04000,
    set_gid = 02000,
    sticky_bit = 01000,

    mask = 07777,
    unknown = 0xffff
};

enum class PermOptions : uint16_t
{
    replace = 3,
    add = 1,
    remove = 2,
    nofollow = 4,
};

enum class CopyOptions : uint16_t
{
    none = 0,

    skip_existing = 1,
    overwrite_existing = 2,
    update_existing = 4,

    recursive = 8,

    copy_symlinks = 0x10,
    skip_symlinks = 0x20,

    directories_only = 0x40,
    create_symlinks = 0x80,
};

enum class DirectoryOptions : uint16_t
{
    none = 0,
    follow_directory_symlink = 1,
    skip_permission_denied = 2,
};

// class FileStatus
class FileStatus
{
public:
    FileStatus() noexcept;
    explicit FileStatus(FileType ft, Perms prms=Perms::unknown) noexcept;
    FileStatus(const FileStatus&) noexcept;
    FileStatus(FileStatus&&) noexcept;
    ~FileStatus();

    // assignments:
    FileStatus& operator=(const FileStatus&) noexcept;
    FileStatus& operator=(FileStatus&&) noexcept;

    // modifiers
    void type(FileType ft) noexcept;
    void permissions(Perms prms) noexcept;

    // observers
    FileType type() const noexcept;
    Perms permissions() const noexcept;
    friend bool operator==(const FileStatus& lhs, const FileStatus& rhs) noexcept {
        return lhs.type() == rhs.type() && lhs.permissions() == rhs.permissions();
    }

private:
    FileType _type;
    Perms _perms;
};

using FileTimeType = std::chrono::time_point<std::chrono::system_clock>;

// class DirectoryEntry
class DirectoryEntry
{
public:
    DirectoryEntry() noexcept = default;
    DirectoryEntry(const DirectoryEntry&) = default;
    DirectoryEntry(DirectoryEntry&&) noexcept = default;
#ifdef FILESYSTEM_WITH_EXCEPTIONS
    explicit DirectoryEntry(const Path& p);
#endif
    DirectoryEntry(const Path& p, std::error_code& ec);
    ~DirectoryEntry();

    // assignments:
    DirectoryEntry& operator=(const DirectoryEntry&) = default;
    DirectoryEntry& operator=(DirectoryEntry&&) noexcept = default;

    // modifiers
#ifdef FILESYSTEM_WITH_EXCEPTIONS
    void assign(const Path& p);
    void replace_filename(const Path& p);
    void refresh();
#endif
    void assign(const Path& p, std::error_code& ec);
    void replace_filename(const Path& p, std::error_code& ec);
    void refresh(std::error_code& ec) noexcept;

    // observers
    const filesystem::Path& path() const noexcept;
    operator const filesystem::Path&() const noexcept;
#ifdef FILESYSTEM_WITH_EXCEPTIONS
    bool exists() const;
    bool is_block_file() const;
    bool is_character_file() const;
    bool is_directory() const;
    bool is_fifo() const;
    bool is_other() const;
    bool is_regular_file() const;
    bool is_socket() const;
    bool is_symlink() const;
    uintmax_t file_size() const;
    FileTimeType last_write_time() const;
    FileStatus status() const;
    FileStatus symlink_status() const;
#endif
    bool exists(std::error_code& ec) const noexcept;
    bool is_block_file(std::error_code& ec) const noexcept;
    bool is_character_file(std::error_code& ec) const noexcept;
    bool is_directory(std::error_code& ec) const noexcept;
    bool is_fifo(std::error_code& ec) const noexcept;
    bool is_other(std::error_code& ec) const noexcept;
    bool is_regular_file(std::error_code& ec) const noexcept;
    bool is_socket(std::error_code& ec) const noexcept;
    bool is_symlink(std::error_code& ec) const noexcept;
    uintmax_t file_size(std::error_code& ec) const noexcept;
    FileTimeType last_write_time(std::error_code& ec) const noexcept;
    FileStatus status(std::error_code& ec) const noexcept;
    FileStatus symlink_status(std::error_code& ec) const noexcept;

#ifdef FILESYSTEM_WITH_EXCEPTIONS
    uintmax_t hard_link_count() const;
#else
    uintmax_t hard_link_count(std::error_code& ec) const noexcept;
#endif

    bool operator<(const DirectoryEntry& rhs) const noexcept;
    bool operator==(const DirectoryEntry& rhs) const noexcept;
    bool operator!=(const DirectoryEntry& rhs) const noexcept;
    bool operator<=(const DirectoryEntry& rhs) const noexcept;
    bool operator>(const DirectoryEntry& rhs) const noexcept;
    bool operator>=(const DirectoryEntry& rhs) const noexcept;

private:
    friend class DirectoryIterator;

#ifdef FILESYSTEM_WITH_EXCEPTIONS
    FileType status_file_type() const;
#endif

    FileType status_file_type(std::error_code& ec) const noexcept;
    filesystem::Path _path;
    FileStatus _status;
    FileStatus _symlink_status;
    uintmax_t _file_size = static_cast<uintmax_t>(-1);

#ifndef OS_WINDOWS
    uintmax_t _hard_link_count = static_cast<uintmax_t>(-1);
#endif
    time_t _last_write_time = 0;
};

// class DirectoryIterator
class DirectoryIterator
{
public:
    class Proxy
    {
    public:
        const DirectoryEntry& operator*() const& noexcept { return _dir_entry; }
        DirectoryEntry operator*() && noexcept { return std::move(_dir_entry); }

    private:
        explicit Proxy(const DirectoryEntry& dir_entry)
            : _dir_entry(dir_entry)
        {
        }
        friend class DirectoryIterator;
        friend class recursive_directory_iterator;
        DirectoryEntry _dir_entry;
    };

    using iterator_category = std::input_iterator_tag;
    using value_type = DirectoryEntry;
    using difference_type = std::ptrdiff_t;
    using pointer = const DirectoryEntry*;
    using reference = const DirectoryEntry&;

    // member functions
    DirectoryIterator() noexcept;
#ifdef FILESYSTEM_WITH_EXCEPTIONS
    explicit DirectoryIterator(const Path& p);
    DirectoryIterator(const Path& p, directory_options options);
#endif
    DirectoryIterator(const Path& p, std::error_code& ec) noexcept;
    DirectoryIterator(const Path& p, DirectoryOptions options, std::error_code& ec) noexcept;
    DirectoryIterator(const DirectoryIterator& rhs);
    DirectoryIterator(DirectoryIterator&& rhs) noexcept;
    ~DirectoryIterator();

    DirectoryIterator& operator=(const DirectoryIterator& rhs);
    DirectoryIterator& operator=(DirectoryIterator&& rhs) noexcept;

    const DirectoryEntry& operator*() const;
    const DirectoryEntry* operator->() const;

#ifdef FILESYSTEM_WITH_EXCEPTIONS
    DirectoryIterator& operator++();
#endif
    DirectoryIterator& increment(std::error_code& ec) noexcept;

    // other members as required by [input.iterators]
#ifdef FILESYSTEM_WITH_EXCEPTIONS
    Proxy operator++(int)
    {
        Proxy p{**this};
        ++*this;
        return p;
    }
#endif
    bool operator==(const DirectoryIterator& rhs) const;
    bool operator!=(const DirectoryIterator& rhs) const;

private:
    friend class recursive_directory_iterator;
    class impl;
    std::shared_ptr<impl> _impl;
};

// DirectoryIterator non-member functions
DirectoryIterator begin(DirectoryIterator iter) noexcept;
DirectoryIterator end(const DirectoryIterator&) noexcept;

// class RecursiveDirectoryIterator
class RecursiveDirectoryIterator
{
public:
    using iterator_category = std::input_iterator_tag;
    using value_type = DirectoryEntry;
    using difference_type = std::ptrdiff_t;
    using pointer = const DirectoryEntry*;
    using reference = const DirectoryEntry&;

    // constructors and destructor
    RecursiveDirectoryIterator() noexcept;
#ifdef FILESYSTEM_WITH_EXCEPTIONS
    explicit RecursiveDirectoryIterator(const Path& p);
    RecursiveDirectoryIterator(const Path& p, DirectoryOptions options);
#endif
    RecursiveDirectoryIterator(const Path& p, DirectoryOptions options, std::error_code& ec) noexcept;
    RecursiveDirectoryIterator(const Path& p, std::error_code& ec) noexcept;
    RecursiveDirectoryIterator(const RecursiveDirectoryIterator& rhs);
    RecursiveDirectoryIterator(RecursiveDirectoryIterator&& rhs) noexcept;
    ~RecursiveDirectoryIterator();

    // observers
    DirectoryOptions options() const;
    int depth() const;
    bool recursion_pending() const;

    const DirectoryEntry& operator*() const;
    const DirectoryEntry* operator->() const;

    // modifiers RecursiveDirectoryIterator&
    RecursiveDirectoryIterator& operator=(const RecursiveDirectoryIterator& rhs);
    RecursiveDirectoryIterator& operator=(RecursiveDirectoryIterator&& rhs) noexcept;
#ifdef FILESYSTEM_WITH_EXCEPTIONS
    RecursiveDirectoryIterator& operator++();
#endif
    RecursiveDirectoryIterator& increment(std::error_code& ec) noexcept;

#ifdef FILESYSTEM_WITH_EXCEPTIONS
    void pop();
#endif
    void pop(std::error_code& ec);
    void disable_recursion_pending();

    // other members as required by [input.iterators]
#ifdef FILESYSTEM_WITH_EXCEPTIONS
    DirectoryIterator::Proxy operator++(int)
    {
        DirectoryIterator::Proxy proxy{**this};
        ++*this;
        return proxy;
    }
#endif
    bool operator==(const RecursiveDirectoryIterator& rhs) const;
    bool operator!=(const RecursiveDirectoryIterator& rhs) const;

private:
    struct recursive_directory_iterator_impl
    {
        DirectoryOptions _options;
        bool _recursion_pending;
        std::stack<DirectoryIterator> _dir_iter_stack;
        recursive_directory_iterator_impl(DirectoryOptions options, bool recursion_pending)
            : _options(options)
            , _recursion_pending(recursion_pending)
        {
        }
    };
    std::shared_ptr<recursive_directory_iterator_impl> _impl;
};

// DirectoryIterator non-member functions
RecursiveDirectoryIterator begin(RecursiveDirectoryIterator iter) noexcept;
RecursiveDirectoryIterator end(const RecursiveDirectoryIterator&) noexcept;

// filesystem operations
#ifdef FILESYSTEM_WITH_EXCEPTIONS
Path absolute(const Path& p);
Path canonical(const Path& p);
void copy(const Path& from, const Path& to);
void copy(const Path& from, const Path& to, CopyOptions options);
bool copy_file(const Path& from, const Path& to);
bool copy_file(const Path& from, const Path& to, CopyOptions option);
void copy_symlink(const Path& existing_symlink, const Path& new_symlink);
bool create_directories(const Path& p);
bool create_directory(const Path& p);
bool create_directory(const Path& p, const Path& attributes);
void create_directory_symlink(const Path& to, const Path& new_symlink);
void create_symlink(const Path& to, const Path& new_symlink);
Path current_path();
void current_path(const Path& p);
bool exists(const Path& p);
bool equivalent(const Path& p1, const Path& p2);
uintmax_t file_size(const Path& p);
bool is_block_file(const Path& p);
bool is_character_file(const Path& p);
bool is_directory(const Path& p);
bool is_empty(const Path& p);
bool is_fifo(const Path& p);
bool is_other(const Path& p);
bool is_regular_file(const Path& p);
bool is_socket(const Path& p);
bool is_symlink(const Path& p);
FileTimeType last_write_time(const Path& p);
void last_write_time(const Path& p, FileTimeType new_time);
void permissions(const Path& p, Perms prms, PermOptions opts=PermOptions::replace);
Path proximate(const Path& p, const Path& base = current_path());
Path read_symlink(const Path& p);
Path relative(const Path& p, const Path& base = current_path());
bool remove(const Path& p);
uintmax_t remove_all(const Path& p);
void rename(const Path& from, const Path& to);
void resize_file(const Path& p, uintmax_t size);
SpaceInfo space(const Path& p);
FileStatus status(const Path& p);
FileStatus symlink_status(const Path& p);
Path temp_directory_path();
Path weakly_canonical(const Path& p);
#endif
Path absolute(const Path& p, std::error_code& ec);
Path canonical(const Path& p, std::error_code& ec);
void copy(const Path& from, const Path& to, std::error_code& ec) noexcept;
void copy(const Path& from, const Path& to, CopyOptions options, std::error_code& ec) noexcept;
bool copy_file(const Path& from, const Path& to, std::error_code& ec) noexcept;
bool copy_file(const Path& from, const Path& to, CopyOptions option, std::error_code& ec) noexcept;
void copy_symlink(const Path& existing_symlink, const Path& new_symlink, std::error_code& ec) noexcept;
bool create_directories(const Path& p, std::error_code& ec) noexcept;
bool create_directory(const Path& p, std::error_code& ec) noexcept;
bool create_directory(const Path& p, const Path& attributes, std::error_code& ec) noexcept;
void create_directory_symlink(const Path& to, const Path& new_symlink, std::error_code& ec) noexcept;
void create_symlink(const Path& to, const Path& new_symlink, std::error_code& ec) noexcept;
Path current_path(std::error_code& ec);
void current_path(const Path& p, std::error_code& ec) noexcept;
bool exists(FileStatus s) noexcept;
bool exists(const Path& p, std::error_code& ec) noexcept;
bool equivalent(const Path& p1, const Path& p2, std::error_code& ec) noexcept;
uintmax_t file_size(const Path& p, std::error_code& ec) noexcept;
bool is_block_file(FileStatus s) noexcept;
bool is_block_file(const Path& p, std::error_code& ec) noexcept;
bool is_character_file(FileStatus s) noexcept;
bool is_character_file(const Path& p, std::error_code& ec) noexcept;
bool is_directory(FileStatus s) noexcept;
bool is_directory(const Path& p, std::error_code& ec) noexcept;
bool is_empty(const Path& p, std::error_code& ec) noexcept;
bool is_fifo(FileStatus s) noexcept;
bool is_fifo(const Path& p, std::error_code& ec) noexcept;
bool is_other(FileStatus s) noexcept;
bool is_other(const Path& p, std::error_code& ec) noexcept;
bool is_regular_file(FileStatus s) noexcept;
bool is_regular_file(const Path& p, std::error_code& ec) noexcept;
bool is_socket(FileStatus s) noexcept;
bool is_socket(const Path& p, std::error_code& ec) noexcept;
bool is_symlink(FileStatus s) noexcept;
bool is_symlink(const Path& p, std::error_code& ec) noexcept;
FileTimeType last_write_time(const Path& p, std::error_code& ec) noexcept;
void last_write_time(const Path& p, FileTimeType new_time, std::error_code& ec) noexcept;
void permissions(const Path& p, Perms prms, std::error_code& ec) noexcept;
void permissions(const Path& p, Perms prms, PermOptions opts, std::error_code& ec) noexcept;
Path proximate(const Path& p, std::error_code& ec);
Path proximate(const Path& p, const Path& base, std::error_code& ec);
Path read_symlink(const Path& p, std::error_code& ec);
Path relative(const Path& p, std::error_code& ec);
Path relative(const Path& p, const Path& base, std::error_code& ec);
bool remove(const Path& p, std::error_code& ec) noexcept;
uintmax_t remove_all(const Path& p, std::error_code& ec) noexcept;
void rename(const Path& from, const Path& to, std::error_code& ec) noexcept;
void resize_file(const Path& p, uintmax_t size, std::error_code& ec) noexcept;
SpaceInfo space(const Path& p, std::error_code& ec) noexcept;
FileStatus status(const Path& p, std::error_code& ec) noexcept;
bool status_known(FileStatus s) noexcept;
FileStatus symlink_status(const Path& p, std::error_code& ec) noexcept;
Path temp_directory_path(std::error_code& ec) noexcept;
Path weakly_canonical(const Path& p, std::error_code& ec) noexcept;

#ifdef FILESYSTEM_WITH_EXCEPTIONS
void create_hard_link(const Path& to, const Path& new_hard_link);
uintmax_t hard_link_count(const Path& p);
#endif
void create_hard_link(const Path& to, const Path& new_hard_link, std::error_code& ec) noexcept;
uintmax_t hard_link_count(const Path& p, std::error_code& ec) noexcept;

// Non-C++17 add-on std::fstream wrappers with Path
template <class charT, class traits = std::char_traits<charT>>
class BasicFilebuf : public std::basic_filebuf<charT, traits>
{
public:
    BasicFilebuf() {}
    ~BasicFilebuf() override {}

    BasicFilebuf(const BasicFilebuf&) = delete;
    const BasicFilebuf& operator=(const BasicFilebuf&) = delete;

    BasicFilebuf<charT, traits>* open(const Path& p, std::ios_base::openmode mode)
    {
        return std::basic_filebuf<charT, traits>::open(p.string().c_str(), mode) ? this : 0;
    }
};

template <class charT, class traits = std::char_traits<charT>>
class BasicIfstream : public std::basic_ifstream<charT, traits>
{
public:
    BasicIfstream() {}

    explicit BasicIfstream(const Path& p, std::ios_base::openmode mode=std::ios_base::in)
        : std::basic_ifstream<charT, traits>(p.string().c_str(), mode)
    {
    }

    void open(const Path& p, std::ios_base::openmode mode=std::ios_base::in)
    {
        std::basic_ifstream<charT, traits>::open(p.string().c_str(), mode);
    }
    
    BasicIfstream(const BasicIfstream&) = delete;
    const BasicIfstream& operator=(const BasicIfstream&) = delete;
    ~BasicIfstream() override {}
};

template <class charT, class traits = std::char_traits<charT>>
class BasicOfstream : public std::basic_ofstream<charT, traits>
{
public:
    BasicOfstream() {}

    explicit BasicOfstream(const Path& p, std::ios_base::openmode mode = std::ios_base::out)
        : std::basic_ofstream<charT, traits>(p.string().c_str(), mode)
    {
    }
    void open(const Path& p, std::ios_base::openmode mode = std::ios_base::out) { std::basic_ofstream<charT, traits>::open(p.string().c_str(), mode); }

    BasicOfstream(const BasicOfstream&) = delete;
    const BasicOfstream& operator=(const BasicOfstream&) = delete;
    ~BasicOfstream() override {}
};

template <class charT, class traits = std::char_traits<charT>>
class BasicFstream : public std::basic_fstream<charT, traits>
{
public:
    BasicFstream() {}

    explicit BasicFstream(const Path& p, std::ios_base::openmode mode=std::ios_base::in | std::ios_base::out)
        : std::basic_fstream<charT, traits>(p.string().c_str(), mode)
    {
    }
    void open(const Path& p, std::ios_base::openmode mode=std::ios_base::in | std::ios_base::out)
    {
        std::basic_fstream<charT, traits>::open(p.string().c_str(), mode);
    }

    BasicFstream(const BasicFstream&) = delete;
    const BasicFstream& operator=(const BasicFstream&) = delete;
    ~BasicFstream() override {}
};

typedef BasicFilebuf<char> filebuf;
typedef BasicFilebuf<wchar_t> wfilebuf;
typedef BasicIfstream<char> ifstream;
typedef BasicIfstream<wchar_t> wifstream;
typedef BasicOfstream<char> ofstream;
typedef BasicOfstream<wchar_t> wofstream;
typedef BasicFstream<char> fstream;
typedef BasicFstream<wchar_t> wfstream;

class u8arguments
{
public:
    u8arguments(int& argc, char**& argv);
    ~u8arguments()
    {
        _refargc = _argc;
        _refargv = _argv;
    }

    bool valid() const { return _isvalid; }

private:
    int _argc;
    char** _argv;
    int& _refargc;
    char**& _refargv;
    bool _isvalid;
#ifdef OS_WINDOWS
    std::vector<std::string> _args;
    std::vector<char*> _argp;
#endif
};

//-------------------------------------------------------------------------------------------------
//  Implementation
//-------------------------------------------------------------------------------------------------

} // namespace filesystem

#endif // __FILE_SYSTEM_H__