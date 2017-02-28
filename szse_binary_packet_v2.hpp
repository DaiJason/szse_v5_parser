// @Copyright 2017, cao.ning, All Rights Reserved
// @Author:   cao.ning
// @Date:     2017/02/21
// @Brief:    ���ݽṹ����

#ifndef __CN_SZSE_BINARY_PACKET_H__v2
#define __CN_SZSE_BINARY_PACKET_H__v2

#include "szse_binary_type_v2.hpp"
#include "szse_binary_internal_v2.hpp"

namespace cn
{
namespace szse
{
namespace binary
{

// @Class:   MsgHeader
// @Author:  cao.ning
// @Date:    2017/02/21
// @Brief:
template <is_mutable b>
class MsgHeader : public Field<b>
{
public:
    // 
    static size_t SSize()
    {
        static size_t ssize = TypeInt<uint32_t>::mem_size() * 2;
        return ssize;
    }
    virtual uint32_t Type() const { return 0; }
    virtual uint32_t Size() const { return SSize(); }
    virtual bool Load(const char* mem_addr, size_t mem_size) override
    {
        return load_from_memory(mem_addr, mem_size, MsgType, BodyLength);
    }
    virtual bool Write(char* mem_addr, size_t mem_size) override
    {
        return write_into_memory(mem_addr, mem_size, MsgType, BodyLength);
    }
    
    TypeInt<uint32_t> MsgType;      // ��Ϣ����
    TypeInt<uint32_t> BodyLength;   // ��Ϣ�峤��
};

// ����У���
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
// @Brief:   ֻ������¼һ����Ч�ı����ֽ�����������ָ����Ϣ
class Packet
{
public:
    typedef MsgHeader<false> HeaderType;
    typedef cn::szse::binary::Int<false, uint32_t> CheckSumType;

    Packet() {}
    virtual ~Packet() {}
    inline const HeaderType* GetHeader() const { return &header_; }
    bool Structure(const char* mem_addr, size_t* mem_size)
    {
        if (*mem_size < HeaderType::SSize())
        {
            *mem_size = HeaderType::SSize();
            return false;
        }
        header_.Load(mem_addr, *mem_size);
        size_t total_packet_size = StreamSize();
        if (*mem_size < total_packet_size)
        {
            *mem_size = total_packet_size;
            return false;
        }
        *mem_size = total_packet_size;
        // range: header + body
        size_t check_sum_range = total_packet_size - CheckSumType::mem_size();
        check_sum_.load(mem_addr + check_sum_range);
        if (GenerateCheckSum((char*)mem_addr, check_sum_range)
            != check_sum_.get_value())
        {
            return false;
        }
        // set pointer
        field_buf_addr_ = mem_addr + HeaderType::SSize();
        field_buf_size_ = header_.BodyLength.get_value();
        return true;
    }
    const void* ToStream()
    {
        // ֱ�ӷ��ص�һ���ֶε��׵�ַ
        return header_.MsgType.mem_addr();
    }
    size_t StreamSize()
    {
        return HeaderType::SSize() 
            + header_.BodyLength.get_value() 
            + CheckSumType::mem_size();
    }
    // ����ʹ��mutable��immutable�������ݽṹ
    template <typename FieldType>
    bool GetField(FieldType* f)
    {
        return f->Load(field_buf_addr_, field_buf_size_);
    }
    // REMARK: ���౨��ֻ������ԭʼ���ֽ���ָ�룬��˲��ṩд�뺯��
protected:
    HeaderType header_;
    CheckSumType check_sum_;
    //
    const char* field_buf_addr_;
    size_t field_buf_size_;
};
} // namespace immutable_ END

namespace mutable_
{
class Packet
{
    static const uint32_t INIT_PACKAGE_STREAM_SIZE = 1024;
public:
    typedef MsgHeader<true> HeaderType;
    typedef cn::szse::binary::Int<true, uint32_t> CheckSumType;

    Packet() : packet_stream_size_(INIT_PACKAGE_STREAM_SIZE) 
    {
        packet_stream_ = (char*)malloc(packet_stream_size_);
        assert(packet_stream_);
        memset(packet_stream_, 0, packet_stream_size_);
        field_stream_ = packet_stream_ + HeaderType::SSize();
    }
    virtual ~Packet()
    {
        if (packet_stream_)
        {
            free(packet_stream_);
            packet_stream_ = nullptr;
        }
    }
    inline const HeaderType* GetHeader() const { return &header_; }
    bool Structure(const char* mem_addr, size_t* mem_size)
    {
        if (*mem_size < HeaderType::SSize())
        {
            *mem_size = HeaderType::SSize();
            return false;
        }
        header_.Load(mem_addr, *mem_size);
        size_t total_packet_size = StreamSize();
        if (*mem_size < total_packet_size)
        {
            *mem_size = total_packet_size;
            return false;
        }
        *mem_size = total_packet_size;
        // range: header + body
        const char* check_sum_pos = 
            mem_addr + total_packet_size - CheckSumType::mem_size();
        uint32_t chech_sum_value = ChangeEndian(*(uint32_t*)check_sum_pos);
        if (GenerateCheckSum((char*)mem_addr, 
            total_packet_size - CheckSumType::mem_size())
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
    const void* ToStream()
    {
        return packet_stream_;
    }
    uint32_t StreamSize()
    {
        return HeaderType::SSize()
            + header_.BodyLength.get_value()
            + CheckSumType::mem_size();
    }
    // ����ʹ��mutable��immutable�������ݽṹ
    template <typename FieldType>
    bool GetField(FieldType* f)
    {
        return f->Load(field_stream_, header_.BodyLength.get_value());
    }
    template <typename FieldType>
    bool InsertField(FieldType* f)
    {
        size_t field_size = f->Size();
        uint32_t need_stream_len = 
            HeaderType::SSize() + field_size + CheckSumType::mem_size();
        if (packet_stream_size_ < need_stream_len)
        {
            resize_package_stream(need_stream_len);
        }
        header_.MsgType.set_value(f->Type());
        header_.BodyLength.set_value(field_size);
        if (!header_.Write(packet_stream_, HeaderType::SSize()))
        {
            return false;
        }
        if (!f->Write(field_stream_, packet_stream_size_ - HeaderType::SSize()))
        {
            return false;
        }
        uint32_t check_sum = GenerateCheckSum((char*)packet_stream_,
            HeaderType::SSize() + field_size);
        char* check_sum_pos = (char*)field_stream_ + field_size;
        *(uint32_t*)check_sum_pos = ChangeEndian(check_sum);
        return true;
    }
protected:
    // ���·��䱨���ֽ�����ԭʼ��Ϣ���ᱻ����
    void resize_package_stream(uint32_t new_size)
    {
        // ����������Ϊ64�ֽڵ�������
        packet_stream_size_ = (new_size / 64 + 1) * 64;
        if (packet_stream_)
        {
            free(packet_stream_);
        }
        packet_stream_ = (char*)malloc(packet_stream_size_);
        memset(packet_stream_, 0, packet_stream_size_);
        field_stream_ = packet_stream_ + HeaderType::SSize();
    }
protected:
    // �����ֽ���
    char* packet_stream_;
    // �ֽ�������
    uint32_t packet_stream_size_;
    // ��Ϣͷ
    HeaderType header_;
    // ��Ϣ����ʼ�ֽڵ�ַ��ʵ��ָ��packet_stream_�е�ĳλ��
    char* field_stream_;
};
} // namespace mutable_ END

// ��¼��Ϣ Logon
template <is_mutable b>
class Logon : public Field<b>
{
public:
    static const uint32_t TypeID = 1;
    
    TypeCompID          SenderCompID;           // ���ͷ�����
    TypeCompID          TargetCompID;           // ���շ�����
    TypeInt<int32_t>    HeartBtInt;             // �����������λ���롣 �û�����ϵͳ��½ʱ�ṩ���������ء�
    TypeString<16>      Password;               // ����
    TypeString<32>      DefaultApplVerID;       // ������Э��汾����дΪ n.xy ��1.00��
public:
    virtual uint32_t Type() const override { return TypeID; }
    virtual uint32_t Size() const override
    {
        return byte_size_sum(SenderCompID, TargetCompID, HeartBtInt, 
            Password, DefaultApplVerID);
    }
    virtual bool Load(const char* mem_addr, size_t mem_size) override
    {
        return load_from_memory(mem_addr, mem_size, SenderCompID, TargetCompID,
            HeartBtInt, Password, DefaultApplVerID);
    }
    virtual bool Write(char* mem_addr, size_t mem_size) override
    {
        return write_into_memory(mem_addr, mem_size, SenderCompID, TargetCompID,
            HeartBtInt, Password, DefaultApplVerID);
    }
};

// ע����Ϣ Logout
template <is_mutable b>
class Logout : public Field<b>
{
public:
    static const uint32_t TypeID = 2;
    static const int32_t kSessionStatus_type_0 = 0;     //�Ự��Ծ
    static const int32_t kSessionStatus_type_1 = 1;     //�Ự�����Ѹ���
    static const int32_t kSessionStatus_type_2 = 2;     //�����ڵĻỰ����
    static const int32_t kSessionStatus_type_3 = 3;     //�»Ự������Ϲ淶
    static const int32_t kSessionStatus_type_4 = 4;     //�Ự�˵����
    static const int32_t kSessionStatus_type_5 = 5;     //���Ϸ����û��������
    static const int32_t kSessionStatus_type_6 = 6;     //�˻�����
    static const int32_t kSessionStatus_type_7 = 7;     //��ǰʱ�䲻�����¼
    static const int32_t kSessionStatus_type_8 = 8;     //�������
    static const int32_t kSessionStatus_type_9 = 9;     //�յ��� MsgSeqNum(34)̫С
    static const int32_t kSessionStatus_type_10 = 10;    //�յ��� NextExpectedMsgSeqNum(789)̫��.
    static const int32_t kSessionStatus_type_101 = 101;   //����
    static const int32_t kSessionStatus_type_102 = 102;   //��Ч��Ϣ
    
    TypeInt<int32_t>    SessionStatus;          // �˳�ʱ�ĻỰ״̬
    TypeString<200>     Text;                   // ע��ԭ��Ľ�һ������˵��
public:
    virtual uint32_t Type() const override { return TypeID; }
    virtual uint32_t Size() const override
    {
        return byte_size_sum(SessionStatus, Text);
    }
    virtual bool Load(const char* mem_addr, size_t mem_size) override
    {
        return load_from_memory(mem_addr, mem_size, SessionStatus, Text);
    }
    virtual bool Write(char* mem_addr, size_t mem_size) override
    {
        return write_into_memory(mem_addr, mem_size, SessionStatus, Text);
    }
};

// ������Ϣ  Heartbeat
template <is_mutable b> 
class Heartbeat : public Field<b>
{
public:
    static const uint32_t TypeID = 3;
public:
    virtual uint32_t Type() const override { return TypeID; }
    virtual uint32_t Size() const override { return 0; }
    virtual bool Load(const char*, size_t) override { return true; }
    virtual bool Write(char*, size_t) override { return true; }
};

// ҵ��ܾ���Ϣ BusinessReject
template <is_mutable b> 
class BusinessReject : public Field<b>
{
public:
    static const uint32_t TypeID = 8;
public:
    TypeSeqNum              RefSeqNum;              // ���ܾ���Ϣ����Ϣ���
    TypeInt<uint32_t>       RefMsgType;             // ���ܾ�����Ϣ����
    TypeString<10>          BusinessRejectRefID;    // ���ܾ���Ϣ��Ӧ��ҵ���ID
    TypeInt<uint16_t>       BusinessRejectReason;   // �ܾ�ԭ��
    TypeString<50>          BusinessRejectText;     // �ܾ�ԭ��˵��
public:
    virtual uint32_t Type() const override { return TypeID; }
    virtual uint32_t Size() const override
    {
        return byte_size_sum(RefSeqNum, RefMsgType, BusinessRejectRefID,
            BusinessRejectReason, BusinessRejectText);
    }
    virtual bool Load(const char* mem_addr, size_t mem_size) override
    {
        return load_from_memory(mem_addr, mem_size, RefSeqNum, RefMsgType,
            BusinessRejectRefID, BusinessRejectReason, BusinessRejectText);
    }
    virtual bool Write(char* mem_addr, size_t mem_size) override
    {
        return write_into_memory(mem_addr, mem_size, RefSeqNum, RefMsgType,
            BusinessRejectRefID, BusinessRejectReason, BusinessRejectText);
    }
};

// Ƶ��������Ϣ  ChannelHeartbeat
template <is_mutable b> 
class ChannelHeartbeat : public Field<b>
{
public:
    static const uint32_t TypeID = 390095;
public:
    TypeInt<uint16_t>    ChannelNo;          // Ƶ������
    TypeSeqNum          ApplLastSeqNum;     // ���һ��������Ϣ�ļ�¼��
    TypeBoolean         EndOfChannel;       // Ƶ��������־
public:
    virtual uint32_t Type() const override { return TypeID; }
    virtual uint32_t Size() const override
    {
        return byte_size_sum(ChannelNo, ApplLastSeqNum, EndOfChannel);
    }
    virtual bool Load(const char* mem_addr, size_t mem_size) override
    {
        return load_from_memory(mem_addr, mem_size, 
            ChannelNo, ApplLastSeqNum, EndOfChannel);
    }
    virtual bool Write(char* mem_addr, size_t mem_size) override
    {
        return write_into_memory(mem_addr, mem_size, 
            ChannelNo, ApplLastSeqNum, EndOfChannel);
    }
};

// ������Ϣ
template <is_mutable b> 
class Announcement : public Field<b>
{
public:
    static const uint32_t TypeID = 390012;
public:
    TypeLocalTimeStamp      OrigTime;          // ����ʱ��
    TypeInt<uint16_t>       ChannelNo;         // Ƶ������
    TypeString<8>           NewsID;            // ����Ψһ��ʶ
    TypeString<128>         Headline;          // �������
    TypeString<8>           RawDataFormat;     // ���������ݸ�ʽ
    TypeLength              RawDataLength;     // ���������ݳ���
    const char*             RawData;           // ���������ݣ��䳤��
public:
    virtual uint32_t Type() const override { return TypeID; }
    virtual uint32_t Size() const override
    {
        return byte_size_sum(OrigTime, ChannelNo, NewsID, Headline, 
            RawDataFormat, RawDataLength)
            + RawDataLength.get_value();
    }
    virtual bool Load(const char* mem_addr, size_t mem_size) override
    {
        if (load_from_memory(mem_addr, mem_size, OrigTime, ChannelNo,
            NewsID, Headline, RawDataFormat, RawDataLength))
        {
            RawData = static_cast<const char*>(mem_addr);
            return true;
        }
        return false;
    }
    virtual bool Write(char* mem_addr, size_t mem_size) override
    {
        if (write_into_memory(mem_addr, mem_size, OrigTime, ChannelNo,
            NewsID, Headline, RawDataFormat, RawDataLength))
        {
            if (RawDataLength.get_value() >= mem_size)
            {
                memcpy(mem_addr, RawData, RawDataLength.get_value());
                return true;
            }
        }
        return false;
    }
};

// �ش���Ϣ  ReTransmit
template <is_mutable b> 
class ReTransmit : public Field<b>
{
public:
    static const uint32_t TypeID = 390094;
    
    static const uint8_t kResendType_type_1 = 1;      //�����������
    static const uint8_t kResendType_type_2 = 2;      //������Ϣ

    static const uint8_t kResendStatus_type_1 = 1;     //���
    static const uint8_t kResendStatus_type_2 = 2;     //������ɣ��в������������û�з��أ�
    static const uint8_t kResendStatus_type_3 = 3;     //��Ȩ��
    static const uint8_t kResendStatus_type_4 = 4;     //���ݲ�����
public:
    TypeInt<uint8_t>    ResendType;         // �ش�����
    TypeInt<uint16_t>   ChannelNo;          // Ƶ������
    TypeSeqNum          ApplBegSeqNum;      // ��ʼ��ţ��� ResendType=1 ʱ��Ч��ָ����¼��ŵķ�Χ
    TypeSeqNum          ApplEndSeqNum;      // ������ţ��� ResendType=1 ʱ��Ч���� ApplEndSeqNum=0 ʱ��
                                            // �������ػὫ ApplEndSeqNum ����Ϊ�յ��ش�����ʱ��
                                            // ��Ƶ���������ڴ��е����ֵ
    TypeString<8>        NewsID;            // ����Ψһ��ʶ���� ResendType=2 ʱ��Ч��Ϊ��ʱ��ʾ���빫���Ҫ
    TypeInt<uint8_t>     ResendStatus;      // �ش�״̬��������������ǰ�û����ظ��û�����ϵͳ������ʱ��Ч
    TypeString<16>       RejectText;        // ������������ǰ�û����ظ��û�����ϵͳ������ʱ��Ч
                                            // ���������������ǰ�û��ܾ����������������򷵻�
public:
    virtual uint32_t Type() const override { return TypeID; }
    virtual uint32_t Size() const override
    {
        return byte_size_sum(ResendType, ChannelNo, ApplBegSeqNum, 
            ApplEndSeqNum, NewsID, ResendStatus, RejectText);
    }
    virtual bool Load(const char* mem_addr, size_t mem_size) override
    {
        return load_from_memory(mem_addr, mem_size, ResendType, ChannelNo, 
            ApplBegSeqNum, ApplEndSeqNum, NewsID, ResendStatus, RejectText);
    }
    virtual bool Write(char* mem_addr, size_t mem_size) override
    {
        return write_into_memory(mem_addr, mem_size, ResendType, ChannelNo,
            ApplBegSeqNum, ApplEndSeqNum, NewsID, ResendStatus, RejectText);
    }
};

// �г�ʵʱ״̬ MarketStatus
template <is_mutable b> 
class MarketStatus : public Field<b>
{
public:
    static const uint32_t TypeID = 390019;
public:
    TypeLocalTimeStamp      OrigTime;            // ��������ʱ��
    TypeInt<uint16_t>       ChannelNo;           // Ƶ������
    TypeString<8>           MarketID;            // �г�����
    TypeString<8>           MarketSegmentID;     // �г������룬Ԥ��
    TypeString<4>           TradingSessionID;    // ���׻Ự ID
    TypeString<4>           TradingSessionSubID; // ���׻Ự�� ID
    TypeInt<uint16_t>       TradSesStatus;       // ���׻Ự״̬��Ԥ��
    TypeLocalTimeStamp      TradSesStartTime;    // ���׻Ự��ʼʱ�䣬Ԥ��
    TypeLocalTimeStamp      TradSesEndTime;      // ���׻Ự����ʱ�䣬Ԥ��
    TypeAmt                 ThresholdAmount;     // ÿ�ճ�ʼ���
    TypeAmt                 PosAmt;              // ����ʣ���ȣ���Ȳ�����ʱ�������̶�ֵ 0.0000
    TypeString<1>           AmountStatus;        // ���״̬
public:
    virtual uint32_t Type() const override { return TypeID; }
    virtual uint32_t Size() const override
    {
        return byte_size_sum(OrigTime, ChannelNo, MarketID, MarketSegmentID,
            TradingSessionID, TradingSessionSubID, TradSesStatus,
            TradSesStartTime, TradSesEndTime, ThresholdAmount, PosAmt,
            AmountStatus);
    }
    virtual bool Load(const char* mem_addr, size_t mem_size) override
    {
        return load_from_memory(mem_addr, mem_size, OrigTime, ChannelNo,
            MarketID, MarketSegmentID, TradingSessionID, TradingSessionSubID,
            TradSesStatus, TradSesStartTime, TradSesEndTime, ThresholdAmount,
            PosAmt, AmountStatus);
    }
    virtual bool Write(char* mem_addr, size_t mem_size) override
    {
        return write_into_memory(mem_addr, mem_size, OrigTime, ChannelNo,
            MarketID, MarketSegmentID, TradingSessionID, TradingSessionSubID,
            TradSesStatus, TradSesStartTime, TradSesEndTime, ThresholdAmount,
            PosAmt, AmountStatus);
    }
};

// ֤ȯʵʱ״̬ SecurityStatus
template <is_mutable b> 
class SecurityStatus : public Field<b>
{
    
public:
    static const uint32_t TypeID = 390013;
    static const uint16_t kSecuritySwitchType_type_1 = 1;      // ��������(���������ʱ��֤ȯ)
    static const uint16_t kSecuritySwitchType_type_2 = 2;      // ��ȯ����(��������ȯ���֤ȯ)
    static const uint16_t kSecuritySwitchType_type_3 = 3;      // �깺(������ ETF�� LOF �ȿ���ʽ���𣬶��ڻƽ� ETF ��ָ�ֽ��깺)
    static const uint16_t kSecuritySwitchType_type_4 = 4;      // ���(������ ETF�� LOF �ȿ���ʽ���𣬶��ڻƽ� ETF ��ָ�ֽ���ؿ���)
    static const uint16_t kSecuritySwitchType_type_5 = 5;      // �Ϲ�(���������Ϸ����Ϲ�����)
    static const uint16_t kSecuritySwitchType_type_6 = 6;      // ת��(�����ڴ���ת�ɻ����ڵĿ�תծ)
    static const uint16_t kSecuritySwitchType_type_7 = 7;      // ����(�����ڴ���ת�ɻ����ڵĿ�תծ)
    static const uint16_t kSecuritySwitchType_type_8 = 8;      // ��Ȩ(�����ڴ�����Ȩ�ڵ�Ȩ֤����Ȩ)
    static const uint16_t kSecuritySwitchType_type_10 = 10;      // �򿪲�(��������Ȩ������Ʒ)
    static const uint16_t kSecuritySwitchType_type_11 = 11;      // ������(��������Ȩ������Ʒ)
    static const uint16_t kSecuritySwitchType_type_12 = 12;      // �ƽ� ETFʵ����(�����ڻƽ� ETF)
    static const uint16_t kSecuritySwitchType_type_13 = 13;      // �ƽ�ETFʵ����(�����ڻƽ� ETF)
    static const uint16_t kSecuritySwitchType_type_14 = 14;      // Ԥ��ҪԼ(�����ڴ���ҪԼ�չ��ڵĹ�Ʊ)
    static const uint16_t kSecuritySwitchType_type_15 = 15;      // ���ҪԼ(�����ڴ���ҪԼ�չ��ڵĹ�Ʊ)
    static const uint16_t kSecuritySwitchType_type_18 = 18;      // ת�ɳ���(�����ڴ���ת�ɻ����ڵĿ�תծ)
    static const uint16_t kSecuritySwitchType_type_19 = 19;      // ���۳���(�����ڴ���ת�ɻ����ڵĿ�תծ)
    static const uint16_t kSecuritySwitchType_type_20 = 20;      // ��Ѻ(��������Ѻʽ�ع�����Ѻ���֤ȯ)
    static const uint16_t kSecuritySwitchType_type_21 = 21;      // ��Ѻ(��������Ѻʽ�ع�����Ѻ���֤ȯ)
    static const uint16_t kSecuritySwitchType_type_22 = 22;      // ���Ȩ(���������ȹ�)
    static const uint16_t kSecuritySwitchType_type_23 = 23;      // ��Ʊ��Ѻʽ��(�����ڿɿ�չ��Ʊ��Ѻʽ�ع�ҵ���֤ȯ)
    static const uint16_t kSecuritySwitchType_type_24 = 24;      // ʵʱ�ֲ�(�����ڷּ�����)
    static const uint16_t kSecuritySwitchType_type_25 = 25;      // ʵʱ�ϲ�(�����ڷּ�����)
    static const uint16_t kSecuritySwitchType_type_26 = 26;      // ���ҿ���(��������Ȩ������Ʒ)
    static const uint16_t kSecuritySwitchType_type_27 = 27;      // �����̱���(��������Ȩ��֧�������̱��۵�֤ȯ)
    static const uint16_t kSecuritySwitchType_type_28 = 28;      // �۹�ͨ������
    static const uint16_t kSecuritySwitchType_type_29 = 29;      // �۹�ͨ������
    static const uint16_t kSecuritySwitchType_type_30 = 30;      // �۹�ͨ�����
    static const uint16_t kSecuritySwitchType_type_31 = 31;      // �۹�ͨ�����
public:
    TypeLocalTimeStamp      OrigTime;          // ��������ʱ��
    TypeInt<uint16_t>       ChannelNo;         // Ƶ������
    TypeSecurityID          SecurityID;        // ֤ȯ����
    TypeString<4>           SecurityIDSource;  // ֤ȯ����Դ,102=����֤ȯ������,103=��۽�����
    TypeString<8>           FinancialStatus;   // ֤ȯ״̬
                                            // A = ���й�˾�����¶��ʾ
                                            // B = ���й�˾�����¶��ʾ
                                            // ÿ���ֽڽ�ʾһ��״̬������ͬʱ��ʾ����״̬
    TypeNumInGroup         NoSwitch;            // ���ظ���
    struct SecuritySwitch
    {
        TypeInt<uint16_t>   SecuritySwitchType;     // �������
        TypeBoolean         SecuritySwitchStatus;   // ����״̬
        uint32_t Size() const
        {
            return byte_size_sum(SecuritySwitchType, SecuritySwitchStatus);
        }
        bool Load(const char* mem_addr, size_t mem_size) 
        {
            return load_from_memory(mem_addr, mem_size, 
                SecuritySwitchType, SecuritySwitchStatus);
        }
        bool Write(char* mem_addr, size_t mem_size)
        {
            return write_into_memory(mem_addr, mem_size,
                SecuritySwitchType, SecuritySwitchStatus);
        }
    };
    TypeFieldArray<SecuritySwitch>      SecuritySwitchArray; // ��������
public:
    virtual uint32_t Type() const override { return TypeID; }
    virtual uint32_t Size() const override
    {
        return byte_size_sum(OrigTime, ChannelNo, SecurityID, SecurityIDSource,
            FinancialStatus, NoSwitch)
            + SecuritySwitchArray.Size();
    }
    virtual bool Load(const char* mem_addr, size_t mem_size) override
    {
        return load_from_memory(mem_addr, mem_size, OrigTime, ChannelNo,
            SecurityID, SecurityIDSource, FinancialStatus, NoSwitch)
            && SecuritySwitchArray.Load(mem_addr, mem_size, NoSwitch.get_value());
    }
    virtual bool Write(char* mem_addr, size_t mem_size) override
    {
        return write_into_memory(mem_addr, mem_size, OrigTime, ChannelNo,
            SecurityID, SecurityIDSource, FinancialStatus, NoSwitch)
            && SecuritySwitchArray.Write(mem_addr, mem_size);
    }
};

// ��������Ƶ��ͳ��
template <is_mutable b> 
class MarketSnapshotStatistic : public Field<b>
{
public:
    static const uint32_t TypeID = 390090;
    TypeLocalTimeStamp      OrigTime;           // ��������ʱ��
    TypeInt<uint16_t>       ChannelNo;          // Ƶ������
    TypeNumInGroup          NoMDStreamID;       // ����������
    struct StreamStatistic
    {
        TypeString<3>           MDStreamID;         // �������
        TypeInt<uint32_t>       StockNum;           // ֤ȯֻ��
        TypeString<8>           TradingPhaseCode;   // ����״̬���� 0 λ��T=�������ۣ�E=�ѱ���
        uint32_t Size() const
        {
            return byte_size_sum(MDStreamID, StockNum, TradingPhaseCode);
        }
        bool Load(const char* mem_addr, size_t mem_size)
        {
            return load_from_memory(mem_addr, mem_size, MDStreamID, StockNum, 
                TradingPhaseCode);
        }
        bool Write(char* mem_addr, size_t mem_size)
        {
            return write_into_memory(mem_addr, mem_size, MDStreamID, StockNum,
                TradingPhaseCode);
        }
    };
    TypeFieldArray<StreamStatistic>    StatisticArray; // ͳ������
public:
    virtual uint32_t Type() const override { return TypeID; }
    virtual uint32_t Size() const override
    {
        return byte_size_sum(OrigTime, ChannelNo, NoMDStreamID)
            + StatisticArray.Size();
    }
    virtual bool Load(const char* mem_addr, size_t mem_size) override
    {
        return load_from_memory(mem_addr, mem_size, OrigTime, ChannelNo,
            NoMDStreamID)
            && StatisticArray.Load(mem_addr, mem_size, NoMDStreamID.get_value());
    }
    virtual bool Write(char* mem_addr, size_t mem_size) override
    {
        return write_into_memory(mem_addr, mem_size, OrigTime, ChannelNo,
            NoMDStreamID)
            && StatisticArray.Write(mem_addr, mem_size);
    }
};

// ��������Ĺ����ֶ�
template <is_mutable b> 
class MarketSnapshotBase : public Field<b>
{
public:
    TypeLocalTimeStamp      OrigTime;           // ��������ʱ��
    TypeInt<uint16_t>       ChannelNo;          // Ƶ������
    TypeString<3>           MDStreamID;         // �������
                                            // 010 �ֻ�����Ʊ������ծȯ�ȣ����о��۽��׿�������
                                            // 020 ��Ѻʽ�ع����׿�������
                                            // 030 ծȯ������������
                                            // 040 ��Ȩ���о��۽��׿�������
                                            // 060 �����̼۽��׵��̺󶨼۽��׿�������
                                            // 061 �Գɽ�����Ȩƽ���۽��׵��̺󶨼۽��׿�������
                                            // 900 ָ����������
                                            // 910 �ɽ���ͳ��ָ���������
    TypeSecurityID          SecurityID;         // ֤ȯ����
    TypeString<4>           SecurityIDSource;   // ֤ȯ����Դ
    TypeString<8>           TradingPhaseCode;   // ��Ʒ�����Ľ��׽׶δ���
                                            // �� 0 λ��
                                            // S=����������ǰ����O=���̼��Ͼ��ۣ�T=�������ۣ�B=����
                                            // C=���̼��Ͼ��ۣ�E=�ѱ��У�H=��ʱͣ�ƣ�A=�̺���
                                            // V=�������ж�
                                            // �� 1 λ��
                                            // 0=����״̬��1=ȫ��ͣ��
    TypePrice               PrevClosePx;        // ���ռ�
    TypeInt<int64_t>        NumTrades;          // �ɽ�����
    TypeQty                 TotalVolumeTrade;   // �ɽ�����
    TypeAmt                 TotalValueTrade;    // �ɽ��ܽ��
    virtual uint32_t Size() const override
    {
        return byte_size_sum(OrigTime, ChannelNo, MDStreamID, SecurityID,
            SecurityIDSource, TradingPhaseCode, PrevClosePx,
            NumTrades, TotalVolumeTrade, TotalValueTrade);
    }
    virtual bool Load(const char* mem_addr, size_t mem_size) override
    {
        mem_head_ = mem_tail_ = mem_addr;
        return load_from_memory(mem_tail_, mem_size, OrigTime, 
            ChannelNo, MDStreamID, SecurityID, SecurityIDSource, 
            TradingPhaseCode, PrevClosePx, NumTrades, TotalVolumeTrade,
            TotalValueTrade);
    }
    virtual bool Write(char* mem_addr, size_t mem_size) override
    {
        mem_head_ = mem_tail_ = mem_addr;
        return write_into_memory((char*&)mem_tail_, mem_size, OrigTime, ChannelNo,
            MDStreamID, SecurityID, SecurityIDSource, TradingPhaseCode,
            PrevClosePx, NumTrades, TotalVolumeTrade, TotalValueTrade);
    }
protected:
    // ��¼�ײ���β��ָ�룬��Load��Writeʱ���£�Ϊ��������ִ�к�����Load��Write����
    // ���ڴ�֮ǰִ����Load����head��tailָ���ȡ�ڴ���ײ����ڴ��β��
    // ���ڴ�֮ǰִ����Write����ָ�����д���ڴ���ײ���β��
    const char* mem_head_;
    const char* mem_tail_;
};

// ���о��۽���ҵ��������� MarketSnapshot_300111
// 010 �ֻ�����Ʊ������ծȯ�ȣ����о��۽��׿�������
// 020 ��Ѻʽ�ع����׿�������
// 030 ծȯ������������
// 040 ��Ȩ���о��۽��׿�������
template <is_mutable b> 
class MarketSnapshot_300111 : public MarketSnapshotBase<b>
{
    typedef MarketSnapshotBase<b> base_type;
public:
    static const uint32_t TypeID = 300111;
public:
    // Extend Fields ҵ����չ�ֶ�
    TypeNumInGroup          NoMDEntries;            // ������Ŀ����
    struct SecurityEntry
    {
        TypeString<2>           MDEntryType;     // ������Ŀ���
        TypeInt<int64_t>        MDEntryPx;       // �۸񣬳���1000000
        TypeQty                 MDEntrySize;     // ����
        TypeInt<uint16_t>       MDPriceLevel;    // �����̵�λ
        TypeInt<int64_t>        NumberOfOrders;  // ��λ��ί�б�����Ϊ 0 ��ʾ����ʾ
        TypeNumInGroup          NoOrders;        // ��λ��ʾί�б�����Ϊ 0 ��ʾ����ʾ
        struct OrderQty
        {
            TypeQty             Qty;
            uint32_t Size() const { return TypeQty::mem_size(); }
            bool Load(const char* mem_addr, size_t mem_size)
            {
                return load_from_memory(mem_addr, mem_size, Qty);
            }
            bool Write(char* mem_addr, size_t mem_size)
            {
                return write_into_memory(mem_addr, mem_size, Qty);
            }
        };
        TypeFieldArray<OrderQty>       OrderQtyArray;        // ί������
        uint32_t Size() const
        {
            return byte_size_sum(MDEntryType, MDEntryPx, MDEntrySize,
                MDPriceLevel, NumberOfOrders, NoOrders)
                + OrderQtyArray.Size();
        }
        bool Load(const char* mem_addr, size_t mem_size)
        {
            return load_from_memory(mem_addr, mem_size, MDEntryType,
                MDEntryPx, MDEntrySize, MDPriceLevel, NumberOfOrders,
                NoOrders)
                && OrderQtyArray.Load(mem_addr, mem_size, NoOrders.get_value());
        }
        bool Write(char* mem_addr, size_t mem_size)
        {
            return write_into_memory(mem_addr, mem_size, MDEntryType,
                MDEntryPx, MDEntrySize, MDPriceLevel, NumberOfOrders,
                NoOrders) && OrderQtyArray.Write(mem_addr, mem_size);
        }
    };
    TypeFieldArray<SecurityEntry> SecurityEntryArray;     // ���о��۽���ҵ�����������չ�ֶ�
public:
    virtual uint32_t Type() const override { return TypeID; }
    virtual uint32_t Size() const override
    {
        return base_type::Size() + NoMDEntries.mem_size() + SecurityEntryArray.Size();
    }
    virtual bool Load(const char* mem_addr, size_t mem_size) override
    {
        if (base_type::Load(mem_addr, mem_size))
        {
            size_t base_type_size = base_type::mem_tail_ - base_type::mem_head_;
            mem_addr += base_type_size;
            mem_size -= base_type_size;
            return load_from_memory(mem_addr, mem_size, NoMDEntries)
                && SecurityEntryArray.Load(mem_addr, mem_size, NoMDEntries.get_value());
        }
        return false;
    }
    virtual bool Write(char* mem_addr, size_t mem_size) override
    {
        if (base_type::Write(mem_addr, mem_size))
        {
            size_t base_type_size = base_type::mem_tail_ - base_type::mem_head_;
            mem_addr += base_type_size;
            mem_size -= base_type_size;
            return write_into_memory(mem_addr, mem_size, NoMDEntries)
                && SecurityEntryArray.Write(mem_addr, mem_size);
        }
        return false;
    }
};


// �̺󶨼۽���ҵ��������� MarketSnapshot_300611
// 060 �����̼۽��׵��̺󶨼۽��׿�������
// 061 �Գɽ�����Ȩƽ���۽��׵��̺󶨼۽��׿�������
template <is_mutable b> 
class MarketSnapshot_300611 : public MarketSnapshotBase<b>
{
    typedef MarketSnapshotBase<b> base_type;
public:
    static const uint32_t TypeID = 300611;
public:
    // Extend Fields ��ҵ����չ�ֶ�
    TypeNumInGroup   NoMDEntries;            // ������Ŀ����
    struct SecurityEntry 
    {
        TypeString<2>           MDEntryType;     // ������Ŀ���0=���룻1=����
        TypeInt<int64_t>        MDEntryPx;       // �۸�
        TypeQty                 MDEntrySize;     // ����
        uint32_t Size() const
        {
            return byte_size_sum(MDEntryType, MDEntryPx, MDEntrySize);
        }
        bool Load(const char* mem_addr, size_t mem_size)
        {
            return load_from_memory(mem_addr, mem_size, 
                MDEntryType, MDEntryPx, MDEntrySize);
        }
        bool Write(char* mem_addr, size_t mem_size)
        {
            return write_into_memory(mem_addr, mem_size,
                MDEntryType, MDEntryPx, MDEntrySize);
        }
    };
    TypeFieldArray<SecurityEntry> SecurityEntryArray;     // ���о��۽���ҵ�����������չ�ֶ�
public:
    virtual uint32_t Type() const override { return TypeID; }
    virtual uint32_t Size() const override
    {
        return base_type::Size() + NoMDEntries.mem_size() + SecurityEntryArray.Size();
    }
    virtual bool Load(const char* mem_addr, size_t mem_size) override
    {
        if (base_type::Load(mem_addr, mem_size))
        {
            size_t base_type_size = base_type::mem_tail_ - base_type::mem_head_;
            mem_addr += base_type_size;
            mem_size -= base_type_size;
            return load_from_memory(mem_addr, mem_size, NoMDEntries)
                && SecurityEntryArray.Load(mem_addr, mem_size, NoMDEntries.get_value());
        }
        return false;
    }
    virtual bool Write(char* mem_addr, size_t mem_size) override
    {
        if (base_type::Write(mem_addr, mem_size))
        {
            size_t base_type_size = base_type::mem_tail_ - base_type::mem_head_;
            mem_addr += base_type_size;
            mem_size -= base_type_size;
            return write_into_memory(mem_addr, mem_size, NoMDEntries)
                && SecurityEntryArray.Write(mem_addr, mem_size);
        }
        return false;
    }
};

// ָ��������� MarketSnapshot_309011
// 900 ָ����������
template <is_mutable b> 
class MarketSnapshot_309011 : public MarketSnapshotBase<b>
{
    typedef MarketSnapshotBase<b> base_type;
public:
    static const uint32_t TypeID = 309011;
public:
    // Extend Fields ��ҵ����չ�ֶ�
    TypeNumInGroup   NoMDEntries;            // ������Ŀ����
    struct IndexEntry
    {
        TypeString<2>           MDEntryType;     // ������Ŀ���
        TypeInt<int64_t>        MDEntryPx;       // ָ����λ
        uint32_t Size() const
        {
            return byte_size_sum(MDEntryType, MDEntryPx);
        }
        bool Load(const char* mem_addr, size_t mem_size)
        {
            return load_from_memory(mem_addr, mem_size, MDEntryType, MDEntryPx);
        }
        bool Write(char* mem_addr, size_t mem_size)
        {
            return write_into_memory(mem_addr, mem_size, MDEntryType, MDEntryPx);
        }
    };
    TypeFieldArray<IndexEntry> IndexEntryArray;     // ���о��۽���ҵ�����������չ�ֶ�
public:
    virtual uint32_t Type() const override { return TypeID; }
    virtual uint32_t Size() const override
    {
        return base_type::Size() + NoMDEntries.mem_size() + IndexEntryArray.Size();
    }
    virtual bool Load(const char* mem_addr, size_t mem_size) override
    {
        if (base_type::Load(mem_addr, mem_size))
        {
            size_t base_type_size = base_type::mem_tail_ - base_type::mem_head_;
            mem_addr += base_type_size;
            mem_size -= base_type_size;
            return load_from_memory(mem_addr, mem_size, NoMDEntries)
                && IndexEntryArray.Load(mem_addr, mem_size, NoMDEntries.get_value());
        }
        return false;
    }
    virtual bool Write(char* mem_addr, size_t mem_size) override
    {
        if (base_type::Write(mem_addr, mem_size))
        {
            size_t base_type_size = base_type::mem_tail_ - base_type::mem_head_;
            mem_addr += base_type_size;
            mem_size -= base_type_size;
            return write_into_memory(mem_addr, mem_size, NoMDEntries)
                && IndexEntryArray.Write(mem_addr, mem_size);
        }
        return false;
    }
};

// �ɽ���ͳ��ָ��������� MarketSnapshot_309111
// 910 �ɽ���ͳ��ָ���������
template <is_mutable b> 
class MarketSnapshot_309111 : public MarketSnapshotBase<b>
{
    typedef MarketSnapshotBase<b> base_type;
public:
    static const uint32_t TypeID = 309111;
public:
    // Extend Fields ��ҵ����չ�ֶ�
    TypeInt<uint32_t>   StockNum;               // ͳ����ָ����������
public:
    virtual uint32_t Type() const override { return TypeID; }
    virtual uint32_t Size() const override
    {
        return base_type::Size() + StockNum.mem_size();
    }
    virtual bool Load(const char* mem_addr, size_t mem_size) override
    {
        if (base_type::Load(mem_addr, mem_size))
        {
            size_t base_type_size = base_type::mem_tail_ - base_type::mem_head_;
            mem_addr += base_type_size;
            mem_size -= base_type_size;
            return load_from_memory(mem_addr, mem_size, StockNum);
        }
        return false;
    }
    virtual bool Write(char* mem_addr, size_t mem_size) override
    {
        if (base_type::Write(mem_addr, mem_size))
        {
            size_t base_type_size = base_type::mem_tail_ - base_type::mem_head_;
            mem_addr += base_type_size;
            mem_size -= base_type_size;
            return write_into_memory(mem_addr, mem_size, StockNum);
        }
        return false;
    }
};

// ���ί������Ĺ����ֶ�
template <is_mutable b> 
class OrderSnapshotBase : public Field<b>
{
public:
    TypeInt<uint16_t>       ChannelNo;          // Ƶ������
    TypeSeqNum              ApplSeqNum;         // ��Ϣ��¼�ţ��� 1 ��ʼ����
    TypeString<3>           MDStreamID;         // �������
                                            // 011 �ֻ�����Ʊ������ծȯ�ȣ����о��۽����������
                                            // 021 ��Ѻʽ�ع������������
                                            // 041 ��Ȩ���о��۽����������
                                            // 051 Э�齻�������������
                                            // 052 Э�齻����ʶ�������
                                            // 071 ת��֤ͨȯ�����������
    TypeSecurityID       SecurityID;         // ֤ȯ����
    TypeString<4>           SecurityIDSource;   // ֤ȯ����Դ
    TypePrice            Price;              // ί�м۸�
    TypeQty              OrderQty;           // ί������
    TypeString<1>           Side;               // ��������1=��2 = ����G = ���룻F = ����
    TypeLocalTimeStamp   OrderTime;          // ί��ʱ��
    virtual uint32_t Size() const override
    {
        return byte_size_sum(ChannelNo, ApplSeqNum, MDStreamID, SecurityID,
            SecurityIDSource, Price, OrderQty, Side, OrderTime);
    }
    virtual bool Load(const char* mem_addr, size_t mem_size) override
    {
        mem_head_ = mem_tail_ = mem_addr;
        return load_from_memory(mem_tail_, mem_size, ChannelNo, ApplSeqNum,
            MDStreamID, SecurityID, SecurityIDSource, Price,
            OrderQty, Side, OrderTime);
    }
    virtual bool Write(char* mem_addr, size_t mem_size) override
    {
        mem_head_ = mem_tail_ = mem_addr;
        return write_into_memory((char*&)mem_tail_, mem_size, ChannelNo, ApplSeqNum,
            MDStreamID, SecurityID, SecurityIDSource, Price,
            OrderQty, Side, OrderTime);
    }
protected:
    const char* mem_head_;
    const char* mem_tail_;
};

// ���о���ҵ�����ί������ OrderSnapshot_300192
// 011 �ֻ�����Ʊ������ծȯ�ȣ����о��۽����������
// 021 ��Ѻʽ�ع������������
// 041 ��Ȩ���о��۽����������
template <is_mutable b> 
class OrderSnapshot_300192 : public OrderSnapshotBase<b>
{
    typedef OrderSnapshotBase<b> base_type;
public:
    static const uint32_t TypeID = 300192;
public:
    // Extend Fields ��ҵ����չ�ֶ�
    TypeString<2>   OrdType;               // �������1 = �мۣ�2 = �޼ۣ�U = ��������
public:
    virtual uint32_t Type() const override { return TypeID; }
    virtual uint32_t Size() const override
    {
        return base_type::Size() + OrdType.mem_size();
    }
    virtual bool Load(const char* mem_addr, size_t mem_size) override
    {
        if (base_type::Load(mem_addr, mem_size))
        {
            size_t base_type_size = base_type::mem_tail_ - base_type::mem_head_;
            mem_addr += base_type_size;
            mem_size -= base_type_size;
            return load_from_memory(mem_addr, mem_size, OrdType);
        }
        return false;
    }
    virtual bool Write(char* mem_addr, size_t mem_size) override
    {
        if (base_type::Write(mem_addr, mem_size))
        {
            size_t base_type_size = base_type::mem_tail_ - base_type::mem_head_;
            mem_addr += base_type_size;
            mem_size -= base_type_size;
            return write_into_memory(mem_addr, mem_size, OrdType);
        }
        return false;
    }
};

// Э�齻��ҵ�����ί������ OrderSnapshot_300592
// 051 Э�齻�������������
// 052 Э�齻����ʶ�������
template <is_mutable b> 
class OrderSnapshot_300592 : public OrderSnapshotBase<b>
{
    typedef OrderSnapshotBase<b> base_type;
public:
    static const uint32_t TypeID = 300592;
public:
    // Extend Fields ��ҵ����չ�ֶ�
    TypeString<8>   ConfirmID;          // ��������Լ���ţ�ConfirmID Ϊ�ձ�ʾ���������飬����Ϊ��������
    TypeString<12>  Contactor;          // ��ϵ��
    TypeString<30>  ContactInfo;        // ��ϵ��ʽ
public:
    virtual uint32_t Type() const override { return TypeID; }
    virtual uint32_t Size() const override
    {
        return base_type::Size() + byte_size_sum(ConfirmID, Contactor, ContactInfo);
    }
    virtual bool Load(const char* mem_addr, size_t mem_size) override
    {
        if (base_type::Load(mem_addr, mem_size))
        {
            size_t base_type_size = base_type::mem_tail_ - base_type::mem_head_;
            mem_addr += base_type_size;
            mem_size -= base_type_size;
            return load_from_memory(mem_addr, mem_size, 
                ConfirmID, Contactor, ContactInfo);
        }
        return false;
    }
    virtual bool Write(char* mem_addr, size_t mem_size) override
    {
        if (base_type::Write(mem_addr, mem_size))
        {
            size_t base_type_size = base_type::mem_tail_ - base_type::mem_head_;
            mem_addr += base_type_size;
            mem_size -= base_type_size;
            return write_into_memory(mem_addr, mem_size, ConfirmID, Contactor, ContactInfo);
        }
        return false;
    }
};

// ת��֤ͨȯ����ҵ�����ί������ OrderSnapshot_300792
// 071 ת��֤ͨȯ�����������
template <is_mutable b> 
class OrderSnapshot_300792 : public OrderSnapshotBase<b>
{
    typedef OrderSnapshotBase<b> base_type;
public:
    static const uint32_t TypeID = 300792;
public:
    // Extend Fields ��ҵ����չ�ֶ�
    TypeInt<uint16_t>   ExpirationDays;               // ���ޣ���λΪ����
    TypeInt<uint8_t>    ExpirationType;               // �������ͣ�1 = �̶�����
public:
    virtual uint32_t Type() const override { return TypeID; }
    virtual uint32_t Size() const override
    {
        return base_type::Size() + byte_size_sum(ExpirationDays, ExpirationType);
    }
    virtual bool Load(const char* mem_addr, size_t mem_size) override
    {
        if (base_type::Load(mem_addr, mem_size))
        {
            size_t base_type_size = base_type::mem_tail_ - base_type::mem_head_;
            mem_addr += base_type_size;
            mem_size -= base_type_size;
            return load_from_memory(mem_addr, mem_size, ExpirationDays, ExpirationType);
        }
        return false;
    }
    virtual bool Write(char* mem_addr, size_t mem_size) override
    {
        if (base_type::Write(mem_addr, mem_size))
        {
            size_t base_type_size = base_type::mem_tail_ - base_type::mem_head_;
            mem_addr += base_type_size;
            mem_size -= base_type_size;
            return write_into_memory(mem_addr, mem_size, ExpirationDays, ExpirationType);
        }
        return false;
    }
};

// ��ʳɽ�����Ĺ����ֶ�
template <is_mutable b> 
class TransactionSnapshotBase : public Field<b>
{
public:
    TypeInt<uint16_t>       ChannelNo;          // Ƶ������
    TypeSeqNum           ApplSeqNum;         // ��Ϣ��¼�ţ��� 1 ��ʼ����
    TypeString<3>           MDStreamID;         // �������
                                        // 011 �ֻ�����Ʊ������ծȯ�ȣ����о��۽����������
                                        // 021 ��Ѻʽ�ع������������
                                        // 041 ��Ȩ���о��۽����������
                                        // 051 Э�齻�������������
                                        // 052 Э�齻����ʶ�������
                                        // 071 ת��֤ͨȯ�����������
    TypeSeqNum           BidApplSeqNum;      // ��ί���������� 1 ��ʼ������ 0 ��ʾ�޶�Ӧί��
    TypeSeqNum           OfferApplSeqNum;    // ����ί���������� 1 ��ʼ������ 0 ��ʾ�޶�Ӧί��
    TypeSecurityID       SecurityID;         // ֤ȯ����
    TypeString<4>           SecurityIDSource;   // ֤ȯ����Դ
    TypePrice            LastPx;             // ί�м۸�
    TypeQty              LastQty;            // ί������
    TypeString<1>           ExecType;           // �ɽ����4 = ������F = �ɽ�
    TypeLocalTimeStamp   TransactTime;       // ί��ʱ��
    virtual uint32_t Size() const override
    {
        return byte_size_sum(ChannelNo, ApplSeqNum, MDStreamID, BidApplSeqNum,
            OfferApplSeqNum, SecurityID, SecurityIDSource, LastPx, LastQty,
            ExecType, TransactTime);
    }
    virtual bool Load(const char* mem_addr, size_t mem_size) override
    {
        return load_from_memory(mem_addr, mem_size, ChannelNo, ApplSeqNum,
            MDStreamID, BidApplSeqNum, OfferApplSeqNum, SecurityID,
            SecurityIDSource, LastPx, LastQty, ExecType, TransactTime);
    }
    virtual bool Write(char* mem_addr, size_t mem_size) override
    {
        return write_into_memory(mem_addr, mem_size, ChannelNo, ApplSeqNum,
            MDStreamID, BidApplSeqNum, OfferApplSeqNum, SecurityID,
            SecurityIDSource, LastPx, LastQty, ExecType, TransactTime);
    }
};

// ���о���ҵ����ʳɽ����� TransactionSnapshot_300191
// 011 �ֻ�����Ʊ������ծȯ�ȣ����о��۽����������
// 021 ��Ѻʽ�ع������������
// 041 ��Ȩ���о��۽����������
template <is_mutable b> 
class TransactionSnapshot_300191 : public TransactionSnapshotBase<b>
{
    typedef TransactionSnapshotBase<b> base_type;
public:
    static const uint32_t TypeID = 300191;
public:
    // Extend Fields ��ҵ����չ�ֶ�
    // ����չ�ֶ�
public:
    virtual uint32_t Type() const override { return TypeID; }
};

// Э�齻��ҵ����ʳɽ����� TransactionSnapshot_300591
// 051 Э�齻�������������
// 052 Э�齻����ʶ�������
template <is_mutable b> 
class TransactionSnapshot_300591 : public TransactionSnapshotBase<b>
{
    typedef TransactionSnapshotBase<b> base_type;
public:
    static const uint32_t TypeID = 300591;
public:
    // Extend Fields ��ҵ����չ�ֶ�
    // ����չ�ֶ�
public:
    virtual uint32_t Type() const override { return TypeID; }
};

// ת��֤ͨȯ����ҵ����ʳɽ����� TransactionSnapshot_300791
// 071 ת��֤ͨȯ�����������
template <is_mutable b> 
class TransactionSnapshot_300791 : public TransactionSnapshotBase<b>
{
    typedef TransactionSnapshotBase<b> base_type;
public:
    static const uint32_t TypeID = 300791;
public:
    // Extend Fields ��ҵ����չ�ֶ�
    // ����չ�ֶ�
public:
    virtual uint32_t Type() const override { return TypeID; }
};

} // namespace binary END
} // namespace szse END
} // namespace cn END

#endif // __CN_SZSE_BINARY_PACKET_H__