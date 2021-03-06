// @Copyright 2017, cao.ning, All Rights Reserved
// @Author:   cao.ning
// @Date:     2017/02/21
// @Brief:    深交所V5binary协议的报文定义

#ifndef __CN_SZSE_BINARY_PACKET_H__
#define __CN_SZSE_BINARY_PACKET_H__

#include "szse_binary_type.hpp"
#include "szse_binary_field.hpp"

namespace cn
{
namespace szse
{
namespace binary
{

// 计算校验和
static uint32_t GenerateCheckSum(char *buf, uint32_t buf_size)
{
    uint32_t idx, cks;
    for (idx = 0L, cks = 0; idx < buf_size; cks += (uint32_t)buf[idx++]);
    return cks % 256;
}

namespace immutable_
{

// @Class:   Packet
// @Author:  cao.ning
// @Date:    2017/02/21
// @Brief:   只用来记录一块有效的报文字节流，仅保存指针信息
class Packet
{
    typedef cn::szse::binary::Int<false, uint32_t> TypeCheckSum;
public:
    Packet() :field_buf_addr_(nullptr), field_buf_size_(0) {}
    virtual ~Packet() {}
    inline const MsgHeader* GetHeader() const { return &header_; }
    bool Structure(const char* mem_addr, size_t* mem_size)
    {
        if (!header_.Load(mem_addr, *mem_size))
        {
            *mem_size = MsgHeader::SSize;
            return false;
        }

        size_t total_packet_size = StreamSize();
        if (*mem_size < total_packet_size)
        {
            *mem_size = total_packet_size;
            return false;
        }
        *mem_size = total_packet_size;
        // range: header + body
        size_t check_sum_range = total_packet_size - TypeCheckSum::mem_size();
        check_sum_.load(mem_addr + check_sum_range);
        if (GenerateCheckSum((char*)mem_addr, check_sum_range)
            != check_sum_.get_value())
        {
            return false;
        }
        // set pointer
        field_buf_addr_ = mem_addr + MsgHeader::SSize;
        field_buf_size_ = header_.BodyLength.get_value();
        return true;
    }
    const char* ToStream()
    {
        // 直接返回第一个字段的首地址
        return header_.MsgType.mem_addr();
    }
    size_t StreamSize()
    {
        return MsgHeader::SSize 
            + header_.BodyLength.get_value() 
            + TypeCheckSum::mem_size();
    }
    // 可以使用mutable或immutable类型数据结构
    template <typename FieldType>
    bool GetField(FieldType* f) const
    {
        return f->Load(field_buf_addr_, field_buf_size_);
    }
    const char* FieldPos() const { return field_buf_addr_; }
    // REMARK: 该类报文只保留了原始的字节流指针，因此不提供写入函数
protected:
    MsgHeader header_;
    TypeCheckSum check_sum_;
    //
    const char* field_buf_addr_;
    size_t field_buf_size_;
};
} // namespace immutable_ END

namespace mutable_
{

class Packet
{
    typedef cn::szse::binary::Int<true, uint32_t> TypeCheckSum;
    static const uint32_t INIT_PACKAGE_STREAM_SIZE = 1024;
public:
    Packet() : packet_stream_size_(INIT_PACKAGE_STREAM_SIZE) 
    {
        packet_stream_ = new char[packet_stream_size_];
        memset(packet_stream_, 0, packet_stream_size_);
    }
    explicit Packet(uint32_t init_size) : packet_stream_size_(init_size)
    {
        packet_stream_ = new char[packet_stream_size_];
        memset(packet_stream_, 0, packet_stream_size_);
    }
    virtual ~Packet()
    {
        if (packet_stream_)
        {
            delete[] packet_stream_;
            packet_stream_ = nullptr;
        }
    }
    inline const MsgHeader* GetHeader() const { return &header_; }
    bool Structure(const char* mem_addr, size_t* mem_size)
    {
        if (!header_.Load(mem_addr, *mem_size))
        {
            *mem_size = MsgHeader::SSize;
            return false;
        }

        size_t total_packet_size = StreamSize();
        if (*mem_size < total_packet_size)
        {
            *mem_size = total_packet_size;
            return false;
        }
        *mem_size = total_packet_size;
        // range: header + body
        const char* check_sum_pos = 
            mem_addr + total_packet_size - TypeCheckSum::mem_size();
        uint32_t chech_sum_value = ChangeEndian(*(uint32_t*)check_sum_pos);
        if (GenerateCheckSum((char*)mem_addr, 
            total_packet_size - TypeCheckSum::mem_size())
            != chech_sum_value)
        {
            return false;
        }
        // copy data
        if (packet_stream_size_ < total_packet_size)
        {
            resize_package_stream(total_packet_size);
        }
        memcpy(packet_stream_, mem_addr, total_packet_size);
        return true;
    }
    const char* ToStream()
    {
        return packet_stream_;
    }
    uint32_t StreamSize()
    {
        return MsgHeader::SSize
            + header_.BodyLength.get_value()
            + TypeCheckSum::mem_size();
    }
        
    // 可以使用mutable或immutable类型数据结构
    template <typename FieldType>
    bool GetField(FieldType* f) const
    {
        return f->Load(field_pos(), header_.BodyLength.get_value());
    }

    template <typename FieldType>
    bool InsertField(FieldType* f)
    {
        size_t field_size = f->Size();
        uint32_t need_stream_len = 
            MsgHeader::SSize + field_size + TypeCheckSum::mem_size();
        if (packet_stream_size_ < need_stream_len)
        {
            resize_package_stream(need_stream_len);
        }
        header_.MsgType.set_value(f->MsgType());
        header_.BodyLength.set_value(field_size);
        if (!header_.Write(packet_stream_, MsgHeader::SSize))
        {
            return false;
        }
        if (!f->Write(field_pos(), packet_stream_size_ - MsgHeader::SSize))
        {
            return false;
        }
        uint32_t check_sum = GenerateCheckSum((char*)packet_stream_,
            MsgHeader::SSize + field_size);
        char* check_sum_pos = field_pos() + field_size;
        *(uint32_t*)check_sum_pos = ChangeEndian(check_sum);
        return true;
    }
protected:
    // 重新分配报文字节流，原始信息将会被销毁
    void resize_package_stream(uint32_t new_size)
    {
        // 缓冲区长度为64字节的整数倍
        packet_stream_size_ = (new_size / 64 + 1) * 64;
        if (packet_stream_)
        {
            delete[] packet_stream_;
        }
        packet_stream_ = new char[packet_stream_size_];
        memset(packet_stream_, 0, packet_stream_size_);
    }
    inline char* field_pos() { return packet_stream_ + MsgHeader::SSize; }
protected:
    // 报文字节流
    char* packet_stream_;
    // 字节流长度
    uint32_t packet_stream_size_;
    // 消息头
    MsgHeader header_;
};
} // namespace mutable_ END

} // namespace binary END
} // namespace szse END
} // namespace cn END

#endif // __CN_SZSE_BINARY_PACKET_H__