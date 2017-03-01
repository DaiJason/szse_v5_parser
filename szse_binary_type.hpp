// @Copyright 2017, cao.ning, All Rights Reserved
// @Author:   cao.ning
// @Date:     2017/02/21
// @Brief:    ���V5binaryЭ���еĻ����������Ͷ���
//            immutable_�����ɱ����ͣ�ֻ����ָ������ṩ�����������ṩд����
//            mutable_  ���ɱ����ͣ�������ʵ�����ݣ��ṩ�˶�д����

#ifndef __CN_SZSE_BINARY_TYPE_H__
#define __CN_SZSE_BINARY_TYPE_H__

#include <stdint.h>
#include <string>
#include <assert.h>
#include <type_traits>

namespace cn
{
namespace szse
{
namespace binary
{

// �ж�����Ty �Ƿ�Ϊsize���ȵ�����
template <typename Ty, size_t Size>
struct is_specific_size_integral
    : std::integral_constant<bool, 
        std::is_integral<Ty>::value && sizeof(Ty) == Size>
{
};


// ����֤ȯ������Binary�������ݽӿڹ淶1.00 
// �������������ֶζ����ø��ֽ��� BIG-ENDIAN�����뷽ʽ

// ���ֽ��������ͣ�int8_t, uint8_t��
template <typename IntTy>
typename std::enable_if<is_specific_size_integral<IntTy, 1>::value, IntTy>::type
ChangeEndian(IntTy d)
{
    return d;
}
// ˫�ֽ��������ͣ�int16_t, uint16_t��
template <typename IntTy>
typename std::enable_if<is_specific_size_integral<IntTy, 2>::value, IntTy>::type
ChangeEndian(IntTy d)
{
    return ((d & 0x00FF) << 8) | ((d & 0xFF00) >> 8);
}
// ���ֽ��������ͣ�int32_t, uint32_t��
template <typename IntTy>
typename std::enable_if<is_specific_size_integral<IntTy, 4>::value, IntTy>::type
ChangeEndian(IntTy d)
{
    return ((d & 0x000000FF) << 24) | ((d & 0x0000FF00) << 8) |
        ((d & 0x00FF0000) >> 8) | ((d & 0xFF000000) >> 24);
}
// ���ֽ��������ͣ�int64_t, uint64_t��
template <typename IntTy>
typename std::enable_if<is_specific_size_integral<IntTy, 8>::value, IntTy>::type
ChangeEndian(IntTy d)
{
    return 
((d & 0x00000000000000FFUI64) << 56) | ((d & 0x000000000000FF00UI64) << 40) |
((d & 0x0000000000FF0000UI64) << 24) | ((d & 0x00000000FF000000UI64) << 8) |
((d & 0x000000FF00000000UI64) >> 8) | ((d & 0x0000FF0000000000UI64) >> 24) |
((d & 0x00FF000000000000UI64) >> 40) | ((d & 0xFF00000000000000UI64) >> 56);
}

typedef bool is_mutable;

namespace immutable_
{

// �������󣺹̶����ȵ�һ�鲻�ɱ���ڴ�����
template <size_t Size> class base_object
{
protected:
    const char* mem_addr_;
public:
    static const is_mutable mutable_type = false;

    base_object() : mem_addr_(nullptr) {}
    void load(const char* buf) { mem_addr_ = buf; }
    void write(char* buf) const { assert(false && "write immutable_ object"); }
    inline const char* mem_addr() const { return mem_addr_; }
    static size_t mem_size() { return Size; }
};

// ����
// �������ݺͶ�ȡ����ʱ��Ҫ���д�С��ת��
template <typename IntTy>
class Int : public base_object<sizeof(IntTy)>
{
    // ȷ���ڴ���ΪBIG-ENDIAN
public:
    inline IntTy get_value() const 
    {
        const IntTy* ptr = (const IntTy*)mem_addr_;
        return ChangeEndian(*ptr); 
    }
};

// ��������
// x��ʾ������С���ܼ�λ����������С���㣬y��ʾС��λ��
// ʹ��int64_t�洢
template <int x, int y> class Number : public Int<int64_t>
{
    double pow_;
public:
    Number() : pow_(std::pow(10, y)) {}
    inline double get_value() const { return double(Int::get_value()) / pow_; }
    inline int64_t raw_value() const { return Int::get_value(); }
};

// bool����
class Boolean : public Int<uint16_t>
{
public:
    static const uint16_t True = 1;
    static const uint16_t False = 0;
    inline uint16_t get_value() const { return Int::get_value(); }
    operator bool() const { return get_value() == True; }
};

// ʱ������ͣ�YYYYMMDDHHMMSSsss
class LocalTimeStamp : public Int<int64_t>
{
public:
    inline uint32_t year()   const 
    { return (uint32_t)(get_value() / 10000000000000); }
    inline uint32_t month()  const 
    { return (uint32_t)(get_value() % 10000000000000 / 100000000000); }
    inline uint32_t day()    const 
    { return (uint32_t)(get_value() % 100000000000 / 1000000000); }
    inline uint32_t hour()   const 
    { return (uint32_t)(get_value() % 1000000000 / 10000000); }
    inline uint32_t minute() const 
    { return (uint32_t)(get_value() % 10000000 / 100000); }
    inline uint32_t sec()    const 
    { return (uint32_t)(get_value() % 100000 / 1000); }
    inline uint32_t msec()   const 
    { return (uint32_t)(get_value() % 1000); }
};

// �������ͣ�YYYYMMDD
class LocalMktDate : public Int<uint32_t>
{
public:
    inline uint32_t year()  const { return get_value() / 10000; }
    inline uint32_t month() const { return get_value() % 10000 / 100; }
    inline uint32_t day()   const { return get_value() % 100; }
};

// �ַ������ͣ�Size��ʾ�ַ�����װ�ص�����ֽ���
// �ַ���ʵ�ʳ���С���ֶ�������󳤶ȵĶ��󲹿ո�
// REMARK: String����ֱ�Ӳ������ڴ棬��˲��������� '\0' �ֽڣ�ֱ�ӻ�ȡ�ַ�����Խ��
template <size_t Size> class String : public base_object<Size>
{
public:
    inline char at(uint32_t i) const { assert(i < Size); return mem_addr_[i]; }
    inline const char* c_str() const { assert(mem_addr_); return mem_addr_; }
    std::string to_string() const { return std::string(mem_addr_, Size); }
};


} // namespace immutable END


namespace mutable_
{

// ��������ֻ�ṩһ���ߴ�ӿ�
template <size_t Size> class base_object
{
public:
    static const is_mutable mutable_type = true;
    static size_t mem_size() { return Size; }
};

// ����
template <typename IntTy>
class Int : public base_object<sizeof(IntTy)>
{
    IntTy data_;
public:
    Int() : data_(0) {}
    explicit Int(IntTy v) { data_ = v; }
    Int(const Int& v) { data_ = v.data_; }
    bool operator==(IntTy v) { return data_ == v; }
    bool operator==(const Int& v) { return data_ == v.data_; }
    void load(const char* buf) { data_ = ChangeEndian(*(IntTy*)buf); }
    void write(char* buf) const { *(IntTy*)buf = ChangeEndian(data_); }
    inline IntTy get_value() const { return data_; }
    inline void set_value(IntTy v) { data_ = v; }
};

// ��������
// x��ʾ������С���ܼ�λ����������С���㣬y��ʾС��λ��
// ʹ��int64_t�洢
template <int x, int y> class Number : public Int<int64_t>
{
    typedef Int<uint16_t> base_type;
    double pow_;
public:
    Number() : pow_(std::pow(10, y)) {}
    inline double get_value() const
    {
        return double(base_type::get_value()) / pow_;
    }
    inline int64_t raw_value() const
    {
        return base_type::get_value();
    }
    inline void set_value(double v)
    {
        base_type::set_value((int64_t)(v * pow_));
    }
};

// bool����
class Boolean : public Int<uint16_t>
{
    typedef Int<uint16_t> base_type;
public:
    static const uint16_t True = 1;
    static const uint16_t False = 0;
    inline bool get_value() const
    {
        return base_type::get_value() == True ? true : false;
    }
    inline void set_value(bool b)
    {
        base_type::set_value(b ? True : False);
    }
    inline uint16_t raw_value() const
    {
        return Int::get_value();
    }
};

// ʱ������ͣ�YYYYMMDDHHMMSSsss
class LocalTimeStamp : public Int<int64_t>
{
public:
    inline uint32_t year()   const
    {
        return (uint32_t)(get_value() / 10000000000000);
    }
    inline uint32_t month()  const
    {
        return (uint32_t)(get_value() % 10000000000000 / 100000000000);
    }
    inline uint32_t day()    const
    {
        return (uint32_t)(get_value() % 100000000000 / 1000000000);
    }
    inline uint32_t hour()   const
    {
        return (uint32_t)(get_value() % 1000000000 / 10000000);
    }
    inline uint32_t minute() const
    {
        return (uint32_t)(get_value() % 10000000 / 100000);
    }
    inline uint32_t sec()    const
    {
        return (uint32_t)(get_value() % 100000 / 1000);
    }
    inline uint32_t msec()   const
    {
        return (uint32_t)(get_value() % 1000);
    }
};

// �������ͣ�YYYYMMDD
class LocalMktDate : public Int<uint32_t>
{
public:
    inline uint32_t year()  const { return get_value() / 10000; }
    inline uint32_t month() const { return get_value() % 10000 / 100; }
    inline uint32_t day()   const { return get_value() % 100; }
};

// �ַ������ͣ�Size��ʾ�ַ�����װ�ص�����ֽ���
// �ַ���ʵ�ʳ���С���ֶ�������󳤶ȵĶ��󲹿ո�
template <size_t Size> class String : public base_object<Size>
{
    char data_[Size + 1];
    void fill_with_blank() { memset(data_, ' ', Size); data_[Size] = '\0'; }
public:
    String() { fill_with_blank(); }
    inline char at(uint32_t i) const { assert(i < Size); return data_[i]; }
    inline const char* c_str() const { return data_; }
    std::string to_string() const { return std::string(data_); }
    inline size_t size() { return Size; }
    void load(const char* buf) { memcpy(data_, buf, Size); }
    void write(char* buf) const { memcpy(buf, data_, Size); }
};


} // namespace mutable END


template <is_mutable, size_t Size>
struct base_object : public immutable_::base_object<Size>
{
};

template <size_t Size> 
struct base_object<true, Size> : public mutable_::base_object<Size>
{
};

template <is_mutable, typename InnerTy>
struct Int : public immutable_::Int<typename InnerTy>
{
};

template <typename InnerTy> 
struct Int <true, InnerTy> : public mutable_::Int<typename InnerTy>
{
};

template <is_mutable, int x, int y>
struct Number : public immutable_::Number<x, y>
{
};

template <int x, int y> 
struct Number <true, x, y> : public mutable_::Number<x, y>
{
};

template <is_mutable>
struct Boolean : public immutable_::Boolean
{
};

template <> 
struct Boolean <true> : public mutable_::Boolean
{
};

template <is_mutable>
struct LocalTimeStamp : public immutable_::LocalTimeStamp
{
};

template <> 
struct LocalTimeStamp <true> : public mutable_::LocalTimeStamp
{
};

template <is_mutable>
struct LocalMktDate :public immutable_::LocalMktDate
{
};

template <> 
struct LocalMktDate <true> : public mutable_::LocalMktDate
{
};

template <is_mutable, size_t Size>
struct String : public immutable_::String<Size>
{
};

template <size_t Size> 
struct String <true, Size> : public mutable_::String<Size>
{
};

} // namespace binary END
} // namespace szse END
} // namespace cn END

#endif // __CN_SZSE_BINARY_TYPE_H__